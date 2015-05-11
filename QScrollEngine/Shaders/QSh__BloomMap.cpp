#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__BloomMap.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh__BloomMap::locationMatrixWVP;
int QSh__BloomMap::locationScreenTexture;
int QSh__BloomMap::locationThreshold;
int QSh__BloomMap::locationGain;

bool QSh__BloomMap::use(QScrollEngineContext* , QOpenGLShaderProgram* program)
{
    program->setUniformValue(locationMatrixWVP, _finalMatrix);
    program->setUniformValue(locationThreshold, _threshold);
    program->setUniformValue(locationGain, _gain);
    program->setUniformValue(locationScreenTexture, 0);
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
    if (!context->loadShader(shaders[0].data(), "Texture1", "BloomMap"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "BloomMap"))
        return;
    QSh__BloomMap::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh__BloomMap::locationScreenTexture = shaders[0]->uniformLocation("screenTexture");
    QSh__BloomMap::locationThreshold = shaders[0]->uniformLocation("threshold");
    QSh__BloomMap::locationGain = shaders[0]->uniformLocation("gain");
}

}
