#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QSprite.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QAnimation3D.h"
#include "QScrollEngine/QCamera3D.h"
#include <cassert>

namespace QScrollEngine {

QEntity::Part::~Part()
{
    if (m_mesh) {
        if (m_sceneObject) {
            m_sceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        if (m_mesh->m_countUsedParts <= 1) {
            m_mesh->m_countUsedParts = 0;
            delete m_mesh;
        } else {
            --m_mesh->m_countUsedParts;
        }
    }
}

void QEntity::Part::setMesh(QMesh* mesh)
{
    if (m_mesh) {
        if (m_sceneObject) {
            m_sceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        if (m_mesh->countUsedParts() <= 1)
            delete m_mesh;
        else
            --m_mesh->m_countUsedParts;
    }
    m_mesh = mesh;
    if (m_mesh) {
        ++m_mesh->m_countUsedParts;
        if (m_sceneObject) {
            m_sceneObject->setChangedTransform();
            _connectMeshToSceneObject();
        }
    }
}

void QEntity::Part::_connectMeshToSceneObject()
{
    m_sceneObject->connect(m_mesh, &QMesh::changedLocalBoundingBox,
                                m_sceneObject, &QSceneObject3D::setChangedTransform, Qt::DirectConnection);
}

void QEntity::Part::_disconnectMeshFromSceneObject()
{
    m_sceneObject->disconnect(m_mesh, &QMesh::changedLocalBoundingBox,
                                   m_sceneObject, &QSceneObject3D::setChangedTransform);
}

void QEntity::Part::_setSceneObject(QSceneObject3D* sceneObject)
{
    if (m_mesh) {
        if (m_sceneObject) {
            m_sceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        m_sceneObject = sceneObject;
        if (m_sceneObject) {
            m_sceneObject->setChangedTransform();
            _connectMeshToSceneObject();
        }
    } else {
        m_sceneObject = sceneObject;
    }
}

QEntity::QEntity(QScene* scene):QSceneObject3D(), QDrawObject3D(this)
{
    m_animation = nullptr;
    m_name.clear();
    m_parentEntity = nullptr;
    //_parentSceneObject = this;
    m_scale = QVector3D(1.0f, 1.0f, 1.0f);
    m_transformHasChanged = true;
    m_scene = scene;
    if (m_scene)
        m_scene->_addEntity(this);
}

QEntity::QEntity(QEntity* parentEntity):QSceneObject3D(), QDrawObject3D(this)
{
    m_animation = nullptr;
    m_name.clear();
    //_parentSceneObject = this;
    m_scale = QVector3D(1.0f, 1.0f, 1.0f);
    m_transformHasChanged = true;
    m_scene = nullptr;
    m_parentEntity = nullptr;
    if (parentEntity)
        parentEntity->addChild(this);
    else
        m_scene = nullptr;
}

void QEntity::_freePart(std::size_t i)
{
    std::vector<QEntity::Part*>::iterator it = m_parts.begin() + i;
    delete *it;
}

QEntity::~QEntity()
{
    if (m_animation) {
        if (m_animation->countUsedEntities() <= 1)
            delete m_animation;
        else
            --m_animation->m_countUsedEntities;
    }
    deleteParts();
    while (!m_childLights.empty())
        delete m_childLights.at(m_childLights.size() - 1);
    while (!m_childSprites.empty())
        delete m_childSprites.at(m_childSprites.size() - 1);
    while (!m_childEntities.empty())
        delete m_childEntities.at(m_childEntities.size() - 1);
    if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
    else if (m_scene)
        m_scene->_deleteEntity(this);
}

void QEntity::convertShaders(const std::function<QSh*(QSh*)>& convertFunction)
{
    for (std::vector<QSprite*>::iterator it = m_childSprites.begin(); it != m_childSprites.end(); ++it) {
        QSprite* sprite = *it;
        QSh* oldShader = sprite->shader().data();
        QSh* newShader = convertFunction(oldShader);
        if (newShader != oldShader)
            sprite->setShader(QShPtr());
    }
    for (std::vector<Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        Part* part = *it;
        QSh* oldShader = part->shader().data();
        QSh* newShader = convertFunction(oldShader);
        if (newShader != oldShader)
            part->setShader(QShPtr());
    }
    for (std::vector<QEntity*>::iterator it = m_childEntities.begin(); it != m_childEntities.end(); ++it) {
        QEntity* entity = *it;
        entity->convertShaders(convertFunction);
    }
}

bool QEntity::haveAnimation() const
{
    if (m_animation)
        return true;
    for (std::size_t i=0; i<m_childEntities.size(); ++i)
        if (m_childEntities[i]->haveAnimation())
            return true;
    return false;
}

void QEntity::setAnimation(QAnimation3D* animation)
{
    if (m_animation) {
        if (m_animation->countUsedEntities() <= 1)
            delete m_animation;
        else
            --m_animation->m_countUsedEntities;
    }
    m_animation = animation;
    if (m_animation) {
        ++m_animation->m_countUsedEntities;
    }
}

void QEntity::swapAnimation(QEntity* entity)
{
    std::swap(m_animation, entity->m_animation);
}

void QEntity::setToAnimation()
{
    if (m_animation && m_animation->enable()) {
        m_animation->entityToAnimation(this);
    }
}

void QEntity::_updateAnimation()
{
    if (m_animation && m_animation->enable()) {
        m_animation->updateFrame();
        m_animation->entityToAnimation(this);
    }
}

void QEntity::setAnimationTime(float time)
{
    if (m_animation)
        m_animation->setAnimationTime(time);
    for (std::size_t i=0; i<m_childEntities.size(); ++i)
        m_childEntities[i]->setAnimationTime(time);
}

void QEntity::setAnimationSpeed(float speed)
{
    if (m_animation)
        m_animation->setAnimationSpeed(speed);
    for (std::size_t i=0; i<m_childEntities.size(); ++i)
        m_childEntities[i]->setAnimationSpeed(speed);
}

void QEntity::setAnimationParameters(float time, float speed)
{
    if (m_animation) {
        m_animation->setAnimationTime(time);
        m_animation->setAnimationSpeed(speed);
    }
    for (std::size_t i=0; i<m_childEntities.size(); ++i) {
        m_childEntities[i]->setAnimationTime(time);
        m_childEntities[i]->setAnimationSpeed(speed);
    }
}

void QEntity::_updateScene(QScene* scene)
{
    if (m_scene == scene)
        return;
    std::size_t i;
    if ((m_scene) && (scene)) {
        if (scene->parentContext()) {
            QScrollEngineContext* context = scene->parentContext();
            for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
                QMesh* mesh = (*it)->mesh();
                if (mesh)
                    mesh->setParentContext(context);
            }
        }
        for (i=0; i<m_childLights.size(); ++i) {
            QLight* light = m_childLights.at(i);
            m_scene->_deleteLight(light);
            light->m_scene = scene;
            scene->_addLight(light);
        }
    } else if (m_scene) {
        for (i=0; i<m_childLights.size(); ++i) {
            QLight* light = m_childLights.at(i);
            m_scene->_deleteLight(light);
            light->m_scene = nullptr;
        }
    } else /*if (scene)*/ {
        if (scene->parentContext()) {
            QScrollEngineContext* context = scene->parentContext();
            for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
                QMesh* mesh = (*it)->mesh();
                if (mesh)
                    mesh->setParentContext(context);
            }
        }
        for (i=0; i<m_childLights.size(); ++i) {
            QLight* light = m_childLights.at(i);
            light->m_scene = scene;
            scene->_addLight(light);
        }
    }
    for (i=0; i<m_childEntities.size(); ++i)
        m_childEntities.at(i)->_updateScene(scene);
    for (i=0; i<m_childSprites.size(); ++i)
        m_childSprites.at(i)->m_scene = scene;
    m_scene = scene;
}

void QEntity::addChild(QEntity* entity)
{
    if (entity->m_parentEntity) {
        entity->m_parentEntity->_deleteChild(entity);
    } else if (entity->m_scene) {
        entity->m_scene->_deleteEntity(entity);
    }
    entity->m_parentEntity = this;
    entity->m_index = m_childEntities.size();
    m_childEntities.push_back(entity);
    entity->_updateScene(m_scene);
    m_transformHasChanged = true;
}

void QEntity::_deleteChild(QEntity* entity)
{
    m_childEntities[entity->m_index] = m_childEntities[m_childEntities.size()-1];
    m_childEntities[entity->m_index]->m_index = entity->m_index;
    m_childEntities.pop_back();
    entity->m_parentEntity = nullptr;
    entity->_updateScene(nullptr);
    entity->m_transformHasChanged = true;
}

void QEntity::addChild(QSprite* sprite)
{
    if (sprite->m_parentEntity) {
        sprite->m_parentEntity->_deleteChild(sprite);
    } else if (sprite->m_scene) {
        sprite->m_scene->_deleteSprite(sprite);
    }
    sprite->m_parentEntity = this;
    sprite->m_index = m_childSprites.size();
    m_childSprites.push_back(sprite);
    sprite->m_scene = m_scene;
}

void QEntity::_deleteChild(QSprite* sprite)
{
    m_childSprites[sprite->m_index] = m_childSprites[m_childSprites.size()-1];
    m_childSprites[sprite->m_index]->m_index = sprite->m_index;
    m_childSprites.pop_back();
    sprite->m_parentEntity = nullptr;
    sprite->m_scene = nullptr;
}

void QEntity::addChild(QLight* light)
{
    if (light->m_parentEntity) {
        light->m_parentEntity->_deleteChild(light);
        if (light->m_scene)
            light->m_scene->_addLight(light);
    } else if (light->m_scene)
        light->m_scene->_deleteLight(light);
    light->m_parentEntity = this;
    light->m_childIndex = m_childLights.size();
    m_childLights.push_back(light);
    light->m_scene = m_scene;
    if (m_scene)
        m_scene->_addLight(light);
}

void QEntity::_deleteChild(QLight* light)
{
    m_childLights[light->m_childIndex] = m_childLights[m_childLights.size()-1];
    m_childLights[light->m_childIndex]->m_childIndex = light->m_childIndex;
    m_childLights.pop_back();
    light->m_parentEntity = nullptr;
    if (light->m_scene)
        light->m_scene->_deleteLight(light);
    light->m_scene = nullptr;
}

bool QEntity::addChild_saveTransform(QSprite* sprite)
{
    updateTransform();
    sprite->updateTransform();
    if (sprite->m_parentEntity) {
        sprite->m_parentEntity->_deleteChild(sprite);
    } else if (sprite->m_scene) {
        sprite->m_scene->_deleteSprite(sprite);
    }
    sprite->m_parentEntity = this;
    sprite->m_index = m_childSprites.size();
    m_childSprites.push_back(sprite);
    sprite->m_scene = m_scene;
    QVector3D globalScale;
    QMatrix3x3 globalMatrixRot;
    if (getGlobalParameters(globalScale, globalMatrixRot)) {
        sprite->m_position = sprite->m_globalPosition - m_globalPosition;
        sprite->m_position.setX(sprite->m_position.x() / globalScale.x());
        sprite->m_position.setY(sprite->m_position.y() / globalScale.y());
        sprite->m_position.setZ(sprite->m_position.z() / globalScale.z());
        sprite->m_position = QOtherMathFunctions::transformTransposed(globalMatrixRot, sprite->m_position);
        return true;
    }
    return false;
}

bool QEntity::addChild_saveTransform(QLight* light)
{
    updateTransform();
    light->updateTransform();
    if (light->m_parentEntity) {
        light->m_parentEntity->_deleteChild(light);
    } else if (light->m_scene) {
        light->m_scene->_deleteLight(light);
    }
    light->m_parentEntity = this;
    light->m_index = m_childLights.size();
    m_childLights.push_back(light);
    m_scene->_addLight(light);
    light->m_transformHasChanged = false;
    QVector3D globalScale;
    QMatrix3x3 globalMatrixRot;
    if (getGlobalParameters(globalScale, globalMatrixRot)) {
        light->m_position = light->m_globalPosition - m_globalPosition;
        light->m_position.setX(light->m_position.x() / globalScale.x());
        light->m_position.setY(light->m_position.y() / globalScale.y());
        light->m_position.setZ(light->m_position.z() / globalScale.z());
        light->m_position = QOtherMathFunctions::transformTransposed(globalMatrixRot, light->m_position);
        return true;
    }
    return false;
}

bool QEntity::addChild_saveTransform(QEntity* entity)
{
    updateTransform();
    entity->updateTransform();
    if (entity->m_parentEntity) {
        entity->m_parentEntity->_deleteChild(entity);
    } else if (entity->m_scene) {
        entity->m_scene->_deleteEntity(entity);
    }
    entity->m_parentEntity = this;
    entity->m_index = m_childEntities.size();
    m_childEntities.push_back(entity);
    entity->_updateScene(m_scene);
    entity->m_transformHasChanged = true;
    QVector3D globalScale, entityGlobalScale;
    QQuaternion globalOrientation, entityGlobalOrientation;
    if (getGlobalParameters(globalScale, globalOrientation) &&
            entity->getGlobalParameters(entityGlobalScale, entityGlobalOrientation)) {
        entity->m_position = entity->m_globalPosition - m_globalPosition;
        entity->m_position.setX(entity->m_position.x() / globalScale.x());
        entity->m_position.setY(entity->m_position.y() / globalScale.y());
        entity->m_position.setZ(entity->m_position.z() / globalScale.z());
        entity->m_position = (globalOrientation.conjugate()).rotatedVector(entity->m_position);
        entity->m_orientation = globalOrientation * entityGlobalOrientation;
        entity->m_scale.setX(entityGlobalScale.x() / globalScale.x());
        entity->m_scale.setY(entityGlobalScale.y() / globalScale.y());
        entity->m_scale.setZ(entityGlobalScale.z() / globalScale.z());
        return true;
    }
    return false;
}

QEntity* QEntity::findChild(const QString& name)
{
    for (std::size_t i=0; i<m_childEntities.size(); ++i) {
        if (m_childEntities[i]->name() == name) {
            return m_childEntities[i];
        } else {
            QEntity* result = m_childEntities[i]->findChild(name);
            if (result)
                return result;
        }
    }
    return nullptr;
}

void QEntity::setParentEntity(QEntity* entity)
{
    if (entity)
        entity->addChild(this);
    else if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
}

bool QEntity::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (m_parentEntity) {
        m_parentEntity->_deleteChild(this);
        updateTransform();
        m_position = m_globalPosition;
        QVector3D globalScale;
        QQuaternion globalOrientation;
        if (getGlobalParameters(globalScale, globalOrientation)) {
            m_orientation = globalOrientation;
            m_scale = globalScale;
            return true;
        }
        return false;
    }
    return true;
}

void QEntity::setParentScene(QScene* scene)
{
    if (m_parentEntity) {
        m_parentEntity->_deleteChild(this);
        m_parentEntity = nullptr;
    } else if (m_scene)
        m_scene->_deleteEntity(this);
    _updateScene(scene);
    if (m_scene)
        m_scene->_addEntity(this);
}

bool QEntity::setParentScene_saveTransform(QScene* scene)
{
    bool flag = true;
    if (m_parentEntity) {
        updateTransform();
        m_parentEntity->_deleteChild(this);
        m_parentEntity = nullptr;
        m_position = m_globalPosition;
        QQuaternion globalOrientation;
        QVector3D globalScale;
        if (getGlobalParameters(globalScale, globalOrientation)) {
            m_orientation = globalOrientation;
            m_scale = globalScale;
        } else
            flag = false;
    } else if (m_scene)
        m_scene->_deleteEntity(this);
    _updateScene(scene);
    if (m_scene)
        m_scene->_addEntity(this);
    return flag;
}

bool QEntity::transformHasChanged() const
{
    if (m_transformHasChanged)
        return true;
    if (m_parentEntity)
        return m_parentEntity->transformHasChanged();
    return false;
}

void QEntity::_updateTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (m_transformHasChanged) {
        _updateLocalMatrixWorld();
        _updateMatrixWorld(parentMatrixWorld);
        m_transformHasChanged = false;
    }
}

