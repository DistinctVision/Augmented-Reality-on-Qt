#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QCamera3D.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QBoundingBox.h"
#include <qmath.h>
#include <cassert>

namespace QScrollEngine {

QLight::QLight(bool isStatic):
    QSceneObject3D()
{
    _parentEntity = nullptr;
    _scene = nullptr;
    _type = Omni;
    _transformHasChanged = true;
    _isStatic = isStatic;
    _index = -1;
    _radius = _radiusSquare = 1.0f;
    _soft = 1.0f;
    _power = 1.0f;
    _color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
}

QLight::QLight(QScene* scene, bool isStatic)
{
    _parentEntity = nullptr;
    _scene = nullptr;
    _type = Omni;
    _transformHasChanged = true;
    _isStatic = isStatic;
    _index = -1;
    _radius = _radiusSquare = 1.0f;
    _soft = 1.0f;
    _power = 1.0f;
    _color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    setParentScene(scene);
}

QLight::QLight(QEntity* parentEntity, bool isStatic)
{
    _parentEntity = nullptr;
    _scene = nullptr;
    _type = Omni;
    _transformHasChanged = true;
    _isStatic = isStatic;
    _index = -1;
    _radius = _radiusSquare = 1.0f;
    _soft = 1.0f;
    _power = 1.0f;
    _color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    setParentEntity(parentEntity);
}

QLight::~QLight()
{
    if (_scene) {
        _scene->_deleteLight(this);
        _scene = nullptr;
    }
    if (_parentEntity)
        _parentEntity->_deleteChild(this);
}

void QLight::setParentEntity(QEntity* entity)
{
    if (entity)
        entity->addChild(this);
    else if (_parentEntity)
        _parentEntity->_deleteChild(this);
}

bool QLight::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (_parentEntity) {
        updateTransform();
        _parentEntity->_deleteChild(this);
        _position = _globalPosition;
        _transformHasChanged = false;
    }
    return false;
}

void QLight::setParentScene(QScene* scene)
{
    if (_parentEntity) {
        _parentEntity->_deleteChild(this);
    } else if (_scene) {
        _scene->_deleteLight(this);
    }
    _scene = scene;
    if (_scene) {
        _scene->_addLight(this);
    }
}

void QLight::setParentScene_saveTransform(QScene* scene)
{
    if (_parentEntity) {
        updateTransform();
        _parentEntity->_deleteChild(this);
        _position = _globalPosition;
        _transformHasChanged = false;
    } else if (_scene) {
        _scene->_deleteLight(this);
    }
    _scene = scene;
    if (_scene) {
        _scene->_addLight(this);
    }
}

bool QLight::transformHasChanged() const
{
    if (_transformHasChanged)
        return true;
    if (_parentEntity)
        return _parentEntity->transformHasChanged();
    return false;
}

void QLight::_updateTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (_transformHasChanged) {
        _globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, _position);
        _transformHasChanged = false;
        _updateMatrixWorld();
        _updateBoundingBox();
    }
}

void QLight::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    _globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, _position);
    _transformHasChanged = false;
    _updateMatrixWorld();
    _updateBoundingBox();
}

void QLight::setRadius(float radius)
{
    assert(radius > 0.0f);
    _radius = radius;
    _radiusSquare = _radius * _radius;
    _transformHasChanged = true;
}

void QLight::_updateMatrixWorld()
{
    /*_matrix.world(3, 0) = 0.0f;
    _matrix.world(3, 1) = 0.0f;
    _matrix.world(3, 2) = 0.0f;
    _matrix.world(3, 3) = 1.0f;*/
    _matrix.world(0, 3) = _globalPosition.x();
    _matrix.world(1, 3) = _globalPosition.y();
    _matrix.world(2, 3) = _globalPosition.z();
    /*_matrix.world(0, 0) = 1.0f;
    _matrix.world(0, 1) = 0.0f;
    _matrix.world(0, 2) = 0.0f;
    _matrix.world(1, 0) = 0.0f;
    _matrix.world(1, 1) = 1.0f;
    _matrix.world(1, 2) = 0.0f;
    _matrix.world(2, 0) = 0.0f;
    _matrix.world(2, 1) = 0.0f;
    _matrix.world(2, 2) = 1.0f;*/
}

void QLight::_updateBoundingBox()
{
    _centerOfGlobalBoundingBox = _globalPosition;
    _boundingBox.toPoint(_centerOfGlobalBoundingBox);
    _boundingBox.expand(_radius);
}

void QLight::solveTransform()
{
    if (_parentEntity) {
        _parentEntity->updateTransform();
        _updateTransformFromParent(_parentEntity->_matrix.world);
    } else {
        _globalPosition = _position;
        _updateMatrixWorld();
    }
    _updateBoundingBox();
    _centerOfGlobalBoundingBox = _globalPosition;
    _transformHasChanged = false;
}

void QLight::updateTransform()
{
    if (_parentEntity) {
        if (_parentEntity->transformHasChanged())
            solveTransform();
    } else if (_transformHasChanged)
        solveTransform();
}

}
