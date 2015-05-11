#include "QScrollEngine/QSprite.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QOtherMathFunctions.h"

#include <QVector2D>

namespace QScrollEngine {

QSprite::QSprite(QScene* scene):QSceneObject3D(), QDrawObject3D(this), QShObject3D(this)
{
    _parentEntity = nullptr;
    //_parentSceneObject = this;
    _scale = QVector2D(1.0f, 1.0f);
    _angle = 0.0f;
    _visibledForCamera = false;
    _isAlpha = false;
    _scene = scene;
    if (_scene) {
        _scene->_addSprite(this);
    }
    _shader->setSceneObject(this);
}

QSprite::QSprite(QEntity* parentEntity):QSceneObject3D(), QDrawObject3D(this), QShObject3D(this)
{
    _parentEntity = nullptr;
    _scene = nullptr;
    //_parentSceneObject = this;
    _scale = QVector2D(1.0f, 1.0f);
    _angle = 0.0f;
    _visibledForCamera = false;
    _isAlpha = false;
    if (parentEntity) {
        parentEntity->addChild(this);
    } else {
        _scene = nullptr;
    }
    _shader->setSceneObject(this);
}

QSprite::~QSprite()
{
    if (_parentEntity)
        _parentEntity->_deleteChild(this);
    else if (_scene)
        _scene->_deleteSprite(this);
}

void QSprite::setParentEntity(QEntity* entity)
{
    if (entity)
        entity->addChild(this);
    else if (_parentEntity)
        _parentEntity->_deleteChild(this);
}

bool QSprite::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (_parentEntity) {
        updateTransform();
        _parentEntity->_deleteChild(this);
        _position = _globalPosition;
    }
    return false;
}

void QSprite::setParentScene(QScene* scene)
{
    if (_parentEntity) {
        _parentEntity->_deleteChild(this);
    } else if (_scene) {
        _scene->_deleteSprite(this);
    }
    _scene = scene;
    if (_scene)
        _scene->_addSprite(this);
}

void QSprite::setParentScene_saveTransform(QScene* scene)
{
    if (_parentEntity) {
        updateTransform();
        _parentEntity->_deleteChild(this);
        _position = _globalPosition;
    } else if (_scene) {
        _scene->_deleteSprite(this);
    }
    _scene = scene;
    if (_scene)
        _scene->_addSprite(this);
}


QSprite* QSprite::copy() const
{
    QSprite* s = new QSprite(_scene);
    s->setShader(_shader->copy());
    s->setAlpha(_isAlpha);
    s->setPosition(_position);
    s->_angle = _angle;
    s->_scale = _scale;
    return s;
}

void QSprite::_updateMatrixWorld(const QQuaternion& globalOrientation)
{
    QOtherMathFunctions::quaternionToMatrix(globalOrientation, _matrix.world);
    /*_matrix.world(3, 0) = 0.0f;
    _matrix.world(3, 1) = 0.0f;
    _matrix.world(3, 2) = 0.0f;
    _matrix.world(3, 3) = 1.0f;*/
    _matrix.world(0, 3) = _globalPosition.x();
    _matrix.world(1, 3) = _globalPosition.y();
    _matrix.world(2, 3) = _globalPosition.z();
    _matrix.world(0, 0) *= _scale.x();
    _matrix.world(1, 0) *= _scale.x();
    _matrix.world(2, 0) *= _scale.x();
    _matrix.world(0, 1) *= _scale.y();
    _matrix.world(1, 1) *= _scale.y();
    _matrix.world(2, 1) *= _scale.y();
}

void QSprite::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    _globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, _position);
    _transformHasChanged = false;
}

void QSprite::updateTransform()
{
    if (_parentEntity) {
        if (_parentEntity->transformHasChanged()) {
            _parentEntity->updateTransform();
            _solveTransformFromParent(_parentEntity->_matrix.world);
        } else if (_transformHasChanged)
            _solveTransformFromParent(_parentEntity->_matrix.world);
    } else if (_transformHasChanged){
        _globalPosition = _position;
        _transformHasChanged = false;
    }
}

void QSprite::draw(QScrollEngineContext* context)
{
    context->drawSprite(this);
}

}
