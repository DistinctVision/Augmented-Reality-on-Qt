#ifndef QBOUNDINGBOX_H
#define QBOUNDINGBOX_H

#include <QVector3D>
#include <QMatrix4x4>

#include "QScrollEngine/QOtherMathFunctions.h"

namespace QScrollEngine {

class QBoundingBox
{
public:    
    void toPoint(const QVector3D& point) { m_min = m_max = point; }
    void toPoint(float x, float y, float z) { m_min.setX(x); m_min.setY(y); m_min.setZ(z); m_max = m_min; }
    void toPoints(const QVector3D& pointA, const QVector3D& pointB) { toPoint(pointA); addPoint(pointB); }
    void set(const QBoundingBox& boundingBox, const QMatrix4x4& transform);
    void deactivate() { m_min.setX(-9999998.0f); m_max.setX(m_min.x() - 1.0f); }
    bool isDeactivated() const { return (m_max.x() < m_min.x()); }
    bool isActivated() const { return (m_max.x() >= m_min.x()); }
    QBoundingBox() { deactivate(); }
    QBoundingBox(const QVector3D& point) { toPoint(point); }
    QBoundingBox(const QVector3D& pointA, const QVector3D& pointB) { toPoints(pointA, pointB); }
    QBoundingBox(const QBoundingBox& boundingBox, const QMatrix4x4& transform) { set(boundingBox, transform); }
    void addPoint_min(const QVector3D& point)
    {
        if (point.x() < m_min.x())
            m_min.setX(point.x());
        if (point.y() < m_min.y())
            m_min.setY(point.y());
        if (point.z() < m_min.z())
            m_min.setZ(point.z());
    }
    void addPoint_max(const QVector3D& point)
    {
        if (point.x() > m_max.x())
            m_max.setX(point.x());
        if (point.y() > m_max.y())
            m_max.setY(point.y());
        if (point.z() > m_max.z())
            m_max.setZ(point.z());
    }
    void addPoint(const QVector3D& point)
    {
        addPoint_min(point);
        addPoint_max(point);
    }
    void expand(float a)
    {
        m_min.setX(m_min.x() - a);
        m_min.setY(m_min.y() - a);
        m_min.setZ(m_min.z() - a);
        m_max.setX(m_max.x() + a);
        m_max.setY(m_max.y() + a);
        m_max.setZ(m_max.z() + a);
    }
    void expand(const QVector3D& a)
    {
        m_min.setX(m_min.x() - a.x());
        m_min.setY(m_min.y() - a.y());
        m_min.setZ(m_min.z() - a.z());
        m_max.setX(m_max.x() + a.x());
        m_max.setY(m_max.y() + a.y());
        m_max.setZ(m_max.z() + a.z());
    }
    QBoundingBox expanded(float a) const
    {
        QBoundingBox bb = *this;
        bb.expand(a);
        return bb;
    }
    QBoundingBox expanded(const QVector3D& a) const
    {
        QBoundingBox bb = *this;
        bb.expand(a);
        return bb;
    }

    void merge(const QBoundingBox& boundingBox) { addPoint_min(boundingBox.min()); addPoint_max(boundingBox.max()); }
    void merge(const QBoundingBox& boundingBox, const QMatrix4x4& transform);
    void merged(const QBoundingBox& boundingBox) const { QBoundingBox bb = *this; bb.merge(boundingBox); }
    QBoundingBox merged(const QBoundingBox& boundingBox, const QMatrix4x4& transform) const
    {
        QBoundingBox bb = *this;
        bb.merge(boundingBox, transform);
        return bb;
    }
    QVector3D max() const { return m_max; }
    QVector3D min() const { return m_min; }
    QVector3D getCenter() const { return (m_max + m_min) * 0.5f; }
    bool collision(const QBoundingBox& boundingBox) const
    {
        if (m_min.x() <= boundingBox.m_max.x())
            if (m_min.y() <= boundingBox.m_max.y())
                if (m_min.z() <= boundingBox.m_max.z())
                    if (boundingBox.m_min.x() <= m_max.x())
                        if (boundingBox.m_min.y() <= m_max.y())
                            if (boundingBox.m_min.z() <= m_max.z())
                                return true;
        return false;
    }
    QBoundingBox intersection(const QBoundingBox& boundingBox) const
    {
        QVector3D min1 = boundingBox.min();
        QVector3D max1 = boundingBox.max();
        return QBoundingBox(QVector3D(qMax(min1.x(), m_min.x()),
                                      qMax(min1.y(), m_min.y()),
                                      qMax(min1.z(), m_min.z())),
                            QVector3D(qMin(max1.x(), m_max.x()),
                                      qMin(max1.y(), m_max.y()),
                                      qMin(max1.z(), m_max.z())));
    }
    QBoundingBox transform(const QMatrix4x4& transformMatrix) const;
    void transform(QBoundingBox& result, const QMatrix4x4& transformMatrix) const;
    float supportValue(const QVector3D& dir) const;
    bool collision(const QMatrix4x4& transform,
                   const QBoundingBox& boundingBoxB, const QMatrix4x4& transformB) const;
    bool collision(const QVector3D& point) const
    {
        if (m_min.x() < point.x())
            if (m_min.y() < point.y())
                if (m_min.z() < point.z())
                    if (m_max.x() > point.x())
                        if (m_max.y() > point.y())
                            if (m_max.z() > point.z())
                                return true;
        return false;
    }

    float area() const
    {
        return (m_max.x() - m_min.x()) * (m_max.y() - m_min.y()) * (m_max.z() - m_min.z());
    }

private:
    QVector3D m_min;
    QVector3D m_max;

};

}

#endif
