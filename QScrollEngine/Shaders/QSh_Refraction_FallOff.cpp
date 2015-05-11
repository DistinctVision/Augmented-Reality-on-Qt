#include "QScrollEngine/Shaders/QSh_Refraction_FallOff.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QScene.h"
#include <QVector3D>

namespace QScrollEngine {

int QSh_Refraction_FallOff::locationMatrixWVP;
int QSh_Refraction_FallOff::locationMatrixW;
int QSh_Refraction_FallOff::locationMatrixScreenTexture;
int QSh_Refraction_FallOff::locationWorldCameraPosition;
int QSh_Refraction_FallOff::locationScreenTexture;
int QSh_Refraction_FallOff::locationRefract;
int QSh_Refraction_FallOff::locationFallOffColor;
int QSh_Refraction_FallOff::locationFallOffIntensity;
int QSh_Refraction_FallOff::locationFallOffSoft;

bool QSh_Refraction_FallOff::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    if (_screenTexture == 0)
        return false;
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(locationMatrixW, _sceneObject->matrixWorld());
    program->setUniformValue(locationWorldCameraPosition, scene->cameraPosition());
    program->setUniformValue(locationScreenTexture, 0);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, _screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue(locationRefract, _refract);
    program->setUniformValue(locationFallOffColor, QVector3D(_fallOffColor.redF(), _fallOffColor.greenF(), _fallOffColor.blueF()));
    program->setUniformValue(locationFallOffIntensity, _fallOffInt);
    program->setUniformValue(locationFallOffSoft, _fallOffSoft);
    program->setUniformValue(locationMatrixScreenTexture, _matrixScreenTexture);
    return true;
}

void QSh_Refraction_FallOff::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Refraction_FallOff", "Refraction_FallOff"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_normal", 2);
    if (!context->checkBindShader(shaders[0].data(), "Refraction_FallOff"))
        return;
    QSh_Refraction_FallOff::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Refraction_FallOff::locationMatrixW = shaders[0]->uniformLocation("matrix_w");
    QSh_Refraction_FallOff::locationWorldCameraPosition = shaders[0]->uniformLocation("worldCameraPosition");
    QSh_Refraction_FallOff::locationScreenTexture = shaders[0]->uniformLocation("screenTexture");
    QSh_Refraction_FallOff::locationRefract = shaders[0]->uniformLocation("refractPower");
    QSh_Refraction_FallOff::locationFallOffColor = shaders[0]->uniformLocation("FallOffColor");
    QSh_Refraction_FallOff::locationFallOffIntensity = shaders[0]->uniformLocation("FallOffInt");
    QSh_Refraction_FallOff::locationFallOffSoft = shaders[0]->uniformLocation("FallOffSoft");
    QSh_Refraction_FallOff::locationMatrixScreenTexture = shaders[0]->uniformLocation("matrix_screen");
}

}

