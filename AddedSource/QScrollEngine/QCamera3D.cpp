#include "QScrollEngine/QCamera3D.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QScene.h"
#include <utility>

namespace QScrollEngine {

QCamera3D::QCamera3D()
{
    setProjectionParameters(1.0f, 100.0f, 30.0f, 640, 480);
    calculateProjectionMatrix();
    update();
}

void QCamera3D::calculateMatrix()
{
    m_matrixView.setToIdentity();
    m_matrixView.translate(m_localPosition);
    m_matrixView.rotate(m_localOrientation);
    m_matrixView = m_matrixView.inverted();
    m_matrixViewProj = m_matrixProj * m_matrixView;
}

void QCamera3D::calculateFrustum()
{
    frustum.calculate(m_matrixViewProj);
}

void QCamera3D::setProjectionParameters(float zNear, float zFar, float fieldOfView, int width, int height)
{
    if ((zNear >= 0.0f) && (zFar > zNear) && (fieldOfView > 0.0f) && (height > 0)) {
        _zNear = zNear;
        _zFar = zFar;
        _fov = fieldOfView;
        _aspect = width / static_cast<float>(height);
    }
}

void QCamera3D::swapAspectXY()
{
    std::swap(m_matrixProj(0, 0), m_matrixProj(1, 1));
}

void QCamera3D::calculateProjectionMatrix()
{
    m_matrixProj.setToIdentity();
    m_matrixProj.perspective(_fov, _aspect, _zNear, _zFar);
}

void QCamera3D::setScene(QScene* scene)
{
    if (scene) {
        m_scenePosition = scene->position;
        m_sceneOrientation = scene->orientation;
        m_sceneScale = scene->scale;
    } else {
        m_scenePosition.setX(0.0f);
        m_scenePosition.setY(0.0f);
        m_scenePosition.setZ(0.0f);
        m_sceneOrientation.setScalar(1.0f);
        m_sceneOrientation.setVector(0.0f, 0.0f, 0.0f);
    }
}

QVector3D QCamera3D::getScenePointToScreen(const QVector3D& point) const
{
    QVector4D r = m_matrixViewProj * QVector4D(point, 1.0f);
    return QVector3D(r.x() / r.w(), r.y() / r.w(), r.z() / r.w());
}

void QCamera3D::getLocalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    QMatrix4x4 m = m_matrixProj.inverted();
    QVector2D d(2.0f * screenPoint.x() - 1.0f, 1.0f - 2.0f * screenPoint.y());
    resultNear = m * QVector3D(d, -1.0f);
    resultFar = m * QVector3D(d, 1.0f);
}

void QCamera3D::getGlobalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    getLocalRayFromScreen(resultNear, resultFar, screenPoint);
    resultNear = m_orientation.rotatedVector(resultNear) + m_position;
    resultFar = m_orientation.rotatedVector(resultFar) + m_position;
}

void QCamera3D::getRayOnSceneFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    getLocalRayFromScreen(resultNear, resultFar, screenPoint);
    resultNear = m_localOrientation.rotatedVector(resultNear) + m_localPosition;
    resultFar = m_localOrientation.rotatedVector(resultFar) + m_localPosition;
}

bool QCamera3D::pointFromScreenToScene(QVector3D& result, const QVector2D& screenPoint) const
{
    float Z;
    QVector3D nearP, farP;
    getLocalRayFromScreen(nearP, farP, screenPoint);
    if (QOtherMathFunctions::collisionPlaneRay(result, Z, QVector3D(0.0f, 0.0f, 1.0f), 0.0f, m_localPosition,
                                                m_localOrientation.rotatedVector(farP - nearP))) {
        if (Z > 0.0f) {
            return true;
        }
    }
    return false;
}

QVector3D QCamera3D::fromCameraToWorld(const QVector2D& screenPoint, float Z) const
{
    QVector3D resultNear, resultFar;
    getLocalRayFromScreen(resultNear, resultFar, screenPoint);
    float t = (Z - _zNear) / (_zFar - _zNear);
    return m_orientation.rotatedVector(resultNear + (resultFar - resultNear) * t) + m_position;
}

}
