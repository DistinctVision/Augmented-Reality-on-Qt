#include "QScrollEngine/QBoundingBox.h"

namespace QScrollEngine {

QBoundingBox QBoundingBox::transform(const QMatrix4x4& transformMatrix) const
{
    QBoundingBox boundingBox;
    transform(boundingBox, transformMatrix);
    return boundingBox;
}

void QBoundingBox::transform(QBoundingBox& result, const QMatrix4x4& transformMatrix) const
{
    result.toPoint(QOtherMathFunctions::transform(transformMatrix, _min));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, _max));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_min.x(), _min.y(), _max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_min.x(), _max.y(), _min.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_min.x(), _max.y(), _max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_max.x(), _min.y(), _min.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_max.x(), _min.y(), _max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(_max.x(), _max.y(), _min.z())));
}

void QBoundingBox::set(const QBoundingBox& boundingBox, const QMatrix4x4& transform)
{
    boundingBox.transform(*this, transform);
}

void QBoundingBox::merge(const QBoundingBox& boundingBox, const QMatrix4x4& transform)
{
    addPoint(QOtherMathFunctions::transform(transform,
                                      boundingBox._min));
    addPoint(QOtherMathFunctions::transform(transform,
                                      boundingBox._max));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._min.x(), boundingBox._min.y(), boundingBox._max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._min.x(), boundingBox._max.y(), boundingBox._min.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._min.x(), boundingBox._max.y(), boundingBox._max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._max.x(), boundingBox._min.y(), boundingBox._min.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._max.x(), boundingBox._min.y(), boundingBox._max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox._max.x(), boundingBox._max.y(), boundingBox._min.z())));
}

float QBoundingBox::supportValue(const QVector3D& dir) const
{
    QVector3D t(_min.x(), _min.y(), _max.z());
    float svalue = QVector3D::dotProduct(t, dir), v;
    t.setY(_max.y());//min.x max.y max.z
    v = QVector3D::dotProduct(t, dir);
    if (v > svalue)
        svalue = v;
    t.setZ(_min.z());//min.x max.y min.z
    v = QVector3D::dotProduct(t, dir);
    if (v > svalue)
        svalue = v;
    t.setX(_max.x());//max.x max.y min.z
    v = QVector3D::dotProduct(t, dir);
    if (v > svalue)
        svalue = v;
    t.setY(_min.y());//max.x min.y min.z
    v = QVector3D::dotProduct(t, dir);
    if (v > svalue)
        svalue = v;
    t.setZ(_max.z());//max.x min.y max.z
    v = QVector3D::dotProduct(t, dir);
    if (v > svalue)
        svalue = v;
    v = QVector3D::dotProduct(_min, dir);
    if (v > svalue)
        svalue = v;
    v = QVector3D::dotProduct(_max, dir);
    if (v > svalue)
        return v;
    return svalue;
}

bool QBoundingBox::collision(const QMatrix4x4& transform,
                             const QBoundingBox& boundingBoxB, const QMatrix4x4& transformB) const
{
    // The Separation of Axis Theorem (SAT)
    QVector3D A_localX(transform(0, 0), transform(1, 0), transform(2, 0));
    QVector3D A_localY(transform(0, 1), transform(1, 1), transform(2, 1));
    QVector3D A_localZ(transform(0, 2), transform(1, 2), transform(2, 2));
    QVector3D B_localX(transformB(0, 0), transformB(1, 0), transformB(2, 0));
    QVector3D B_localY(transformB(0, 1), transformB(1, 1), transformB(2, 1));
    QVector3D B_localZ(transformB(0, 2), transformB(1, 2), transformB(2, 2));
    //A
    QVector3D tempDir = QOtherMathFunctions::transformTransposed(transformB, A_localX);
    if ((_min.x() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((_max.x() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transformB, A_localY);
    if ((_min.y() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((_max.y() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transformB, A_localZ);
    if ((_min.z() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((_max.z() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    //B
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localX);
    if ((boundingBoxB._min.x() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB._max.x() + supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localY);
    if ((boundingBoxB._min.y() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB._max.y() + supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localZ);
    if ((boundingBoxB._min.z() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB._max.z() + supportValue(-tempDir)) > 0.0f)
        return false;
    //edge
    QVector3D edge[6];
    edge[0] = QVector3D::crossProduct(A_localX, B_localX);
    edge[1] = QVector3D::crossProduct(A_localX, B_localY);
    edge[2] = QVector3D::crossProduct(A_localX, B_localZ);
    edge[3] = QVector3D::crossProduct(A_localY, B_localY);
    edge[4] = QVector3D::crossProduct(A_localY, B_localZ);
    edge[5] = QVector3D::crossProduct(A_localZ, B_localZ);
    //edge dirs
    float sA, sB;
    for (int i=0; i<6; ++i) {
        sA = supportValue(QOtherMathFunctions::transformTransposed(transform, edge[i]));
        sB = boundingBoxB.supportValue(QOtherMathFunctions::transformTransposed(transformB, -edge[i]));
        if ((sA + sB) > 0.0f)
            return false;
        sA = supportValue(QOtherMathFunctions::transformTransposed(transform, -edge[i]));
        sB = boundingBoxB.supportValue(QOtherMathFunctions::transformTransposed(transformB, edge[i]));
        if ((sA + sB) > 0.0f)
            return false;
    }
    return true;
}

}
