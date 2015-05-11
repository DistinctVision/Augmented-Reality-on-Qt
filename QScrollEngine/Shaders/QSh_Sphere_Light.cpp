#include "QScrollEngine/Shaders/QSh_Sphere_Light.h"
#include "QScrollEngine/Shaders/QSh_Sphere_Texture1.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"

namespace QScrollEngine {

int QSh_Sphere_Light::locationMatrixWVP[3];
int QSh_Sphere_Light::locationMatrixW[3];
int QSh_Sphere_Light::locationColor[3];
int QSh_Sphere_Light::locationTexture0[3];
int QSh_Sphere_Light::locationViewPosition[3];
int QSh_Sphere_Light::locationSphereCenter[3];
int QSh_Sphere_Light::locationSphereRadiusSquared[3];
int QSh_Sphere_Light::locationSphereMatrixRotation[3];
int QSh_Sphere_Light::locationAmbientColor[3];
int QSh_Sphere_Light::locationLight0_color[3];
int QSh_Sphere_Light::locationLight0_position[3];
int QSh_Sphere_Light::locationLight0_radius[3];
int QSh_Sphere_Light::locationLight0_soft[3];
int QSh_Sphere_Light::locationLight1_color[3];
int QSh_Sphere_Light::locationLight1_position[3];
int QSh_Sphere_Light::locationLight1_radius[3];
int QSh_Sphere_Light::locationLight1_soft[3];
int QSh_Sphere_Light::locationLight_dir[3];
int QSh_Sphere_Light::locationLight_spotCutOff[3];
int QSh_Sphere_Light::locationLight_diffuseIntensity[3];
int QSh_Sphere_Light::locationLight_specularIntensity[3];
int QSh_Sphere_Light::locationLight_specularPower[3];

void QSh_Sphere_Light::preprocess()
{
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    _findLights(scene);
    if (_light[0]) {
        if (_light[1]) {
            if (_light[0]->type() == QLight::Omni) {
                if (_light[1]->type() == QLight::Omni) {
                    _currentIndexType = Sphere_Light;
                    _subIndexType = 0;
                    return;
                }
            } else {
                if (_light[1]->type() == QLight::Spot) {
                    _currentIndexType = Sphere_Light;
                    _subIndexType = 2;
                    return;
                }
            }
        }
        _currentIndexType = Sphere_Light;
        _subIndexType = 1;
        return;
    }
    _currentIndexType = Sphere_Texture1;
    _subIndexType = 0;
    return;
}

bool QSh_Sphere_Light::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    if (_currentIndexType == Sphere_Light) {
        program->setUniformValue(locationMatrixWVP[_subIndexType], _sceneObject->matrixWorldViewProj());
        program->setUniformValue(locationMatrixW[_subIndexType], _sceneObject->matrixWorld());
        program->setUniformValue(locationColor[_subIndexType], _color);
        program->setUniformValue(locationTexture0[_subIndexType], 0);
        context->glActiveTexture(GL_TEXTURE0);
        context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        program->setUniformValue(locationViewPosition[_subIndexType], scene->cameraPosition());
        program->setUniformValue(locationSphereCenter[_subIndexType], _sceneObject->globalPosition());
        program->setUniformValue(locationSphereMatrixRotation[_subIndexType], _sphereMatrixRotation);
        program->setUniformValue(locationSphereRadiusSquared[_subIndexType], _sphereRadius * _sphereRadius);
        program->setUniformValue(locationAmbientColor[_subIndexType], scene->ambientColor());
        program->setUniformValue(locationLight0_color[_subIndexType], _light[0]->colorVector());
        program->setUniformValue(locationLight0_position[_subIndexType], _light[0]->globalPosition());
        program->setUniformValue(locationLight0_radius[_subIndexType], _light[0]->radius());
        program->setUniformValue(locationLight0_soft[_subIndexType], _light[0]->lightSoft());
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
        program->setUniformValue(locationLight_diffuseIntensity[_subIndexType], _diffuseIntensity);
        program->setUniformValue(locationLight_specularIntensity[_subIndexType], _specularIntensity);
        program->setUniformValue(locationLight_specularPower[_subIndexType], _specularPower);
        return true;
    }
    program->setUniformValue(QSh_Sphere_Texture1::locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(QSh_Sphere_Texture1::locationMatrixW, _sceneObject->matrixWorld());
    QColor ambient = scene->ambientColor();
    program->setUniformValue(QSh_Sphere_Texture1::locationColor, QVector4D(_color.redF() * ambient.redF(),
                                                                           _color.greenF() * ambient.greenF(),
                                                                           _color.blueF() * ambient.blueF(),
                                                                           _color.alphaF()));
    program->setUniformValue(QSh_Sphere_Texture1::locationTexture0, 0);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue(QSh_Sphere_Texture1::locationViewPosition, scene->cameraPosition());
    program->setUniformValue(QSh_Sphere_Texture1::locationSphereCenter, _sceneObject->globalPosition());
    program->setUniformValue(QSh_Sphere_Texture1::locationSphereMatrixRotation, _sphereMatrixRotation);
    program->setUniformValue(QSh_Sphere_Texture1::locationSphereRadiusSquared, _sphereRadius * _sphereRadius);
    return true;
}

void QSh_Sphere_Light::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(3);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Sphere_", "Sphere_OmniOmni"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    if (!context->checkBindShader(shaders[0].data(), "Sphere Omni-Omni"))
        return;
    QSh_Sphere_Light::locationMatrixWVP[0] = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Sphere_Light::locationMatrixW[0] = shaders[0]->uniformLocation("matrix_w");
    QSh_Sphere_Light::locationColor[0] = shaders[0]->uniformLocation("color");
    QSh_Sphere_Light::locationTexture0[0] = shaders[0]->uniformLocation("texture0");
    QSh_Sphere_Light::locationViewPosition[0] = shaders[0]->uniformLocation("viewPosition");
    QSh_Sphere_Light::locationSphereCenter[0] = shaders[0]->uniformLocation("sphereCenter");
    QSh_Sphere_Light::locationSphereRadiusSquared[0] = shaders[0]->uniformLocation("sphereRadiusSquared");
    QSh_Sphere_Light::locationSphereMatrixRotation[0] = shaders[0]->uniformLocation("sphereMatrixRotation");
    QSh_Sphere_Light::locationAmbientColor[0] = shaders[0]->uniformLocation("ambientColor");
    QSh_Sphere_Light::locationLight0_color[0] = shaders[0]->uniformLocation("light0.color");
    QSh_Sphere_Light::locationLight0_position[0] = shaders[0]->uniformLocation("light0.position");
    QSh_Sphere_Light::locationLight0_radius[0] = shaders[0]->uniformLocation("light0.radius");
    QSh_Sphere_Light::locationLight0_soft[0] = shaders[0]->uniformLocation("light0.soft");
    QSh_Sphere_Light::locationLight1_color[0] = shaders[0]->uniformLocation("light1.color");
    QSh_Sphere_Light::locationLight1_position[0] = shaders[0]->uniformLocation("light1.position");
    QSh_Sphere_Light::locationLight1_radius[0] = shaders[0]->uniformLocation("light1.radius");
    QSh_Sphere_Light::locationLight1_soft[0] = shaders[0]->uniformLocation("light1.soft");
    QSh_Sphere_Light::locationLight_diffuseIntensity[0] = shaders[0]->uniformLocation("diffuseIntensity");
    QSh_Sphere_Light::locationLight_specularIntensity[0] = shaders[0]->uniformLocation("specularIntensity");
    QSh_Sphere_Light::locationLight_specularPower[0] = shaders[0]->uniformLocation("specularPower");

