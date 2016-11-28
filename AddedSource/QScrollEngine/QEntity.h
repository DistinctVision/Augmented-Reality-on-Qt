#ifndef QENTITY_H
#define QENTITY_H

#include <functional>

#include <QSharedPointer>
#include <QQuaternion>
#include <QOpenGLShaderProgram>
#include <vector>
#include <QMatrix4x4>

#include "QScrollEngine/QShObject3D.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QGLPrimitiv.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/QBoundingBox.h"

namespace QScrollEngine {

class QScene;
class QMesh;
class QScrollEngineContext;
class QSprite;
class QLight;
class QAnimation3D;
class QCamera3D;

class QEntity:
        public QSceneObject3D,
        public QDrawObject3D
{
public:
    class Part:
            public QDrawObject3D,
            public QShObject3D
    {
    public:
        Part():QDrawObject3D(), QShObject3D() { m_mesh = nullptr; }
        Part(QMesh* mesh, QShPtr shader):QDrawObject3D(), QShObject3D(shader)
        {
            m_mesh = nullptr;
            setMesh(mesh);
        }
        ~Part();
        QMesh* mesh() const { return m_mesh; }
        void setMesh(QMesh* mesh);
        void updateGlobalBoundingBox(const QMatrix4x4& transform);
        void draw(QScrollEngineContext* context) override;

    private:
        friend class QEntity;
        friend class QScene;
        friend  class QScrollEngineContext;

        QMesh* m_mesh;

        void _connectMeshToSceneObject();
        void _disconnectMeshFromSceneObject();
        void _setSceneObject(QSceneObject3D* sceneObject);
    };

public:
    QEntity(QScene* scene = nullptr);
    QEntity(QEntity* parentEntity);
    ~QEntity();

    void addChild(QEntity* entity);
    void addChild(QSprite* sprite);
    void addChild(QLight* light);
    bool addChild_saveTransform(QEntity* entity);
    bool addChild_saveTransform(QSprite* sprite);
    bool addChild_saveTransform(QLight* light);
    void setParentEntity(QEntity* entity);
    bool setParentEntity_saveTransform(QEntity* entity);
    void setParentScene(QScene* scene);
    bool setParentScene_saveTransform(QScene* scene);
    std::size_t countEntityChilds() const { return m_childEntities.size(); }
    QEntity* childEntity(std::size_t i) const { return m_childEntities[i]; }
    std::size_t countSpriteChilds() const { return m_childSprites.size(); }
    QSprite* childSprite(std::size_t i) const { return m_childSprites[i]; }
    std::size_t countLightChilds() const { return m_childLights.size(); }
    QLight* childLight(std::size_t i) const { return m_childLights[i]; }
    QEntity* findChild(const QString& name);

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    void setPosition(const QVector3D& position) { m_position = position; m_transformHasChanged = true; }
    void setPosition(float x, float y, float z) { m_position.setX(x); m_position.setY(y);
                                                  m_position.setZ(z); m_transformHasChanged = true; }
    void setOrientation(const QQuaternion& orientation) { m_orientation = orientation; m_transformHasChanged = true; }
    void setScale(const QVector3D& scale) { m_scale = scale; m_transformHasChanged = true; }
    void setScale(float scale) { m_scale.setX(scale); m_scale.setY(scale); m_scale.setZ(scale);
                                 m_transformHasChanged = true; }
    void setScale(float x, float y, float z) { m_scale.setX(x); m_scale.setY(y); m_scale.setZ(z);
                                               m_transformHasChanged = true; }
    QQuaternion orientation() const { return m_orientation; }
    QVector3D scale() const { return m_scale; }
    bool transformHasChanged() const;
    std::size_t countParts() const { return m_parts.size(); }
    Part* part(std::size_t i) const { return m_parts[i]; }
    Part* addPart(QMesh* mesh = nullptr, QShPtr shader = QShPtr(nullptr),
                         bool clone = true);
    Part* addPart(QScrollEngineContext* parentContext, QGLPrimitiv::Primitives meshFlag, QShPtr shader);
    void addPart(Part* part, bool clone = true);
    QEntity* copy() const;
    QEntity* clone() const;
    void deletePart(std::size_t i);
    void deleteParts();
    void setAlpha(bool enable);
    void foreach_parts(const std::function<void(Part *)>& functor);
    void setShaderToParts(QShPtr shader);
    void setShaderToChildSprites(QShPtr shader);
    void setShaderToChildEntities(QShPtr shader);
    void setShaderToAll(QShPtr shader);
    void convertShaders(const std::function<QSh*(QSh*)>& convertFunction);
    QShPtr shader(std::size_t i) const;
    void setShaderAtPart(std::size_t i, QShPtr shader);
    void updateNormals();
    void updateNormalsWithChilds();
    QAnimation3D* animation() const { return m_animation; }
    void setAnimation(QAnimation3D* animation);
    void swapAnimation(QEntity* entity);
    void setToAnimation();
    void setAnimationTime(float time);
    void setAnimationSpeed(float speed);
    void setAnimationParameters(float time, float speed);
    bool haveAnimation() const;
    std::size_t index() const { return m_index; }

    void updateTransform();
    void updateTransform(bool& change);
    QVector3D transformPoint(const QVector3D& point) const;
    QVector3D transformDir(const QVector3D& dir) const;
    QVector3D inverseTransformPoint(const QVector3D& point) const;
    QVector3D inverseTransformDir(const QVector3D& dir) const;

    QBoundingBox boundingBox() const { return m_boundingBox; }
    QEntity* rootParentEntity() { return (m_parentEntity) ? m_parentEntity->rootParentEntity() : this; }

    void draw(QScrollEngineContext* context) override;

protected:
    friend class QScene;
    friend class QMesh;
    friend class QScrollEngineContext;
    friend class QSprite;
    friend class QLight;
    friend class QAnimation3D;

    std::size_t m_index;
    std::vector<Part*> m_parts;
    std::vector<QEntity*> m_childEntities;
    std::vector<QSprite*> m_childSprites;
    std::vector<QLight*> m_childLights;
    QString m_name;
    QAnimation3D* m_animation;

    QQuaternion m_orientation;
    QVector3D m_scale;

    QMatrix4x4 m_localMatrixWorld;

    inline void _freePart(std::size_t i);

    void _updateScene(QScene* scene);
    void _deleteChild(QSprite* sprite);
    void _deleteChild(QEntity* entity);
    void _deleteChild(QLight* light);

    void _updateLocalMatrixWorld();
    void _updateMatrixWorld();
    void _updateMatrixWorld(const QMatrix4x4 &parentMatrixWorld);
    void _updateTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _solveTransformFromParent(const QMatrix4x4& parentMatrixWorld);
    void _updateBoundingBox();
    void _mergeChildsBoundingBoxes();

    void _prevStepDrawing_solveTransformChilds(const QCamera3D* camera);
    void _prevStepDrawing_solveTransform(const QCamera3D* camera);
    void _prevStepDrawing_updateTransform(bool& change, const QCamera3D *camera);
    void _postStepDrawing(const QCamera3D* camera);
    void _solveTransformChilds();
    void _updateTransformChilds(bool& change);
    void _updateTransformChilds();

    void _updateAnimation();

};

}
#endif
