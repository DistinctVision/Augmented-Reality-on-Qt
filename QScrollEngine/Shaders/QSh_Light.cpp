#include "QScrollEngine/Shaders/QSh_Light.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"

namespace QScrollEngine {

int QSh_Light::locationMatrixWVP[6];
int QSh_Light::locationMatrixW[6];
int QSh_Light::locationColor[6];
int QSh_Light::locationTexture0[6];
int QSh_Light::locationAmbientColor[6];
int QSh_Light::locationLight0_color[6];
int QSh_Light::locationLight0_position[6];
int QSh_Light::locationLight0_radius[6];
int QSh_Light::locationLight0_soft[6];
int QSh_Light::locationLight1_color[6];
int QSh_Light::locationLight1_position[6];
int QSh_Light::locationLight1_radius[6];
int QSh_Light::locationLight1_soft[6];
int QSh_Light::locationLight_dir[6];
int QSh_Light::locationLight_spotCutOff[6];
int QSh_Light::locationLight_specularIntensity[3];
int QSh_Light::locationLight_specularPower[3];
int QSh_Light::locationLight_viewPosition[3];

void QSh_Light::preprocess()
{
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    _currentIndexType = Light;
    _findLights(scene);
    if (_light[0]) {
        if (_light[1]) {
            if (_light[0]->type() == QLight::Omni) {
                if (_light[1]->type() == QLight::Omni) {
                    if (_specularIntensity > 0.0f)
                        _subIndexType = 3;
                    else
                        _subIndexType = 0;
                    return;
                }
            } else {
                if (_light[1]->type() == QLight::Spot) {
                    if (_specularIntensity > 0.0f)
                        _subIndexType = 5;
                    else
                        _subIndexType = 2;
                    return;
                }
            }
        }
        if (_specularIntensity > 0.0f)
            _subIndexType = 4;
        else
            _subIndexType = 1;
        return;
    }
    _subIndexType = 0;
    _currentIndexType = Texture1;
    return;
}

bool QSh_Light::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    if (_currentIndexType == Light) {
        context->glActiveTexture(GL_TEXTURE0);
        context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
        program->setUniformValue(locationTexture0[_subIndexType], 0);
        program->setUniformValue(locationMatrixWVP[_subIndexType], _sceneObject->matrixWorldViewProj());
        program->setUniformValue(locationMatrixW[_subIndexType], _sceneObject->matrixWorld());
        program->setUniformValue(locationColor[_subIndexType], _color);
        program->setUniformValue(locationAmbientColor[_subIndexType], scene->ambientColor());
        program->setUniformValue(locationLight0_color[_subIndexType], _light[0]->colorVector());
        program->setUniformValue(locationLight0_position[_subIndexType], _light[0]->globalPosition());
        program->setUniformValue(locationLight0_radius[_subIndexType], _light[0]->radius());
        program->setUniformValue(locationLight0_soft[_subIndexType], _light[0]->lightSoft());
        if (_subIndexType < 3) {
            if (_subIndexType == 2) {
                QSpotLight* spotLight = static_cast<QSpotLight*>(_light[0]);
                program->setUniformValue(locationLight_spotCutOff[2], spotLight->spotCutOff());
                program->setUniformValue(locationLight_dir[2], spotLight->directional());
            }
            if (_light[1] == nullptr) {
                program->setUniformValue(locationLight1_radius[_subIndexType], -1.0f);
            } else {
                program->setUniformValue(locationLight1_color[_subIndexType], _light[1]->colorVector());
                program->setUniformValue(locationLight1_position[_subIndexType], _light[1]->globalPosition());
                program->setUniformValue(locationLight1_radius[_subIndexType], _light[1]->radius());
                program->setUniformValue(locationLight1_soft[_subIndexType], _light[1]->lightSoft());
                if (_subIndexType >= 1) {
                    QSpotLight* spotLight = static_cast<QSpotLight*>(_light[1]);
                    program->setUniformValue(locationLight_spotCutOff[_subIndexType - 1], spotLight->spotCutOff());
                    program->setUniformValue(locationLight_dir[_subIndexType - 1], spotLight->directional());
                }
            }
        } else {
            if (_subIndexType == 5) {
                QSpotLight* spotLight = static_cast<QSpotLight*>(_light[0]);
                program->setUniformValue(locationLight_spotCutOff[5], spotLight->spotCutOff());
                program->setUniformValue(locationLight_dir[5], spotLight->directional());
            }
            if (_light[1] == nullptr) {
                program->setUniformValue(locationLight1_radius[_subIndexType], -1.0f);
            } else {
                program->setUniformValue(locationLight1_color[_subIndexType], _light[1]->colorVector());
                program->setUniformValue(locationLight1_position[_subIndexType], _light[1]->globalPosition());
                program->setUniformValue(locationLight1_radius[_subIndexType], _light[1]->radius());
                program->setUniformValue(locationLight1_soft[_subIndexType], _light[1]->lightSoft());
                if (_subIndexType >= 4) {
                    QSpotLight* spotLight = static_cast<QSpotLight*>(_light[1]);
                    program->setUniformValue(locationLight_spotCutOff[_subIndexType - 1], spotLight->spotCutOff());
                    program->setUniformValue(locationLight_dir[_subIndexType - 1], spotLight->directional());
                }
            }
            int subIndexType_3 = _subIndexType - 3;
            program->setUniformValue(locationLight_specularIntensity[subIndexType_3], _specularIntensity);
            program->setUniformValue(locationLight_specularPower[subIndexType_3], _specularPower);
            program->setUniformValue(locationLight_viewPosition[subIndexType_3], scene->cameraPosition());
        }
        return true;
    }
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
    program->setUniformValue(QSh_Texture1::locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    QColor ambient = scene->ambientColor();
    program->setUniformValue(QSh_Texture1::locationColor, QVector4D(_color.redF() * ambient.redF(),
                                                                    _color.greenF() * ambient.greenF(),
                                                                    _color.blueF() * ambient.blueF(),
                                                                    _color.alphaF()));
    program->setUniformValue(QSh_Texture1::locationTexture0, 0);
    return true;
}


void QSh_Light::_findLights(QScene* scene)
{
    _light[0] = _light[1] = nullptr;
    float intensity, maxIntensity = -999999.0f, prevMaxIntensity = maxIntensity;
    QBoundingBox boundingBox = _drawObject->boundingBox();
    QLight* light;
    int i;
    for (i=0; i<scene->countLights(); i++) {
        light = scene->light(i);
        if (light->boundingBox().collision(boundingBox)) {
            intensity = light->intensityAtPoint(_drawObject->centerOfBoundingBox());
            if (intensity > maxIntensity) {
                prevMaxIntensity = maxIntensity;
                _light[1] = _light[0];
                maxIntensity = intensity;
                _light[0] = light;
            } else if (intensity > prevMaxIntensity) {
                prevMaxIntensity = intensity;
                _light[1] = light;
            }
        }
    }
}

void QSh_Light::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(6);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[1] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[2] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[3] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[4] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[5] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    //Light Omni-Omni
    if (!context->loadShader(shaders[0].data(), "Light_", "Light_OmniOmni"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[0]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[0].data(), "Light Omni-Omni"))
        return;
    QSh_Light::locationMatrixWVP[0] = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[0] = shaders[0]->uniformLocation("matrix_w");
    QSh_Light::locationColor[0] = shaders[0]->uniformLocation("color");
    QSh_Light::locationTexture0[0] = shaders[0]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[0] = shaders[0]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[0] = shaders[0]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[0] = shaders[0]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[0] = shaders[0]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[0] = shaders[0]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[0] = shaders[0]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[0] = shaders[0]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[0] = shaders[0]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[0] = shaders[0]->uniformLocation("light1.soft");