void QEntity::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (m_transformHasChanged) {
        _updateLocalMatrixWorld();
        m_transformHasChanged = false;
    }
    _updateMatrixWorld(parentMatrixWorld);
}

void QEntity::_solveTransformChilds()
{
    std::size_t i;
    for (i=0; i<m_childLights.size(); ++i)
        m_childLights[i]->_solveTransformFromParent(m_matrix.world);
    for (i=0; i<m_childSprites.size(); ++i)
        m_childSprites[i]->_solveTransformFromParent(m_matrix.world);
    i = 0;
    if (m_boundingBox.isDeactivated()) {
        for (; i<m_childEntities.size(); ++i) {
            m_childEntities[i]->_solveTransformFromParent(m_matrix.world);
            m_childEntities[i]->_solveTransformChilds();
            if (m_childEntities[i]->m_boundingBox.isActivated()) {
                m_boundingBox = m_childEntities[i]->m_boundingBox;
                break;
            }
        }
    }
    for (; i<m_childEntities.size(); ++i) {
        m_childEntities[i]->_solveTransformFromParent(m_matrix.world);
        m_childEntities[i]->_solveTransformChilds();
        if (m_childEntities[i]->m_boundingBox.isActivated())
            m_boundingBox.merge(m_childEntities[i]->m_boundingBox);
    }
    m_centerOfBoundingBox = m_boundingBox.getCenter();
}

