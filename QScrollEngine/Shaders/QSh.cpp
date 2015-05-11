#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_All.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QShObject3D.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QDrawObject3D.h"

namespace QScrollEngine {

void QSh::setObject(QDrawObject3D* object)
{
    _drawObject = object;
    if (_drawObject) {
        setSceneObject(_drawObject->parentSceneObject());
        return;
    }
    _drawObject = nullptr;
    _sceneObject = nullptr;
}

}
