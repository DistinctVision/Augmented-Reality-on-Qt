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

    QScrollEngineContext* parentContext() const { return m_parentContext; }
    QScene();
    QScene(QScrollEngineContext* parentContext, int order = 0);
    ~QScene();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    void setParentContext(QScrollEngineContext* parentContext);
    void clearScene();

    int order() const { return m_order; }
    void setOrder(int order);

    QBoundingBox boundingBox() const { return m_boundingBox; }
    void solveBoundingBox(float expand = 0.0f);

    void setAmbientColor(const QColor& color) { m_ambientColor = color; }
    void setAmbientColor(int r, int g, int b, int a = 255) { m_ambientColor.setRgb(r, g, b, a); }
    QColor ambientColor() const { return m_ambientColor; }

    void updateCameraInfo(QCamera3D* camera);
    CameraInfo cameraInfo() const { return m_cameraInfo; }
    QVector3D cameraPosition() const { return m_cameraInfo.position; }
    QQuaternion cameraOrientation() const { return m_cameraInfo.orientation; }
    QVector3D cameraLocalX() const { return m_cameraInfo.localX; }
    QVector3D cameraLocalY() const { return m_cameraInfo.localY; }
    QVector3D cameraLocalZ() const { return m_cameraInfo.localZ; }

    std::size_t countSprites() const { return  m_sprites.size(); }
    QSprite* sprite(std::size_t i) const { return m_sprites[i]; }
    std::size_t countEntities() const { return m_entities.size(); }
    QEntity* entity(std::size_t i) const { return m_entities[i]; }
    std::size_t countLights() const { return m_lights.size(); }
    QLight* light(std::size_t i) const { return m_lights[i]; }

    QEntity* findEntity(const QString& name) const;
    QEntity* findEntityWithChilds(const QString& name) const;

signals:
    void parentContextChanged();
    void beginDrawing();
    void beginDrawingAlphaObjects();
    void endDrawing();
    void deleting();

private:
    friend class QScrollEngineContext;
    friend class QSprite;
    friend class QEntity;
    friend class QLight;

    std::size_t m_index;
    int m_order;

    bool m_enabled;

    CameraInfo m_cameraInfo;
    QBoundingBox m_boundingBox;
    QColor m_ambientColor;

    QScrollEngineContext* m_parentContext;
    std::vector<QSprite*> m_sprites;
    std::vector<QEntity*> m_entities;
    std::vector<QLight*> m_lights;
    QMesh* m_quad;

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
