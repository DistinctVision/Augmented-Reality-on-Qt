#ifndef AR_KALMANFILTER_H
#define AR_KALMANFILTER_H

#include "TMath/TMath.h"

#include <QDebug>

namespace AR {

template <typename Type>
class KalmanFilter
{
public:
    KalmanFilter()
    {
    }

    KalmanFilter(const TMath::TMatrix<Type>& measurementNoise,
                 const TMath::TMatrix<Type>& environmentNoise,
                 const TMath::TMatrix<Type>& stateTransitionMatrix,
                 const TMath::TMatrix<Type>& factorOfMeasuredToState)
    {
        m_Q = measurementNoise;
        m_R = environmentNoise;
        m_F = stateTransitionMatrix;
        m_H = factorOfMeasuredToState;
    }

    TMath::TMatrix<Type> environmentNoise() const { return m_Q; }
    void setEnvironmentNoise(const TMath::TMatrix<Type>& environmentNoise) { m_Q = environmentNoise; }

    TMath::TMatrix<Type> measurementNoise() const { return m_R; }
    void setMeasurementNoise(const TMath::TMatrix<Type>& measurementNoise) { m_R = measurementNoise; }

    TMath::TMatrix<Type> stateTransitionMatrix() const { return m_F; }
    void setStateTransitionMatrix(const TMath::TMatrix<Type>& stateTransitionMatrix) { m_F = stateTransitionMatrix; }

    TMath::TMatrix<Type> factorOfMeasuredToState() const { return m_H; }
    void setFactorOfMeasuredToState(const TMath::TMatrix<Type>& factorOfMeasuredToState) { m_H = factorOfMeasuredToState; }

    TMath::TVector<Type> state() const { return m_state; }
    TMath::TMatrix<Type> covariance() const { return m_covariance; }
    void setState(const TMath::TVector<Type>& state, const TMath::TMatrix<Type>& covariance)
    {
        m_state = state;
        m_covariance = covariance;
    }

    TMath::TVector<Type> currentX() const
    {
        return m_H * m_state;
    }

    TMath::TVector<Type> correct(const TMath::TVector<Type>& x)
    {
        using namespace TMath;

        qDebug() << "x =" << x(0) << x(1) << x(2);

        qDebug() << "prev";
        qDebug() << "p =" << m_state(0) << m_state(1) << m_state(2);
        qDebug() << "v =" << m_state(3) << m_state(4) << m_state(5);

        //prediction
        TVector<Type> X0 = m_F * m_state;
        TMatrix<Type> P0 = m_F * m_covariance * m_F.refTransposed() + m_Q;

        qDebug() << "prediction";
        qDebug() << "p =" << X0(0) << X0(1) << X0(2);
        qDebug() << "v =" << X0(3) << X0(4) << X0(5);

        //correction
        TMatrix<Type> S = m_H * P0 * m_H.refTransposed() + m_R;
        if (!S.invert())
            return TVector<Type>();
        TMatrix<Type> K = P0 * m_H.refTransposed() * S;
        m_state = X0 + K * (x - m_H * X0);
        m_covariance = (TMatrix<Type>::Identity(m_H.cols()) - K * m_H) * P0;

        qDebug() << "current";
        qDebug() << "p =" << m_state(0) << m_state(1) << m_state(2);
        qDebug() << "v =" << m_state(3) << m_state(4) << m_state(5) << "\n";

        return m_H * m_state;
    }

private:
    TMath::TMatrix<Type> m_Q;
    TMath::TMatrix<Type> m_R;
    TMath::TMatrix<Type> m_F;
    TMath::TMatrix<Type> m_H;

    TMath::TVector<Type> m_state;
    TMath::TMatrix<Type> m_covariance;
};

}

#endif // AR_KALMANFILTER_H
