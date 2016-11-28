#include "QScrollEngine/Shaders/QSh_Refraction_FallOff.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QScene.h"
#include <QVector3D>

namespace QScrollEngine {

QSh_Refraction_FallOff::UniformLocation QSh_Refraction_FallOff::m_locations;

bool QSh_Refraction_FallOff::use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    if (m_screenTexture == 0)
        return false;
    QSH_ASSERT(sceneObject != nullptr);
    m_locations.bindParameters(context, program, this, sceneObject);
    return true;
}

void QSh_Refraction_FallOff::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Refraction_FallOff", "Refraction_FallOff"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh_Refraction_FallOff::UniformLocation::bindParameters(QScrollEngineContext* context,
                                                             QOpenGLShaderProgram* program,
                                                             const QSh_Refraction_FallOff* shader,
                                                             const QSceneObject3D* sceneObject) const
{
    QScene* scene = sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    program->setUniformValue(matrixWVP, sceneObject->matrixWorldViewProj());
    program->setUniformValue(matrixW, sceneObject->matrixWorld());
    program->setUniformValue(worldCameraPosition, scene->cameraPosition());
    program->setUniformValue(screenTexture, 0);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, shader->screenTexture());
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue(refract, shader->refract());
    QColor fallOffColorV = shader->fallOffColor();
    program->setUniformValue(fallOffColor, QVector3D(fallOffColorV.redF(), fallOffColorV.greenF(), fallOffColorV.blueF()));
    program->setUniformValue(fallOffIntensity, shader->fallOffIntensity());
    program->setUniformValue(fallOffSoft, shader->fallOffSoft());
    program->setUniformValue(matrixScreenTexture, shader->matrixScreenTexture());
}

void QSh_Refraction_FallOff::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    matrixWVP = shader->uniformLocation("matrix_wvp");
    matrixW = shader->uniformLocation("matrix_w");
    worldCameraPosition = shader->uniformLocation("worldCameraPosition");
    screenTexture = shader->uniformLocation("screenTexture");
    refract = shader->uniformLocation("refractPower");
    fallOffColor = shader->uniformLocation("FallOffColor");
    fallOffIntensity = shader->uniformLocation("FallOffInt");
    fallOffSoft = shader->uniformLocation("FallOffSoft");
    matrixScreenTexture = shader->uniformLocation("matrix_screen");
}

}

