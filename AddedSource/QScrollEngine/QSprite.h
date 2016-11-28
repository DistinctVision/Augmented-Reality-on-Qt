#ifndef QSPRITE_H
#define QSPRITE_H

#include <QVector3D>
#include <QSharedPointer>

#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QShObject3D.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/Shaders/QSh_All.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QEntity;

class QSprite:
        public QDrawObject3D,
        public QSceneObject3D,
        public QShObject3D
{
public:
    QSprite(QScene* scene = nullptr);
    QSprite(QEntity* parentEntity);
    ~QSprite();
    void setParentEntity(QEntity* entity);
    bool setParentEntity_saveTransform(QEntity* entity);
    void setParentScene(QScene* scene);
    void setParentScene_saveTransform(QScene* scene);
    void setAngle(float angle) { m_angle = angle; }
    void setScale(const QVector2D& scale) { m_scale = scale; }
    void setScale(float scale) { m_scale.setX(scale); m_scale.setY(scale); }
    void setScale(float x, float y) { m_scale.setX(x); m_scale.setY(y); }
    float angle() const { return m_angle; }
    QVector2D scale() const { return m_scale; }
    bool isAlpha() const { return m_isAlpha; }
    void setAlpha(bool enable) { m_isAlpha = enable; }
    void setPosition(const QVector3D& position) { m_position = position; m_transformHasChanged = true; }
    void setPosition(float x, float y, float z) { m_position.setX(x); m_position.setY(y); m_position.setZ(z); m_transformHasChanged = true; }
    void updateTransform();
    int index() const { return m_index; }

    QSprite* copy() const;

    void draw(QScrollEngineContext* context) override;

private:
    friend class QScene;
    friend class QScrollEngineContext;
    friend class QEntity;

    int m_index;

    float m_angle;
    QVector2D m_scale;

    void _updateMatrixWorld(const QQuaternion& globalOrientation);
    void _solveTransformFromParent(const QMatrix4x4& parentMatrixWorld);
};

}

#endif