void QEntity::_updateTransformChilds(bool& change)
{
    std::size_t i;
    for (i=0; i<m_childLights.size(); ++i)
        m_childLights[i]->_updateTransformFromParent(m_matrix.world);
    for (i=0; i<m_childSprites.size(); ++i)
        m_childSprites[i]->_solveTransformFromParent(m_matrix.world);
    for (i=0; i<m_childEntities.size(); ++i) {
        if (m_childEntities[i]->m_transformHasChanged) {
            change = true;
            m_childEntities[i]->_updateLocalMatrixWorld();
            m_childEntities[i]->_updateMatrixWorld(m_matrix.world);
            m_childEntities[i]->m_transformHasChanged = false;
            m_childEntities[i]->_solveTransformChilds();
        } else
            m_childEntities[i]->_updateTransformChilds(change);
    }
}

void QEntity::_updateTransformChilds()
{
    std::size_t i;
    for (i=0; i<m_childLights.size(); ++i)
        m_childLights[i]->_updateTransformFromParent(m_matrix.world);
    for (i=0; i<m_childSprites.size(); ++i)
        m_childSprites[i]->_solveTransformFromParent(m_matrix.world);
    for (i=0; i<m_childEntities.size(); ++i) {
        if (m_childEntities[i]->m_transformHasChanged) {
            m_childEntities[i]->_updateLocalMatrixWorld();
            m_childEntities[i]->_updateMatrixWorld(m_matrix.world);
            m_childEntities[i]->m_transformHasChanged = false;
            m_childEntities[i]->_solveTransformChilds();
        } else
            m_childEntities[i]->_updateTransformChilds();
    }
}

