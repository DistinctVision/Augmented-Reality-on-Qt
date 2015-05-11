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

QEntity::QPartEntity::~QPartEntity()
{
    if (_mesh) {
        if (_parentSceneObject) {
            _parentSceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        if (_mesh->_countUsedParts <= 1) {
            _mesh->_countUsedParts = 0;
            delete _mesh;
        } else {
            --_mesh->_countUsedParts;
        }
    }
}

void QEntity::QPartEntity::setMesh(QMesh* mesh)
{
    if (_mesh) {
        if (_parentSceneObject) {
            _parentSceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        if (_mesh->countUsedParts() <= 1)
            delete _mesh;
        else
            --_mesh->_countUsedParts;
    }
    _mesh = mesh;
    if (_mesh) {
        ++_mesh->_countUsedParts;
        if (_parentSceneObject) {
            _parentSceneObject->setChangedTransform();
            _connectMeshToSceneObject();
        }
    }
}

void QEntity::QPartEntity::_connectMeshToSceneObject()
{
    _parentSceneObject->connect(_mesh, &QMesh::changedLocalBoundingBox,
                                _parentSceneObject, &QSceneObject3D::setChangedTransform, Qt::DirectConnection);
}

void QEntity::QPartEntity::_disconnectMeshFromSceneObject()
{
    _parentSceneObject->disconnect(_mesh, &QMesh::changedLocalBoundingBox,
                                   _parentSceneObject, &QSceneObject3D::setChangedTransform);
}

void QEntity::QPartEntity::_setParentSceneObject(QSceneObject3D* sceneObject)
{
    if (_mesh) {
        if (_parentSceneObject) {
            _parentSceneObject->setChangedTransform();
            _disconnectMeshFromSceneObject();
        }
        _parentSceneObject = sceneObject;
        if (_parentSceneObject) {
            _parentSceneObject->setChangedTransform();
            _connectMeshToSceneObject();
        }
    } else {
        _parentSceneObject = sceneObject;
    }
}

QEntity::QEntity(QScene* scene):QSceneObject3D(), QDrawObject3D(this)
{
    _animation = nullptr;
    _name.clear();
    _parentEntity = nullptr;
    //_parentSceneObject = this;
    _scale = QVector3D(1.0f, 1.0f, 1.0f);
    _transformHasChanged = true;
    _scene = scene;
    if (_scene)
        _scene->_addEntity(this);
}

QEntity::QEntity(QEntity* parentEntity):QSceneObject3D(), QDrawObject3D(this)
{
    _animation = nullptr;
    _name.clear();
    //_parentSceneObject = this;
    _scale = QVector3D(1.0f, 1.0f, 1.0f);
    _transformHasChanged = true;
    _scene = nullptr;
    _parentEntity = nullptr;
    if (parentEntity)
        parentEntity->addChild(this);
    else
        _scene = nullptr;
}

void QEntity::_freePart(unsigned int i)
{
    std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin() + i;
    delete *it;
}

QEntity::~QEntity()
{
    if (_animation) {
        if (_animation->countUsedEntities() <= 1)
            delete _animation;
        else
            --_animation->_countUsedEntities;
    }
    deleteParts();
    while (!_childLights.empty())
        delete _childLights.at(_childLights.size() - 1);
    while (!_childSprites.empty())
        delete _childSprites.at(_childSprites.size() - 1);
    while (!_childEntities.empty())
        delete _childEntities.at(_childEntities.size() - 1);
    if (_parentEntity)
        _parentEntity->_deleteChild(this);
    else if (_scene)
        _scene->_deleteEntity(this);
}

void QEntity::convertShaders(const std::function<QSh*(QSh*)>& convertFunction)
{
    for (std::vector<QSprite*>::iterator it = _childSprites.begin(); it != _childSprites.end(); ++it) {
        QSprite* sprite = *it;
        sprite->setShader(convertFunction(sprite->shader()));
    }
    for (std::vector<QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
        QPartEntity* part = *it;
        part->setShader(convertFunction(part->shader()));
    }
    for (std::vector<QEntity*>::iterator it = _childEntities.begin(); it != _childEntities.end(); ++it) {
        QEntity* entity = *it;
        entity->convertShaders(convertFunction);
    }
}

bool QEntity::haveAnimation() const
{
    if (_animation)
        return true;
    for (unsigned int i=0; i<_childEntities.size(); ++i)
        if (_childEntities[i]->haveAnimation())
            return true;
    return false;
}

void QEntity::setAnimation(QAnimation3D* animation)
{
    if (_animation) {
        if (_animation->countUsedEntities() <= 1)
            delete _animation;
        else
            --_animation->_countUsedEntities;
    }
    _animation = animation;
    if (_animation) {
        ++_animation->_countUsedEntities;
    }
}

void QEntity::swapAnimation(QEntity* entity)
{
    std::swap(_animation, entity->_animation);
}

void QEntity::setToAnimation()
{
    if (_animation && _animation->enable()) {
        _animation->entityToAnimation(this);
    }
}

void QEntity::_updateAnimation()
{
    if (_animation && _animation->enable()) {
        _animation->updateFrame();
        _animation->entityToAnimation(this);
    }
}

void QEntity::setAnimationTime(float time)
{
    if (_animation)
        _animation->setAnimationTime(time);
    for (unsigned int i=0; i<_childEntities.size(); ++i)
        _childEntities[i]->setAnimationTime(time);
}

void QEntity::setAnimationSpeed(float speed)
{
    if (_animation)
        _animation->setAnimationSpeed(speed);
    for (unsigned int i=0; i<_childEntities.size(); ++i)
        _childEntities[i]->setAnimationSpeed(speed);
}

void QEntity::setAnimationParameters(float time, float speed)
{
    if (_animation) {
        _animation->setAnimationTime(time);
        _animation->setAnimationSpeed(speed);
    }
    for (unsigned int i=0; i<_childEntities.size(); ++i) {
        _childEntities[i]->setAnimationTime(time);
        _childEntities[i]->setAnimationSpeed(speed);
    }
}

void QEntity::_updateScene(QScene* scene)
{
    if (_scene == scene)
        return;
    unsigned int i;
    if ((_scene) && (scene)) {
        if (scene->parentContext()) {
            QScrollEngineContext* context = scene->parentContext();
            for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
                QMesh* mesh = (*it)->mesh();
                if (mesh)
                    mesh->setParentContext(context);
            }
        }
        for (i=0; i<_childLights.size(); ++i) {
            QLight* light = _childLights.at(i);
            _scene->_deleteLight(light);
            light->_scene = scene;
            scene->_addLight(light);
        }
    } else if (_scene) {
        for (i=0; i<_childLights.size(); ++i) {
            QLight* light = _childLights.at(i);
            _scene->_deleteLight(light);
            light->_scene = nullptr;
        }
    } else /*if (scene)*/ {
        if (scene->parentContext()) {
            QScrollEngineContext* context = scene->parentContext();
            for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
                QMesh* mesh = (*it)->mesh();
                if (mesh)
                    mesh->setParentContext(context);
            }
        }
        for (i=0; i<_childLights.size(); ++i) {
            QLight* light = _childLights.at(i);
            light->_scene = scene;
            scene->_addLight(light);
        }
    }
    for (i=0; i<_childEntities.size(); ++i)
        _childEntities.at(i)->_updateScene(scene);
    for (i=0; i<_childSprites.size(); ++i)
        _childSprites.at(i)->_scene = scene;
    _scene = scene;
}

