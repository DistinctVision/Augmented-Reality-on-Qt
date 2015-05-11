#ifndef QBOUNDINGBOX_H
#define QBOUNDINGBOX_H

#include <QVector3D>
#include <QMatrix4x4>

#include "QScrollEngine/QOtherMathFunctions.h"

namespace QScrollEngine {

class QBoundingBox
{
public:    
    void toPoint(const QVector3D& point) { _min = _max = point; }
    void toPoint(float x, float y, float z) { _min.setX(x); _min.setY(y); _min.setZ(z); _max = _min; }
    void toPoints(const QVector3D& pointA, const QVector3D& pointB) { toPoint(pointA); addPoint(pointB); }
    void set(const QBoundingBox& boundingBox, const QMatrix4x4& transform);
    void deactivate() { _min.setX(-9999998.0f); _max.setX(_min.x() - 1.0f); }
    bool isDeactivated() const { return (_max.x() < _min.x()); }
    bool isActivated() const { return (_max.x() >= _min.x()); }
    QBoundingBox() { deactivate(); }
    QBoundingBox(const QVector3D& point) { toPoint(point); }
    QBoundingBox(const QVector3D& pointA, const QVector3D& pointB) { toPoints(pointA, pointB); }
    QBoundingBox(const QBoundingBox& boundingBox, const QMatrix4x4& transform) { set(boundingBox, transform); }
    void addPoint_min(const QVector3D& point)
    {
        if (point.x() < _min.x())
            _min.setX(point.x());
        if (point.y() < _min.y())
            _min.setY(point.y());
        if (point.z() < _min.z())
            _min.setZ(point.z());
    }
    void addPoint_max(const QVector3D& point)
    {
        if (point.x() > _max.x())
            _max.setX(point.x());
        if (point.y() > _max.y())
            _max.setY(point.y());
        if (point.z() > _max.z())
            _max.setZ(point.z());
    }
    void addPoint(const QVector3D& point)
    {
        addPoint_min(point);
        addPoint_max(point);
    }
    void expand(float a)
    {
        _min.setX(_min.x() - a);
        _min.setY(_min.y() - a);
        _min.setZ(_min.z() - a);
        _max.setX(_max.x() + a);
        _max.setY(_max.y() + a);
        _max.setZ(_max.z() + a);
    }
    void expand(const QVector3D& a)
    {
        _min.setX(_min.x() - a.x());
        _min.setY(_min.y() - a.y());
        _min.setZ(_min.z() - a.z());
        _max.setX(_max.x() + a.x());
        _max.setY(_max.y() + a.y());
        _max.setZ(_max.z() + a.z());
    }
    void merge(const QBoundingBox& boundingBox) { addPoint_min(boundingBox.min()); addPoint_max(boundingBox.max()); }
    void merge(const QBoundingBox& boundingBox, const QMatrix4x4& transform);
    QVector3D max() const { return _max; }
    QVector3D min() const { return _min; }
    QVector3D getCenter() const { return (_max + _min) * 0.5f; }
    bool collision(const QBoundingBox& boundingBox) const
    {
        if (_min.x() <= boundingBox._max.x())
            if (_min.y() <= boundingBox._max.y())
                if (_min.z() <= boundingBox._max.z())
                    if (boundingBox._min.x() <= _max.x())
                        if (boundingBox._min.y() <= _max.y())
                            if (boundingBox._min.z() <= _max.z())
                                return true;
        return false;
    }
    QBoundingBox transform(const QMatrix4x4& transformMatrix) const;
    void transform(QBoundingBox& result, const QMatrix4x4& transformMatrix) const;
    float supportValue(const QVector3D& dir) const;
    bool collision(const QMatrix4x4& transform,
                   const QBoundingBox& boundingBoxB, const QMatrix4x4& transformB) const;
private:
    QVector3D _min;
    QVector3D _max;

};

}

#endif