void QEntity::updateTransform()
{
    bool change;
    updateTransform(change);
}

void QEntity::updateTransform(bool& change)
{
    if (m_parentEntity) {
        m_parentEntity->updateTransform(change);
    } else {
        if (m_transformHasChanged) {
            change = true;
            _updateLocalMatrixWorld();
            m_transformHasChanged = false;
            _updateMatrixWorld();
            _solveTransformChilds();
        } else {
            change = false;
            _updateTransformChilds(change);
            if (change) {
                _updateBoundingBox();
                _mergeChildsBoundingBoxes();
            }
        }
    }
}

void QEntity::_prevStepDrawing_solveTransformChilds(const QCamera3D* camera)
{
    std::size_t i;
    for (i=0; i<m_childLights.size(); ++i) {
        m_childLights[i]->_solveTransformFromParent(m_matrix.world);
    }
    for (i=0; i<m_childSprites.size(); ++i) {
        m_childSprites[i]->_solveTransformFromParent(m_matrix.world);
        m_scene->_spriteToDrawing(m_childSprites[i], camera);
    }
    QEntity* entity;
    i = 0;
    if (m_boundingBox.isDeactivated()) {
        for (; i<m_childEntities.size(); ++i) {
            entity = m_childEntities[i];
            entity->_updateAnimation();
            entity->_solveTransformFromParent(m_matrix.world);
            entity->_prevStepDrawing_solveTransformChilds(camera);
            entity->m_centerOfBoundingBox = entity->m_boundingBox.getCenter();
            if (entity->m_boundingBox.isActivated()) {
                m_boundingBox = entity->m_boundingBox;
                break;
            }
        }
    }
    for (; i<m_childEntities.size(); ++i) {
        entity = m_childEntities[i];
        entity->_updateAnimation();
        entity->_solveTransformFromParent(m_matrix.world);
        entity->_prevStepDrawing_solveTransformChilds(camera);
        entity->m_centerOfBoundingBox = entity->m_boundingBox.getCenter();
        if (entity->m_boundingBox.isActivated())
            m_boundingBox.merge(entity->m_boundingBox);
    }
}

