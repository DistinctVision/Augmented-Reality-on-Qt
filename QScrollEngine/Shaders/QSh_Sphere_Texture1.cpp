#include "QScrollEngine/Shaders/QSh_Sphere_Texture1.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"

namespace QScrollEngine {

int QSh_Sphere_Texture1::locationMatrixWVP;
int QSh_Sphere_Texture1::locationMatrixW;
int QSh_Sphere_Texture1::locationColor;
int QSh_Sphere_Texture1::locationTexture0;
int QSh_Sphere_Texture1::locationViewPosition;
int QSh_Sphere_Texture1::locationSphereCenter;
int QSh_Sphere_Texture1::locationSphereRadiusSquared;
int QSh_Sphere_Texture1::locationSphereMatrixRotation;

bool QSh_Sphere_Texture1::use(QScrollEngineContext* context, QOpenGLShaderProgram* program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    QScene* scene = _sceneObject->scene();
    QSH_ASSERT(scene != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(locationMatrixW, _sceneObject->matrixWorld());
    program->setUniformValue(locationColor, _color);
    program->setUniformValue(locationTexture0, 0);
    context->glActiveTexture(GL_TEXTURE0);
    context->glBindTexture(GL_TEXTURE_2D, (_texture0) ? _texture0->textureId() : context->emptyTexture()->textureId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    program->setUniformValue(locationViewPosition, scene->cameraPosition());
    program->setUniformValue(locationSphereCenter, _sceneObject->globalPosition());
    program->setUniformValue(locationSphereMatrixRotation, _sphereMatrixRotation);
    program->setUniformValue(locationSphereRadiusSquared, _sphereRadius * _sphereRadius);
    return true;
}

void QSh_Sphere_Texture1::load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    if (!context->loadShader(shaders[0].data(), "Sphere_", "Sphere_Texture1"))
        return;
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    if (!context->checkBindShader(shaders[0].data(), "Sphere Texture1"))
        return;
    QSh_Sphere_Texture1::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    QSh_Sphere_Texture1::locationMatrixW = shaders[0]->uniformLocation("matrix_w");
    QSh_Sphere_Texture1::locationColor = shaders[0]->uniformLocation("color");
    QSh_Sphere_Texture1::locationTexture0 = shaders[0]->uniformLocation("texture0");
    QSh_Sphere_Texture1::locationViewPosition = shaders[0]->uniformLocation("viewPosition");
    QSh_Sphere_Texture1::locationSphereCenter = shaders[0]->uniformLocation("sphereCenter");
    QSh_Sphere_Texture1::locationSphereRadiusSquared = shaders[0]->uniformLocation("sphereRadiusSquared");
    QSh_Sphere_Texture1::locationSphereMatrixRotation = shaders[0]->uniformLocation("sphereMatrixRotation");
}

}
