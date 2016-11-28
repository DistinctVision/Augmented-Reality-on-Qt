#include "QScrollEngine/Shaders/QSh_Light.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"

namespace QScrollEngine {

QSharedPointer<QSh_Light::UniformLocationOmniOmni> QSh_Light::m_locations[6];

void QSh_Light::preprocess(const QDrawObject3D* drawObject)
{
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    QScene* scene = sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    _findLights(scene, drawObject);
    if (m_lights[0] != nullptr) {
        m_currentTypeIndex = (int)Type::Light;
        if (m_lights[1] != nullptr) {
            m_subTypeIndex = (int)m_lights[0]->type() + (int)m_lights[1]->type();
        } else {
            m_subTypeIndex = (int)m_lights[0]->type();
        }
        if (m_specularIntensity > 0.0f)
            m_subTypeIndex += 3;
    } else {
        m_currentTypeIndex = (int)Type::Texture;
        m_subTypeIndex = 0;
    }
}

bool QSh_Light::use(QScrollEngineContext* context, QOpenGLShaderProgram* program,
                    const QDrawObject3D* drawObject) {
    const QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);
    if (m_currentTypeIndex == (int)Type::Light) {
        m_locations[m_subTypeIndex]->bindParameters(context, program, this, sceneObject);
    } else {
        QSh_Texture::getLocations().bindParameters(context, program, this, sceneObject);
    }
    return true;
}

void QSh_Light::_findLights(QScene* scene, const QDrawObject3D* drawObject)
{
    m_lights[0] = m_lights[1] = nullptr;
    float intensity, maxIntensity = -999999.0f, prevMaxIntensity = maxIntensity;
    QBoundingBox boundingBox = drawObject->boundingBox();
    QLight* light;
    std::size_t i;
    for (i=0; i<scene->countLights(); i++) {
        light = scene->light(i);
        if (light->boundingBox().collision(boundingBox)) {
            intensity = light->intensityAtPoint(drawObject->centerOfBoundingBox());
            if (intensity > maxIntensity) {
                prevMaxIntensity = maxIntensity;
                m_lights[1] = m_lights[0];
                maxIntensity = intensity;
                m_lights[0] = light;
            } else if (intensity > prevMaxIntensity) {
                prevMaxIntensity = intensity;
                m_lights[1] = light;
            }
        }
    }
}

void QSh_Light::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) {
    shaders.resize(6);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[1] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[2] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[3] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[4] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    shaders[5] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);

    //Light Omni-Omni
    m_locations[0] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniOmni());
    if (!context->loadShader(shaders[0].data(), "Light_", "Light_OmniOmni"))
        return;
    m_locations[0].staticCast<UniformLocationOmniOmni>()->loadLocations(shaders[0].data());

    //Light Omni-Spot
    m_locations[1] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniSpot());
    if (!context->loadShader(shaders[1].data(), "Light_", "Light_OmniSpot"))
        return;
    m_locations[1].staticCast<UniformLocationOmniSpot>()->loadLocations(shaders[1].data());

    //Light Spot-Spot
    m_locations[2] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationSpotSpot());
    if (!context->loadShader(shaders[2].data(), "Light_", "Light_SpotSpot"))
        return;
    m_locations[2].staticCast<UniformLocationSpotSpot>()->loadLocations(shaders[2].data());

    //Light Omni-Omni Specular
    m_locations[3] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniOmniSpecular());
    if (!context->loadShader(shaders[3].data(), "Light_", "Light_OmniOmni_Specular"))
        return;
    m_locations[3].staticCast<UniformLocationOmniOmniSpecular>()->loadLocations(shaders[3].data());

    //Light Omni-Spot Specular
    m_locations[4] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationOmniSpotSpecular());
    if (!context->loadShader(shaders[4].data(), "Light_", "Light_OmniSpot_Specular"))
        return;
    m_locations[4].staticCast<UniformLocationOmniSpotSpecular>()->loadLocations(shaders[4].data());

    //Light Spot-Spot Specular
    m_locations[5] = QSharedPointer<UniformLocationOmniOmni>(new UniformLocationSpotSpotSpecular());
    if (!context->loadShader(shaders[5].data(), "Light_", "Light_SpotSpot_Specular"))
        return;
    m_locations[5].staticCast<UniformLocationSpotSpotSpecular>()->loadLocations(shaders[5].data());
}

