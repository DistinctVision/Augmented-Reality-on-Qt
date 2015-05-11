#ifndef QSPRITE_H
#define QSPRITE_H

#include <QVector3D>

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
    friend class QScene;
    friend class QScrollEngineContext;
    friend class QEntity;

public:
    QSprite(QScene* scene = nullptr);
    QSprite(QEntity* parentEntity);
    ~QSprite();
    void setParentEntity(QEntity* entity);
    bool setParentEntity_saveTransform(QEntity* entity);
    void setParentScene(QScene* scene);
    void setParentScene_saveTransform(QScene* scene);
    void setAngle(float angle) { _angle = angle; }
    void setScale(const QVector2D& scale) { _scale = scale; }
    void setScale(float scale) { _scale.setX(scale); _scale.setY(scale); }
    void setScale(float x, float y) { _scale.setX(x); _scale.setY(y); }
    float angle() const { return _angle; }
    QVector2D scale() const { return _scale; }
    bool isAlpha() const { return _isAlpha; }
    void setAlpha(bool enable) { _isAlpha = enable; }
    void setPosition(const QVector3D& position) { _position = position; _transformHasChanged = true; }
    void setPosition(float x, float y, float z) { _position.setX(x); _position.setY(y); _position.setZ(z); _transformHasChanged = true; }
    void updateTransform();
    int index() const { return _index; }

    void setShader(QSh* shader)
    {
        delete _shader;
        _shader = shader;
        _shader->setObject(this);
    }
    QSprite* copy() const;

    void draw(QScrollEngineContext* context) override;

private:
    int _index;

    float _angle;
    QVector2D _scale;
    void _updateMatrixWorld(const QQuaternion& globalOrientation);
    void _solveTransformFromParent(const QMatrix4x4& parentMatrixWorld);
};

}

#endif
