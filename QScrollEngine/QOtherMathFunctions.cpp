#include "QScrollEngine/QOtherMathFunctions.h"

namespace QScrollEngine {

const float QOtherMathFunctions::EPS_omf = 0.0005f;

QMatrix4x4 QOtherMathFunctions::invertedWorldMatrix(const QMatrix4x4& matrix)
{
    QMatrix4x4 result;
    result(0, 0) = matrix(0, 0);
    result(0, 1) = matrix(1, 0);
    result(0, 2) = matrix(2, 0);
    result(1, 0) = matrix(0, 1);
    result(1, 1) = matrix(1, 1);
    result(1, 2) = matrix(2, 1);
    result(2, 0) = matrix(0, 2);
    result(2, 1) = matrix(1, 2);
    result(2, 2) = matrix(2, 2);
    result(0, 3) = - (matrix(0, 0) * matrix(0, 3) + matrix(1, 0) * matrix(1, 3) + matrix(2, 0) * matrix(2, 3));
    result(1, 3) = - (matrix(0, 1) * matrix(0, 3) + matrix(1, 1) * matrix(1, 3) + matrix(2, 1) * matrix(2, 3));
    result(2, 3) = - (matrix(0, 2) * matrix(0, 3) + matrix(1, 2) * matrix(1, 3) + matrix(2, 2) * matrix(2, 3));
    result(3, 0) = 0.0f;
    result(3, 1) = 0.0f;
    result(3, 2) = 0.0f;
    result(3, 3) = 1.0f;
    return result;
}

QVector2D QOtherMathFunctions::transform(const QMatrix2x2& matrix, const QVector2D& vector)
{
    return QVector2D(vector.x() * matrix(0, 0) + vector.y() * matrix(0, 1),
                     vector.x() * matrix(1, 0) + vector.y() * matrix(1, 1));
}

QVector2D QOtherMathFunctions::transform(const QMatrix3x3& matrix, const QVector2D& vector)
{
    return QVector2D(vector.x() * matrix(0, 0) + vector.y() * matrix(0, 1) + matrix(0, 2),
                     vector.x() * matrix(1, 0) + vector.y() * matrix(1, 1) + matrix(1, 2));
}

QVector3D QOtherMathFunctions::transform(const QMatrix3x3& matrix, const QVector3D& vector)
{
    return QVector3D(vector.x() * matrix(0, 0) + vector.y() * matrix(0, 1) + vector.z() * matrix(0, 2),
                     vector.x() * matrix(1, 0) + vector.y() * matrix(1, 1) + vector.z() * matrix(1, 2),
                     vector.x() * matrix(2, 0) + vector.y() * matrix(2, 1) + vector.z() * matrix(2, 2));
}

QVector3D QOtherMathFunctions::transform(const QMatrix4x4& matrix, const QVector3D& vector)
{
    return QVector3D(vector.x() * matrix(0, 0) + vector.y() * matrix(0, 1) + vector.z() * matrix(0, 2) + matrix(0, 3),
                     vector.x() * matrix(1, 0) + vector.y() * matrix(1, 1) + vector.z() * matrix(1, 2) + matrix(1, 3),
                     vector.x() * matrix(2, 0) + vector.y() * matrix(2, 1) + vector.z() * matrix(2, 2) + matrix(2, 3));
}

QVector2D QOtherMathFunctions::transformTransposed(const QMatrix2x2& matrix, const QVector2D& vector)
{
    return QVector2D(vector.x() * matrix(0, 0) + vector.y() * matrix(1, 0) + matrix(2, 0),
                     vector.x() * matrix(0, 1) + vector.y() * matrix(1, 1) + matrix(2, 1));
}

QVector2D QOtherMathFunctions::transformTransposed(const QMatrix3x3& matrix, const QVector2D& vector)
{
    return QVector2D(vector.x() * matrix(0, 0) + vector.y() * matrix(1, 0) + matrix(2, 0),
                     vector.x() * matrix(0, 1) + vector.y() * matrix(1, 1) + matrix(2, 1));
}

QVector3D QOtherMathFunctions::transformTransposed(const QMatrix3x3& matrix, const QVector3D& vector)
{
    return QVector3D(vector.x() * matrix(0, 0) + vector.y() * matrix(1, 0) + vector.z() * matrix(2, 0),
                     vector.x() * matrix(0, 1) + vector.y() * matrix(1, 1) + vector.z() * matrix(2, 1),
                     vector.x() * matrix(0, 2) + vector.y() * matrix(1, 2) + vector.z() * matrix(2, 2));
}

QVector3D QOtherMathFunctions::transformTransposed(const QMatrix4x4& matrix, const QVector3D& vector)
{
    return QVector3D(vector.x() * matrix(0, 0) + vector.y() * matrix(1, 0) + vector.z() * matrix(2, 0),
                     vector.x() * matrix(0, 1) + vector.y() * matrix(1, 1) + vector.z() * matrix(2, 1),
                     vector.x() * matrix(0, 2) + vector.y() * matrix(1, 2) + vector.z() * matrix(2, 2));
}


void QOtherMathFunctions::quaternionToMatrix(const QQuaternion& quaternion, QMatrix3x3& matrix)
{
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    matrix(0, 0) = 1.0f - 2.0f * (yy + zz);
    matrix(1, 0) = 2.0f * (xy + zw);
    matrix(2, 0) = 2.0f * (xz - yw);
    matrix(0, 1) = 2.0f * (xy - zw);
    matrix(1, 1) = 1.0f - 2.0f * (xx + zz);
    matrix(2, 1) = 2.0f * (yz + xw);
    matrix(0, 2) = 2.0f * (xz + yw);
    matrix(1, 2) = 2.0f * (yz - xw);
    matrix(2, 2) = 1.0f - 2.0f * (xx + yy);
}

void QOtherMathFunctions::quaternionToMatrix(const QQuaternion& quaternion, QMatrix4x4& matrix)
{
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    matrix(0, 0) = 1.0f - 2.0f * (yy + zz);
    matrix(1, 0) = 2.0f * (xy + zw);
    matrix(2, 0) = 2.0f * (xz - yw);
    matrix(0, 1) = 2.0f * (xy - zw);
    matrix(1, 1) = 1.0f - 2.0f * (xx + zz);
    matrix(2, 1) = 2.0f * (yz + xw);
    matrix(0, 2) = 2.0f * (xz + yw);
    matrix(1, 2) = 2.0f * (yz - xw);
    matrix(2, 2) = 1.0f - 2.0f * (xx + yy);
}

void QOtherMathFunctions::quaternionToMatrix(const QQuaternion& quaternion, const QVector3D& scale, QMatrix3x3& matrix)
{
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    matrix(0, 0) = (1.0f - 2.0f * (yy + zz)) * scale.x();
    matrix(1, 0) = (2.0f * (xy + zw)) * scale.x();
    matrix(2, 0) = (2.0f * (xz - yw)) * scale.x();
    matrix(0, 1) = (2.0f * (xy - zw)) * scale.y();
    matrix(1, 1) = (1.0f - 2.0f * (xx + zz)) * scale.y();
    matrix(2, 1) = (2.0f * (yz + xw)) * scale.y();
    matrix(0, 2) = (2.0f * (xz + yw)) * scale.z();
    matrix(1, 2) = (2.0f * (yz - xw)) * scale.z();
    matrix(2, 2) = (1.0f - 2.0f * (xx + yy)) * scale.z();
}

void QOtherMathFunctions::quaternionToMatrix(const QQuaternion& quaternion, const QVector3D& scale, QMatrix4x4& matrix)
{
    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.scalar();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.scalar();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.scalar();
    matrix(0, 0) = (1.0f - 2.0f * (yy + zz)) * scale.x();
    matrix(1, 0) = (2.0f * (xy + zw)) * scale.x();
    matrix(2, 0) = (2.0f * (xz - yw)) * scale.x();
    matrix(0, 1) = (2.0f * (xy - zw)) * scale.y();
    matrix(1, 1) = (1.0f - 2.0f * (xx + zz)) * scale.y();
    matrix(2, 1) = (2.0f * (yz + xw)) * scale.y();
    matrix(0, 2) = (2.0f * (xz + yw)) * scale.z();
    matrix(1, 2) = (2.0f * (yz - xw)) * scale.z();
    matrix(2, 2) = (1.0f - 2.0f * (xx + yy)) * scale.z();
}

void QOtherMathFunctions::matrixToQuaternion(const QMatrix3x3& matrix, QQuaternion& quaternion)
{
    float trace = matrix(0, 0) + matrix(1, 1) + matrix(2, 2) + 1.0f;
    if (trace > 0.0f) {
        const float s = 0.5f / sqrtf(trace);
        quaternion.setVector((matrix(2, 1) - matrix(1, 2)) * s,
                             (matrix(0, 2) - matrix(2, 0)) * s,
                             (matrix(1, 0) - matrix(0, 1)) * s);
        quaternion.setScalar(0.25f / s);
    } else if (matrix(0, 0) > matrix(1, 1) && matrix(0, 0) > matrix(2, 2)) {
        const float s = 0.5f / (sqrtf(1.0) + matrix(0, 0) - matrix(1, 1) - matrix(2, 2));
        quaternion.setVector(0.25f / s,
                             (matrix(0, 1) + matrix(1, 0)) * s,
                             (matrix(2, 0) + matrix(0, 2)) * s);
        quaternion.setScalar((matrix(2, 1) - matrix(1, 2)) * s);
    } else if (matrix(1, 1) > matrix(2, 2)) {
        const float s = 0.5f / sqrtf(1.0f + matrix(1, 1) - matrix(0, 0) - matrix(2, 2));
        quaternion.setVector((matrix(0, 1) + matrix(1, 0)) * s,
                             0.25f / s,
                             (matrix(1, 2) + matrix(2, 1)) * s);
        quaternion.setScalar((matrix(0, 2) - matrix(2, 0)) * s);
    } else {
        const float s = 0.5f / sqrtf(1.0f + matrix(2, 2) - matrix(0, 0) - matrix(1, 1));
        quaternion.setVector((matrix(0, 2) + matrix(2, 0)) * s,
                             (matrix(1, 2) + matrix(2, 1)) * s,
                             0.25f / s);
        quaternion.setScalar((matrix(1, 0) - matrix(0, 1)) * s);
    }
}

void QOtherMathFunctions::matrixToQuaternion(const QMatrix4x4& matrix, QQuaternion& quaternion)
{
    float trace = matrix(0, 0) + matrix(1, 1) + matrix(2, 2) + 1.0f;
    if (trace > 0.0f) {
        const float s = 0.5f / sqrtf(trace);
        quaternion.setVector((matrix(2, 1) - matrix(1, 2)) * s,
                             (matrix(0, 2) - matrix(2, 0)) * s,
                             (matrix(1, 0) - matrix(0, 1)) * s);
        quaternion.setScalar(0.25f / s);
    } else if (matrix(0, 0) > matrix(1, 1) && matrix(0, 0) > matrix(2, 2)) {
        const float s = 0.5f / (sqrtf(1.0) + matrix(0, 0) - matrix(1, 1) - matrix(2, 2));
        quaternion.setVector(0.25f / s,
                             (matrix(0, 1) + matrix(1, 0)) * s,
                             (matrix(2, 0) + matrix(0, 2)) * s);
        quaternion.setScalar((matrix(2, 1) - matrix(1, 2)) * s);
    } else if (matrix(1, 1) > matrix(2, 2)) {
        const float s = 0.5f / sqrtf(1.0f + matrix(1, 1) - matrix(0, 0) - matrix(2, 2));
        quaternion.setVector((matrix(0, 1) + matrix(1, 0)) * s,
                             0.25f / s,
                             (matrix(1, 2) + matrix(2, 1)) * s);
        quaternion.setScalar((matrix(0, 2) - matrix(2, 0)) * s);
    } else {
        const float s = 0.5f / sqrtf(1.0f + matrix(2, 2) - matrix(0, 0) - matrix(1, 1));
        quaternion.setVector((matrix(0, 2) + matrix(2, 0)) * s,
                             (matrix(1, 2) + matrix(2, 1)) * s,
                             0.25f / s);
        quaternion.setScalar((matrix(1, 0) - matrix(0, 1)) * s);
    }
}

void QOtherMathFunctions::quaternionToAngleAxis(const QQuaternion& quaternion, float& angle, QVector3D& axis)
{
    QQuaternion q = quaternion.normalized();
    angle = 2.0f * qAcos(q.scalar());
    float s = qSqrt(1.0f - q.scalar() * q.scalar());
    if (s < EPS_omf)
        axis = q.vector();
    else
        axis = q.vector() / s;
}

bool QOtherMathFunctions::collisionTriangleRay(QVector3D& result,
                                               const QVector3D& v0, const QVector3D& v1, const QVector3D& v2,
                                               const QVector3D& rayStart, const QVector3D& rayEnd)
{
    //TODO можно быстрее
    QVector3D edge0 = v1 - v0;
    QVector3D edge1 = v2 - v1;
    QVector3D normal = QVector3D::crossProduct(edge0, edge1);
    normal.normalize();
    // вычисляем расстояния между концами отрезка и плоскостью треугольника.
    float r1 = QVector3D::dotProduct(normal, rayStart - v0);
    float r2 = QVector3D::dotProduct(normal, rayEnd - v0);
    // если оба конца отрезка лежат по одну сторону от плоскости, то отрезок
    // не пересекает треугольник.
    if (!((r1 > EPS_omf && r2 < -EPS_omf) || (r1 < -EPS_omf && r2 > EPS_omf)))
        return false;
    // вычисляем точку пересечения отрезка с плоскостью треугольника.
    result = (rayStart + ((rayEnd - rayStart) * (- r1 / (r2 - r1))));
    // проверяем, находится ли точка пересечения внутри треугольника.
    if (QVector3D::dotProduct(QVector3D::crossProduct(edge0, result - v0), normal) <= 0.0f)
            return false;
    if (QVector3D::dotProduct(QVector3D::crossProduct(edge1, result - v1), normal) <= 0.0f)
        return false;
    if (QVector3D::dotProduct(QVector3D::crossProduct(v0 - v2, result - v2), normal) <= 0.0f)
        return false;
    return true;
}

bool QOtherMathFunctions::collisionPlaneLine(QVector3D& result, const QVector3D& planeNormal, float planeD,
                                             const QVector3D& linePoint, const QVector3D& lineDir, float& tLine)
{
    tLine = QVector3D::dotProduct(lineDir, planeNormal);
    if (qAbs(tLine) < EPS_omf)
        return false;
    tLine = - ((QVector3D::dotProduct(linePoint, planeNormal) + planeD) / tLine);
    result = linePoint + lineDir * tLine;
    return true;
}

bool QOtherMathFunctions::collisionLines(QVector2D& result, float& tLineA, float& tLineB,
                                         const QVector2D& pointA, const QVector2D& dirA,
                                         const QVector2D& pointB, const QVector2D& dirB)
{
    if (qAbs(dirB.x()) <= EPS_omf) {
        if (qAbs(dirA.x()) <= EPS_omf)
            return false;
        result.setY(dirA.y() / dirA.x());
        tLineA = dirB.y() - dirB.x() * result.y();
        if (qAbs(tLineA) <= EPS_omf)
            return false;
        result.setX(pointB.x() - pointA.x());
        tLineB = (result.x() * result.y() + pointA.y() - pointB.y()) / tLineA;
        tLineA = (result.x() + dirB.x() * tLineB) / dirA.x();
        result = pointA + (dirA * tLineA);
        return true;
    }
    //if (qAbs(dirA.x()) <= EPS_omf)
    //    return false;
    result.setY(dirB.y() / dirB.x());
    tLineB = dirA.y() - dirA.x() * result.y();
    if (qAbs(tLineB) <= EPS_omf)
        return false;
    result.setX(pointA.x() - pointB.x());
    tLineA = (result.x() * result.y() + pointB.y() - pointA.y()) / tLineB;
    tLineB = (result.x() + dirA.x() * tLineA) / dirB.x();
    result = pointB + (dirB * tLineB);
    return true;
}

bool QOtherMathFunctions::circumCircle(QVector2D& resultPosition, const QVector2D& p1,
                                              const QVector2D& p2, const QVector2D& p3)
{
    float s1 = p1.lengthSquared(), s2 = p2.lengthSquared(), s3 = p3.lengthSquared(), a;
    a = p1.x() * p2.y() + p1.y() * p3.x() + p3.y() * p2.x() - p2.y() * p3.x() - p1.y() * p2.x() - p1.x() * p3.y();
    a *= 2.0f;
    if (a < EPS_omf)
        return false;
    resultPosition.setX(s1 * (p2.y() - p3.y()) + s2 * (p3.y() - p1.y()) + s3 * (p1.y() - p2.y()));
    resultPosition.setY(- (s1 * (p2.x() - p3.x()) + s2 * (p3.x() - p1.x()) + s3 * (p1.x() - p2.x())));
    resultPosition /= a;
    return true;
}

bool QOtherMathFunctions::circumSphere(QVector3D& resultPosition, const QVector3D& p1,
                                              const QVector3D& p2, const QVector3D& p3, const QVector3D& p4)
{
    float A, a[4], s[4];//, sa3, C;
    QVector3D b[3], D, t[3], k[4];

    s[0] = p1.lengthSquared();
    s[1] = p2.lengthSquared();
    s[2] = p3.lengthSquared();

    b[0] = QVector3D::crossProduct(p2, p3);
    b[1] = QVector3D::crossProduct(p1, p3);
    b[2] = QVector3D::crossProduct(p1, p2);

    t[0].setX(s[1] * p3.x() - s[2] * p2.x());
    t[0].setY(- s[1] * p3.y() + s[2] * p2.y());
    t[0].setZ(s[1] * p3.z() - s[2] * p2.z());

    t[1].setX(s[0] * p3.x() - s[2] * p1.x());
    t[1].setY(- s[0] * p3.y() + s[2] * p1.y());
    t[1].setZ(s[0] * p3.z() - s[2] * p1.z());

    t[2].setX(s[0] * p2.x() - s[1] * p1.x());
    t[2].setY(- s[0] * p2.y() + s[1] * p1.y());
    t[2].setZ(s[0] * p2.z() - s[1] * p1.z());

    k[0].setX(- b[0].x() + b[1].x() - b[2].x());
    k[1].setX(t[0].z() - t[1].z() + t[2].z());
    k[2].setX(t[0].y() - t[1].y() + t[2].y());
    k[3].setX(s[0] * b[0].x() - s[1] * b[1].x() + s[2] * b[2].x());

    k[0].setY(- b[0].y() + b[1].y() - b[2].y());
    k[1].setY(- k[1].x());
    k[2].setY(t[0].x() - t[1].x() + t[2].x());
    k[3].setY(s[0] * b[0].y() - s[1] * b[1].y() + s[2] * b[2].y());

    k[0].setZ(- b[0].z() + b[1].z() - b[2].z());
    k[1].setZ(- k[2].x());
    k[2].setZ(- k[2].y());
    k[3].setZ(s[0] * b[0].z() - s[1] * b[1].z() + s[2] * b[2].z());

    a[3] = QVector3D::dotProduct(p3, b[2]);
    //sa3 = - (s[3] * a[3]);
	
	//--------------------------------------------------------
    s[3] = p4.lengthSquared();

    a[0] = QVector3D::dotProduct(p4, b[0]);
    a[1] = QVector3D::dotProduct(p4, b[1]);
    a[2] = QVector3D::dotProduct(p4, b[2]);

	A = - a[0] + a[1] - a[2] + a[3];
    A *= 2.0f;

    if (A < EPS_omf)
        return false;

    //C = s[0] * a[0] - s[1] * a[1] + s[2] * a[2] + sa3;

    D.setX(s[3] * k[0].x() + p4.y() * k[1].x() + p4.z() * k[2].x() + k[3].x());
    D.setY(s[3] * k[0].y() + p4.x() * k[1].y() + p4.z() * k[2].y() + k[3].y());
    D.setZ(s[3] * k[0].z() + p4.x() * k[1].z() + p4.y() * k[2].z() + k[3].z());

    resultPosition = D / A;
    return true;
}

bool QOtherMathFunctions::nearestPoint(QVector3D& result, const QVector3D& point1,
                                              QVector3D& point2, QVector3D* simplex, int& n)
{
    QVector3D vec21 = point2 - point1;
    float lS = vec21.lengthSquared();
    if (lS < EPS_omf) {
		result = point1;
		simplex[0] = point1;
		n = 1;
        if (isNull(result))
            return true;
		return false;
	}
    float dpi = - (QVector3D::dotProduct(point1, vec21) / lS);
    if (dpi < 0.0f) {
		result = point1;
		simplex[0] = point1;
		n = 1;
    } else if (dpi > 1.0f) {
		result = point2;
		simplex[0] = point2;
		n = 1;
    } else {
		result = (vec21 * dpi) + point1;
		simplex[0] = point1;
		simplex[1] = point2;
		n = 2;
        if (isNull(result))
            return true;
	}
	return false;
}

void QOtherMathFunctions::_nearestPoint_edge(QVector3D& result, const QVector3D &point1,
                                                    const QVector3D &point2, QVector3D &vec12,
                                                    QVector3D* simplex, int& n)
{
    float lS = vec12.lengthSquared();
    float t = - QVector3D::dotProduct(point1, vec12) / lS;
    if (t < 0.0f) {
		result = point1;
		simplex[0] = point1;
		n = 1;
    } else if (t > 1.0f) {
		result = point2;
		simplex[0] = point2;
		n = 1;
    } else {
		result = point1 + (vec12 * t);
		simplex[0] = point1;
		simplex[1] = point2;
		n = 2;
	}
}

void QOtherMathFunctions::_nearestPoint_2edges(QVector3D& result, const QVector3D &point1,
                                                      const QVector3D &point2, const QVector3D &point3,
                                                      QVector3D& vec12, QVector3D& vec23,
                                                      QVector3D* simplex, int& n)
{
    float lS = vec12.lengthSquared(), t;
    t = - QVector3D::dotProduct(point1, vec12) / lS;
    if (t < 0.0f) {
		result = point1;
		simplex[0] = point1;
		n = 1;
    } else if (t > 1.0f) {
        _nearestPoint_edge(result, point2, point3, vec23, simplex, n);
    } else {
		result = point1 + (vec12 * t);
		simplex[0] = point1;
		simplex[1] = point2;
		n = 2;
	}
}

void QOtherMathFunctions::_nearestPoint_triangle(QVector3D& result, const QVector3D &point1,
                                                        const QVector3D &point2, const QVector3D &point3,
                                                        QVector3D &vec12, QVector3D &vec23,
                                                        QVector3D &vec31, QVector3D &dir,
                                                        QVector3D *simplex, int& n)
{
    float zn1 = QVector3D::dotProduct(point1, QVector3D::crossProduct(dir, vec12)),
        zn2 = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir, vec23)),
        zn3 = QVector3D::dotProduct(point3, QVector3D::crossProduct(dir, vec31));
