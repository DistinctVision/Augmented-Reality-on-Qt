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
        TMatrixd H_position(3, 6);
        H_position.setToIdentity();
        m_positionFilter.setFactorOfMeasuredToState(H_position);
        TMatrixd H_orientation(4, 8);
        H_orientation.setToIdentity();
        m_orientationFilter.setFactorOfMeasuredToState(H_orientation);
        updatePositionStateTranslationMatrix();
        setMeasurementPositionNoise(0.01);
        setMeasurementOrientationNoise(1.0 * (M_PI / 180.0));
        setPredictedPositionNoise(TVectord::create(0.0001, 0.0001));
        setPredictedOrientationNoise(TVectord::create(0.001, 0.001) * (M_PI / 180.0));
        setState(TMatrixd::Identity(3), TVectord::create(0.0, 0.0, 0.0));
    }

    void setState(const TMath::TMatrixd& rotation, const TMath::TVectord& translation)
    {
        using namespace TMath;
        const double L = 0.1;
        TVectord worldPosition(3);
        TMatrixd invRotation = rotation;
        if (TTools::matrix3x3Invert(invRotation)) {
            worldPosition = - invRotation * translation;
        } else {
            invRotation = TMatrixd::Identity(3);
            worldPosition.setZero();
        }
        TVectord quaternion = TTools::rotationMatrixToQuaternion(invRotation);
        TVectord v(6);
        v.fill(0.0);
        v.fill(0, 3, worldPosition);
        m_positionFilter.setState(v, TMatrixd::Identity(6) * L);
        v = TVectord(8);
        v.fill(0, 4, quaternion);
        v.fill(4, 4, TVectord::create(0.0, 0.0, 0.0, 1.0));
        m_orientationFilter.setState(v, TMatrixd::Identity(8) * L);
    }

    void getCurrentX(TMath::TMatrixd& rotation, TMath::TVectord& translation) const
    {
        rotation = TMath::TTools::quaterionToRotationMatrix(m_orientationFilter.currentX());
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
        TVectord quaternion = TTools::rotationMatrixToQuaternion(invRotation);
        updateOrientationStateTranslationMatrix();
        m_positionFilter.correct(worldPosition);
        m_orientationFilter.correct(quaternion);
        TVectord state = m_orientationFilter.state();
        state.fill(0, 4, state.slice(0, 4).normalized());
        state.fill(4, 4, state.slice(4, 4).normalized());
        m_orientationFilter.setState(state, m_orientationFilter.covariance());
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
        TMath::TMatrixd enviromentNoise(8, 8);
        enviromentNoise.setZero();
        enviromentNoise(0, 0) = noise(0);
        enviromentNoise(1, 1) = noise(0);
        enviromentNoise(2, 2) = noise(0);
        enviromentNoise(3, 3) = noise(0);
        enviromentNoise(4, 4) = noise(1);
        enviromentNoise(5, 5) = noise(1);
        enviromentNoise(6, 6) = noise(1);
        enviromentNoise(7, 7) = noise(1);
        m_orientationFilter.setEnvironmentNoise(enviromentNoise);
    }
    double measurementOrientationNoise() const { return m_orientationFilter.measurementNoise()(0, 0); }
    void setMeasurementOrientationNoise(double noise)
    {
        TMath::TMatrixd m(4, 4);
        m.setZero();
        m(0, 0) = noise;
        m(1, 1) = noise;
        m(2, 2) = noise;
        m(3, 3) = noise;
        m_orientationFilter.setMeasurementNoise(m);
    }

    void updatePositionStateTranslationMatrix()
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
    }

    void updateOrientationStateTranslationMatrix()
    {
        using namespace TMath;
        TVectord q = m_orientationFilter.state().slice(0, 4);
        TVectord qV = m_orientationFilter.state().slice(4, 4);

        TMatrixd F(8, 8);

        F(0, 0) = qV(3);
        F(0, 1) = - qV(2);
        F(0, 2) = qV(1);
        F(0, 3) = qV(0);
        F(0, 4) = q(3);
        F(0, 5) = q(2);
        F(0, 6) = - q(1);
        F(0, 7) = q(0);

        F(1, 0) = qV(2);
        F(1, 1) = qV(3);
        F(1, 2) = - qV(0);
        F(1, 3) = qV(1);
        F(1, 4) = - q(2);
        F(1, 5) = q(3);
        F(1, 6) = q(0);
        F(1, 7) = q(1);

        F(2, 0) = - qV(1);
        F(2, 1) = qV(0);
        F(2, 2) = qV(3);
        F(2, 3) = qV(2);
        F(2, 4) = q(1);
        F(2, 5) = - q(0);
        F(2, 6) = q(3);
        F(2, 7) = q(2);

        F(3, 0) = - qV(0);
        F(3, 1) = - qV(1);
        F(3, 2) = - qV(2);
        F(3, 3) = qV(3);
        F(3, 4) = - q(0);
        F(3, 5) = - q(1);
        F(3, 6) = - q(2);
        F(3, 7) = q(3);

        int i, j;
        for (i = 4; i < 8; ++i) {
            for (j = 0; j < i; ++j)
                F(i, j) = 0.0;
            F(i, i) = 1.0;
            for (j = i + 1; j < 8; ++j)
                F(i, j) = 0.0;
        }

        m_orientationFilter.setStateTransitionMatrix(F);
    }

private:
    KalmanFilter<double> m_positionFilter;
    KalmanFilter<double> m_orientationFilter;
};

}

#endif // AR_POSEFILTER_H
