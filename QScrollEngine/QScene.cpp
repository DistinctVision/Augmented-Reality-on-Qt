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

QScene::QScene(QScrollEngineContext* parentContext, int order)
{
    scale.setX(1.0f); scale.setY(1.0f); scale.setZ(1.0f);
    _currentResetedSprite = _currentResetedEntity = 0;
    _order = order;
    _boundingBox.toPoint(0.0f, 0.0f, 0.0f);
    _boundingBox.expand(10000.0f);
    _parentContext = parentContext;
    if (_parentContext) {
        _quad = _parentContext->_quad;
        _parentContext->_addScene(this);
    } else {
        _quad = NULL;
        _index = 0;
    }
    _ambientColor.setRgb(0, 0, 0, 255);
}

QScene::~QScene()
{
    while (!_lights.empty())
        delete _lights.at(_lights.size() - 1);
    while (!_sprites.empty())
        delete _sprites.at(_sprites.size() - 1);
    while (!_entities.empty())
        delete _entities.at(_entities.size() - 1);
    if (_parentContext)
        _parentContext->_deleteScene(this);
    emit deleting();
}

QEntity* QScene::findEntity(const QString& name) const
{
    for (unsigned int i=0; i<_entities.size(); ++i) {
        if (_entities[i]->name() == name)
            return _entities[i];
    }
    return nullptr;
}

QEntity* QScene::findEntityWithChilds(const QString& name) const
{
    for (unsigned int i=0; i<_entities.size(); ++i) {
        if (_entities[i]->name() == name)
            return _entities[i];
        QEntity* child = _entities[i]->findChild(name);
        if (child)
            return child;
    }
    return nullptr;
}

void QScene::setParentContext(QScrollEngineContext* parentContext)
{
    if (_parentContext)
        _parentContext->_deleteScene(this);
    _parentContext = parentContext;
    if (_parentContext) {
        _quad = _parentContext->_quad;
        _parentContext->_addScene(this);
    } else {
        _quad = nullptr;
    }
}

void QScene::setOrder(int order)
{
    _order = order;
    if (_parentContext) {
        _parentContext->_deleteScene(this);
        _parentContext->_addScene(this);
    }
}

void QScene::_addSprite(QSprite* sprite)
{
    sprite->_index = _sprites.size();
    _sprites.push_back(sprite);
}

void QScene::_deleteSprite(QSprite* sprite)
{
    _sprites[sprite->_index] = _sprites[_sprites.size()-1];
    _sprites[sprite->_index]->_index = sprite->_index;
    _sprites.pop_back();
}

void QScene::_addEntity(QEntity* entity)
{
    entity->_index = _entities.size();
    _entities.push_back(entity);
}

void QScene::_deleteEntity(QEntity* entity)
{
    _entities[entity->_index] = _entities[_entities.size()-1];
    _entities[entity->_index]->_index = entity->_index;
    _entities.pop_back();
}

void QScene::_addLight(QLight* light)
{
    light->_index = _lights.size();
    _lights.push_back(light);
}

void QScene::_deleteLight(QLight* light)
{
    _lights[light->_index] = _lights[_lights.size()-1];
    _lights[light->_index]->_index = light->_index;
    _lights.pop_back();
}

void QScene::solveBoundingBox(float expand)
{
    if (_sprites.empty()) {
        if (_entities.empty()) {
            if (_lights.empty()) {
                _boundingBox.toPoint(0.0f, 0.0f, 0.0f);
                _boundingBox.expand(expand);
                return;
            } else {
                _lights[0]->updateTransform();
                _boundingBox = _lights[0]->boundingBox();
            }
        } else {
            _entities[0]->updateTransform();
            _boundingBox = _entities[0]->boundingBox();
            _boundingBox.expand(expand);
        }
    } else {
        _boundingBox.toPoint(_sprites[0]->position());
        _boundingBox.expand(_sprites[0]->_scale);
    }
    for (std::vector<QLight*>::iterator it = _lights.begin(); it != _lights.end(); ++it) {
        (*it)->updateTransform();
        _boundingBox.merge((*it)->boundingBox());
    }
    QBoundingBox temp;
    QSprite* sprite;
    for (std::vector<QSprite*>::iterator it = _sprites.begin(); it != _sprites.end(); ++it) {
        sprite = *it;
        temp.toPoint(sprite->position());
        temp.expand(sprite->_scale);
        _boundingBox.merge(temp);
    }
    QEntity* entity;
    for (std::vector<QEntity*>::iterator it = _entities.begin(); it != _entities.end(); ++it) {
        entity = *it;
        entity->updateTransform();
        _boundingBox.merge(entity->boundingBox());
    }
    _boundingBox.expand(expand);
}