/*#if checkNearFunc
    QVector3 dirq = - QVector3D::crossProduct(vec12, vec31);
    if (dirq != dir) {
		error("nearestPoint_triangle");
	}
#endif*/
    if (zn1 <= 0.0f) {
        if (zn2 <= 0.0f) {
            if (zn3 <= 0.0f) {
                zn1 = QVector3D::dotProduct(point1, dir) / dir.lengthSquared();
				result = dir * zn1;
				n = 3;
            } else {
                _nearestPoint_edge(result, point3, point1, vec31, simplex, n);
			}
        } else {
            if (zn3 <= 0.0f) {
                _nearestPoint_edge(result, point2, point3, vec23, simplex, n);
            } else {
                _nearestPoint_2edges(result, point2, point3, point1, vec23, vec31, simplex, n);
			}
		}
    } else {
        if (zn2 <= 0.0f) {
            if (zn3 <= 0.0f) {
                _nearestPoint_edge(result, point1, point2, vec12, simplex, n);
            } else {
                _nearestPoint_2edges(result, point3, point1, point2, vec31, vec12, simplex, n);
			}
        } else {
            if (zn3 <= 0.0f) {
                _nearestPoint_2edges(result, point1, point2, point3, vec12, vec23, simplex, n);
            } else { // imposible
				result = point1;
				simplex[0] = point1;
				n = 1;
			}
		}
	}
}

