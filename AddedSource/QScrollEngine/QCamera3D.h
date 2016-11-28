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
        m_localOrientation = m_sceneOrientation.conjugate() * m_orientation;
        m_localPosition = m_sceneOrientation.conjugate().rotatedVector(m_position - m_scenePosition);
        m_localPosition.setX((qAbs(m_sceneScale.x()) > std::numeric_limits<float>::epsilon()) ? (m_localPosition.x() / m_sceneScale.x()) : 0.0f);
        m_localPosition.setY((qAbs(m_sceneScale.y()) > std::numeric_limits<float>::epsilon()) ? (m_localPosition.y() / m_sceneScale.y()) : 0.0f);
        m_localPosition.setZ((qAbs(m_sceneScale.z()) > std::numeric_limits<float>::epsilon()) ? (m_localPosition.z() / m_sceneScale.z()) : 0.0f);
    }
    void setPosition(const QVector3D& position) { m_position = position; }
    void setOrientation(const QQuaternion& orientation) { m_orientation = orientation; }
    QVector3D position() const { return m_position; }
    QQuaternion orientation() const { return m_orientation; }
    QVector3D scenePosition() const { return m_scenePosition; }
    QQuaternion sceneOrientation() const { return m_sceneOrientation; }
    void setScenePosition(const QVector3D& scenePosition) { m_scenePosition = scenePosition; }
    void setSceneOrientation(const QQuaternion& sceneOrientation) { m_sceneOrientation = sceneOrientation; }
    void setSceneScale(const QVector3D& scale) { m_sceneScale = scale; }
    void setScene(QScene* scene);
    QVector3D localPosition() const { return m_localPosition; }
    QQuaternion localOrientation() const { return m_localOrientation; }

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

    QMatrix4x4 matrixView() const { return m_matrixView; }
    QMatrix4x4 matrixProj() const { return m_matrixProj; }
    QMatrix4x4 matrixViewProj() const { return m_matrixViewProj; }

    QVector3D getScenePointToScreen(const QVector3D& point) const;

    bool pointFromScreenToScene(QVector3D& result, const QVector2D& screenPoint) const;
    QVector3D fromCameraToWorld(const QVector2D& screenPoint, float Z) const;
    void getLocalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;
    void getRayOnSceneFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;
    void getGlobalRayFromScreen(QVector3D& resultNear, QVector3D& resultFar, const QVector2D& screenPoint) const;


private:
    QMatrix4x4 m_matrixView;
    QMatrix4x4 m_matrixProj;
    QMatrix4x4 m_matrixViewProj;
    QVector3D m_position;
    QQuaternion m_orientation;
    QVector3D m_scenePosition;
    QQuaternion m_sceneOrientation;
    QVector3D m_sceneScale;
    QVector3D m_localPosition;
    QQuaternion m_localOrientation;
    float _zNear;
    float _zFar;
    float _fov;
    float _aspect;

};

}
#endif
