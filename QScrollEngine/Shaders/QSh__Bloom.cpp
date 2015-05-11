#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__Bloom.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh__Bloom::locationMatrixWVP;
int QSh__Bloom::locationScreenTexture;
int QSh__Bloom::locationBloomMapTexture;
int QSh__Bloom::locationTexelParam;
const QVector2D QSh__Bloom::_D = QVector2D(1.66f, 1.0f);

bool QSh__Bloom::use(QScrollEngineContext* , QOpenGLShaderProgram* program)
{
    program->setUniformValue(locationMatrixWVP, _finalMatrix);
    program->setUniformValue(locationScreenTexture, 0);
    program->setUniformValue(locationBloomMapTexture, 1);
    program->setUniformValue(locationTexelParam, QVector3D(_texel.x(), _texel.y(), _intensity));
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
    if (!context->loadShader(shaders[0].data(), "Texture1", "Bloom"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "Bloom"))
        return;
    QSh__Bloom::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh__Bloom::locationScreenTexture = shaders[0]->uniformLocation("screenTexture");
    QSh__Bloom::locationBloomMapTexture = shaders[0]->uniformLocation("bloomMapTexture");
    QSh__Bloom::locationTexelParam = shaders[0]->uniformLocation("texelParam");
}

}
