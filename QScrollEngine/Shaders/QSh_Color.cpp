#include "QScrollEngine/Shaders/QSh_Color.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

int QSh_Color::locationMatrixWVP;
int QSh_Color::locationColor;

bool QSh_Color::use(QScrollEngineContext* , QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(locationColor, _color);
    return true;
}

void QSh_Color::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Color", "Color"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "Color"))
        return;
    QSh_Color::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Color::locationColor = shaders[0]->uniformLocation("color");
}

}
