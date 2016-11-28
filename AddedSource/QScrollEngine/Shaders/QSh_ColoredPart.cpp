#include "QScrollEngine/Shaders/QSh_ColoredPart.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh_ColoredPart::UniformLocation QSh_ColoredPart::m_locations;

bool QSh_ColoredPart::use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    m_locations.bindParameters(context, program, this, sceneObject);
    return true;
}

void QSh_ColoredPart::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture", "ColoredPart"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh_ColoredPart::UniformLocation::bindParameters(QScrollEngineContext* context, QOpenGLShaderProgram* program,
                                                      const QSh_ColoredPart* shader,
                                                      const QSceneObject3D* sceneObject) const
{
    QSh_Color::UniformLocation::bindParameters(program, shader, sceneObject);
    program->setUniformValue(backColor, shader->backColor());
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (shader->texturePart()) ? shader->texturePart()->textureId() :
                                           context->emptyTexture()->textureId());
    program->setUniformValue(texturePart, 0);
    program->setUniformValue(partParam, shader->partParams());
}

void QSh_ColoredPart::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    QSh_Color::UniformLocation::loadLocations(shader);
    backColor = shader->uniformLocation("backColor");
    texturePart = shader->uniformLocation("texturePart");
    partParam = shader->uniformLocation("partParam");
}

}