void QEntity::addChild(QEntity* entity)
{
    if (entity->_parentEntity) {
        entity->_parentEntity->_deleteChild(entity);
    } else if (entity->_scene) {
        entity->_scene->_deleteEntity(entity);
    }
    entity->_parentEntity = this;
    entity->_index = _childEntities.size();
    _childEntities.push_back(entity);
    entity->_updateScene(_scene);
    _transformHasChanged = true;
}

void QEntity::_deleteChild(QEntity* entity)
{
    _childEntities[entity->_index] = _childEntities[_childEntities.size()-1];
    _childEntities[entity->_index]->_index = entity->_index;
    _childEntities.pop_back();
    entity->_parentEntity = nullptr;
    entity->_updateScene(nullptr);
    entity->_transformHasChanged = true;
}

void QEntity::addChild(QSprite* sprite)
{
    if (sprite->_parentEntity) {
        sprite->_parentEntity->_deleteChild(sprite);
    } else if (sprite->_scene) {
        sprite->_scene->_deleteSprite(sprite);
    }
    sprite->_parentEntity = this;
    sprite->_index = _childSprites.size();
    _childSprites.push_back(sprite);
    sprite->_scene = _scene;
}

void QEntity::_deleteChild(QSprite* sprite)
{
    _childSprites[sprite->_index] = _childSprites[_childSprites.size()-1];
    _childSprites[sprite->_index]->_index = sprite->_index;
    _childSprites.pop_back();
    sprite->_parentEntity = nullptr;
    sprite->_scene = nullptr;
}

