#ifndef QSH_LIGHT_H
#define QSH_LIGHT_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Texture1.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>

#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QLight.h"

namespace QScrollEngine {

class QSh_Light: public QSh_Texture1
{
public:
    static int locationMatrixWVP[6];
    static int locationMatrixW[6];
    static int locationColor[6];
    static int locationTexture0[6];
    static int locationAmbientColor[6];
    static int locationLight0_color[6];
    static int locationLight0_position[6];
    static int locationLight0_radius[6];
    static int locationLight0_soft[6];
    static int locationLight1_color[6];
    static int locationLight1_position[6];
    static int locationLight1_radius[6];
    static int locationLight1_soft[6];
    static int locationLight_dir[6];
    static int locationLight_spotCutOff[6];
    static int locationLight_specularIntensity[3];
    static int locationLight_specularPower[3];
    static int locationLight_viewPosition[3];

public:
    QSh_Light()
    {
        _currentIndexType = Texture1;
        _subIndexType = 0;
        _texture0 = nullptr;
        _color.setRgb(255, 255, 255, 255);
        _specularIntensity = 0.0f;
        _specularPower = 1.0f;
        _light[0] = _light[1] = nullptr;
        _drawObject = nullptr;
        _sceneObject = nullptr;
    }
    QSh_Light(QOpenGLTexture* texture0, const QColor& color = QColor(255, 255, 255, 255),
              float specularIntensity = 0.0f, float specularPower = 1.0f)
    {
        _currentIndexType = Texture1;
        _subIndexType = 0;
        _texture0 = texture0;
        _color = color;
        _specularIntensity = specularIntensity;
        _specularPower = specularPower;
        _light[0] = _light[1] = nullptr;
        _drawObject = nullptr;
        _sceneObject = nullptr;
    }

    float specularIntensity() const { return _specularIntensity; }
    void setSpecularIntensity(float intensity) { _specularIntensity = intensity; }
    float specularPower() const { return _specularPower; }
    void setSpecularPower(float power) { _specularPower = power; }

    int indexType() const override { return Light; }
    void preprocess() override;
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;

    QLight* light0() const { return _light[0]; }
    QLight* light1() const { return _light[1]; }

    QSh_Light(const QSh_Light* s)
    {
        _currentIndexType = Light;
        _subIndexType = 0;
        _texture0 = s->texture0();
        _color = s->color();
        _specularIntensity = s->specularIntensity();
        _specularPower = s->specularPower();
    }
    QSh* copy() const override
    {
        return new QSh_Light(this);
    }

protected:
    QLight* _light[2];
    float _specularIntensity;
    float _specularPower;

    void _findLights(QScene* scene);
};

}
#endif
