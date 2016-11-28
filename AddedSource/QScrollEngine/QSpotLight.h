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

    void setDirectional(const QVector3D& dir) { m_dir = dir.normalized(); }
    void setDirectional(float x, float y, float z) { m_dir = QVector3D(x, y, z).normalized(); }
    QVector3D directional() const { return m_dir; }
    void setAngle(float angle) { if (angle > 0.01f) { m_angle = angle; m_spotCutOff = qCos(m_angle); } }
    float angle() const { return m_angle; }

    float spotCutOff() const { return m_spotCutOff; }


    float intensityAtPoint(const QVector3D& point) const override
    {
        QVector3D d = point - m_position;
        float distance = d.length();
        if (distance < 0.0005f)
            return 1.0f;
        d /= distance;
        if (QVector3D::dotProduct(d, m_dir) < m_spotCutOff) {
            return (1.0f - distance / m_radius) * m_color.alphaF();
        }
        return -0.5f;
    }


private:
    QVector3D m_dir;
    float m_angle;
    float m_spotCutOff;
};

}
#endif
