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
    friend class QScrollEngineContext;
    friend class QScene;

public:
    QDrawObject3D()
    {
        _drawMode = GL_TRIANGLES;
        _parentSceneObject = nullptr;
        _visibledForCamera = false;
        _visible = true;
    }
    QDrawObject3D(QSceneObject3D* sceneObject)
    {
        _drawMode = GL_TRIANGLES;
        _parentSceneObject = sceneObject;
        _visibledForCamera = false;
        _visible = true;
    }
    QSceneObject3D* parentSceneObject() const { return _parentSceneObject; }
    bool visibleForCamera() const { return _visibledForCamera; }
    QBoundingBox boundingBox() const { return _boundingBox; }
    QVector3D centerOfBoundingBox() const { return _centerOfBoundingBox; }
    inline GLenum drawMode() const { return _drawMode; }
    void setDrawMode(GLenum mode) { _drawMode = mode; }
    inline bool visible() const { return _visible; }
    void setVisible(bool visible) { _visible = visible; }

    virtual void draw(QScrollEngineContext* context) = 0;
    virtual ~QDrawObject3D() {}

protected:
    QSceneObject3D* _parentSceneObject;
    bool _visibledForCamera;
    QBoundingBox _boundingBox;
    QVector3D _centerOfBoundingBox;
    GLenum _drawMode;
    bool _visible;
};

}
#endif
