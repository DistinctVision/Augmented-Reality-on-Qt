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
    result.toPoint(QOtherMathFunctions::transform(transformMatrix, m_min));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, m_max));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_min.x(), m_min.y(), m_max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_min.x(), m_max.y(), m_min.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_min.x(), m_max.y(), m_max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_max.x(), m_min.y(), m_min.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_max.x(), m_min.y(), m_max.z())));
    result.addPoint(QOtherMathFunctions::transform(transformMatrix, QVector3D(m_max.x(), m_max.y(), m_min.z())));
}

void QBoundingBox::set(const QBoundingBox& boundingBox, const QMatrix4x4& transform)
{
    boundingBox.transform(*this, transform);
}

void QBoundingBox::merge(const QBoundingBox& boundingBox, const QMatrix4x4& transform)
{
    addPoint(QOtherMathFunctions::transform(transform,
                                      boundingBox.m_min));
    addPoint(QOtherMathFunctions::transform(transform,
                                      boundingBox.m_max));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_min.x(), boundingBox.m_min.y(), boundingBox.m_max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_min.x(), boundingBox.m_max.y(), boundingBox.m_min.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_min.x(), boundingBox.m_max.y(), boundingBox.m_max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_max.x(), boundingBox.m_min.y(), boundingBox.m_min.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_max.x(), boundingBox.m_min.y(), boundingBox.m_max.z())));
    addPoint(QOtherMathFunctions::transform(transform,
                                      QVector3D(boundingBox.m_max.x(), boundingBox.m_max.y(), boundingBox.m_min.z())));
}

float QBoundingBox::supportValue(const QVector3D& dir) const
{
    float svalue = m_min.x() * dir.x() + m_min.y() * dir.y() + m_min.z() * dir.z(), v;
    v = m_min.x() * dir.x() + m_min.y() * dir.y() + m_max.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_min.x() * dir.x() + m_max.y() * dir.y() + m_min.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_max.x() * dir.x() + m_min.y() * dir.y() + m_min.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_max.x() * dir.x() + m_max.y() * dir.y() + m_min.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_min.x() * dir.x() + m_max.y() * dir.y() + m_max.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_max.x() * dir.x() + m_min.y() * dir.y() + m_max.z() * dir.z();
    if (v > svalue)
        svalue = v;
    v = m_max.x() * dir.x() + m_max.y() * dir.y() + m_max.z() * dir.z();
    if (v > svalue)
        svalue = v;
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
    if ((m_min.x() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((m_max.x() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transformB, A_localY);
    if ((m_min.y() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((m_max.y() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transformB, A_localZ);
    if ((m_min.z() + boundingBoxB.supportValue(tempDir)) > 0.0f)
        return false;
    if ((m_max.z() + boundingBoxB.supportValue(-tempDir)) > 0.0f)
        return false;
    //B
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localX);
    if ((boundingBoxB.m_min.x() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB.m_max.x() + supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localY);
    if ((boundingBoxB.m_min.y() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB.m_max.y() + supportValue(-tempDir)) > 0.0f)
        return false;
    tempDir = QOtherMathFunctions::transformTransposed(transform, B_localZ);
    if ((boundingBoxB.m_min.z() + supportValue(tempDir)) > 0.0f)
        return false;
    if ((boundingBoxB.m_max.z() + supportValue(-tempDir)) > 0.0f)
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