void QEntity::_prevStepDrawing_solveTransform(const QCamera3D* camera)
{
    emit onSceneUpdate();
    _updateLocalMatrixWorld();
    if (m_parentEntity)
        _updateMatrixWorld(m_parentEntity->m_matrix.world);
    else
        _updateMatrixWorld();
    m_transformHasChanged = false;
    _prevStepDrawing_solveTransformChilds(camera);
    m_centerOfBoundingBox = m_boundingBox.getCenter();
}

void QEntity::_prevStepDrawing_updateTransform(bool& change, const QCamera3D* camera)
{
    if ((m_animation) && (m_animation->enable())) {
        m_animation->updateFrame();
        m_animation->entityToAnimation(this);
        _prevStepDrawing_solveTransform(camera);
        change = true;
    } else if (m_transformHasChanged) {
        _prevStepDrawing_solveTransform(camera);
        change = true;
    } else {
        emit onSceneUpdate();
        std::size_t i;
        for (i = 0; i < m_childLights.size(); ++i) {
            m_childLights[i]->_updateTransformFromParent(m_matrix.world);
        }
        for (i = 0; i < m_childSprites.size(); ++i) {
            m_childSprites[i]->_solveTransformFromParent(m_matrix.world);
            m_scene->_spriteToDrawing(m_childSprites[i], camera);
        }
        for (i = 0; i < m_childEntities.size(); ++i) {
            m_childEntities[i]->_prevStepDrawing_updateTransform(m_transformHasChanged, camera);
        }
        if (m_transformHasChanged) {
            _updateBoundingBox();
            _mergeChildsBoundingBoxes();
            change = true;
            m_transformHasChanged = false;
        }
    }
}