void QSh_Light::UniformLocationOmniOmni::loadLocations(QOpenGLShaderProgram* shader)
{
    QSh_Texture::UniformLocation::loadLocations(shader);
    matrixW = shader->uniformLocation("matrix_w");
    ambientColor = shader->uniformLocation("ambientColor");
    light0_color = shader->uniformLocation("light0.color");
    light0_position = shader->uniformLocation("light0.position");
    light0_radius = shader->uniformLocation("light0.radius");
    light0_soft = shader->uniformLocation("light0.soft");
    light1_color = shader->uniformLocation("light1.color");
    light1_position = shader->uniformLocation("light1.position");
    light1_radius = shader->uniformLocation("light1.radius");
    light1_soft = shader->uniformLocation("light1.soft");
}

void QSh_Light::UniformLocationOmniSpot::loadLocations(QOpenGLShaderProgram* shader)
{
    UniformLocationOmniOmni::loadLocations(shader);
    light1_dir = shader->uniformLocation("light1.dir");
    light1_spotCutOff = shader->uniformLocation("light1.spotCutOff");
}

void QSh_Light::UniformLocationSpotSpot::loadLocations(QOpenGLShaderProgram* shader)
{
    UniformLocationOmniSpot::loadLocations(shader);
    light0_dir = shader->uniformLocation("light0.dir");
    light0_spotCutOff = shader->uniformLocation("light0.spotCutOff");
}

void QSh_Light::UniformLocationOmniOmniSpecular::loadLocations(QOpenGLShaderProgram* shader)
{
    UniformLocationOmniOmni::loadLocations(shader);
    specularIntensity = shader->uniformLocation("specularIntensity");
    specularPower = shader->uniformLocation("specularPower");
    viewPosition = shader->uniformLocation("viewPosition");
}

void QSh_Light::UniformLocationOmniSpotSpecular::loadLocations(QOpenGLShaderProgram* shader)
{
    UniformLocationOmniSpot::loadLocations(shader);
    specularIntensity = shader->uniformLocation("specularIntensity");
    specularPower = shader->uniformLocation("specularPower");
    viewPosition = shader->uniformLocation("viewPosition");
}

void QSh_Light::UniformLocationSpotSpotSpecular::loadLocations(QOpenGLShaderProgram* shader)
{
    UniformLocationSpotSpot::loadLocations(shader);
    specularIntensity = shader->uniformLocation("specularIntensity");
    specularPower = shader->uniformLocation("specularPower");
    viewPosition = shader->uniformLocation("viewPosition");
}

void QSh_Light::UniformLocationOmniOmni::bindParameters(QScrollEngineContext* context,
                                                        QOpenGLShaderProgram* program,
                                                        const QSh_Light* shader,
                                                        const QSceneObject3D* sceneObject) const
{
    QSh_Texture::UniformLocation::bindParameters(context, program, shader, sceneObject);
    program->setUniformValue(ambientColor, sceneObject->scene()->ambientColor());
    program->setUniformValue(matrixW, sceneObject->matrixWorld());
    bindLightParameters(context, program, shader, sceneObject);
}

void QSh_Light::UniformLocationOmniOmni::bindLightParameters(QScrollEngineContext* context,
                                                             QOpenGLShaderProgram* program,
                                                             const QSh_Light* shader,
                                                             const QSceneObject3D* sceneObject) const
{
    Q_UNUSED(context);
    Q_UNUSED(sceneObject);
    QLight* light0 = shader->light0();
    if (light0 != nullptr) {
        program->setUniformValue(light0_color, light0->colorVector());
        program->setUniformValue(light0_position, light0->globalPosition());
        program->setUniformValue(light0_radius, light0->radius());
        program->setUniformValue(light0_soft, light0->lightSoft());
    } else {
        program->setUniformValue(light0_radius, -1.0f);
    }
    QLight* light1 = shader->light1();
    if (light1 != nullptr) {
        program->setUniformValue(light1_color, light1->colorVector());
        program->setUniformValue(light1_position, light1->globalPosition());
        program->setUniformValue(light1_radius, light1->radius());
        program->setUniformValue(light1_soft, light1->lightSoft());
    } else {
        program->setUniformValue(light1_radius, -1.0f);
    }
}