bool QOtherMathFunctions::nearestPoint(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                              const QVector3D &point3, QVector3D *simplex, int& n)
{
    QVector3D vec12 = point2 - point1;
    QVector3D vec23 = point3 - point2;
    QVector3D vec31 = point1 - point3;
    QVector3D dir = QVector3D::crossProduct(vec31, vec12);
    float lengthSquared = dir.lengthSquared();
    if (lengthSquared < EPS_omf * EPS_omf) {
		result = point1;
		simplex[0] = point1;
		n = 1;
        if (isNull(result))
            return true;
		return false;
	}
    _nearestPoint_triangle(result, point1, point2, point3, vec12, vec23, vec31, dir, simplex, n);
    if (isNull(result))
        return true;
	return false;
}

void QOtherMathFunctions::_nearestPoint_2triangles(QVector3D& result, const QVector3D& point1,
                                                          const QVector3D& point2, const QVector3D& point3,
                                                          const QVector3D& point4, QVector3D& vec12,
                                                          QVector3D& vec23, QVector3D& vec31,
                                                          QVector3D& vec24, QVector3D& vec43,
                                                          QVector3D& dir1, QVector3D& dir2,
                                                          QVector3D *simplex, int& n)
{
    float zn1, zn2 = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir1, vec23)), zn3;