void QScene::updateCameraInfo(QCamera3D* camera)
{
    camera->setScenePosition(position);
    camera->setSceneOrientation(orientation);
    camera->setSceneScale(scale);
    camera->update();
    _cameraInfo.position = camera->localPosition();
    _cameraInfo.orientation = camera->localOrientation();
#if defined(__ANDROID__)
    QQuaternion invOrientation = _cameraInfo.orientation;
#else
    QQuaternion invOrientation = _cameraInfo.orientation;
#endif
    _cameraInfo.localX = invOrientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
    _cameraInfo.localY = invOrientation.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f));
    _cameraInfo.localZ = invOrientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
}

void QScene::_updateGlobalPosition(QSprite* sprite){
    if (sprite->_transformHasChanged) {
        sprite->_globalPosition = sprite->_position;
        sprite->_transformHasChanged = false;
    }
}

void QScene::_spriteToDrawing(QSprite* sprite, const QCamera3D* camera)
{
    if (!sprite->visible())
        return;
    float sina = sinf(sprite->_angle);
    float cosa = cosf(sprite->_angle);
    QVector2D diagonal = sprite->_scale * 0.5, d, dr;
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
    QVector3D center = _cameraInfo.localX * d.x() + _cameraInfo.localY * d.y();
    sprite->_centerOfBoundingBox = sprite->_globalPosition;
    sprite->_boundingBox.toPoint(sprite->_centerOfBoundingBox);
    sprite->_boundingBox.expand(center);
    if (!camera->frustum.boxInFrustum(sprite->_centerOfBoundingBox, center)) {
        sprite->_visibledForCamera = false;
        return;
    }
    sprite->_visibledForCamera = true;
    QQuaternion globalOrientation = _cameraInfo.orientation * QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, sprite->_angle);
    globalOrientation.normalize();
    sprite->_updateMatrixWorld(globalOrientation);
    sprite->updateMatrxWorldViewProj(camera->matrixViewProj());
    if (sprite->isAlpha()) {
        QScrollEngineContext::TempAlphaObject temp;
        temp.drawObject = sprite;
        temp.zDistance = QVector3D::dotProduct(_cameraInfo.position - sprite->_centerOfBoundingBox, _cameraInfo.localZ);
        _parentContext->_addTempAlphaObject(temp);
    } else {
        QSh* shaderOfObject = sprite->shader();
        if (shaderOfObject) {
            shaderOfObject->preprocess();
            QScrollEngineContext::_Drawing& drawing = _parentContext->_drawings[shaderOfObject->currentIndexType()];
            drawing.currentObjects[shaderOfObject->subIndexType()].sprites.push_back(sprite);
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
    QCamera3D* camera = _parentContext->camera;
    updateCameraInfo(camera);
    if (!camera->frustum.boundingBoxInFrustum(_boundingBox)) {
        return;
    }
    unsigned int i;
    for (i=0; i<_lights.size(); ++i) {
        _lights[i]->updateTransform();
    }
    for (i=0; i<_sprites.size(); ++i) {
        _updateGlobalPosition(_sprites[i]);
        _spriteToDrawing(_sprites[i], camera);
    }
    for (i=0; i<_entities.size(); ++i) {
        _entityToDrawing(_entities[i], camera);
    }
}

}
