#ifndef QSH_H
#define QSH_H

#include <QSharedPointer>
#include <QOpenGLShaderProgram>
#include <vector>
#include <cassert>

#define QSH_ASSERT(x) assert(x)
//#define QSH_ASSERT(x) static_cast<void>(x)

namespace QScrollEngine {

class QShObject3D;
class QDrawObject3D;
class QSceneObject3D;
class QScrollEngineContext;

class QSh;
typedef QSharedPointer<QSh> QShPtr;

class QSh
{
public:
    enum class Type : int
    {
        _None = -1,
        Color = 0,
        ColoredPart = 1,
        Texture = 2,
        Light = 3,
        Refraction_FallOff = 4,
        Sphere_Texture = 5,
        Sphere_Light = 6,
        LightVC = 7
    };

    enum class VertexAttributes: int
    {
        TextureCoords = 2,
        Normals = 3,
        RgbColors = 4
    };

public:
    QSh() { m_currentTypeIndex = static_cast<int>(Type::_None); m_subTypeIndex = 0; }
    int subTypeIndex() const { return m_subTypeIndex; }
    int currentTypeIndex() const { return m_currentTypeIndex; }
    virtual int typeIndex() const = 0;
    virtual std::vector<VertexAttributes> attributes() const = 0;
    virtual void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& programs) = 0;
    virtual void preprocess(const QDrawObject3D* ) { }
    virtual bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program,
                     const QDrawObject3D* drawObject) = 0;
    virtual ~QSh() {}

    virtual QShPtr copy() const = 0;

protected:
    friend class QScrollEngineContext;
    friend class QShObject;

    int m_currentTypeIndex;
    int m_subTypeIndex;
};


}
#endif