/*#if checkNearFunc
    QVector3D dirq1 = QVector3D::crossProduct((point2 - point1), (point3 - point1));
    QVector3D dirq2 = QVector3D::crossProduct((point3 - point4), (point2 - point4));
    if (equally(dirq1, dir1))
		error("nearestPoint_2triangles[1] - dir1");
    if (equally(dirq2, dir2))
		error("nearestPoint_2triangles[1] - dir2");
    dirq1 = - QVector3D::crossProduct(vec12, vec31);
    dirq2 = QVector3D::crossProduct(vec24, vec23);
    if (equally(dirq1, dir1))
		error("nearestPoint_2triangles[1] - dir1 - b");
    if (equally(dirq2, dir2))
		error("nearestPoint_2triangles[1] - dir2 - b");
#endif*/
    if (zn2 <= 0.0f) {
        zn1 = QVector3D::dotProduct(point1, QVector3D::crossProduct(dir1, vec12));
        zn3 = QVector3D::dotProduct(point3, QVector3D::crossProduct(dir1, vec31));
        if (zn1 <= 0.0f) {
            if (zn3 <= 0.0f) {
                zn1 = QVector3D::dotProduct(point1, dir1) / dir1.lengthSquared();
				result = dir1 * zn1;
				simplex[0] = point1;
				simplex[1] = point2;
				simplex[2] = point3;
				n = 3;
            } else {
                _nearestPoint_2edges(result, point4, point3, point1, vec43, vec31, simplex, n);
			}
        } else {
            if (zn3 <= 0.0f) {
                _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
            } else {
                _nearestPoint_2edges(result, point3, point1, point2, vec31, vec12, simplex, n);
			}
		}
    } else {
		// 2, 4, 3
        zn2 = - QVector3D::dotProduct(point3, QVector3D::crossProduct(dir2, vec23));
        float zn1b = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir2, vec24)),
            zn3b = QVector3D::dotProduct(point4, QVector3D::crossProduct(dir2, vec43));
        if (zn2 <= 0.0f) {
            if (zn1b <= 0.0f) {
                if (zn3b <= 0.0f) {
                    zn1 = QVector3D::dotProduct(point2, dir2) / dir2.lengthSquared();
					result = dir2 * zn1;
					simplex[0] = point2;
					simplex[1] = point4;
					simplex[2] = point3;
					n = 3;
                } else {
                    _nearestPoint_2edges(result, point4, point3, point1, vec43, vec31, simplex, n);
				}
            } else {
                if (zn3b <= 0.0f) {
                    _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
                } else {
                    _nearestPoint_2edges(result, point2, point4, point3, vec24, vec43, simplex, n);
				}
			}
        } else {
            float t = - QVector3D::dotProduct(point2, vec23) / vec23.lengthSquared();
            if ((t >= 0.0f) && (t <= 1.0f)) {
				result = point2 + (vec23 * t);
				simplex[0] = point2;
				simplex[1] = point3;
				n = 2;
            } else {
                if (zn1b <= 0.0f) {
                    if (zn3b <= 0.0f) {
                        if (t < 0.0f) {
							result = point2;
							simplex[0] = point2;
                        } else {
							result = point3;
							simplex[0] = point3;
						}
						n = 1;
                    } else {
                        _nearestPoint_2edges(result, point4, point3, point1, vec43, vec31, simplex, n);
					}
                } else {
                    if (zn3b <= 0.0f) {
                        _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
                    } else { // imposible
						result = point1;
						simplex[0] = point1;
						n = 1;
					}
				}
			}
		}
	}
}

