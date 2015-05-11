#ifndef QSHOBJECT3D_H
#define QSHOBJECT3D_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Color.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QScene;
class QDrawObject3D;

class QShObject3D
{
    friend class QScrollEngineContext;
    friend class QScene;

public:
    QShObject3D(QDrawObject3D* object)
    {
        _isAlpha = false;
        _shader = new QSh_Color();
        _shader->setObject(object);
    }
    virtual ~QShObject3D() { delete _shader; }

    bool isAlpha() const { return _isAlpha; }
    void setAlpha(bool enable) { _isAlpha = enable; }

    QSh* shader() const { return _shader; }

protected:
    bool _isAlpha;
    QSh* _shader;
};

}
#endif