void QEntity::addChild(QLight* light)
{
    if (light->_parentEntity) {
        light->_parentEntity->_deleteChild(light);
        if (light->_scene)
            light->_scene->_addLight(light);
    } else if (light->_scene)
        light->_scene->_deleteLight(light);
    light->_parentEntity = this;
    light->_childIndex = _childLights.size();
    _childLights.push_back(light);
    light->_scene = _scene;
    if (_scene)
        _scene->_addLight(light);
}

void QEntity::_deleteChild(QLight* light)
{
    _childLights[light->_childIndex] = _childLights[_childLights.size()-1];
    _childLights[light->_childIndex]->_childIndex = light->_childIndex;
    _childLights.pop_back();
    light->_parentEntity = nullptr;
    if (light->_scene)
        light->_scene->_deleteLight(light);
    light->_scene = nullptr;
}

bool QEntity::addChild_saveTransform(QSprite* sprite)
{
    updateTransform();
    sprite->updateTransform();
    if (sprite->_parentEntity) {
        sprite->_parentEntity->_deleteChild(sprite);
    } else if (sprite->_scene) {
        sprite->_scene->_deleteSprite(sprite);
    }
    sprite->_parentEntity = this;
    sprite->_index = _childSprites.size();
    _childSprites.push_back(sprite);
    sprite->_scene = _scene;
    QVector3D globalScale;
    QMatrix3x3 globalMatrixRot;
    if (getGlobalParameters(globalScale, globalMatrixRot)) {
        sprite->_position = sprite->_globalPosition - _globalPosition;
        sprite->_position.setX(sprite->_position.x() / globalScale.x());
        sprite->_position.setY(sprite->_position.y() / globalScale.y());
        sprite->_position.setZ(sprite->_position.z() / globalScale.z());
        sprite->_position = QOtherMathFunctions::transformTransposed(globalMatrixRot, sprite->_position);
        return true;
    }
    return false;
}

bool QEntity::addChild_saveTransform(QLight* light)
{
    updateTransform();
    light->updateTransform();
    if (light->_parentEntity) {
        light->_parentEntity->_deleteChild(light);
    } else if (light->_scene) {
        light->_scene->_deleteLight(light);
    }
    light->_parentEntity = this;
    light->_index = _childLights.size();
    _childLights.push_back(light);
    _scene->_addLight(light);
    light->_transformHasChanged = false;
    QVector3D globalScale;
    QMatrix3x3 globalMatrixRot;
    if (getGlobalParameters(globalScale, globalMatrixRot)) {
        light->_position = light->_globalPosition - _globalPosition;
        light->_position.setX(light->_position.x() / globalScale.x());
        light->_position.setY(light->_position.y() / globalScale.y());
        light->_position.setZ(light->_position.z() / globalScale.z());
        light->_position = QOtherMathFunctions::transformTransposed(globalMatrixRot, light->_position);
        return true;
    }
    return false;
}

bool QEntity::addChild_saveTransform(QEntity* entity)
{
    updateTransform();
    entity->updateTransform();
    if (entity->_parentEntity) {
        entity->_parentEntity->_deleteChild(entity);
    } else if (entity->_scene) {
        entity->_scene->_deleteEntity(entity);
    }
    entity->_parentEntity = this;
    entity->_index = _childEntities.size();
    _childEntities.push_back(entity);
    entity->_updateScene(_scene);
    entity->_transformHasChanged = true;
    QVector3D globalScale, entityGlobalScale;
    QQuaternion globalOrientation, entityGlobalOrientation;
    if (getGlobalParameters(globalScale, globalOrientation) &&
            entity->getGlobalParameters(entityGlobalScale, entityGlobalOrientation)) {
        entity->_position = entity->_globalPosition - _globalPosition;
        entity->_position.setX(entity->_position.x() / globalScale.x());
        entity->_position.setY(entity->_position.y() / globalScale.y());
        entity->_position.setZ(entity->_position.z() / globalScale.z());
        entity->_position = (globalOrientation.conjugate()).rotatedVector(entity->_position);
        entity->_orientation = globalOrientation * entityGlobalOrientation;
        entity->_scale.setX(entityGlobalScale.x() / globalScale.x());
        entity->_scale.setY(entityGlobalScale.y() / globalScale.y());
        entity->_scale.setZ(entityGlobalScale.z() / globalScale.z());
        return true;
    }
    return false;
}

