#include "QScrollEngine/QSpotLight.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QScene.h"
#include <qmath.h>

namespace QScrollEngine {

QSpotLight::QSpotLight(bool isStatic):
    QLight(isStatic)
{
    _type = Spot;
    setDirectional(QVector3D(1.0f, 0.0f, 0.0f));
    setAngle(0.1f);
}

QSpotLight::QSpotLight(QScene* scene, bool isStatic):
    QLight(isStatic)
{
    _type = Spot;
    setDirectional(QVector3D(1.0f, 0.0f, 0.0f));
    setAngle(0.1f);
    setParentScene(scene);
}

}