void QEntity::_postStepDrawing(const QCamera3D* camera)
{
    if (!m_visibled) {
        m_visibledForCamera = false;
        return;
    }
    if (!camera->frustum.boxInFrustum(m_centerOfBoundingBox, m_boundingBox.max() - m_centerOfBoundingBox)) {
        m_visibledForCamera = false;
        return;
    }
    m_visibledForCamera = true;
    updateMatrxWorldViewProj(camera->matrixViewProj());
    QEntity::Part* part;
    for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        part = *it;
        if (!part->m_visibled) {
            part->m_visibledForCamera = false;
            continue;
        }
        if (camera->frustum.boxInFrustum(part->m_centerOfBoundingBox,
                                         part->m_boundingBox.max() - part->m_centerOfBoundingBox)) {
            part->m_visibledForCamera = true;
            if (part->isAlpha()) {
                QScrollEngineContext::TempAlphaObject temp;
                temp.drawObject = part;
                temp.zDistance = QVector3D::dotProduct(m_scene->m_cameraInfo.position, m_scene->m_cameraInfo.localZ) -
                                                       part->m_boundingBox.supportValue(m_scene->m_cameraInfo.localZ);
                m_scene->m_parentContext->_addTempAlphaObject(temp);
            } else {
                QSh* shaderOfObject = part->shader().data();
                if (shaderOfObject) {
                    shaderOfObject->preprocess(part);
                    QScrollEngineContext::_Drawing& drawing =
                            m_scene->m_parentContext->m_drawings[shaderOfObject->currentTypeIndex()];
                    drawing.currentObjects[shaderOfObject->subTypeIndex()].partEntities.push_back(part);
                }
            }
        } else {
            part->m_visibledForCamera = false;
        }
    }
    for (std::size_t i=0; i<m_childEntities.size(); ++i)
        m_childEntities[i]->_postStepDrawing(camera);
}


QVector3D QEntity::transformPoint(const QVector3D& point) const
{
    return transformDir(point) + m_position;
}

QVector3D QEntity::transformDir(const QVector3D& dir) const
{
    return m_orientation.rotatedVector(QVector3D(dir.x() * m_scale.x(), dir.y() * m_scale.y(), dir.z() * m_scale.z()));
}

QVector3D QEntity::inverseTransformPoint(const QVector3D& point) const
{
    return inverseTransformDir(point - m_position);
}

QVector3D QEntity::inverseTransformDir(const QVector3D& dir) const
{
    QVector3D result = m_orientation.conjugate().rotatedVector(dir);
    return QVector3D(result.x() / m_scale.x(), result.y() / m_scale.y(), result.z() / m_scale.z());
}

void QEntity::setAlpha(bool enable)
{
    for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        (*it)->setAlpha(enable);
    }
}

QEntity::Part* QEntity::addPart(QMesh* mesh, QShPtr shader, bool clone)
{
    Part* part;
    if (clone)
        part = new Part(mesh, shader);
    else
        part = new Part(new QMesh(mesh), shader->copy());
    part->_setSceneObject(this);
    m_parts.push_back(part);
    m_transformHasChanged = true;
    return m_parts.at(m_parts.size() - 1);
}

QEntity::Part* QEntity::addPart(QScrollEngineContext* parentContext,
                                       QGLPrimitiv::Primitives meshFlag,
                                       QShPtr shader)
{
    assert(parentContext != nullptr);
    QMesh* mesh = nullptr;
    switch(meshFlag) {
    case QGLPrimitiv::Primitives::None:
        break;
    case QGLPrimitiv::Primitives::Quad:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createQuad(mesh);
        break;
    case QGLPrimitiv::Primitives::Cube:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createCube(mesh);
        break;
    case QGLPrimitiv::Primitives::Sphere:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createSphere(mesh);
        break;
    case QGLPrimitiv::Primitives::Cylinder:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createCylinder(mesh);
        break;
    }
    if (mesh) {
        m_transformHasChanged = true;
        Part* part = new Part(mesh, shader);
        part->_setSceneObject(this);
        m_parts.push_back(part);
        return m_parts.at(m_parts.size() - 1);
    }
    return nullptr;
}