    //Light Omni-Spot
    if (!context->loadShader(shaders[1].data(), "Light_", "Light_OmniSpot"))
        return;
    shaders[1]->bindAttributeLocation("vertex_position", 0);
    shaders[1]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[1]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[1].data(), "Light Omni-Spot"))
        return;
    QSh_Light::locationMatrixWVP[1] = shaders[1]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[1] = shaders[1]->uniformLocation("matrix_w");
    QSh_Light::locationColor[1] = shaders[1]->uniformLocation("color");
    QSh_Light::locationTexture0[1] = shaders[1]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[1] = shaders[1]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[1] = shaders[1]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[1] = shaders[1]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[1] = shaders[1]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[1] = shaders[1]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[1] = shaders[1]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[1] = shaders[1]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[1] = shaders[1]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[1] = shaders[1]->uniformLocation("light1.soft");
    QSh_Light::locationLight_dir[0] = shaders[1]->uniformLocation("light1.dir");
    QSh_Light::locationLight_spotCutOff[0] = shaders[1]->uniformLocation("light1.spotCutOff");

    //Light Spot-Spot
    if (!context->loadShader(shaders[2].data(), "Light_", "Light_SpotSpot"))
        return;
    shaders[2]->bindAttributeLocation("vertex_position", 0);
    shaders[2]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[2]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[2].data(), "Light Spot-Spot"))
        return;
    QSh_Light::locationMatrixWVP[2] = shaders[2]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[2] = shaders[2]->uniformLocation("matrix_w");
    QSh_Light::locationColor[2] = shaders[2]->uniformLocation("color");
    QSh_Light::locationTexture0[2] = shaders[2]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[2] = shaders[2]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[2] = shaders[2]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[2] = shaders[2]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[2] = shaders[2]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[2] = shaders[2]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[2] = shaders[2]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[2] = shaders[2]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[2] = shaders[2]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[2] = shaders[2]->uniformLocation("light1.soft");
    QSh_Light::locationLight_dir[1] = shaders[2]->uniformLocation("light1.dir");
    QSh_Light::locationLight_spotCutOff[1] = shaders[2]->uniformLocation("light1.spotCutOff");
    QSh_Light::locationLight_dir[2] = shaders[2]->uniformLocation("light0.dir");
    QSh_Light::locationLight_spotCutOff[2] = shaders[2]->uniformLocation("light0.spotCutOff");

    //Light Omni-Omni Specular
    if (!context->loadShader(shaders[3].data(), "Light_", "Light_OmniOmni_Specular"))
        return;
    shaders[3]->bindAttributeLocation("vertex_position", 0);
    shaders[3]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[3]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[3].data(), "Light Omni-Omni Specular"))
        return;
    QSh_Light::locationMatrixWVP[3] = shaders[3]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[3] = shaders[3]->uniformLocation("matrix_w");
    QSh_Light::locationColor[3] = shaders[3]->uniformLocation("color");
    QSh_Light::locationTexture0[3] = shaders[3]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[3] = shaders[3]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[3] = shaders[3]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[3] = shaders[3]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[3] = shaders[3]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[3] = shaders[3]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[3] = shaders[3]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[3] = shaders[3]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[3] = shaders[3]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[3] = shaders[3]->uniformLocation("light1.soft");
    QSh_Light::locationLight_specularIntensity[0] = shaders[3]->uniformLocation("specularIntensity");
    QSh_Light::locationLight_specularPower[0] = shaders[3]->uniformLocation("specularPower");
    QSh_Light::locationLight_viewPosition[0] = shaders[3]->uniformLocation("viewPosition");

    //Light Omni-Spot Specular
    if (!context->loadShader(shaders[4].data(), "Light_", "Light_OmniSpot_Specular"))
        return;
    shaders[4]->bindAttributeLocation("vertex_position", 0);
    shaders[4]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[4]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[4].data(), "Light Omni-Spot Specular"))
        return;
    QSh_Light::locationMatrixWVP[4] = shaders[4]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[4] = shaders[4]->uniformLocation("matrix_w");
    QSh_Light::locationColor[4] = shaders[4]->uniformLocation("color");
    QSh_Light::locationTexture0[4] = shaders[4]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[4] = shaders[4]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[4] = shaders[4]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[4] = shaders[4]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[4] = shaders[4]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[4] = shaders[4]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[4] = shaders[4]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[4] = shaders[4]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[4] = shaders[4]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[4] = shaders[4]->uniformLocation("light1.soft");
    QSh_Light::locationLight_dir[3] = shaders[4]->uniformLocation("light1.dir");
    QSh_Light::locationLight_spotCutOff[3] = shaders[4]->uniformLocation("light1.spotCutOff");
    QSh_Light::locationLight_specularIntensity[1] = shaders[4]->uniformLocation("specularIntensity");
    QSh_Light::locationLight_specularPower[1] = shaders[4]->uniformLocation("specularPower");
    QSh_Light::locationLight_viewPosition[1] = shaders[4]->uniformLocation("viewPosition");

    //Light Spot-Spot Specular
    if (!context->loadShader(shaders[5].data(), "Light_", "Light_SpotSpot_Specular"))
        return;
    shaders[5]->bindAttributeLocation("vertex_position", 0);
    shaders[5]->bindAttributeLocation("vertex_texcoord", 1);
    shaders[5]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[5].data(), "Light Spot-Spot Specular"))
        return;
    QSh_Light::locationMatrixWVP[5] = shaders[5]->uniformLocation("matrix_wvp");
    QSh_Light::locationMatrixW[5] = shaders[5]->uniformLocation("matrix_w");
    QSh_Light::locationColor[5] = shaders[5]->uniformLocation("color");
    QSh_Light::locationTexture0[5] = shaders[5]->uniformLocation("texture0");
    QSh_Light::locationAmbientColor[5] = shaders[5]->uniformLocation("ambientColor");
    QSh_Light::locationLight0_color[5] = shaders[5]->uniformLocation("light0.color");
    QSh_Light::locationLight0_position[5] = shaders[5]->uniformLocation("light0.position");
    QSh_Light::locationLight0_radius[5] = shaders[5]->uniformLocation("light0.radius");
    QSh_Light::locationLight0_soft[5] = shaders[5]->uniformLocation("light0.soft");
    QSh_Light::locationLight1_color[5] = shaders[5]->uniformLocation("light1.color");
    QSh_Light::locationLight1_position[5] = shaders[5]->uniformLocation("light1.position");
    QSh_Light::locationLight1_radius[5] = shaders[5]->uniformLocation("light1.radius");
    QSh_Light::locationLight1_soft[5] = shaders[5]->uniformLocation("light1.soft");
    QSh_Light::locationLight_dir[4] = shaders[5]->uniformLocation("light1.dir");
    QSh_Light::locationLight_spotCutOff[4] = shaders[5]->uniformLocation("light1.spotCutOff");
    QSh_Light::locationLight_dir[5] = shaders[5]->uniformLocation("light0.dir");
    QSh_Light::locationLight_spotCutOff[5] = shaders[5]->uniformLocation("light0.spotCutOff");
    QSh_Light::locationLight_specularIntensity[2] = shaders[5]->uniformLocation("specularIntensity");
    QSh_Light::locationLight_specularPower[2] = shaders[5]->uniformLocation("specularPower");
    QSh_Light::locationLight_viewPosition[2] = shaders[5]->uniformLocation("viewPosition");
}

}