QEntity* QEntity::findChild(const QString& name)
{
    for (unsigned int i=0; i<_childEntities.size(); ++i) {
        if (_childEntities[i]->name() == name) {
            return _childEntities[i];
        } else {
            QEntity* result = _childEntities[i]->findChild(name);
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
    else if (_parentEntity)
        _parentEntity->_deleteChild(this);
}

bool QEntity::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (_parentEntity) {
        _parentEntity->_deleteChild(this);
        updateTransform();
        _position = _globalPosition;
        QVector3D globalScale;
        QQuaternion globalOrientation;
        if (getGlobalParameters(globalScale, globalOrientation)) {
            _orientation = globalOrientation;
            _scale = globalScale;
            return true;
        }
        return false;
    }
    return true;
}

void QEntity::setParentScene(QScene* scene)
{
    if (_parentEntity) {
        _parentEntity->_deleteChild(this);
        _parentEntity = nullptr;
    } else if (_scene)
        _scene->_deleteEntity(this);
    _updateScene(scene);
    if (_scene)
        _scene->_addEntity(this);
}

bool QEntity::setParentScene_saveTransform(QScene* scene)
{
    bool flag = true;
    if (_parentEntity) {
        updateTransform();
        _parentEntity->_deleteChild(this);
        _parentEntity = nullptr;
        _position = _globalPosition;
        QQuaternion globalOrientation;
        QVector3D globalScale;
        if (getGlobalParameters(globalScale, globalOrientation)) {
            _orientation = globalOrientation;
            _scale = globalScale;
        } else
            flag = false;
    } else if (_scene)
        _scene->_deleteEntity(this);
    _updateScene(scene);
    if (_scene)
        _scene->_addEntity(this);
    return flag;
}

bool QEntity::transformHasChanged() const
{
    if (_transformHasChanged)
        return true;
    if (_parentEntity)
        return _parentEntity->transformHasChanged();
    return false;
}

void QEntity::_updateTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (_transformHasChanged) {
        _updateLocalMatrixWorld();
        _updateMatrixWorld(parentMatrixWorld);
    }
    _transformHasChanged = false;
}

void QEntity::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (_transformHasChanged) {
        _updateLocalMatrixWorld();
    }
    _transformHasChanged = false;
    _updateMatrixWorld(parentMatrixWorld);
}

void QEntity::_solveTransformChilds()
{
    unsigned int i;
    for (i=0; i<_childLights.size(); ++i)
        _childLights[i]->_solveTransformFromParent(_matrix.world);
    for (i=0; i<_childSprites.size(); ++i)
        _childSprites[i]->_solveTransformFromParent(_matrix.world);
    i = 0;
    if (_boundingBox.isDeactivated()) {
        for (; i<_childEntities.size(); ++i) {
            _childEntities[i]->_solveTransformFromParent(_matrix.world);
            _childEntities[i]->_solveTransformChilds();
            if (_childEntities[i]->_boundingBox.isActivated()) {
                _boundingBox = _childEntities[i]->_boundingBox;
                break;
            }
        }
    }
    for (; i<_childEntities.size(); ++i) {
        _childEntities[i]->_solveTransformFromParent(_matrix.world);
        _childEntities[i]->_solveTransformChilds();
        if (_childEntities[i]->_boundingBox.isActivated())
            _boundingBox.merge(_childEntities[i]->_boundingBox);
    }
    _centerOfBoundingBox = _boundingBox.getCenter();
}

