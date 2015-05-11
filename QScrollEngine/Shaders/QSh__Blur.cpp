#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh__Blur.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh__Blur::locationMatrixWVP;
int QSh__Blur::locationScreenTexture;
int QSh__Blur::locationTexelParam;
const QVector2D QSh__Blur::_D = QVector2D(1.2f, 0.66f);

bool QSh__Blur::use(QScrollEngineContext* , QOpenGLShaderProgram* program)
{
    program->setUniformValue(locationMatrixWVP, _finalMatrix);
    if (_axisX)
        program->setUniformValue(locationTexelParam, QVector3D(_texelParam.x(), 0.0f, _texelParam.z()));
    else
        program->setUniformValue(locationTexelParam, QVector3D(0.0f, _texelParam.y(), _texelParam.z()));
    program->setUniformValue(locationScreenTexture, 0);
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
    if (!context->loadShader(shaders[0].data(), "Texture1", "Blur"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "Blur"))
        return;
    QSh__Blur::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh__Blur::locationScreenTexture = shaders[0]->uniformLocation("screenTexture");
    QSh__Blur::locationTexelParam = shaders[0]->uniformLocation("texelParam");
}

}
