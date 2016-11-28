#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__BloomMap.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh__BloomMap::UniformLocation QSh__BloomMap::m_locations;

bool QSh__BloomMap::use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* )
{
    m_locations.bindParameters(program, this);
    return true;
}

void QSh__BloomMap::bindScreenTexture(QScrollEngineContext* context, GLuint screenTexture)
{
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, screenTexture);
}

void QSh__BloomMap::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture", "BloomMap"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh__BloomMap::UniformLocation::bindParameters(QOpenGLShaderProgram* program, const QSh__BloomMap* shader) const
{
    program->setUniformValue(matrixWVP, shader->finalMatrix());
    program->setUniformValue(screenTexture, 0);
    program->setUniformValue(threshold, shader->threshold());
    program->setUniformValue(gain, shader->gain());
}

void QSh__BloomMap::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    matrixWVP = shader->uniformLocation("matrix_wvp");
    screenTexture = shader->uniformLocation("screenTexture");
    threshold = shader->uniformLocation("threshold");
    gain = shader->uniformLocation("gain");
}

}
