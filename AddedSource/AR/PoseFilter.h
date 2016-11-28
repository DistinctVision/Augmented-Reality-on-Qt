#ifndef AR_POSEFILTER_H
#define AR_POSEFILTER_H

#include "KalmanFilter.h"
#include "TMath/TTools.h"

namespace AR {

class PoseFilter
{
public:
    PoseFilter()
    {
        using namespace TMath;
        TMatrixd H(3, 6);
        H.setToIdentity();
        m_positionFilter.setFactorOfMeasuredToState(H);
        m_orientationFilter.setFactorOfMeasuredToState(H);
        updateStateTranslationMatrix();
        setMeasurementPositionNoise(0.01);
        setMeasurementOrientationNoise(1.0 * (M_PI / 180.0));
        setPredictedPositionNoise(TVectord::create(0.0001, 0.0001));
        setPredictedOrientationNoise(TVectord::create(1.0, 1.0) * (M_PI / 180.0));
        setState(TMatrixd::Identity(3), TVectord::create(0.0, 0.0, 0.0));
    }

    void setState(const TMath::TMatrixd& rotation, const TMath::TVectord& translation)
    {
        using namespace TMath;
        const double L = 10.0;
        TVectord worldPosition(3);
        TMatrixd invRotation = rotation;
        if (TTools::matrix3x3Invert(invRotation)) {
            worldPosition = - invRotation * translation;
        } else {
            invRotation = TMatrixd::Identity(3);
            worldPosition.setZero();
        }
        TVectord angles = TTools::rotationMatrixToEulerAngles_3d(invRotation);
        TVectord v(6);
        v.fill(0.0);
        v.fill(0, 3, worldPosition);
        m_positionFilter.setState(v, TMatrixd::Identity(6) * L);
        v.fill(0, 3, angles);
        m_orientationFilter.setState(v, TMatrixd::Identity(6) * (L * (M_PI / 180.0)));
    }

    void getCurrentX(TMath::TMatrixd& rotation, TMath::TVectord& translation) const
    {
        rotation = TMath::TTools::eulerAnglesToRotationMatrix_3d(m_orientationFilter.currentX());
        if (TMath::TTools::matrix3x3Invert(rotation)) {
            translation = - rotation * m_positionFilter.currentX();
        } else {
            rotation.setToIdentity();
            translation.setZero();
        }
    }

    void correct(const TMath::TMatrixd& rotation, const TMath::TVectord& translation)
    {
        using namespace TMath;
        TVectord worldPosition(3);
        TMatrixd invRotation = rotation;
        if (TTools::matrix3x3Invert(invRotation)) {
            worldPosition = - invRotation * translation;
        } else {
            invRotation = TMatrixd::Identity(3);
            worldPosition.setZero();
        }
        TVectord angles = TTools::rotationMatrixToEulerAngles_3d(invRotation);
        m_positionFilter.correct(worldPosition);
        m_orientationFilter.correct(angles);
    }

    TMath::TVectord predictedPositionNoise() const
    {
        TMath::TMatrixd noise = m_positionFilter.environmentNoise();
        return TMath::TVectord::create(noise(0, 0), noise(3, 3));
    }
    void setPredictedPositionNoise(const TMath::TVectord& noise)
    {
        TMath::TMatrixd enviromentNoise(6, 6);
        enviromentNoise.setZero();
        enviromentNoise(0, 0) = noise(0);
        enviromentNoise(1, 1) = noise(0);
        enviromentNoise(2, 2) = noise(0);
        enviromentNoise(3, 3) = noise(1);
        enviromentNoise(4, 4) = noise(1);
        enviromentNoise(5, 5) = noise(1);
        m_positionFilter.setEnvironmentNoise(enviromentNoise);
    }
    double measurementPositionNoise() const { return m_positionFilter.measurementNoise()(0, 0); }
    void setMeasurementPositionNoise(double noise)
    {
        TMath::TMatrixd m(3, 3);
        m.setZero();
        m(0, 0) = noise;
        m(1, 1) = noise;
        m(2, 2) = noise;
        m_positionFilter.setMeasurementNoise(m);
    }

    TMath::TVectord predictedOrientationNoise() const
    {
        TMath::TMatrixd noise = m_orientationFilter.environmentNoise();
        return TMath::TVectord::create(noise(0, 0), noise(3, 3));
    }
    void setPredictedOrientationNoise(const TMath::TVectord& noise)
    {
        TMath::TMatrixd enviromentNoise(6, 6);
        enviromentNoise.setZero();
        enviromentNoise(0, 0) = noise(0);
        enviromentNoise(1, 1) = noise(0);
        enviromentNoise(2, 2) = noise(0);
        enviromentNoise(3, 3) = noise(1);
        enviromentNoise(4, 4) = noise(1);
        enviromentNoise(5, 5) = noise(1);
        m_orientationFilter.setEnvironmentNoise(enviromentNoise);
    }
    double measurementOrientationNoise() const { return m_orientationFilter.measurementNoise()(0, 0); }
    void setMeasurementOrientationNoise(double noise)
    {
        TMath::TMatrixd m(3, 3);
        m.setZero();
        m(0, 0) = noise;
        m(1, 1) = noise;
        m(2, 2) = noise;
        m_orientationFilter.setMeasurementNoise(m);
    }

    void updateStateTranslationMatrix()
    {
        TMath::TMatrixd F(6, 6);
        F(0, 0) = 1.0;
        F(0, 1) = 0.0;
        F(0, 2) = 0.0;
        F(0, 3) = 1.0;
        F(0, 4) = 0.0;
        F(0, 5) = 0.0;
        int i, j;
        for (i = 1; i < 6; ++i) {
            for (j = 0; j < i; ++j)
                F(i, j) = 0.0;
            for (j = i; j < 6; ++j)
                F(i, j) = F(i - 1, j - 1);
        }
        m_positionFilter.setStateTransitionMatrix(F);
        m_orientationFilter.setStateTransitionMatrix(F);
    }

private:
    KalmanFilter<double> m_positionFilter;
    KalmanFilter<double> m_orientationFilter;
};

}

#endif // AR_POSEFILTER_H
