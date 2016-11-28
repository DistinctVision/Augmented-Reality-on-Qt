#include "QScrollEngine/QSprite.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QOtherMathFunctions.h"

#include <QVector2D>

namespace QScrollEngine {

QSprite::QSprite(QScene* scene):QDrawObject3D(this), QSceneObject3D(), QShObject3D()
{
    m_parentEntity = nullptr;
    //_sceneObject = this;
    m_scale = QVector2D(1.0f, 1.0f);
    m_angle = 0.0f;
    m_visibledForCamera = false;
    m_isAlpha = false;
    m_scene = scene;
    if (m_scene)
        m_scene->_addSprite(this);
}

QSprite::QSprite(QEntity* parentEntity):QDrawObject3D(this), QSceneObject3D(), QShObject3D()
{
    m_parentEntity = nullptr;
    m_scene = nullptr;
    //_sceneObject = this;
    m_scale = QVector2D(1.0f, 1.0f);
    m_angle = 0.0f;
    m_visibledForCamera = false;
    m_isAlpha = false;
    if (parentEntity) {
        parentEntity->addChild(this);
    } else {
        m_scene = nullptr;
    }
}

QSprite::~QSprite()
{
    if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
    else if (m_scene)
        m_scene->_deleteSprite(this);
}

void QSprite::setParentEntity(QEntity* entity)
{
    if (entity)
        entity->addChild(this);
    else if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
}

bool QSprite::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (m_parentEntity) {
        updateTransform();
        m_parentEntity->_deleteChild(this);
        m_position = m_globalPosition;
    }
    return false;
}

void QSprite::setParentScene(QScene* scene)
{
    if (m_parentEntity) {
        m_parentEntity->_deleteChild(this);
    } else if (m_scene) {
        m_scene->_deleteSprite(this);
    }
    m_scene = scene;
    if (m_scene)
        m_scene->_addSprite(this);
}

void QSprite::setParentScene_saveTransform(QScene* scene)
{
    if (m_parentEntity) {
        updateTransform();
        m_parentEntity->_deleteChild(this);
        m_position = m_globalPosition;
    } else if (m_scene) {
        m_scene->_deleteSprite(this);
    }
    m_scene = scene;
    if (m_scene)
        m_scene->_addSprite(this);
}


QSprite* QSprite::copy() const
{
    QSprite* s = new QSprite(m_scene);
    s->setShader(m_shader->copy());
    s->setAlpha(m_isAlpha);
    s->setPosition(m_position);
    s->m_angle = m_angle;
    s->m_scale = m_scale;
    return s;
}

void QSprite::_updateMatrixWorld(const QQuaternion& globalOrientation)
{
    QOtherMathFunctions::quaternionToMatrix(globalOrientation, m_matrix.world);
    /*_matrix.world(3, 0) = 0.0f;
    _matrix.world(3, 1) = 0.0f;
    _matrix.world(3, 2) = 0.0f;
    _matrix.world(3, 3) = 1.0f;*/
    m_matrix.world(0, 3) = m_globalPosition.x();
    m_matrix.world(1, 3) = m_globalPosition.y();
    m_matrix.world(2, 3) = m_globalPosition.z();
    m_matrix.world(0, 0) *= m_scale.x();
    m_matrix.world(1, 0) *= m_scale.x();
    m_matrix.world(2, 0) *= m_scale.x();
    m_matrix.world(0, 1) *= m_scale.y();
    m_matrix.world(1, 1) *= m_scale.y();
    m_matrix.world(2, 1) *= m_scale.y();
}

void QSprite::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    m_globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, m_position);
    m_transformHasChanged = false;
}

void QSprite::updateTransform()
{
    if (m_parentEntity) {
        if (m_parentEntity->transformHasChanged()) {
            m_parentEntity->updateTransform();
            _solveTransformFromParent(m_parentEntity->m_matrix.world);
        } else if (m_transformHasChanged)
            _solveTransformFromParent(m_parentEntity->m_matrix.world);
    } else if (m_transformHasChanged){
        m_globalPosition = m_position;
        m_transformHasChanged = false;
    }
}

void QSprite::draw(QScrollEngineContext* context)
{
    context->drawSprite(this);
}

}
