#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QSprite.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QAnimation3D.h"
#include "QScrollEngine/QCamera3D.h"
#include "QScrollEngine/QOtherMathFunctions.h"

#include <QQuaternion>


namespace QScrollEngine {

QScene::QScene():QScene(nullptr, 0) {}

QScene::QScene(QScrollEngineContext* parentContext, int order)
{
    scale.setX(1.0f); scale.setY(1.0f); scale.setZ(1.0f);
    m_enabled = true;
    m_order = order;
    m_boundingBox.toPoint(0.0f, 0.0f, 0.0f);
    m_boundingBox.expand(10000.0f);
    m_parentContext = parentContext;
    if (m_parentContext) {
        m_quad = m_parentContext->m_quad;
        m_parentContext->_addScene(this);
    } else {
        m_quad = nullptr;
        m_index = 0;
    }
    m_ambientColor.setRgb(0, 0, 0, 255);
}

QScene::~QScene()
{
    emit deleting();
    clearScene();
    if (m_parentContext != nullptr)
        m_parentContext->_deleteScene(this);
}

void QScene::clearScene()
{
    while (!m_lights.empty())
        delete m_lights.at(m_lights.size() - 1);
    while (!m_sprites.empty())
        delete m_sprites.at(m_sprites.size() - 1);
    while (!m_entities.empty())
        delete m_entities.at(m_entities.size() - 1);
}

QEntity* QScene::findEntity(const QString& name) const
{
    for (unsigned int i=0; i<m_entities.size(); ++i) {
        if (m_entities[i]->name() == name)
            return m_entities[i];
    }
    return nullptr;
}

QEntity* QScene::findEntityWithChilds(const QString& name) const
{
    for (unsigned int i=0; i<m_entities.size(); ++i) {
        if (m_entities[i]->name() == name)
            return m_entities[i];
        QEntity* child = m_entities[i]->findChild(name);
        if (child)
            return child;
    }
    return nullptr;
}

void QScene::setParentContext(QScrollEngineContext* parentContext)
{
    if (m_parentContext == parentContext)
        return;
    if (m_parentContext)
        m_parentContext->_deleteScene(this);
    m_parentContext = parentContext;
    if (m_parentContext) {
        m_quad = m_parentContext->m_quad;
        m_parentContext->_addScene(this);
    } else {
        m_quad = nullptr;
    }
    emit parentContextChanged();
}

void QScene::setOrder(int order)
{
    m_order = order;
    if (m_parentContext) {
        m_parentContext->_deleteScene(this);
        m_parentContext->_addScene(this);
    }
}

void QScene::_addSprite(QSprite* sprite)
{
    sprite->m_index = m_sprites.size();
    m_sprites.push_back(sprite);
}

void QScene::_deleteSprite(QSprite* sprite)
{
    m_sprites[sprite->m_index] = m_sprites[m_sprites.size()-1];
    m_sprites[sprite->m_index]->m_index = sprite->m_index;
    m_sprites.pop_back();
}

void QScene::_addEntity(QEntity* entity)
{
    entity->m_index = m_entities.size();
    m_entities.push_back(entity);
}

void QScene::_deleteEntity(QEntity* entity)
{
    m_entities[entity->m_index] = m_entities[m_entities.size()-1];
    m_entities[entity->m_index]->m_index = entity->m_index;
    m_entities.pop_back();
}

void QScene::_addLight(QLight* light)
{
    light->m_index = m_lights.size();
    m_lights.push_back(light);
}

void QScene::_deleteLight(QLight* light)
{
    m_lights[light->m_index] = m_lights[m_lights.size()-1];
    m_lights[light->m_index]->m_index = light->m_index;
    m_lights.pop_back();
}

void QScene::solveBoundingBox(float expand)
{
    if (m_sprites.empty()) {
        if (m_entities.empty()) {
            if (m_lights.empty()) {
                m_boundingBox.toPoint(0.0f, 0.0f, 0.0f);
                m_boundingBox.expand(expand);
                return;
            } else {
                m_lights[0]->updateTransform();
                m_boundingBox = m_lights[0]->boundingBox();
            }
        } else {
            m_entities[0]->updateTransform();
            m_boundingBox = m_entities[0]->boundingBox();
            m_boundingBox.expand(expand);
        }
    } else {
        m_boundingBox.toPoint(m_sprites[0]->position());
        m_boundingBox.expand(m_sprites[0]->m_scale);
    }
    for (std::vector<QLight*>::iterator it = m_lights.begin(); it != m_lights.end(); ++it) {
        (*it)->updateTransform();
        m_boundingBox.merge((*it)->boundingBox());
    }
    QBoundingBox temp;
    QSprite* sprite;
    for (std::vector<QSprite*>::iterator it = m_sprites.begin(); it != m_sprites.end(); ++it) {
        sprite = *it;
        temp.toPoint(sprite->position());
        temp.expand(sprite->m_scale);
        m_boundingBox.merge(temp);
    }
    QEntity* entity;
    for (std::vector<QEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
        entity = *it;
        entity->updateTransform();
        m_boundingBox.merge(entity->boundingBox());
    }
    m_boundingBox.expand(expand);
}

void QScene::updateCameraInfo(QCamera3D* camera)
{
    camera->setScenePosition(position);
    camera->setSceneOrientation(orientation);
    camera->setSceneScale(scale);
    camera->update();
    m_cameraInfo.position = camera->localPosition();
    m_cameraInfo.orientation = camera->localOrientation();
#if defined(__ANDROID__)
    QQuaternion invOrientation = m_cameraInfo.orientation;
#else
    QQuaternion invOrientation = m_cameraInfo.orientation;
#endif
    m_cameraInfo.localX = invOrientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
    m_cameraInfo.localY = invOrientation.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f));
    m_cameraInfo.localZ = invOrientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
}