void QOtherMathFunctions::_nearestPoint_2triangles(QVector3D& result, const QVector3D &point1,
                                                          const QVector3D &point2, const QVector3D &point3,
                                                          const QVector3D &point4, QVector3D &vec12,
                                                          QVector3D &vec23, QVector3D &vec24,
                                                          QVector3D &dir1, QVector3D &dir2,
                                                          QVector3D *simplex, int& n)
{
    float zn1, zn2 = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir1, vec23));
/*#if checkNearFunc
    QVector3D dirq1 = QVector3D::crossProduct((point2 - point1), (point3 - point1));
    QVector3D dirq2 = QVector3D::crossProduct((point3 - point4), (point2 - point4));
	if (dirq1 != dir1)
		error("nearestPoint_2triangles[2] - dir1");
	if (dirq2 != dir2)
		error("nearestPoint_2triangles[2] - dir2");
#endif*/
    if (zn2 <= 0.0f) {
        zn1 = QVector3D::dotProduct(point1, QVector3D::crossProduct(dir1, vec12));
        if (zn1 <= 0.0f) {
            zn1 = QVector3D::dotProduct(point1, dir1) / dir1.lengthSquared();
			result = dir1 * zn1;
			simplex[0] = point1;
			simplex[1] = point2;
			simplex[2] = point3;
			n = 3;
        } else {
            _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
		}
    } else {
		// 2, 4, 3
        zn2 = - QVector3D::dotProduct(point3, QVector3D::crossProduct(dir2, vec23));
        float zn1b = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir2, vec24));
        if (zn2 <= 0.0f) {
            if (zn1b <= 0.0f) {
                zn1 = QVector3D::dotProduct(point2, dir2) / dir2.lengthSquared();
				result = dir2 * zn1;
				simplex[0] = point2;
				simplex[1] = point4;
				simplex[2] = point3;
				n = 3;
            } else {
                _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
			}
        } else {
            float t = - QVector3D::dotProduct(point2, vec23) / vec23.lengthSquared();
            if ((t >= 0.0f) && (t <= 1.0f)) {
				result = point2 + (vec23 * t);
				simplex[0] = point2;
				simplex[1] = point3;
				n = 2;
            } else {
                if (zn1b <= 0.0f) {
                    if (t < 0.0f) {
						result = point2;
						simplex[0] = point2;
                    } else {
						result = point3;
						simplex[0] = point3;
					}
					n = 1;
                } else {
                    _nearestPoint_2edges(result, point1, point2, point4, vec12, vec24, simplex, n);
				}
			}
		}
	}
}