void QEntity::_updateTransformChilds(bool& change)
{
    unsigned int i;
    for (i=0; i<_childLights.size(); ++i)
        _childLights[i]->_updateTransformFromParent(_matrix.world);
    for (i=0; i<_childSprites.size(); ++i)
        _childSprites[i]->_solveTransformFromParent(_matrix.world);
    for (i=0; i<_childEntities.size(); ++i) {
        if (_childEntities[i]->_transformHasChanged) {
            change = true;
            _childEntities[i]->_updateLocalMatrixWorld();
            _childEntities[i]->_updateMatrixWorld(_matrix.world);
            _childEntities[i]->_transformHasChanged = false;
            _childEntities[i]->_solveTransformChilds();
        } else
            _childEntities[i]->_updateTransformChilds(change);
    }
}

void QEntity::_updateTransformChilds()
{
    unsigned int i;
    for (i=0; i<_childLights.size(); ++i)
        _childLights[i]->_updateTransformFromParent(_matrix.world);
    for (i=0; i<_childSprites.size(); ++i)
        _childSprites[i]->_solveTransformFromParent(_matrix.world);
    for (i=0; i<_childEntities.size(); ++i) {
        if (_childEntities[i]->_transformHasChanged) {
            _childEntities[i]->_updateLocalMatrixWorld();
            _childEntities[i]->_updateMatrixWorld(_matrix.world);
            _childEntities[i]->_transformHasChanged = false;
            _childEntities[i]->_solveTransformChilds();
        } else
            _childEntities[i]->_updateTransformChilds();
    }
}

void QEntity::updateTransform()
{
    bool change;
    updateTransform(change);
}

void QEntity::updateTransform(bool& change)
{
    if (_parentEntity) {
        _parentEntity->updateTransform(change);
    } else {
        if (_transformHasChanged) {
            change = true;
            _updateLocalMatrixWorld();
            _transformHasChanged = false;
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
    unsigned int i;
    for (i=0; i<_childLights.size(); ++i) {
        _childLights[i]->_solveTransformFromParent(_matrix.world);
    }
    for (i=0; i<_childSprites.size(); ++i) {
        _childSprites[i]->_solveTransformFromParent(_matrix.world);
        _scene->_spriteToDrawing(_childSprites[i], camera);
    }
    QEntity* entity;
    i = 0;
    if (_boundingBox.isDeactivated()) {
        for (; i<_childEntities.size(); ++i) {
            entity = _childEntities[i];
            entity->_updateAnimation();
            entity->_solveTransformFromParent(_matrix.world);
            entity->_prevStepDrawing_solveTransformChilds(camera);
            entity->_centerOfBoundingBox = entity->_boundingBox.getCenter();
            if (entity->_boundingBox.isActivated()) {
                _boundingBox = entity->_boundingBox;
                break;
            }
        }
    }
    for (; i<_childEntities.size(); ++i) {
        entity = _childEntities[i];
        entity->_updateAnimation();
        entity->_solveTransformFromParent(_matrix.world);
        entity->_prevStepDrawing_solveTransformChilds(camera);
        entity->_centerOfBoundingBox = entity->_boundingBox.getCenter();
        if (entity->_boundingBox.isActivated())
            _boundingBox.merge(entity->_boundingBox);
    }
}

void QEntity::_prevStepDrawing_solveTransform(const QCamera3D* camera)
{
    _updateLocalMatrixWorld();
    if (_parentEntity)
        _updateMatrixWorld(_parentEntity->_matrix.world);
    else
        _updateMatrixWorld();
    _transformHasChanged = false;
    _prevStepDrawing_solveTransformChilds(camera);
    _centerOfBoundingBox = _boundingBox.getCenter();
}

void QEntity::_prevStepDrawing_updateTransform(bool& change, const QCamera3D* camera)
{
    if ((_animation) && (_animation->enable())) {
        _animation->updateFrame();
        _animation->entityToAnimation(this);
        _prevStepDrawing_solveTransform(camera);
        change = true;
    } else if (_transformHasChanged) {
        _prevStepDrawing_solveTransform(camera);
        change = true;
    } else {
        unsigned int i;
        for (i=0; i<_childLights.size(); ++i) {
            _childLights[i]->_updateTransformFromParent(_matrix.world);
        }
        for (i=0; i<_childSprites.size(); ++i) {
            _childSprites[i]->_solveTransformFromParent(_matrix.world);
            _scene->_spriteToDrawing(_childSprites[i], camera);
        }
        for (i=0; i<_childEntities.size(); ++i) {
            _childEntities[i]->_prevStepDrawing_updateTransform(_transformHasChanged, camera);
        }
        if (_transformHasChanged) {
            _updateBoundingBox();
            _mergeChildsBoundingBoxes();
            change = true;
            _transformHasChanged = false;
        }
    }
}

void QEntity::_postStepDrawing(const QCamera3D* camera)
{
    if (!_visible) {
        _visibledForCamera = false;
        return;
    }
    if (!camera->frustum.boxInFrustum(_centerOfBoundingBox, _boundingBox.max() - _centerOfBoundingBox)) {
        _visibledForCamera = false;
        return;
    }
    _visibledForCamera = true;
    updateMatrxWorldViewProj(camera->matrixViewProj());
    QEntity::QPartEntity* part;
    for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
        part = *it;
        if (!part->_visible) {
            part->_visibledForCamera = false;
            continue;
        }
        if (camera->frustum.boxInFrustum(part->_centerOfBoundingBox, part->_boundingBox.max() -
                                         part->_centerOfBoundingBox)) {
            part->_visibledForCamera = true;
            if (part->isAlpha()) {
                QScrollEngineContext::TempAlphaObject temp;
                temp.drawObject = part;
                temp.zDistance = QVector3D::dotProduct(_scene->_cameraInfo.position, _scene->_cameraInfo.localZ) -
                        part->_boundingBox.supportValue(_scene->_cameraInfo.localZ);
                _scene->_parentContext->_addTempAlphaObject(temp);
            } else {
                QSh* shaderOfObject = part->shader();
                if (shaderOfObject) {
                    shaderOfObject->preprocess();
                    QScrollEngineContext::_Drawing& drawing = _scene->_parentContext->_drawings[shaderOfObject->currentIndexType()];
                    drawing.currentObjects[shaderOfObject->subIndexType()].partEntities.push_back(part);
                }
            }
        } else {
            part->_visibledForCamera = false;
        }
    }
    for (unsigned int i=0; i<_childEntities.size(); ++i)
        _childEntities[i]->_postStepDrawing(camera);
}


QVector3D QEntity::getTransformPoint(const QVector3D& point) const
{
    return getTransformDir(point) + _position;
}

QVector3D QEntity::getTransformDir(const QVector3D& dir) const
{
    return _orientation.rotatedVector(QVector3D(dir.x() * _scale.x(), dir.y() * _scale.y(), dir.z() * _scale.z()));
}

QVector3D QEntity::getInverseTransformPoint(const QVector3D& point) const
{
    return getInverseTransformDir(point - _position);
}

QVector3D QEntity::getInverseTransformDir(const QVector3D& dir) const
{
    QVector3D result = _orientation.conjugate().rotatedVector(dir);
    return QVector3D(result.x() / _scale.x(), result.y() / _scale.y(), result.z() / _scale.z());
}

void QEntity::setAlpha(bool enable)
{
    for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
        (*it)->setAlpha(enable);
    }
}