void QSh_Light::UniformLocationOmniSpot::bindLightParameters(QScrollEngineContext* context,
                                                             QOpenGLShaderProgram* program,
                                                             const QSh_Light* shader,
                                                             const QSceneObject3D* sceneObject) const
{
    Q_UNUSED(context);
    Q_UNUSED(sceneObject);
    QLight* light0 = shader->light0();
    QSpotLight* spotLight1;
    if (light0->type() == QLight::Type::Spot) {
        spotLight1 = static_cast<QSpotLight*>(light0);
        light0 = shader->light1();
    } else {
        spotLight1 = static_cast<QSpotLight*>(shader->light1());
    }
    program->setUniformValue(light0_color, light0->colorVector());
    program->setUniformValue(light0_position, light0->globalPosition());
    program->setUniformValue(light0_radius, light0->radius());
    program->setUniformValue(light0_soft, light0->lightSoft());
    program->setUniformValue(light1_color, spotLight1->colorVector());
    program->setUniformValue(light1_position, spotLight1->globalPosition());
    program->setUniformValue(light1_radius, spotLight1->radius());
    program->setUniformValue(light1_soft, spotLight1->lightSoft());
    program->setUniformValue(light1_dir, spotLight1->directional());
    program->setUniformValue(light1_spotCutOff, spotLight1->spotCutOff());
}

void QSh_Light::UniformLocationSpotSpot::bindLightParameters(QScrollEngineContext* context,
                                                             QOpenGLShaderProgram* program,
                                                             const QSh_Light* shader,
                                                             const QSceneObject3D* sceneObject) const
{
    Q_UNUSED(context);
    Q_UNUSED(sceneObject);
    QSpotLight* spotLight0 = static_cast<QSpotLight*>(shader->light0());
    program->setUniformValue(light0_color, spotLight0->colorVector());
    program->setUniformValue(light0_position, spotLight0->globalPosition());
    program->setUniformValue(light0_radius, spotLight0->radius());
    program->setUniformValue(light0_soft, spotLight0->lightSoft());
    program->setUniformValue(light0_dir, spotLight0->directional());
    program->setUniformValue(light0_spotCutOff, spotLight0->spotCutOff());
    QSpotLight* spotLight1 = static_cast<QSpotLight*>(shader->light1());
    if (spotLight1 != nullptr) {
        program->setUniformValue(light1_color, spotLight1->colorVector());
        program->setUniformValue(light1_position, spotLight1->globalPosition());
        program->setUniformValue(light1_radius, spotLight1->radius());
        program->setUniformValue(light1_soft, spotLight1->lightSoft());
        program->setUniformValue(light1_dir, spotLight1->directional());
        program->setUniformValue(light1_spotCutOff, spotLight1->spotCutOff());
    } else {
        program->setUniformValue(light1_radius, -1.0f);
    }
}

void QSh_Light::UniformLocationSpecular::bindSpecularParameters(QOpenGLShaderProgram* program, const QSh_Light* shader,
                                                                const QScene* scene) const
{
    QSH_ASSERT(scene != nullptr);
    program->setUniformValue(specularIntensity, shader->specularIntensity());
    program->setUniformValue(specularPower, shader->specularPower());
    program->setUniformValue(viewPosition, scene->cameraPosition());
}

void QSh_Light::UniformLocationOmniOmniSpecular::bindLightParameters(QScrollEngineContext* context,
                                                                     QOpenGLShaderProgram* program,
                                                                     const QSh_Light* shader,
                                                                     const QSceneObject3D* sceneObject) const
{
    UniformLocationOmniOmni::bindLightParameters(context, program, shader, sceneObject);
    bindSpecularParameters(program, shader, sceneObject->scene());
}

void QSh_Light::UniformLocationOmniSpotSpecular::bindLightParameters(QScrollEngineContext* context,
                                                                     QOpenGLShaderProgram* program,
                                                                     const QSh_Light* shader,
                                                                     const QSceneObject3D* sceneObject) const
{
    UniformLocationOmniSpot::bindLightParameters(context, program, shader, sceneObject);
    bindSpecularParameters(program, shader, sceneObject->scene());
}

void QSh_Light::UniformLocationSpotSpotSpecular::bindLightParameters(QScrollEngineContext* context,
                                                                     QOpenGLShaderProgram* program,
                                                                     const QSh_Light* shader,
                                                                     const QSceneObject3D* sceneObject) const
{
    UniformLocationSpotSpot::bindLightParameters(context, program, shader, sceneObject);
    bindSpecularParameters(program, shader, sceneObject->scene());
}

}
