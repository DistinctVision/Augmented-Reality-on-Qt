#include "QScrollEngine/Shaders/QSh_ColoredPart.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh_ColoredPart::locationMatrixWVP;
int QSh_ColoredPart::locationColor;
int QSh_ColoredPart::locationBackColor;
int QSh_ColoredPart::locationTexturePart;
int QSh_ColoredPart::locationPartParam;

bool QSh_ColoredPart::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(locationColor, _color);
    program->setUniformValue(locationBackColor, _backColor);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (_texturePart) ? _texturePart->textureId() : context->emptyTexture()->textureId());
    program->setUniformValue(locationTexturePart, 0);
    program->setUniformValue(locationPartParam, QVector2D(_partParam.x() * (1.0f + _partParam.y()), _partParam.y()));
    return true;
}

void QSh_ColoredPart::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture1", "ColoredPart"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "ColoredPart"))
        return;
    QSh_ColoredPart::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh_ColoredPart::locationColor = shaders[0]->uniformLocation("color");
    QSh_ColoredPart::locationBackColor = shaders[0]->uniformLocation("backColor");
    QSh_ColoredPart::locationTexturePart = shaders[0]->uniformLocation("texturePart");
    QSh_ColoredPart::locationPartParam = shaders[0]->uniformLocation("partParam");
}

}