void QOtherMathFunctions::_nearestPoint_3triangles(QVector3D& result, const QVector3D& point1,
                                                          const QVector3D& point2, const QVector3D& point3,
                                                          const QVector3D& point4, QVector3D& vec12,
                                                          QVector3D& vec23, QVector3D& vec31,
                                                          QVector3D& vec24, QVector3D& vec43,
                                                          QVector3D& vec14, QVector3D& dir1,
                                                          QVector3D& dir2, QVector3D& dir3,
                                                          QVector3D *simplex, int& n)
{
    float zn2, zn3 = QVector3D::dotProduct(point3, QVector3D::crossProduct(dir1, vec31));
/*#if checkNearFunc
    QVector3D dirq1 = QVector3D::crossProduct((point2 - point1), (point3 - point1));
    QVector3D dirq2 = QVector3D::crossProduct((point3 - point4), (point2 - point4));
    QVector3D dirq3 = QVector3D::crossProduct((point3 - point1), (point4 - point1));
	if (dirq1 != dir1)
		error("nearestPoint_3triangles - dir1");
	if (dirq2 != dir2)
		error("nearestPoint_3triangles - dir2");
	if (dirq3 != dir3)
		error("nearestPoint_3triangles - dir3");
    dirq1 = - QVector3D::crossProduct(vec12, vec31);
    dirq2 = - QVector3D::crossProduct(vec43, vec24);
    dirq3 = - QVector3D::crossProduct(vec31, vec14);
	if (dirq1 != dir1)
		error("nearestPoint_3triangles - dir1 - b");
	if (dirq2 != dir2)
		error("nearestPoint_3triangles - dir2 - b");
	if (dirq3 != dir3)
		error("nearestPoint_3triangles - dir3 - b");
#endif*/
	if (zn3 <= 0.0f)
	{
        //_nearestPoint_2triangles(result, point1, point2, point3, point4, vec12, vec23, vec24, dir1, dir2, simplex, n);
        _nearestPoint_2triangles(result, point1, point2, point3, point4, vec12, vec24, vec31, vec24, vec43, dir1, dir2, simplex, n);
    } else {
        zn2 = QVector3D::dotProduct(point2, QVector3D::crossProduct(dir1, vec23));
        if (zn2 <= 0.0f) {
            vec43 = - vec43;
            vec14 = - vec14;
            //_nearestPoint_2triangles(result, point2, point3, point1, point4, vec23, vec31, vec43, dir1, dir3, simplex, n);
            _nearestPoint_2triangles(result, point2, point3, point1, point4, vec23, vec31, vec12, vec43, vec14, dir1, dir3, simplex, n);
        } else {
            vec14 = - vec14;
            vec23 = - vec23;
            vec31 = - vec31;
            //_nearestPoint_2triangles(result, point2, point4, point3, point1, vec24, vec43, vec14, dir2, dir3, simplex, n);
            _nearestPoint_2triangles(result, point2, point4, point3, point1, vec24, vec43, vec23, vec14, vec31, dir2, dir3, simplex, n);
		}
	}
}

