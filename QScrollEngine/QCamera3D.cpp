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
    _matrixView.setToIdentity();
    _matrixView.translate(_localPosition);
    _matrixView.rotate(_localOrientation);
    _matrixView = _matrixView.inverted();
    _matrixViewProj = _matrixProj * _matrixView;
}

void QCamera3D::calculateFrustum()
{
    frustum.calculate(_matrixViewProj);
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
    std::swap(_matrixProj(0, 0), _matrixProj(1, 1));
}

void QCamera3D::calculateProjectionMatrix()
{
    _matrixProj.setToIdentity();
    _matrixProj.perspective(_fov, _aspect, _zNear, _zFar);
}

void QCamera3D::setScene(QScene* scene)
{
    if (scene) {
        _scenePosition = scene->position;
        _sceneOrientation = scene->orientation;
        _sceneScale = scene->scale;
    } else {
        _scenePosition.setX(0.0f);
        _scenePosition.setY(0.0f);
        _scenePosition.setZ(0.0f);
        _sceneOrientation.setScalar(1.0f);
        _sceneOrientation.setVector(0.0f, 0.0f, 0.0f);
    }
}

QVector3D QCamera3D::getScenePointToScreen(const QVector3D& point) const
{
    QVector4D r = _matrixViewProj * QVector4D(point, 1.0f);
    return QVector3D(r.x() / r.w(), r.y() / r.w(), r.z() / r.w());
}

void QCamera3D::getLocalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    QMatrix4x4 m = _matrixProj.inverted();
    QVector2D d(2.0f * screenPoint.x() - 1.0f, 1.0f - 2.0f * screenPoint.y());
    resultNear = m * QVector3D(d, -1.0f);
    resultFar = m * QVector3D(d, 1.0f);
}

void QCamera3D::getGlobalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    getLocalRayFromScreen(resultNear, resultFar, screenPoint);
    resultNear = _orientation.rotatedVector(resultNear) + _position;
    resultFar = _orientation.rotatedVector(resultFar) + _position;
}

void QCamera3D::getRayOnSceneFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const
{
    getLocalRayFromScreen(resultNear, resultFar, screenPoint);
    resultNear = _localOrientation.rotatedVector(resultNear) + _localPosition;
    resultFar = _localOrientation.rotatedVector(resultFar) + _localPosition;
}

bool QCamera3D::pointFromScreenToScene(QVector3D& result, const QVector2D& screenPoint) const
{
    float Z;
    QVector3D nearP, farP;
    getLocalRayFromScreen(nearP, farP, screenPoint);
    if (QOtherMathFunctions::collisionPlaneLine(result, QVector3D(0.0f, 0.0f, 1.0f), 0.0f, _localPosition,
                                                _localOrientation.rotatedVector(farP - nearP), Z)) {
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
    return _orientation.rotatedVector(resultNear + (resultFar - resultNear) * t) + _position;
}

}
