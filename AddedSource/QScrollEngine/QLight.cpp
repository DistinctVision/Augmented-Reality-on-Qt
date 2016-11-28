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
    m_parentEntity = nullptr;
    m_scene = nullptr;
    m_type = Type::Omni;
    m_transformHasChanged = true;
    m_isStatic = isStatic;
    m_index = -1;
    m_radius = m_radiusSquared = 1.0f;
    m_soft = 1.0f;
    m_power = 1.0f;
    m_color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
}

QLight::QLight(QScene* scene, bool isStatic)
{
    m_parentEntity = nullptr;
    m_scene = nullptr;
    m_type = Type::Omni;
    m_transformHasChanged = true;
    m_isStatic = isStatic;
    m_index = -1;
    m_radius = m_radiusSquared = 1.0f;
    m_soft = 1.0f;
    m_power = 1.0f;
    m_color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    setParentScene(scene);
}

QLight::QLight(QEntity* parentEntity, bool isStatic)
{
    m_parentEntity = nullptr;
    m_scene = nullptr;
    m_type = Type::Omni;
    m_transformHasChanged = true;
    m_isStatic = isStatic;
    m_index = -1;
    m_radius = m_radiusSquared = 1.0f;
    m_soft = 1.0f;
    m_power = 1.0f;
    m_color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    setParentEntity(parentEntity);
}

QLight::~QLight()
{
    if (m_scene) {
        m_scene->_deleteLight(this);
        m_scene = nullptr;
    }
    if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
}

void QLight::setParentEntity(QEntity* entity)
{
    if (entity)
        entity->addChild(this);
    else if (m_parentEntity)
        m_parentEntity->_deleteChild(this);
}

bool QLight::setParentEntity_saveTransform(QEntity* entity)
{
    if (entity)
        return entity->addChild_saveTransform(this);
    else if (m_parentEntity) {
        updateTransform();
        m_parentEntity->_deleteChild(this);
        m_position = m_globalPosition;
        m_transformHasChanged = false;
    }
    return false;
}

void QLight::setParentScene(QScene* scene)
{
    if (m_parentEntity) {
        m_parentEntity->_deleteChild(this);
    } else if (m_scene) {
        m_scene->_deleteLight(this);
    }
    m_scene = scene;
    if (m_scene) {
        m_scene->_addLight(this);
    }
}

void QLight::setParentScene_saveTransform(QScene* scene)
{
    if (m_parentEntity) {
        updateTransform();
        m_parentEntity->_deleteChild(this);
        m_position = m_globalPosition;
        m_transformHasChanged = false;
    } else if (m_scene) {
        m_scene->_deleteLight(this);
    }
    m_scene = scene;
    if (m_scene) {
        m_scene->_addLight(this);
    }
}

bool QLight::transformHasChanged() const
{
    if (m_transformHasChanged)
        return true;
    if (m_parentEntity)
        return m_parentEntity->transformHasChanged();
    return false;
}

void QLight::_updateTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    if (m_transformHasChanged) {
        m_globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, m_position);
        m_transformHasChanged = false;
        _updateMatrixWorld();
        _updateBoundingBox();
    }
}

void QLight::_solveTransformFromParent(const QMatrix4x4& parentMatrixWorld)
{
    m_globalPosition = QOtherMathFunctions::transform(parentMatrixWorld, m_position);
    m_transformHasChanged = false;
    _updateMatrixWorld();
    _updateBoundingBox();
}

void QLight::setRadius(float radius)
{
    assert(radius > 0.0f);
    m_radius = radius;
    m_radiusSquared = m_radius * m_radius;
    m_transformHasChanged = true;
}

void QLight::_updateMatrixWorld()
{
    /*_matrix.world(3, 0) = 0.0f;
    _matrix.world(3, 1) = 0.0f;
    _matrix.world(3, 2) = 0.0f;
    _matrix.world(3, 3) = 1.0f;*/
    m_matrix.world(0, 3) = m_globalPosition.x();
    m_matrix.world(1, 3) = m_globalPosition.y();
    m_matrix.world(2, 3) = m_globalPosition.z();
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
    m_centerOfGlobalBoundingBox = m_globalPosition;
    m_boundingBox.toPoint(m_centerOfGlobalBoundingBox);
    m_boundingBox.expand(m_radius);
}

void QLight::solveTransform()
{
    if (m_parentEntity) {
        m_parentEntity->updateTransform();
        _updateTransformFromParent(m_parentEntity->m_matrix.world);
    } else {
        m_globalPosition = m_position;
        _updateMatrixWorld();
    }
    _updateBoundingBox();
    m_centerOfGlobalBoundingBox = m_globalPosition;
    m_transformHasChanged = false;
}

void QLight::updateTransform()
{
    if (m_parentEntity) {
        if (m_parentEntity->transformHasChanged())
            solveTransform();
    } else if (m_transformHasChanged)
        solveTransform();
}

}
