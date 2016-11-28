#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh_Texture::UniformLocation QSh_Texture::m_locations;

bool QSh_Texture::use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    m_locations.bindParameters(context, program, this, sceneObject);
    return true;
}

void QSh_Texture::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Texture", "Texture"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh_Texture::UniformLocation::bindParameters(QScrollEngineContext* context,
                                                  QOpenGLShaderProgram* program,
                                                  const QSh_Texture* shader,
                                                  const QSceneObject3D* sceneObject) const
{
    QSh_Color::UniformLocation::bindParameters(program, shader, sceneObject);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (shader->texture()) ? shader->texture()->textureId() :
                                           context->emptyTexture()->textureId());
     program->setUniformValue(texture, 0);
}

void QSh_Texture::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    QSh_Color::UniformLocation::loadLocations(shader);
    texture = shader->uniformLocation("texture");
}

}
