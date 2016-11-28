#include "QScrollEngine/Shaders/QSh_Color.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSh_Color::UniformLocation QSh_Color::m_locations;

bool QSh_Color::use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    m_locations.bindParameters(program, this, sceneObject);
    return true;
}

void QSh_Color::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Color", "Color"))
        return;
    m_locations.loadLocations(shaders[0].data());
}

void QSh_Color::UniformLocation::bindParameters(QOpenGLShaderProgram* program,
                                                const QSh_Color* shader,
                                                const QSceneObject3D* sceneObject) const
{
    program->setUniformValue(matrixWVP, sceneObject->matrixWorldViewProj());
    program->setUniformValue(color, shader->color());
}

void QSh_Color::UniformLocation::loadLocations(QOpenGLShaderProgram* shader)
{
    matrixWVP = shader->uniformLocation("matrix_wvp");
    color = shader->uniformLocation("color");
}

}