bool QOtherMathFunctions::nearestPoint(QVector3D &result, const QVector3D& point1, const QVector3D& point2,
                                              const QVector3D& point3, const QVector3D& point4,
                                              QVector3D* simplex, int& n)
{
    QVector3D vec12 = point2 - point1,
		vec13 = point3 - point1,
		vec14 = point4 - point1, vec23, vec34, vec24;
    QVector3D dir1 = QVector3D::crossProduct(vec12, vec13), // 1, 2, 3
		dir2, dir3, dir4;
    float lengthSquared = dir1.lengthSquared(), set;
    if (lengthSquared < EPS_omf * EPS_omf) {
        if (isNull(point2 - point1)) {
            vec14 = - vec14;
			vec34 = point4 - point3;
            _nearestPoint_triangle(result, point1, point3, point4, vec13, vec34, vec14, dir1, simplex, n);
            if (isNull(result))
                return true;
            if (n == 3) {
				simplex[0] = point1;
				simplex[1] = point3;
				simplex[2] = point4;
			}
			return false;
        } else {
            vec14 = - vec14;
			vec24 = point4 - point2;
            _nearestPoint_triangle(result, point1, point2, point4, vec12, vec24, vec14, dir1, simplex, n);
            if (isNull(result))
                return true;
            if (n == 3) {
				simplex[0] = point1;
				simplex[1] = point2;
				simplex[2] = point4;
			}
			return false;
		}
	}
    set = QVector3D::dotProduct(vec14, dir1);
    if (qAbs(set) < EPS_omf * EPS_omf) {
		vec13 = point1 - point3;
        _nearestPoint_triangle(result, point1, point2, point3, vec12, vec23, vec13, dir1, simplex, n);
        if (isNull(result))
            return true;
        if (n == 3) {
			simplex[0] = point1;
			simplex[1] = point2;
			simplex[2] = point3;
		}
		return false;
	}
    QVector3D p2, p3;
    if (set > 0.0f)  {
        p2 = point3;
        p3 = point2;
        std::swap(vec12, vec13);
        dir1 = - dir1;
    } else {
        p2 = point2;
        p3 = point3;
    }
    vec23 = p3 - p2;
    vec34 = point4 - p3;
    vec24 = point4 - p2;

    dir2 = QVector3D::crossProduct(vec14, vec12); // 1, 4, 2
    dir3 = QVector3D::crossProduct(vec13, vec14); // 1, 3, 4
    dir4 = QVector3D::crossProduct(vec24, vec23); // 2, 4, 3
    float zn1 = - QVector3D::dotProduct(point1, dir1), zn2 = - QVector3D::dotProduct(point1, dir2),
            zn3 = - QVector3D::dotProduct(point1, dir3), zn4 = - QVector3D::dotProduct(p2, dir4);
    if (zn1 >= 0.0f) {
        if (zn2 >= 0.0f) {
            if (zn3 >= 0.0f) {
                if (zn4 >= 0.0f) {
					//imposible
					result = point1;
					simplex[0] = point1;
					n = 1;
                } else {
                    vec13 = - vec13;
                    vec14 = - vec14;
                    _nearestPoint_3triangles(result, p2, p3, point1, point4, vec23, vec13, vec12, vec34, vec14, vec24, dir1, dir2, dir3, simplex, n);
				}
            } else {
                if (zn4 >= 0.0f) {
                    vec13 = - vec13;
                    vec24 = - vec24;
                    _nearestPoint_3triangles(result, p3, point1, p2, point4, vec13, vec12, vec23, vec14, vec24, vec34, dir1, dir4, dir2, simplex, n);
                } else {
                    vec13 = - vec13;
                    vec14 = - vec14;
                    _nearestPoint_2triangles(result, p2, p3, point1, point4, vec23, vec13, vec12, vec34, vec14, dir1, dir2, simplex, n);
				}
			}
        } else {
            if (zn3 >= 0.0f) {
                if (zn4 >= 0.0f) {
                    vec13 = - vec13;
                    vec34 = - vec34;
                    _nearestPoint_3triangles(result, point1, p2, p3, point4, vec12, vec23, vec13, vec24, vec34, vec14, dir1, dir3, dir4, simplex, n);
                } else {
                    vec14 = - vec14;
                    _nearestPoint_2triangles(result, point4, point1, p3, p2, vec14, vec13, vec34, vec12, vec23, dir3, dir1, simplex, n);
				}
            } else {
                if (zn4 >= 0.0f) {
                    vec13 = - vec13;
                    vec34 = - vec34;
                    _nearestPoint_2triangles(result, point1, p2, p3, point4, vec12, vec23, vec13, vec24, vec34, dir1, dir4, simplex, n);
                } else {
                    vec13 = - vec13;
                    _nearestPoint_triangle(result, point1, p2, p3, vec12, vec23, vec13, dir1, simplex, n);
                    if (n == 3) {
						simplex[0] = point1;
                        simplex[1] = p2;
                        simplex[2] = p3;
					}
				}
			}
		}
    } else {
        if (zn2 >= 0.0f) {
            if (zn3 >= 0.0f) {
                if (zn4 >= 0.0f) {
                    vec12 = - vec12;
                    vec24 = - vec24;
                    _nearestPoint_3triangles(result, p2, point1, point4, p3, vec12, vec14, vec24, vec13, vec34, vec23, dir2, dir3, dir4, simplex, n);
                } else {
                    vec12 = - vec12;
                    vec24 = - vec24;
                    _nearestPoint_2triangles(result, p2, point1, point4, p3, vec12, vec14, vec24, vec13, vec34, dir2, dir3, simplex, n);
				}
            } else {
                if (zn4 >= 0.0f) {
                    vec24 = - vec24;
                    vec12 = - vec12;
                    vec34 = - vec34;
                    vec23 = - vec23;
                    _nearestPoint_2triangles(result, point1, point4, p2, p3, vec14, vec24, vec12, vec34, vec23, dir2, dir4, simplex, n);
                } else {
                    vec24 = - vec24;
                    vec12 = - vec12;
                    _nearestPoint_triangle(result, point1, point4, p2, vec14, vec24, vec12, dir2, simplex, n);
                    if (n == 3) {
						simplex[0] = point1;
						simplex[1] = point4;
                        simplex[2] = p2;
					}
				}
			}
        } else {
            if (zn3 >= 0.0f) {
                if (zn4 >= 0.0f) {
                    vec14 = - vec14;
                    vec23 = - vec23;
                    _nearestPoint_2triangles(result, point1, p3, point4, p2, vec13, vec34, vec14, vec23, vec24, dir3, dir4, simplex, n);
                } else {
                    vec14 = - vec14;
                    _nearestPoint_triangle(result, point1, p3, point4, vec13, vec34, vec14, dir3, simplex, n);
                    if (n == 3) {
						simplex[0] = point1;
                        simplex[1] = p3;
						simplex[2] = point4;
					}
				}
            } else {
                if (zn4 >= 0.0f) {
                    vec34 = - vec34;
                    vec23 = - vec23;
                    _nearestPoint_triangle(result, p2, point4, p3, vec24, vec34, vec23, dir4, simplex, n);
                    if (n == 3) {
                        simplex[0] = p2;
						simplex[1] = point4;
                        simplex[2] = p3;
					}
                } else {
					return true;
				}
			}
		}
	}
	return false;
}

}