    shaders[1] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[1].data(), "Sphere_", "Sphere_OmniSpot"))
        return;
    shaders[1]->bindAttributeLocation("vertex_position", 0);
    if (!context->checkBindShader(shaders[1].data(), "Sphere Omni-Omni"))
        return;
    QSh_Sphere_Light::locationMatrixWVP[1] = shaders[1]->uniformLocation("matrix_wvp");
    QSh_Sphere_Light::locationMatrixW[1] = shaders[1]->uniformLocation("matrix_w");
    QSh_Sphere_Light::locationColor[1] = shaders[1]->uniformLocation("color");
    QSh_Sphere_Light::locationTexture0[1] = shaders[1]->uniformLocation("texture0");
    QSh_Sphere_Light::locationViewPosition[1] = shaders[1]->uniformLocation("viewPosition");
    QSh_Sphere_Light::locationSphereCenter[1] = shaders[1]->uniformLocation("sphereCenter");
    QSh_Sphere_Light::locationSphereRadiusSquared[1] = shaders[1]->uniformLocation("sphereRadiusSquared");
    QSh_Sphere_Light::locationSphereMatrixRotation[1] = shaders[1]->uniformLocation("sphereMatrixRotation");
    QSh_Sphere_Light::locationAmbientColor[1] = shaders[1]->uniformLocation("ambientColor");
    QSh_Sphere_Light::locationLight0_color[1] = shaders[1]->uniformLocation("light0.color");
    QSh_Sphere_Light::locationLight0_position[1] = shaders[1]->uniformLocation("light0.position");
    QSh_Sphere_Light::locationLight0_radius[1] = shaders[1]->uniformLocation("light0.radius");
    QSh_Sphere_Light::locationLight0_soft[1] = shaders[1]->uniformLocation("light0.soft");
    QSh_Sphere_Light::locationLight1_color[1] = shaders[1]->uniformLocation("light1.color");
    QSh_Sphere_Light::locationLight1_position[1] = shaders[1]->uniformLocation("light1.position");
    QSh_Sphere_Light::locationLight1_radius[1] = shaders[1]->uniformLocation("light1.radius");
    QSh_Sphere_Light::locationLight1_soft[1] = shaders[1]->uniformLocation("light1.soft");
    QSh_Sphere_Light::locationLight_diffuseIntensity[1] = shaders[1]->uniformLocation("diffuseIntensity");
    QSh_Sphere_Light::locationLight_specularIntensity[1] = shaders[1]->uniformLocation("specularIntensity");
    QSh_Sphere_Light::locationLight_specularPower[1] = shaders[1]->uniformLocation("specularPower");
    QSh_Sphere_Light::locationLight_dir[0] = shaders[1]->uniformLocation("light1.dir");
    QSh_Sphere_Light::locationLight_spotCutOff[0] = shaders[1]->uniformLocation("light1.spotCutOff");

    shaders[2] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[2].data(), "Sphere_", "Sphere_SpotSpot"))
        return;
    shaders[2]->bindAttributeLocation("vertex_position", 0);
    if (!context->checkBindShader(shaders[2].data(), "Sphere Spot-Spot"))
        return;
    QSh_Sphere_Light::locationMatrixWVP[2] = shaders[2]->uniformLocation("matrix_wvp");
    QSh_Sphere_Light::locationMatrixW[2] = shaders[2]->uniformLocation("matrix_w");
    QSh_Sphere_Light::locationColor[2] = shaders[2]->uniformLocation("color");
    QSh_Sphere_Light::locationTexture0[2] = shaders[2]->uniformLocation("texture0");
    QSh_Sphere_Light::locationViewPosition[2] = shaders[2]->uniformLocation("viewPosition");
    QSh_Sphere_Light::locationSphereCenter[2] = shaders[2]->uniformLocation("sphereCenter");
    QSh_Sphere_Light::locationSphereRadiusSquared[2] = shaders[2]->uniformLocation("sphereRadiusSquared");
    QSh_Sphere_Light::locationSphereMatrixRotation[2] = shaders[2]->uniformLocation("sphereMatrixRotation");
    QSh_Sphere_Light::locationAmbientColor[2] = shaders[2]->uniformLocation("ambientColor");
    QSh_Sphere_Light::locationLight0_color[2] = shaders[2]->uniformLocation("light0.color");
    QSh_Sphere_Light::locationLight0_position[2] = shaders[2]->uniformLocation("light0.position");
    QSh_Sphere_Light::locationLight0_radius[2] = shaders[2]->uniformLocation("light0.radius");
    QSh_Sphere_Light::locationLight0_soft[2] = shaders[2]->uniformLocation("light0.soft");
    QSh_Sphere_Light::locationLight1_color[2] = shaders[2]->uniformLocation("light1.color");
    QSh_Sphere_Light::locationLight1_position[2] = shaders[2]->uniformLocation("light1.position");
    QSh_Sphere_Light::locationLight1_radius[2] = shaders[2]->uniformLocation("light1.radius");
    QSh_Sphere_Light::locationLight1_soft[2] = shaders[2]->uniformLocation("light1.soft");
    QSh_Sphere_Light::locationLight_diffuseIntensity[2] = shaders[2]->uniformLocation("diffuseIntensity");
    QSh_Sphere_Light::locationLight_specularIntensity[2] = shaders[2]->uniformLocation("specularIntensity");
    QSh_Sphere_Light::locationLight_specularPower[2] = shaders[2]->uniformLocation("specularPower");
    QSh_Sphere_Light::locationLight_dir[1] = shaders[2]->uniformLocation("light1.dir");
    QSh_Sphere_Light::locationLight_spotCutOff[1] = shaders[2]->uniformLocation("light1.spotCutOff");
    QSh_Sphere_Light::locationLight_dir[2] = shaders[2]->uniformLocation("light0.dir");
    QSh_Sphere_Light::locationLight_spotCutOff[2] = shaders[2]->uniformLocation("light0.spotCutOff");
}

}