QEntity::QPartEntity* QEntity::addPart(QMesh* mesh, QSh* shader, bool clone)
{
    QPartEntity* part = new QPartEntity((clone ? mesh : new QMesh(mesh)), shader);
    part->_setParentSceneObject(this);
    if (part->_shader)
        part->_shader->setSceneObject(this);
    _parts.push_back(part);
    _transformHasChanged = true;
    return _parts.at(_parts.size() - 1);
}

QEntity::QPartEntity* QEntity::addPart(QScrollEngineContext* parentContext, QGLPrimitiv::Primitives meshFlag, QSh* shader)
{
    assert(parentContext != nullptr);
    QMesh* mesh = nullptr;
    switch(meshFlag) {
    case QGLPrimitiv::None:
        break;
    case QGLPrimitiv::Quad:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createQuad(mesh);
        break;
    case QGLPrimitiv::Cube:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createCube(mesh);
        break;
    case QGLPrimitiv::Sphere:
        mesh = new QMesh(parentContext);
        QGLPrimitiv::createSphere(mesh);
        break;
    }
    if (mesh) {
        _transformHasChanged = true;
        QPartEntity* part = new QPartEntity(mesh, shader);
        part->_setParentSceneObject(this);
        if (part->_shader)
            part->_shader->setSceneObject(this);
        _parts.push_back(part);
        return _parts.at(_parts.size() - 1);
    }
    return nullptr;
}

