#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__Bloom.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh__Bloom::UniformLocation QSh__Bloom::m_locations;
const QVector2D QSh__Bloom::m_D = QVector2D(1.66f, 1.0f);

bool QSh__Bloom::use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* )
{
    m_locations.bindParameters(program, this);
    return true;
}

void QSh__Bloom::bindScreenTexture(QScrollEngineContext* parentContext, GLuint screenTexture)
{
    parentContext->glActiveTexture(GL_TEXTURE0);
    parentContext->glBindTexture(GL_TEXTURE_2D, screenTexture);
}

void QSh__Bloom::bindBloomMapTexture(QScrollEngineContext* parentContext, GLuint bloomMapTexture)
{
    parentContext->glActiveTexture(GL_TEXTURE1);
    parentContext->glBindTexture(GL_TEXTURE_2D, bloomMapTexture);
}

void QSh__Bloom::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture", "Bloom"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh__Bloom::UniformLocation::bindParameters(QOpenGLShaderProgram* program, const QSh__Bloom* shader) const
{
    program->setUniformValue(matrixWVP, shader->finalMatrix());
    program->setUniformValue(texelParam, shader->texelParam());
    program->setUniformValue(screenTexture, 0);
    program->setUniformValue(bloomMapTexture, 1);
}

void QSh__Bloom::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    matrixWVP = shader->uniformLocation("matrix_wvp");
    screenTexture = shader->uniformLocation("screenTexture");
    bloomMapTexture = shader->uniformLocation("bloomMapTexture");
    texelParam = shader->uniformLocation("texelParam");
}

}
