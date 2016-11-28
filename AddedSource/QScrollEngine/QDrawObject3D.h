#ifndef QDRAWOBJECT3D_H
#define QDRAWOBJECT3D_H

#include <QMatrix4x4>
#include <qgl.h>

#include "QScrollEngine/QBoundingBox.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QScene;
class QSceneObject3D;

class QDrawObject3D
{
public:
    QDrawObject3D()
    {
        m_drawMode = GL_TRIANGLES;
        m_sceneObject = nullptr;
        m_visibledForCamera = false;
        m_visibled = true;
    }
    QDrawObject3D(QSceneObject3D* sceneObject)
    {
        m_drawMode = GL_TRIANGLES;
        m_sceneObject = sceneObject;
        m_visibledForCamera = false;
        m_visibled = true;
    }
    QSceneObject3D* sceneObject() { return m_sceneObject; }
    const QSceneObject3D* sceneObject() const { return m_sceneObject; }
    bool visibleForCamera() const { return m_visibledForCamera; }
    QBoundingBox boundingBox() const { return m_boundingBox; }
    QVector3D centerOfBoundingBox() const { return m_centerOfBoundingBox; }
    inline GLenum drawMode() const { return m_drawMode; }
    void setDrawMode(GLenum mode) { m_drawMode = mode; }
    inline bool visibled() const { return m_visibled; }
    void setVisibled(bool visibled) { m_visibled = visibled; }

    virtual void draw(QScrollEngineContext* context) = 0;
    virtual ~QDrawObject3D() {}

protected:
    friend class QScrollEngineContext;
    friend class QScene;

    QSceneObject3D* m_sceneObject;
    bool m_visibledForCamera;
    QBoundingBox m_boundingBox;
    QVector3D m_centerOfBoundingBox;
    GLenum m_drawMode;
    bool m_visibled;
};

}
#endif
