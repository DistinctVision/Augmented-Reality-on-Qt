#ifndef QSHOBJECT3D_H
#define QSHOBJECT3D_H

#include <QSharedPointer>

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Color.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QScene;
class QDrawObject3D;

class QShObject3D
{
public:
    QShObject3D()
    {
        m_isAlpha = false;
        m_shader.reset(new QSh_Color());
    }
    QShObject3D(QShPtr shader)
    {
        m_isAlpha = false;
        m_shader = shader;
    }

    bool isAlpha() const { return m_isAlpha; }
    void setAlpha(bool enable) { m_isAlpha = enable; }

    void setShader(QShPtr shader)
    {
        m_shader = shader;
    }
    QShPtr shader() const { return m_shader; }

protected:
    friend class QScrollEngineContext;
    friend class QScene;

    bool m_isAlpha;
    QShPtr m_shader;
};

}
#endif
