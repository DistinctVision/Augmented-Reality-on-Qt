#ifndef QSH_SPHERE_TEXTURE1_H
#define QSH_SPHERE_TEXTURE1_H

#include "QScrollEngine/Shaders/QSh_Texture1.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QMatrix3x3>
#include <QQuaternion>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh_Sphere_Texture1:
        public QSh_Texture1
{
public:
    static int locationMatrixWVP;
    static int locationMatrixW;
    static int locationColor;
    static int locationTexture0;
    static int locationViewPosition;
    static int locationSphereCenter;
    static int locationSphereRadiusSquared;
    static int locationSphereMatrixRotation;

public:
    QSh_Sphere_Texture1()
    {
        _currentIndexType = Sphere_Texture1;
        _subIndexType = 0;
        _texture0 = nullptr;
        _color.setRgb(255, 255, 255, 255);
        _sphereRadius = 0.5f;
    }
    QSh_Sphere_Texture1(float sphereRadius, const QQuaternion& sphereOrientation = QQuaternion(),
               QOpenGLTexture* texture0 = nullptr, const QColor& color = QColor(255, 255, 255, 255))
    {
        _currentIndexType = Sphere_Texture1;
        _subIndexType = 0;
        _texture0 = texture0;
        _color = color;
        _sphereRadius = sphereRadius;
        setSphereOrientation(sphereOrientation);
    }

    void setSphereOrientation(const QQuaternion& orientation)
    {
        _sphereOrientation = orientation;
        QVector3D axisX(1.0f, 0.0f, 0.0f);
        axisX = _sphereOrientation.rotatedVector(axisX);
        _sphereMatrixRotation(0, 0) = axisX.x();
        _sphereMatrixRotation(0, 1) = axisX.y();
        _sphereMatrixRotation(0, 2) = axisX.z();
        QVector3D axisY(0.0f, 1.0f, 0.0f);
        axisY = _sphereOrientation.rotatedVector(axisY);
        _sphereMatrixRotation(1, 0) = axisY.x();
        _sphereMatrixRotation(1, 1) = axisY.y();
        _sphereMatrixRotation(1, 2) = axisY.z();
        QVector3D axisZ = QVector3D::crossProduct(axisX, axisY);
        _sphereMatrixRotation(2, 0) = axisZ.x();
        _sphereMatrixRotation(2, 1) = axisZ.y();
        _sphereMatrixRotation(2, 2) = axisZ.z();

    }
    QQuaternion sphereOrientation() const { return _sphereOrientation; }
    QMatrix3x3 sphereMatrixRotation() const { return _sphereMatrixRotation; }
    float sphereRadius() const { return _sphereRadius; }
    void setSphereRadius(float radius) { _sphereRadius = radius; }

    int indexType() const override { return Sphere_Texture1; }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& program) override;
    QSh_Sphere_Texture1(const QSh_Sphere_Texture1* s)
    {
        _currentIndexType = Sphere_Texture1;
        _subIndexType = 0;
        _texture0 = s->texture0();
        _color = s->color();
        _sphereRadius = s->sphereRadius();
        _sphereOrientation = s->sphereOrientation();
        _sphereMatrixRotation = s->sphereMatrixRotation();
    }
    QSh* copy() const override
    {
        return new QSh_Sphere_Texture1(this);
    }

protected:
    QQuaternion _sphereOrientation;
    QMatrix3x3 _sphereMatrixRotation;
    float _sphereRadius;
};

}
#endif
