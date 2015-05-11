#ifndef QSH_H
#define QSH_H

#include <QColor>
#include <QSharedPointer>
#include <QOpenGLShaderProgram>
#include <vector>
#include <cassert>

//#define QSH_ASSERT(x) assert(x)
#define QSH_ASSERT(x) static_cast<void>(x)

namespace QScrollEngine {

class QShObject3D;
class QDrawObject3D;
class QSceneObject3D;
class QScrollEngineContext;

class QSh
{
    friend class QScrollEngineContext;
    friend class QShObject;
public:
    enum Type : int
    {
        _None = -1,
        Color = 0,
        ColoredPart = 1,
        Texture1 = 2,
        Light = 3,
        Refraction_FallOff = 4,
        Sphere_Texture1 = 5,
        Sphere_Light = 6
    };

protected:
    int _currentIndexType;
    int _subIndexType;
    QSceneObject3D* _sceneObject;
    QDrawObject3D* _drawObject;

public:
    void setObject(QDrawObject3D* object);
    void setSceneObject(QSceneObject3D* sceneObject) { _sceneObject = sceneObject; }
    QSh() { _currentIndexType = _None; _subIndexType = 0; }
    int subIndexType() const { return _subIndexType; }
    int currentIndexType() const { return _currentIndexType; }
    QDrawObject3D* object() const { return _drawObject; }
    QSceneObject3D* sceneObject() const { return _sceneObject; }
    virtual int indexType() const = 0;
    virtual void preprocess() { }
    virtual void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& program) = 0;
    virtual bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) = 0;
    virtual ~QSh() {}

    virtual QSh* copy() const = 0;
};

}
#endif
