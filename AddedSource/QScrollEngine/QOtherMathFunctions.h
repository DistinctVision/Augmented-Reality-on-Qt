#ifndef QOTHERMATHFUNCTIONS_H
#define QOTHERMATHFUNCTIONS_H

#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <qmath.h>

#define PI 3.14159265359
#define RADIAN_TO_EULER 57.295779513082
#define EULER_TO_RADIAN 0.0174532925199432

namespace QScrollEngine {

class QOtherMathFunctions
{
public:
    const static float EPS;

public:
    static QMatrix4x4 invertedWorldMatrix(const QMatrix4x4& matrix);

    static QVector2D transform(const QMatrix2x2& matrix, const QVector2D& vector);
    static QVector2D transform(const QMatrix3x3& matrix, const QVector2D& vector);
    static QVector3D transform(const QMatrix3x3& matrix, const QVector3D& vector);
    static QVector3D transform(const QMatrix4x4& matrix, const QVector3D& vector);
    static QVector2D transformTransposed(const QMatrix2x2& matrix, const QVector2D& vector);
    static QVector2D transformTransposed(const QMatrix3x3& matrix, const QVector2D& vector);
    static QVector3D transformTransposed(const QMatrix3x3& matrix, const QVector3D& vector);
    static QVector3D transformTransposed(const QMatrix4x4& matrix, const QVector3D& vector);

    static void quaternionToMatrix(const QQuaternion& quaternion, QMatrix3x3& matrix);
    static void quaternionToMatrix(const QQuaternion& quaternion, QMatrix4x4& matrix);
    static void quaternionToMatrix(const QQuaternion& quaternion, const QVector3D& scale, QMatrix3x3& matrix);
    static void quaternionToMatrix(const QQuaternion& quaternion, const QVector3D& scale, QMatrix4x4& matrix);
    static void matrixToQuaternion(const QMatrix3x3& matrix, QQuaternion& quaternion);
    static void matrixToQuaternion(const QMatrix4x4& matrix, QQuaternion& quaternion);
    static void quaternionToAngleAxis(const QQuaternion& quaternion, float& angle, QVector3D& axis);

public:
    static bool collisionTriangleRay(QVector3D& result,
                                     const QVector3D& v0, const QVector3D& v1, const QVector3D& v2,
                                     const QVector3D& rayStart, const QVector3D& rayEnd);
    static bool collisionPlaneRay(QVector3D& result, float& tRay, const QVector3D& planeNormal, float planeD,
                                  const QVector3D& rayPoint, const QVector3D& rayDir);

    static bool collisionRays(QVector2D& result, float& tLineA, float& tLineB, const QVector2D& pointA, const QVector2D& dirA,
                              const QVector2D& pointB, const QVector2D& dirB);

    static bool collisionSphereRay(QVector3D& result, float& tRay, const QVector3D& spherePosition, float sphereRadius,
                                   const QVector3D& rayPoint, const QVector3D& rayDir);
    static bool collisionSphereNRay(QVector3D& result, float& tRay, const QVector3D& spherePosition, float sphereRadiusSquared,
                                    const QVector3D& rayPoint, const QVector3D& rayNDir, float rayLength);

    static bool collisionCylinderRay(QVector3D& result, float& tRay,
                                     const QVector3D& cylinderPointA, const QVector3D& cylinderPointB, float cylinderRadius,
                                     const QVector3D& rayPoint, const QVector3D& rayDir);

public:
    static bool circumCircle(QVector2D& resultPosition, const QVector2D& p1, const QVector2D& p2,
                             const QVector2D& p3);
    static bool circumSphere(QVector3D& resultPosition, const QVector3D& p1, const QVector3D& p2,
                             const QVector3D& p3, const QVector3D& p4);
};

}
#endif
