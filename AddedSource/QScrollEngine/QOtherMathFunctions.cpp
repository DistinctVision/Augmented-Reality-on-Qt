#include "QScrollEngine/QOtherMathFunctions.h"

namespace QScrollEngine {

const float QOtherMathFunctions::EPS = 0.0005f;

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
    if (s < EPS)
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
    if (!((r1 > EPS && r2 < -EPS) || (r1 < -EPS && r2 > EPS)))
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

bool QOtherMathFunctions::collisionPlaneRay(QVector3D& result, float& tRay, const QVector3D& planeNormal, float planeD,
                                             const QVector3D& rayPoint, const QVector3D& rayDir)
{
    tRay = QVector3D::dotProduct(rayDir, planeNormal);
    if (qAbs(tRay) < EPS)
        return false;
    tRay = - ((QVector3D::dotProduct(rayPoint, planeNormal) + planeD) / tRay);
    result = rayPoint + rayDir * tRay;
    return true;
}

bool QOtherMathFunctions::collisionRays(QVector2D& result, float& tLineA, float& tLineB,
                                         const QVector2D& pointA, const QVector2D& dirA,
                                         const QVector2D& pointB, const QVector2D& dirB)
{
    if (qAbs(dirB.x()) <= EPS) {
        if (qAbs(dirA.x()) <= EPS)
            return false;
        result.setY(dirA.y() / dirA.x());
        tLineA = dirB.y() - dirB.x() * result.y();
        if (qAbs(tLineA) <= EPS)
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
    if (qAbs(tLineB) <= EPS)
        return false;
    result.setX(pointA.x() - pointB.x());
    tLineA = (result.x() * result.y() + pointB.y() - pointA.y()) / tLineB;
    tLineB = (result.x() + dirA.x() * tLineA) / dirB.x();
    result = pointB + (dirB * tLineB);
    return true;
}

bool QOtherMathFunctions::collisionSphereRay(QVector3D& result, float& tRay, const QVector3D& spherePosition, float sphereRadius,
                                             const QVector3D& rayPoint, const QVector3D& rayDir)
{
    QVector3D delta_sphere_rayPoint = spherePosition - rayPoint;
    float lengthSquared_rayDir = rayDir.lengthSquared();
    tRay = QVector3D::dotProduct(delta_sphere_rayPoint, rayDir) / lengthSquared_rayDir;
    if (tRay > 1.0f)
        return false;
    QVector3D project = rayDir * tRay + rayPoint;
    float lengthSquared_project_sphere = (spherePosition - project).lengthSquared();
    float sphereRadiusSquared = sphereRadius * sphereRadius;
    if (lengthSquared_project_sphere > sphereRadiusSquared)
        return false;
    float deltaT = qSqrt(sphereRadiusSquared - lengthSquared_project_sphere) / qSqrt(lengthSquared_rayDir);
    if (tRay < deltaT) {
        tRay += deltaT;
        if (tRay < 1.0f) {
            result = rayPoint + rayDir * tRay;
            return true;
        } else {
            return false;
        }
    }
    tRay -= deltaT;
    result = rayPoint + rayDir * tRay;
    return true;
}

bool QOtherMathFunctions::collisionSphereNRay(QVector3D& result, float& tRay, const QVector3D& spherePosition, float sphereRadiusSquared,
                                              const QVector3D& rayPoint, const QVector3D& rayNDir, float rayLength)
{
    QVector3D delta_sphere_rayPoint = spherePosition - rayPoint;
    tRay = QVector3D::dotProduct(delta_sphere_rayPoint, rayNDir);
    if (tRay > rayLength)
        return false;
    QVector3D project = rayNDir * tRay + rayPoint;
    float lengthSquared_project_sphere = (spherePosition - project).lengthSquared();
    if (lengthSquared_project_sphere > sphereRadiusSquared)
        return false;
    float deltaT = qSqrt(sphereRadiusSquared - lengthSquared_project_sphere) / rayLength;
    if (tRay < deltaT) {
        tRay += deltaT;
        if (tRay < 1.0f) {
            result = rayPoint + rayNDir * tRay;
            return true;
        } else {
            return false;
        }
    }
    tRay -= deltaT;
    result = rayPoint + rayNDir * tRay;
    return true;
}


bool QOtherMathFunctions::collisionCylinderRay(QVector3D& result, float& tRay,
                                               const QVector3D& cylinderPointA, const QVector3D& cylinderPointB, float cylinderRadius,
                                               const QVector3D& rayPoint, const QVector3D& rayDir)
{
    QVector3D cylinderAxis = cylinderPointB - cylinderPointA;
    float cylinderHeight = cylinderAxis.length();
    cylinderAxis = (cylinderHeight > EPS) ? cylinderAxis / cylinderHeight : QVector3D(0.0f, 0.0f, 0.0f);

    QVector3D projDir = rayDir - cylinderAxis * QVector3D::dotProduct(cylinderAxis, rayDir);
    float lengthProjDir = projDir.length();
    QVector3D delta = rayPoint - cylinderPointA;
    if (lengthProjDir < EPS) {
        float h = QVector3D::dotProduct(cylinderAxis, delta);
        if (h < 0.0f) {
            if (collisionPlaneRay(result, tRay, cylinderAxis, - QVector3D::dotProduct(cylinderPointA, cylinderAxis), rayPoint, rayDir)) {
                delta = result - cylinderPointA;
                return (delta.lengthSquared() < cylinderRadius * cylinderRadius);
            }
        } else if (h > cylinderHeight) {
            if (collisionPlaneRay(result, tRay, cylinderAxis, - QVector3D::dotProduct(cylinderPointB, cylinderAxis), rayPoint, rayDir)) {
                delta = result - cylinderPointB;
                return (delta.lengthSquared() < cylinderRadius * cylinderRadius);
            }
        }
        return true;
    }
    projDir /= lengthProjDir;

    QVector3D perpDir = QVector3D::crossProduct(projDir, cylinderAxis);

    QVector2D projRayPoint(QVector3D::dotProduct(delta, projDir), QVector3D::dotProduct(delta, perpDir));

    if (std::fabs(projRayPoint.y()) > cylinderRadius)
        return false;

    tRay = - projRayPoint.x();
    float deltaT = std::sqrt(cylinderRadius * cylinderRadius - projRayPoint.y() * projRayPoint.y());
    if (tRay < deltaT) {
        tRay += deltaT;
        if (tRay < lengthProjDir) {
            tRay /= lengthProjDir;
            result = rayPoint + rayDir * tRay;
        } else {
            return false;
        }
    } else {
        tRay = (tRay - deltaT) / lengthProjDir;
        result = rayPoint + rayDir * tRay;
    }
    float h = QVector3D::dotProduct(result - cylinderPointA, cylinderAxis);
    if (h < 0.0f) {
        if (collisionPlaneRay(result, tRay, cylinderAxis, - QVector3D::dotProduct(cylinderPointA, cylinderAxis), rayPoint, rayDir)) {
            delta = result - cylinderPointA;
            return (delta.lengthSquared() < cylinderRadius * cylinderRadius);
        }
    } else if (h > cylinderHeight) {
        if (collisionPlaneRay(result, tRay, cylinderAxis, - QVector3D::dotProduct(cylinderPointB, cylinderAxis), rayPoint, rayDir)) {
            delta = result - cylinderPointB;
            return (delta.lengthSquared() < cylinderRadius * cylinderRadius);
        }
    }
    return true;
}

bool QOtherMathFunctions::circumCircle(QVector2D& resultPosition, const QVector2D& p1,
                                       const QVector2D& p2, const QVector2D& p3)
{
    float s1 = p1.lengthSquared(), s2 = p2.lengthSquared(), s3 = p3.lengthSquared(), a;
    a = p1.x() * p2.y() + p1.y() * p3.x() + p3.y() * p2.x() - p2.y() * p3.x() - p1.y() * p2.x() - p1.x() * p3.y();
    a *= 2.0f;
    if (a < EPS)
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

    if (A < EPS)
        return false;

    //C = s[0] * a[0] - s[1] * a[1] + s[2] * a[2] + sa3;

    D.setX(s[3] * k[0].x() + p4.y() * k[1].x() + p4.z() * k[2].x() + k[3].x());
    D.setY(s[3] * k[0].y() + p4.x() * k[1].y() + p4.z() * k[2].y() + k[3].y());
    D.setZ(s[3] * k[0].z() + p4.x() * k[1].z() + p4.y() * k[2].z() + k[3].z());

    resultPosition = D / A;
    return true;
}

}
