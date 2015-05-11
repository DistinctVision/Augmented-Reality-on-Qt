#ifndef QSCENE_H
#define QSCENE_H

#include <QObject>
#include <vector>
#include <QQuaternion>
#include <QColor>
#include <QOpenGLShaderProgram>

#include "QScrollEngine/QCamera3D.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QShObject3D.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QBoundingBox.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QSprite;
class QMesh;
class QLight;

class QScene:
        public QObject
{
    Q_OBJECT
    friend class QScrollEngineContext;
    friend class QSprite;
    friend class QEntity;
    friend class QLight;

public:
    typedef struct CameraInfo
    {
        QVector3D position;
        QQuaternion orientation;
        QVector3D localX;
        QVector3D localY;
        QVector3D localZ;
    } CameraInfo;

public:
    QVector3D position;
    QQuaternion orientation;
    QVector3D scale;

    int order() const { return _order; }
    void setOrder(int order);

    QBoundingBox boundingBox() const { return _boundingBox; }
    void solveBoundingBox(float expand = 0.0f);

    void setAmbientColor(const QColor& color) { _ambientColor = color; }
    void setAmbientColor(int r, int g, int b, int a = 255) { _ambientColor.setRgb(r, g, b, a); }
    QColor ambientColor() const { return _ambientColor; }

    void updateCameraInfo(QCamera3D* camera);
    CameraInfo cameraInfo() const { return _cameraInfo; }
    QVector3D cameraPosition() const { return _cameraInfo.position; }
    QQuaternion cameraOrientation() const { return _cameraInfo.orientation; }
    QVector3D cameraLocalX() const { return _cameraInfo.localX; }
    QVector3D cameraLocalY() const { return _cameraInfo.localY; }
    QVector3D cameraLocalZ() const { return _cameraInfo.localZ; }

    QScrollEngineContext* parentContext() const { return _parentContext; }
    QScene(QScrollEngineContext* parentContext = nullptr, int order = 0);
    virtual ~QScene();
    void setParentContext(QScrollEngineContext* parentContext);

    int countSprites() const { return  _sprites.size(); }
    QSprite* sprite(int i) const { return _sprites[i]; }
    int countEntities() const { return _entities.size(); }
    QEntity* entity(int i) const { return _entities[i]; }
    int countLights() const { return _lights.size(); }
    QLight* light(int i) const { return _lights[i]; }

    QEntity* findEntity(const QString& name) const;
    QEntity* findEntityWithChilds(const QString& name) const;

signals:
    void beginDrawing();
    void beginDrawingAlphaObjects();
    void endDrawing();
    void deleting();

private:
    int _index;
    int _order;

    CameraInfo _cameraInfo;
    QBoundingBox _boundingBox;
    QColor _ambientColor;

    int _currentResetedSprite;
    int _currentResetedEntity;

    QScrollEngineContext* _parentContext;
    std::vector<QSprite*> _sprites;
    std::vector<QEntity*> _entities;
    std::vector<QLight*> _lights;
    QMesh* _quad;

    void _addSprite(QSprite* sprite);
    void _deleteSprite(QSprite* sprite);
    void _addEntity(QEntity* entity);
    void _deleteEntity(QEntity* entity);
    void _addLight(QLight* light);
    void _deleteLight(QLight* light);

    void _updateGlobalPosition(QSprite* sprite);
    void _spriteToDrawing(QSprite* sprite, const QCamera3D* camera);
    void _entityToDrawing(QEntity* entity, const QCamera3D* camera);

    void _update();

};

}
#endif
