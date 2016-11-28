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
public:
    enum class Type: int
    {
        Omni = 0,
        Spot = 1
    };

public:
    QLight(bool isStatic);
    QLight(QScene* scene = nullptr, bool isStatic = false);
    QLight(QEntity* entity, bool isStatic = false);
    virtual ~QLight();

    void setParentScene(QScene* scene);
    void setParentScene_saveTransform(QScene* scene);
    void setParentEntity(QEntity* entity);
    bool setParentEntity_saveTransform(QEntity* entity);
    void setPosition(const QVector3D& position) { m_position = position; m_transformHasChanged = true; }
    void setPosition(float x, float y, float z) { m_position.setX(x); m_position.setY(y);
                                                  m_position.setZ(z); m_transformHasChanged = true; }
    bool transformHasChanged() const;
    Type type() const { return m_type; }
    bool isStatic() const { return m_isStatic; }
    float radius() const { return m_radius; }
    void setRadius(float radius);
    float lightSoft() const { return m_soft; }
    void setLightSoft(float lightSoft) { m_soft = lightSoft; }
    QColor color() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    QVector4D colorVector() const { return QVector4D(m_color.redF(), m_color.greenF(), m_color.blueF(), m_power); }
    float power() const { return m_power; }
    void setPower(float power) { m_power = power; }
    void solveTransform();
    void updateTransform();
    int indexParentScene() const { return m_index; }
    int indexParentEntity() const { return m_childIndex; }
    QBoundingBox boundingBox() const { return m_boundingBox; }
    QVector3D centerOfGlobalBoundingBox() const { return m_centerOfGlobalBoundingBox; }

    virtual float intensityAtPoint(const QVector3D& point) const
    {
        float distance = (point - m_position).length();
        return (1.0f - distance / m_radius) * m_color.alphaF();
    }

protected:
    friend class QScene;
    friend class QScrollEngineContext;
    friend class QEntity;

    Type m_type;
    bool m_isStatic;
    int m_index;
    int m_childIndex;
    float m_radius;
    float m_radiusSquared;
    float m_soft;
    float m_power;
    QColor m_color;
    QBoundingBox m_boundingBox;
    QVector3D m_centerOfGlobalBoundingBox;

    void _updateBoundingBox();
    void _updateTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _solveTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _updateMatrixWorld();
};

}
#endif
