#ifndef QENTITY_H
#define QENTITY_H

#include <functional>

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
    friend class QScene;
    friend class QMesh;
    friend class QScrollEngineContext;
    friend class QSprite;
    friend class QLight;
    friend class QAnimation3D;

public:
    class QPartEntity:
            public QDrawObject3D,
            public QShObject3D
    {
        friend class QEntity;
        friend class QScene;
        friend  class QScrollEngineContext;

    public:
        QPartEntity():QDrawObject3D(), QShObject3D(this) { _mesh = nullptr; }
        QPartEntity(QMesh* mesh, QSh* shader):QDrawObject3D(), QShObject3D(this)
        {
            _mesh = nullptr;
            setMesh(mesh);
            _shader = shader;
            if (_shader)
                _shader->setObject(this);
            else
                _shader = nullptr;
        }
        ~QPartEntity();
        QMesh* mesh() const { return _mesh; }
        void setMesh(QMesh* mesh);
        void setShader(QSh* shader) { delete _shader; _shader = shader; _shader->setObject(this); }
        void updateGlobalBoundingBox(const QMatrix4x4& transform);
        void draw(QScrollEngineContext* context) override;

    private:
        QMesh* _mesh;

        void _connectMeshToSceneObject();
        void _disconnectMeshFromSceneObject();
        void _setParentSceneObject(QSceneObject3D* sceneObject);
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
    unsigned int countEntityChilds() const { return _childEntities.size(); }
    QEntity* childEntity(unsigned int i) const { return _childEntities[i]; }
    unsigned int countSpriteChilds() const { return _childSprites.size(); }
    QSprite* childSprite(unsigned int i) const { return _childSprites[i]; }
    unsigned int countLightChilds() const { return _childLights.size(); }
    QLight* childLight(unsigned int i) const { return _childLights[i]; }
    QEntity* findChild(const QString& name);

    QString name() const { return _name; }
    void setName(const QString& name) { _name = name; }
    void setPosition(const QVector3D& position) { _position = position; _transformHasChanged = true; }
    void setPosition(float x, float y, float z) { _position.setX(x); _position.setY(y);
                                                  _position.setZ(z); _transformHasChanged = true; }
    void setOrientation(const QQuaternion& orientation) { _orientation = orientation; _transformHasChanged = true; }
    void setScale(const QVector3D& scale) { _scale = scale; _transformHasChanged = true; }
    void setScale(float scale) { _scale.setX(scale); _scale.setY(scale); _scale.setZ(scale);
                                 _transformHasChanged = true; }
    void setScale(float x, float y, float z) { _scale.setX(x); _scale.setY(y); _scale.setZ(z);
                                               _transformHasChanged = true; }
    QQuaternion orientation() const { return _orientation; }
    QVector3D scale() const { return _scale; }
    bool transformHasChanged() const;
    unsigned int countParts() const { return _parts.size(); }
    QPartEntity* part(int i) const { return _parts[i]; }
    QPartEntity* addPart(QMesh* mesh = nullptr, QSh* shader = nullptr, bool clone = true);
    QPartEntity* addPart(QScrollEngineContext* parentContext, QGLPrimitiv::Primitives meshFlag, QSh* shader);
    void addPart(QPartEntity* part, bool clone = true);
    QEntity* copy() const;
    QEntity* clone() const;
    void deletePart(unsigned int i);
    void deleteParts();
    void setAlpha(bool enable);
    void setShaderToParts(QSh* shader);
    void setShaderToChildSprites(QSh* shader);
    void setShaderToChildEntities(QSh* shader);
    void setShaderToAll(QSh* shader);
    void convertShaders(const std::function<QSh*(QSh*)> &convertFunction);
    QSh* shader(unsigned int i) const;
    void setShaderAtPart(unsigned int i, QSh* shader);
    void updateNormals();
    void updateNormalsWithChilds();
    QAnimation3D* animation() const { return _animation; }
    void setAnimation(QAnimation3D* animation);
    void swapAnimation(QEntity* entity);
    void setToAnimation();
    void setAnimationTime(float time);
    void setAnimationSpeed(float speed);
    void setAnimationParameters(float time, float speed);
    bool haveAnimation() const;
    unsigned int index() const { return _index; }

    void updateTransform();
    void updateTransform(bool& change);
    QVector3D getTransformPoint(const QVector3D& point) const;
    QVector3D getTransformDir(const QVector3D& dir) const;
    QVector3D getInverseTransformPoint(const QVector3D& point) const;
    QVector3D getInverseTransformDir(const QVector3D& dir) const;

    QBoundingBox boundingBox() const { return _boundingBox; }
    QEntity* rootParentEntity() { return (_parentEntity) ? _parentEntity->rootParentEntity() : this; }

    void draw(QScrollEngineContext* context) override;

protected:
    unsigned int _index;
    std::vector<QPartEntity*> _parts;
    std::vector<QEntity*> _childEntities;
    std::vector<QSprite*> _childSprites;
    std::vector<QLight*> _childLights;
    QString _name;
    QAnimation3D* _animation;

    QQuaternion _orientation;
    QVector3D _scale;

    QMatrix4x4 _localMatrixWorld;

    inline void _freePart(unsigned int i);

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