void QEntity::addPart(QPartEntity* part, bool clone)
{
    QPartEntity* newPart = new QPartEntity((clone ? part->_mesh : new QMesh(part->_mesh)), part->shader()->copy());
    newPart->_setParentSceneObject(this);
    if (newPart->_shader)
        newPart->_shader->setSceneObject(this);
    newPart->_isAlpha = part->_isAlpha;
    newPart->_drawMode = part->_drawMode;
    _parts.push_back(newPart);
    _transformHasChanged = true;
}

QEntity* QEntity::copy() const
{//TODO light sources
    QEntity* entity;
    if (_parentEntity)
        entity = new QEntity(_parentEntity);
    else
        entity = new QEntity(_scene);
    entity->_name = _name;
    unsigned int i;
    for (i=0; i<_parts.size(); i++)
        entity->addPart(part(i), false);
    entity->_position = _position;
    entity->_orientation = _orientation;
    entity->_scale = _scale;
    entity->_boundingBox = _boundingBox;
    entity->_transformHasChanged = _transformHasChanged;
    entity->_visible = _visible;
    for (i=0; i<_childEntities.size(); ++i)
        entity->addChild(_childEntities[i]->copy());
    for (i=0; i<_childSprites.size(); ++i)
        entity->addChild(_childSprites[i]->copy());
    return entity;
}

QEntity* QEntity::clone() const
{//TODO and here
    QEntity* entity;
    if (_parentEntity)
        entity = new QEntity(_parentEntity);
    else
        entity = new QEntity(_scene);
    entity->_name = _name;
    unsigned int i;
    for (i=0; i<_parts.size(); i++)
        entity->addPart(part(i), true);
    entity->_position = _position;
    entity->_orientation = _orientation;
    entity->_scale = _scale;
    entity->_boundingBox = _boundingBox;
    entity->_transformHasChanged = _transformHasChanged;
    entity->_visible = _visible;
    for (i=0; i<_childEntities.size(); ++i)
        entity->addChild(_childEntities[i]->clone());
    for (i=0; i<_childSprites.size(); ++i)
        entity->addChild(_childSprites[i]->copy());
    return entity;
}

void QEntity::deletePart(unsigned int i)
{
    delete _parts[i];
    _parts.erase(_parts.begin() + i);
    //_transformHasChanged = true;
}

void QEntity::deleteParts()
{
    for (unsigned int i=0; i<_parts.size(); ++i)
        delete _parts[i];
    _parts.clear();
    //_transformHasChanged = true;
}

void QEntity::setShaderToParts(QSh* shader)
{
   for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it)
       (*it)->setShader(shader->copy());
}

void QEntity::setShaderToChildSprites(QSh* shader)
{
   for (unsigned int i=0; i<_childSprites.size(); ++i)
       _childSprites[i]->setShader(shader->copy());
}

void QEntity::setShaderToChildEntities(QSh* shader)
{
   for (unsigned int i=0; i<_childEntities.size(); ++i) {
       _childEntities[i]->setShaderToParts(shader->copy());
       _childEntities[i]->setShaderToChildEntities(shader);
   }
}

void QEntity::setShaderToAll(QSh* shader)
{
    for (std::vector<QEntity::QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it)
        (*it)->setShader(shader->copy());
    unsigned int i;
    for (i=0; i<_childSprites.size(); ++i)
        _childSprites[i]->setShader(shader->copy());
    for (i=0; i<_childEntities.size(); ++i)
        _childEntities[i]->setShaderToAll(shader);
}

void QEntity::setShaderAtPart(unsigned int i, QSh* shader)
{
    _parts[i]->setShader(shader->copy());
}

QSh* QEntity::shader(unsigned int i) const
{
    return _parts[i]->shader();
}