void QEntity::addPart(Part* part, bool clone)
{
    Part* newPart;
    if (clone)
        newPart = new Part(part->m_mesh, part->shader());
    else
        newPart = new Part(new QMesh(part->m_mesh), part->shader()->copy());
    newPart->_setSceneObject(this);
    newPart->m_isAlpha = part->m_isAlpha;
    newPart->m_drawMode = part->m_drawMode;
    m_parts.push_back(newPart);
    m_transformHasChanged = true;
}

QEntity* QEntity::copy() const
{//TODO light sources
    QEntity* entity;
    if (m_parentEntity)
        entity = new QEntity(m_parentEntity);
    else
        entity = new QEntity(m_scene);
    entity->m_name = m_name;
    std::size_t i;
    for (i=0; i<m_parts.size(); i++)
        entity->addPart(part(i), false);
    entity->m_position = m_position;
    entity->m_orientation = m_orientation;
    entity->m_scale = m_scale;
    entity->m_boundingBox = m_boundingBox;
    entity->m_transformHasChanged = m_transformHasChanged;
    entity->m_visibled = m_visibled;
    for (i=0; i<m_childEntities.size(); ++i)
        entity->addChild(m_childEntities[i]->copy());
    for (i=0; i<m_childSprites.size(); ++i)
        entity->addChild(m_childSprites[i]->copy());
    return entity;
}

QEntity* QEntity::clone() const
{//TODO and here
    QEntity* entity;
    if (m_parentEntity != nullptr)
        entity = new QEntity(m_parentEntity);
    else
        entity = new QEntity(m_scene);
    entity->m_name = m_name;
    std::size_t i;
    for (i=0; i<m_parts.size(); i++)
        entity->addPart(part(i), true);
    entity->m_position = m_position;
    entity->m_orientation = m_orientation;
    entity->m_scale = m_scale;
    entity->m_boundingBox = m_boundingBox;
    entity->m_transformHasChanged = m_transformHasChanged;
    entity->m_visibled = m_visibled;
    for (i=0; i<m_childEntities.size(); ++i)
        entity->addChild(m_childEntities[i]->clone());
    for (i=0; i<m_childSprites.size(); ++i)
        entity->addChild(m_childSprites[i]->copy());
    return entity;
}

void QEntity::deletePart(std::size_t i)
{
    delete m_parts[i];
    m_parts.erase(m_parts.begin() + i);
    //_transformHasChanged = true;
}

void QEntity::deleteParts()
{
    for (std::size_t i=0; i<m_parts.size(); ++i)
        delete m_parts[i];
    m_parts.clear();
    //_transformHasChanged = true;
}

void QEntity::foreach_parts(const std::function<void (QEntity::Part *)>& functor)
{
    for (std::vector<Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        functor(*it);
    for (std::vector<QEntity*>::iterator it = m_childEntities.begin(); it != m_childEntities.end(); ++it)
        (*it)->foreach_parts(functor);
}

void QEntity::setShaderToParts(QShPtr shader)
{
   for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
       (*it)->setShader(shader);
}

void QEntity::setShaderToChildSprites(QShPtr shader)
{
   for (std::size_t i=0; i<m_childSprites.size(); ++i)
       m_childSprites[i]->setShader(shader);
}

void QEntity::setShaderToChildEntities(QShPtr shader)
{
   for (std::size_t i=0; i<m_childEntities.size(); ++i) {
       m_childEntities[i]->setShaderToParts(shader);
       m_childEntities[i]->setShaderToChildEntities(shader);
   }
}

void QEntity::setShaderToAll(QShPtr shader)
{
    for (std::vector<QEntity::Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it)
        (*it)->setShader(shader);
    std::size_t i;
    for (i=0; i<m_childSprites.size(); ++i)
        m_childSprites[i]->setShader(shader);
    for (i=0; i<m_childEntities.size(); ++i)
        m_childEntities[i]->setShaderToAll(shader);
}

void QEntity::setShaderAtPart(std::size_t i, QShPtr shader)
{
    m_parts[i]->setShader(shader);
}

QShPtr QEntity::shader(std::size_t i) const
{
    return m_parts[i]->shader();
}

void QEntity::_updateBoundingBox()
{
    if (m_parts.empty()) {
        m_boundingBox.deactivate();
    } else {
        std::vector<Part*>::iterator it = m_parts.begin();
        Part* part = *it;
        while (part->mesh()->m_localBoundingBox.isDeactivated()) {
            part->m_boundingBox.deactivate();
            if (it == m_parts.end()) {
                m_boundingBox.deactivate();
                return;
            }
            part = *it;
            ++it;
        }
        part->mesh()->m_localBoundingBox.transform(part->m_boundingBox, m_matrix.world);
        part->m_centerOfBoundingBox = part->m_boundingBox.getCenter();
        m_boundingBox = part->m_boundingBox;
        for (++it; it != m_parts.end(); ++it) {
            part = *it;
            if (part->mesh()->m_localBoundingBox.isActivated()) {
                part->mesh()->m_localBoundingBox.transform(part->m_boundingBox, m_matrix.world);
                part->m_centerOfBoundingBox = part->m_boundingBox.getCenter();
                m_boundingBox.merge(part->m_boundingBox);
            } else {
                part->m_boundingBox.deactivate();
            }
        }
    }
}

void QEntity::_mergeChildsBoundingBoxes()
{
    //TODO ?
    std::size_t i = 0;
    if (m_boundingBox.isDeactivated()) {
        for (; i<m_childEntities.size(); ++i) {
            if ((m_childEntities[i]->visibled()) && (m_childEntities[i]->boundingBox().isActivated())) {
                m_boundingBox = m_childEntities[i]->boundingBox();
                break;
            }
        }
    }
    for (; i<m_childEntities.size(); ++i) {
        if ((m_childEntities[i]->visibled()) && (m_childEntities[i]->boundingBox().isActivated()))
            m_boundingBox.merge(m_childEntities[i]->boundingBox());
    }
    m_centerOfBoundingBox = m_boundingBox.getCenter();
}

void QEntity::updateNormals()
{
    QMesh* mesh;
    for (std::vector<Part*>::iterator it = m_parts.begin(); it != m_parts.end(); ++it) {
        mesh = (*it)->mesh();
        mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);
        mesh->updateNormals();
        mesh->applyChanges();
    }
}

