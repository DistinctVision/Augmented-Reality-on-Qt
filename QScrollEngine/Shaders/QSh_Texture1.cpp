#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh_Texture1::locationMatrixWVP;
int QSh_Texture1::locationColor;
int QSh_Texture1::locationTexture0;

bool QSh_Texture1::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
    program->setUniformValue(locationTexture0, 0);
    program->setUniformValue(locationColor, _color);
    return true;
}

void QSh_Texture1::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture1", "Texture1"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "Texture1"))
        return;
    QSh_Texture1::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Texture1::locationColor = shaders[0]->uniformLocation("color");
    QSh_Texture1::locationTexture0 = shaders[0]->uniformLocation("texture0");
}

}
