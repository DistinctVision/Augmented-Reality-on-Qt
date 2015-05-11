#ifndef QSH_SPHERE_LIGHT_H
#define QSH_SPHERE_LIGHT_H

#include "QScrollEngine/Shaders/QSh_Light.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QMatrix3x3>
#include <QQuaternion>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh_Sphere_Light:
        public QSh_Light
{
public:
    static int locationMatrixWVP[3];
    static int locationMatrixW[3];
    static int locationColor[3];
    static int locationTexture0[3];
    static int locationViewPosition[3];
    static int locationSphereCenter[3];
    static int locationSphereRadiusSquared[3];
    static int locationSphereMatrixRotation[3];
    static int locationAmbientColor[3];
    static int locationLight0_color[3];
    static int locationLight0_position[3];
    static int locationLight0_radius[3];
    static int locationLight0_soft[3];
    static int locationLight1_color[3];
    static int locationLight1_position[3];
    static int locationLight1_radius[3];
    static int locationLight1_soft[3];
    static int locationLight_dir[3];
    static int locationLight_spotCutOff[3];
    static int locationLight_diffuseIntensity[3];
    static int locationLight_specularIntensity[3];
    static int locationLight_specularPower[3];

public:
    QSh_Sphere_Light()
    {
        _currentIndexType = Sphere_Light;
        _subIndexType = 0;
        _texture0 = nullptr;
        _color.setRgb(255, 255, 255, 255);
        _sphereRadius = 0.5f;
        _diffuseIntensity = 1.0f;
        _specularIntensity = 1.0f;
        _specularPower = 1.0f;
    }
    QSh_Sphere_Light(float sphereRadius, const QQuaternion& sphereOrientation = QQuaternion(),
               QOpenGLTexture* texture0 = nullptr, const QColor& color = QColor(255, 255, 255, 255),
               float diffuseIntensity = 1.0, float specularIntensity = 1.0f, float specularPower = 1.0f)
    {
        _currentIndexType = Sphere_Light;
        _subIndexType = 0;
        _texture0 = texture0;
        _color = color;
        _sphereRadius = sphereRadius;
        setSphereOrientation(sphereOrientation);
        _diffuseIntensity = diffuseIntensity;
        _specularIntensity = specularIntensity;
        _specularPower = specularPower;
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
    float diffuseIntensity() const { return _diffuseIntensity; }
    void setDiffuseIntensity(float intensity) { _diffuseIntensity = intensity; }

    int indexType() const override { return Sphere_Light; }
    void preprocess() override;
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& program) override;
    QSh_Sphere_Light(const QSh_Sphere_Light* s)
    {
        _currentIndexType = Sphere_Light;
        _subIndexType = 0;
        _texture0 = s->texture0();
        _color = s->color();
        _sphereRadius = s->sphereRadius();
        _sphereOrientation = s->sphereOrientation();
        _sphereMatrixRotation = s->sphereMatrixRotation();
        _diffuseIntensity = s->diffuseIntensity();
        _specularIntensity = s->specularIntensity();
        _specularPower = s->specularPower();
    }
    QSh* copy() const override
    {
        return new QSh_Sphere_Light(this);
    }

protected:
    QQuaternion _sphereOrientation;
    QMatrix3x3 _sphereMatrixRotation;
    float _sphereRadius;
    float _diffuseIntensity;
};

}
#endif