void QScene::_updateGlobalPosition(QSprite* sprite){
    if (sprite->m_transformHasChanged) {
        sprite->m_globalPosition = sprite->m_position;
        sprite->m_transformHasChanged = false;
    }
}

void QScene::_spriteToDrawing(QSprite* sprite, const QCamera3D* camera)
{
    emit sprite->onSceneUpdate();
    if (!sprite->visibled())
        return;
    float sina = sinf(sprite->m_angle);
    float cosa = cosf(sprite->m_angle);
    QVector2D diagonal = sprite->m_scale * 0.5, d, dr;
    diagonal.setX(fabsf(diagonal.x()));
    diagonal.setY(fabsf(diagonal.y()));
    d.setX(qAbs(diagonal.x() * cosa - diagonal.y() * sina));
    d.setY(qAbs(diagonal.x() * sina + diagonal.y() * cosa));
    dr.setX(qAbs(diagonal.x() * cosa + diagonal.y() * sina));
    dr.setY(qAbs(diagonal.x() * sina - diagonal.y() * cosa));
    if (dr.x() > d.x())
        d.setX(dr.x());
    if (dr.y() > d.y())
        d.setY(dr.y());
    QVector3D center = m_cameraInfo.localX * d.x() + m_cameraInfo.localY * d.y();
    sprite->m_centerOfBoundingBox = sprite->m_globalPosition;
    sprite->m_boundingBox.toPoint(sprite->m_centerOfBoundingBox);
    sprite->m_boundingBox.expand(center);
    if (!camera->frustum.boxInFrustum(sprite->m_centerOfBoundingBox, center)) {
        sprite->m_visibledForCamera = false;
        return;
    }
    sprite->m_visibledForCamera = true;
    QQuaternion globalOrientation = m_cameraInfo.orientation * QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, sprite->m_angle);
    globalOrientation.normalize();
    sprite->_updateMatrixWorld(globalOrientation);
    sprite->updateMatrxWorldViewProj(camera->matrixViewProj());
    if (sprite->isAlpha()) {
        QScrollEngineContext::TempAlphaObject temp;
        temp.drawObject = sprite;
        temp.zDistance = QVector3D::dotProduct(m_cameraInfo.position - sprite->m_centerOfBoundingBox, m_cameraInfo.localZ);
        m_parentContext->_addTempAlphaObject(temp);
    } else {
        QSh* shaderOfObject = sprite->shader().data();
        if (shaderOfObject) {
            shaderOfObject->preprocess(sprite);
            QScrollEngineContext::_Drawing& drawing = m_parentContext->m_drawings[shaderOfObject->currentTypeIndex()];
            drawing.currentObjects[shaderOfObject->subTypeIndex()].sprites.push_back(sprite);
        }
    }
}

void QScene::_entityToDrawing(QEntity* entity, const QCamera3D* camera)
{
    bool change;// = false;
    entity->_prevStepDrawing_updateTransform(change, camera);
    entity->_postStepDrawing(camera);
}

void QScene::_update()
{
    QCamera3D* camera = m_parentContext->camera;
    updateCameraInfo(camera);
    if (!camera->frustum.boundingBoxInFrustum(m_boundingBox)) {
        return;
    }
    for (auto it = m_lights.begin(); it != m_lights.end(); ++it) {
        emit (*it)->onSceneUpdate();
        (*it)->updateTransform();
    }
    for (auto it = m_sprites.begin(); it != m_sprites.end(); ++it) {
        _updateGlobalPosition(*it);
        _spriteToDrawing(*it, camera);
    }
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        _entityToDrawing(*it, camera);
    }
}

}