void QEntity::_updateBoundingBox()
{
    if (_parts.empty()) {
        _boundingBox.deactivate();
    } else {
        std::vector<QPartEntity*>::iterator it = _parts.begin();
        QPartEntity* part = *it;
        while (part->mesh()->_localBoundingBox.isDeactivated()) {
            part->_boundingBox.deactivate();
            if (it == _parts.end()) {
                _boundingBox.deactivate();
                return;
            }
            part = *it;
            ++it;
        }
        part->mesh()->_localBoundingBox.transform(part->_boundingBox, _matrix.world);
        part->_centerOfBoundingBox = part->_boundingBox.getCenter();
        _boundingBox = part->_boundingBox;
        for (++it; it != _parts.end(); ++it) {
            part = *it;
            if (part->mesh()->_localBoundingBox.isActivated()) {
                part->mesh()->_localBoundingBox.transform(part->_boundingBox, _matrix.world);
                part->_centerOfBoundingBox = part->_boundingBox.getCenter();
                _boundingBox.merge(part->_boundingBox);
            } else {
                part->_boundingBox.deactivate();
            }
        }
    }
}

void QEntity::_mergeChildsBoundingBoxes()
{
    //TODO ?
    unsigned int i = 0;
    if (_boundingBox.isDeactivated()) {
        for (; i<_childEntities.size(); ++i) {
            if ((_childEntities[i]->visible()) && (_childEntities[i]->boundingBox().isActivated())) {
                _boundingBox = _childEntities[i]->boundingBox();
                break;
            }
        }
    }
    for (; i<_childEntities.size(); ++i) {
        if ((_childEntities[i]->visible()) && (_childEntities[i]->boundingBox().isActivated()))
            _boundingBox.merge(_childEntities[i]->boundingBox());
    }
    _centerOfBoundingBox = _boundingBox.getCenter();
}

void QEntity::updateNormals()
{
    QMesh* mesh;
    for (std::vector<QPartEntity*>::iterator it = _parts.begin(); it != _parts.end(); ++it) {
        mesh = (*it)->mesh();
        mesh->setEnable_vertex_normal(true);
        mesh->updateNormals();
        mesh->applyChanges();
    }
}

void QEntity::updateNormalsWithChilds()
{
    updateNormals();
    for (unsigned int i=0; i<_childEntities.size(); ++i)
        _childEntities[i]->updateNormalsWithChilds();
}

void QEntity::_updateLocalMatrixWorld()
{
    QOtherMathFunctions::quaternionToMatrix(_orientation, _scale, _localMatrixWorld);
    /*_localMatrixWorld(3, 0) = 0.0f;
    _localMatrixWorld(3, 1) = 0.0f;
    _localMatrixWorld(3, 2) = 0.0f;
    _localMatrixWorld(3, 3) = 1.0f;*/
    _localMatrixWorld(0, 3) = _position.x();
    _localMatrixWorld(1, 3) = _position.y();
    _localMatrixWorld(2, 3) = _position.z();
}

void QEntity::_updateMatrixWorld()
{
    _matrix.world(3, 0) = 0.0f;
    _matrix.world(3, 1) = 0.0f;
    _matrix.world(3, 2) = 0.0f;
    _matrix.world(3, 3) = 1.0f;
    _matrix.world(0, 3) = _localMatrixWorld(0, 3);
    _matrix.world(1, 3) = _localMatrixWorld(1, 3);
    _matrix.world(2, 3) = _localMatrixWorld(2, 3);
    _matrix.world(0, 0) = _localMatrixWorld(0, 0);
    _matrix.world(0, 1) = _localMatrixWorld(0, 1);
    _matrix.world(0, 2) = _localMatrixWorld(0, 2);
    _matrix.world(1, 0) = _localMatrixWorld(1, 0);
    _matrix.world(1, 1) = _localMatrixWorld(1, 1);
    _matrix.world(1, 2) = _localMatrixWorld(1, 2);
    _matrix.world(2, 0) = _localMatrixWorld(2, 0);
    _matrix.world(2, 1) = _localMatrixWorld(2, 1);
    _matrix.world(2, 2) = _localMatrixWorld(2, 2);
    _globalPosition = _position;
    _updateBoundingBox();
}

void QEntity::_updateMatrixWorld(const QMatrix4x4& parentMatrixWorld)
{
    _matrix.world = parentMatrixWorld * _localMatrixWorld;
    _globalPosition.setX(_matrix.world(0, 3));
    _globalPosition.setY(_matrix.world(1, 3));
    _globalPosition.setZ(_matrix.world(2, 3));
    _updateBoundingBox();
}

void QEntity::draw(QScrollEngineContext* context)
{
    context->drawEntity(this);
}

void QEntity::QPartEntity::draw(QScrollEngineContext* context)
{
    context->drawPartEntity(this);
}

}
