#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__Blur.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh__Blur::UniformLocation QSh__Blur::m_locations;
const QVector2D QSh__Blur::m_D = QVector2D(1.2f, 0.66f);

bool QSh__Blur::use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* )
{
    m_locations.bindParameters(program, this);
    return true;
}

void QSh__Blur::bindScreenTexture(QScrollEngineContext* context, GLuint screenTexture)
{
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, screenTexture);
}

void QSh__Blur::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture", "Blur"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh__Blur::UniformLocation::bindParameters(QOpenGLShaderProgram* program, const QSh__Blur* shader) const
{
    program->setUniformValue(matrixWVP, shader->finalMatrix());
    program->setUniformValue(texelParam, shader->texelParam());
    program->setUniformValue(screenTexture, 0);
}

void QSh__Blur::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    matrixWVP = shader->uniformLocation("matrix_wvp");
    screenTexture = shader->uniformLocation("screenTexture");
    texelParam = shader->uniformLocation("texelParam");
}

}
