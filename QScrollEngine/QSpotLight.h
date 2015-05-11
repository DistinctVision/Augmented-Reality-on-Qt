#ifndef QSPOTLIGHT_H
#define QSPOTLIGHT_H

#include <qmath.h>
#include <QVector3D>

#include "QScrollEngine/QLight.h"

namespace QScrollEngine {

class QSpotLight:
        public QLight
{
public:
    QSpotLight(bool isStatic = false);
    QSpotLight(QScene* scene, bool isStatic = false);

    void setDirectional(const QVector3D& dir) { _dir = dir.normalized(); }
    void setDirectional(float x, float y, float z) { _dir = QVector3D(x, y, z).normalized(); }
    QVector3D directional() const { return _dir; }
    void setAngle(float angle) { if (angle > 0.01f) { _angle = angle; _spotCutOff = qCos(_angle); } }
    float angle() const { return _angle; }

    float spotCutOff() const { return _spotCutOff; }


    float intensityAtPoint(const QVector3D& point) const override
    {
        QVector3D d = point - _position;
        float distance = d.length();
        if (distance < 0.0005f)
            return 1.0f;
        d /= distance;
        if (QVector3D::dotProduct(d, _dir) < _spotCutOff) {
            return (1.0f - distance / _radius) * _color.alphaF();
        }
        return -0.5f;
    }


private:
    QVector3D _dir;
    float _angle;
    float _spotCutOff;
};

}
#endif
