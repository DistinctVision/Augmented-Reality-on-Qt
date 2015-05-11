#ifndef QLIGHT_H
#define QLIGHT_H

#include <vector>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QBoundingBox.h"


namespace QScrollEngine {

class QMesh;
class QScene;
class QCamera3D;
class QScrollEngineContext;
class QEntity;

class QLight: public QSceneObject3D
{
    friend class QScene;
    friend class QScrollEngineContext;
    friend class QEntity;

public:
    enum Type
    {
        Omni,
        Spot
    };

public:
    QLight(bool isStatic = false);
    QLight(QScene* scene, bool isStatic = false);
    QLight(QEntity* entity, bool isStatic = false);
    virtual ~QLight();

    void setParentScene(QScene* scene);
    void setParentScene_saveTransform(QScene* scene);
    void setParentEntity(QEntity* entity);
    bool setParentEntity_saveTransform(QEntity* entity);
    void setPosition(const QVector3D& position) { _position = position; _transformHasChanged = true; }
    void setPosition(float x, float y, float z) { _position.setX(x); _position.setY(y);
                                                  _position.setZ(z); _transformHasChanged = true; }
    bool transformHasChanged() const;
    Type type() const { return _type; }
    bool isStatic() const { return _isStatic; }
    float radius() const { return _radius; }
    void setRadius(float radius);
    float lightSoft() const { return _soft; }
    void setLightSoft(float lightSoft) { _soft = lightSoft; }
    QColor color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }
    QVector4D colorVector() const { return QVector4D(_color.redF(), _color.greenF(), _color.blueF(), _power); }
    float power() const { return _power; }
    void setPower(float power) { _power = power; }
    void solveTransform();
    void updateTransform();
    int indexParentScene() const { return _index; }
    int indexParentEntity() const { return _childIndex; }
    QBoundingBox boundingBox() const { return _boundingBox; }
    QVector3D centerOfGlobalBoundingBox() const { return _centerOfGlobalBoundingBox; }

    virtual float intensityAtPoint(const QVector3D& point) const
    {
        float distance = (point - _position).length();
        return (1.0f - distance / _radius) * _color.alphaF();
    }

protected:
    Type _type;
    bool _isStatic;
    int _index;
    int _childIndex;
    float _radius;
    float _radiusSquare;
    float _soft;
    float _power;
    QColor _color;
    QBoundingBox _boundingBox;
    QVector3D _centerOfGlobalBoundingBox;

    void _updateBoundingBox();
    void _updateTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _solveTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _updateMatrixWorld();
};

}
#endif