void QEntity::updateNormalsWithChilds()
{
    updateNormals();
    for (std::size_t i=0; i<m_childEntities.size(); ++i)
        m_childEntities[i]->updateNormalsWithChilds();
}

void QEntity::_updateLocalMatrixWorld()
{
    QOtherMathFunctions::quaternionToMatrix(m_orientation, m_scale, m_localMatrixWorld);
    /*_localMatrixWorld(3, 0) = 0.0f;
    _localMatrixWorld(3, 1) = 0.0f;
    _localMatrixWorld(3, 2) = 0.0f;
    _localMatrixWorld(3, 3) = 1.0f;*/
    m_localMatrixWorld(0, 3) = m_position.x();
    m_localMatrixWorld(1, 3) = m_position.y();
    m_localMatrixWorld(2, 3) = m_position.z();
}

void QEntity::_updateMatrixWorld()
{
    m_matrix.world(3, 0) = 0.0f;
    m_matrix.world(3, 1) = 0.0f;
    m_matrix.world(3, 2) = 0.0f;
    m_matrix.world(3, 3) = 1.0f;
    m_matrix.world(0, 3) = m_localMatrixWorld(0, 3);
    m_matrix.world(1, 3) = m_localMatrixWorld(1, 3);
    m_matrix.world(2, 3) = m_localMatrixWorld(2, 3);
    m_matrix.world(0, 0) = m_localMatrixWorld(0, 0);
    m_matrix.world(0, 1) = m_localMatrixWorld(0, 1);
    m_matrix.world(0, 2) = m_localMatrixWorld(0, 2);
    m_matrix.world(1, 0) = m_localMatrixWorld(1, 0);
    m_matrix.world(1, 1) = m_localMatrixWorld(1, 1);
    m_matrix.world(1, 2) = m_localMatrixWorld(1, 2);
    m_matrix.world(2, 0) = m_localMatrixWorld(2, 0);
    m_matrix.world(2, 1) = m_localMatrixWorld(2, 1);
    m_matrix.world(2, 2) = m_localMatrixWorld(2, 2);
    m_globalPosition = m_position;
    _updateBoundingBox();
}

void QEntity::_updateMatrixWorld(const QMatrix4x4& parentMatrixWorld)
{
    m_matrix.world = parentMatrixWorld * m_localMatrixWorld;
    m_globalPosition.setX(m_matrix.world(0, 3));
    m_globalPosition.setY(m_matrix.world(1, 3));
    m_globalPosition.setZ(m_matrix.world(2, 3));
    _updateBoundingBox();
}

void QEntity::draw(QScrollEngineContext* context)
{
    context->drawEntity(this);
}

void QEntity::Part::draw(QScrollEngineContext* context)
{
    context->drawPartEntity(this);
}

}
