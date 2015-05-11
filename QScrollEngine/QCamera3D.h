#ifndef QCAMERA3D_H
#define QCAMERA3D_H

#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QSize>
#include <climits>

#include "QScrollEngine/QFrustum.h"

namespace QScrollEngine {

class QScene;

class QCamera3D
{
public:
    QFrustum frustum;

    QCamera3D();
    void calculateMatrix();
    void calculateFrustum();
    void update()
    {
        updateLocalPosition();
        calculateMatrix();
        calculateFrustum();
    }
    void updateLocalPosition()
    {
        _localOrientation = _sceneOrientation.conjugate() * _orientation;
        _localPosition = _sceneOrientation.conjugate().rotatedVector(_position - _scenePosition);
        _localPosition.setX((qAbs(_sceneScale.x()) > std::numeric_limits<float>::epsilon()) ? (_localPosition.x() / _sceneScale.x()) : 0.0f);
        _localPosition.setY((qAbs(_sceneScale.y()) > std::numeric_limits<float>::epsilon()) ? (_localPosition.y() / _sceneScale.y()) : 0.0f);
        _localPosition.setZ((qAbs(_sceneScale.z()) > std::numeric_limits<float>::epsilon()) ? (_localPosition.z() / _sceneScale.z()) : 0.0f);
    }
    void setPosition(const QVector3D& position) { _position = position; }
    void setOrientation(const QQuaternion& orientation) { _orientation = orientation; }
    QVector3D position() const { return _position; }
    QQuaternion orientation() const { return _orientation; }
    QVector3D scenePosition() const { return _scenePosition; }
    QQuaternion sceneOrientation() const { return _sceneOrientation; }
    void setScenePosition(const QVector3D& scenePosition) { _scenePosition = scenePosition; }
    void setSceneOrientation(const QQuaternion& sceneOrientation) { _sceneOrientation = sceneOrientation; }
    void setSceneScale(const QVector3D& scale) { _sceneScale = scale; }
    void setScene(QScene* scene);
    QVector3D localPosition() const { return _localPosition; }
    QQuaternion localOrientation() const { return _localOrientation; }

    void setProjectionParameters(float zNear, float zFar, float fieldOfView, int width, int height);
    void setProjectionParameters(float zNear, float zFar, float fieldOfView, const QSize& resolution)
    {
        setProjectionParameters(zNear, zFar, fieldOfView, resolution.width(), resolution.height());
    }
    void setResolution(int width, int height)
    {
        if (height > 0)
            _aspect = width / static_cast<float>(height);
    }
    void setResolution(const QSize& resolution) { setResolution(resolution.width(), resolution.height()); }
    void swapAspectXY();

    void calculateProjectionMatrix();
    float zNear() const { return _zNear; }
    float zFar() const { return _zFar; }
    float fieldOfView() const { return _fov; }
    float aspect() const { return _aspect; }

    QMatrix4x4 matrixView() const { return _matrixView; }
    QMatrix4x4 matrixProj() const { return _matrixProj; }
    QMatrix4x4 matrixViewProj() const { return _matrixViewProj; }

    QVector3D getScenePointToScreen(const QVector3D& point) const;

    bool pointFromScreenToScene(QVector3D& result, const QVector2D& screenPoint) const;
    QVector3D fromCameraToWorld(const QVector2D& screenPoint, float Z) const;
    void getLocalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;
    void getRayOnSceneFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;
    void getGlobalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;


private:
    QMatrix4x4 _matrixView;
    QMatrix4x4 _matrixProj;
    QMatrix4x4 _matrixViewProj;
    QVector3D _position;
    QQuaternion _orientation;
    QVector3D _scenePosition;
    QQuaternion _sceneOrientation;
    QVector3D _sceneScale;
    QVector3D _localPosition;
    QQuaternion _localOrientation;
    float _zNear;
    float _zFar;
    float _fov;
    float _aspect;

};

}
#endif
