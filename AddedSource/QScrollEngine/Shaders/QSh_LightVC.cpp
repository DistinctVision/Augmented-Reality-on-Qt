#include "QScrollEngine/Shaders/QSh_LightVC.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"

namespace QScrollEngine {

QSharedPointer<QSh_LightVC::UniformLocationOmniOmni> QSh_LightVC::m_locations[6];

void QSh_LightVC::preprocess(const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    QScene* scene = sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    _findLights(scene, drawObject);
    if (m_lights[0] != nullptr) {
        m_currentTypeIndex = static_cast<int>(Type::LightVC);
        if (m_lights[1] != nullptr) {
            m_subTypeIndex = (int)m_lights[0]->type() + (int)m_lights[1]->type();
        } else {
            m_subTypeIndex = (int)m_lights[0]->type();
        }
        if (m_specularIntensity > 0.0f)
            m_subTypeIndex += 3;
    } else {
        m_currentTypeIndex = static_cast<int>(Type::LightVC);
        m_subTypeIndex = 0;
    }
}

bool QSh_LightVC::use(QScrollEngineContext* context, QOpenGLShaderProgram* program,
                      const QDrawObject3D* drawObject) {
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    m_locations[m_subTypeIndex]->bindParameters(context, program, this, sceneObject);
    return true;
}

void QSh_LightVC::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) {
    shaders.resize(6);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[1] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[2] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[3] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[4] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[5] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);

    //LightVC Omni-Omni
    m_locations[0] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniOmni());
    if (!context->loadShader(shaders[0].data(), "LightVC_", "LightVC_OmniOmni"))
        return;
    m_locations[0].staticCast<UniformLocationOmniOmni>()->loadLocations(shaders[0].data());

    //LightVC Omni-Spot
    m_locations[1] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniSpot());
    if (!context->loadShader(shaders[1].data(), "LightVC_", "LightVC_OmniSpot"))
        return;
    m_locations[1].staticCast<UniformLocationOmniSpot>()->loadLocations(shaders[1].data());

    //LightVC Spot-Spot
    m_locations[2] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationSpotSpot());
    if (!context->loadShader(shaders[2].data(), "LightVC_", "LightVC_SpotSpot"))
        return;
    m_locations[2].staticCast<UniformLocationSpotSpot>()->loadLocations(shaders[2].data());

    //LightVC Omni-Omni Specular
    m_locations[3] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniOmniSpecular());
    if (!context->loadShader(shaders[3].data(), "LightVC_", "LightVC_OmniOmni_Specular"))
        return;
    m_locations[3].staticCast<UniformLocationOmniOmniSpecular>()->loadLocations(shaders[3].data());

    //LightVC Omni-Spot Specular
    m_locations[4] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniSpotSpecular());
    if (!context->loadShader(shaders[4].data(), "LightVC_", "LightVC_OmniSpot_Specular"))
        return;
    m_locations[4].staticCast<UniformLocationOmniSpotSpecular>()->loadLocations(shaders[4].data());

    //LightVC Spot-Spot Specular
    m_locations[5] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationSpotSpotSpecular());
    if (!context->loadShader(shaders[5].data(), "LightVC_", "LightVC_SpotSpot_Specular"))
        return;
    m_locations[5].staticCast<UniformLocationSpotSpotSpecular>()->loadLocations(shaders[5].data());
}

}
