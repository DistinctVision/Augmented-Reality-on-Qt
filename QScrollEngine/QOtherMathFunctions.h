#ifndef QOTHERMATHFUNCTIONS_H
#define QOTHERMATHFUNCTIONS_H

#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <qmath.h>

#define PI 3.14159265359f
#define RADIAN_TO_EULER 57.295779513082f
#define EULER_TO_RADIAN 0.0174532925199432f

namespace QScrollEngine {

class QOtherMathFunctions
{
public:
    const static float EPS_omf;

public:
    static float sign(float a)
    {
        if (a > 0.0f)
            return 1.0f;
        return -1.0f;
    }
    static bool isNull(const QVector2D& v)
    {
        if (qAbs(v.x()) > EPS_omf)
            return false;
        if (qAbs(v.y()) > EPS_omf)
            return false;
        return true;
    }
    static bool isNull(const QVector3D& v)
    {
        if (qAbs(v.x()) > EPS_omf)
            return false;
        if (qAbs(v.y()) > EPS_omf)
            return false;
        if (qAbs(v.z()) > EPS_omf)
            return false;
        return true;
    }
    static bool equally(const QVector3D& a, const QVector3D& b)
    {
        if (qAbs(a.x() - b.x()) > EPS_omf)
            return false;
        if (qAbs(a.y() - b.y()) > EPS_omf)
            return false;
        if (qAbs(a.z() - b.z()) > EPS_omf)
            return false;
        return true;
    }
    static bool equalDir(const QVector3D& a, const QVector3D& b)
    {
        float kx, ky, kz;
        if (qAbs(a.x()) < EPS_omf)
        {
            if (qAbs(b.x()) < EPS_omf)
                kx = 0.0f;
            else
                return false;
        } else {
            kx = b.x() / a.x();
        }
        if (qAbs(a.y()) < EPS_omf) {
            if (qAbs(b.y()) < EPS_omf)
                ky = 0.0f;
            else
                return false;
        } else {
            ky = b.y() / a.y();
            if (kx != 0.0f)
                if (qAbs(kx - ky) >= EPS_omf)
                    return false;
        }
        if (qAbs(a.z()) < EPS_omf) {
            if (qAbs(b.z()) < EPS_omf)
                kz = 0.0f;
            else
                return false;
        } else {
            kz = b.z() / a.z();
            if (kx != 0.0f)
                if (qAbs(kx - kz) >= EPS_omf)
                    return false;
            if (ky != 0.0f)
                if (qAbs(ky - kz) >= EPS_omf)
                    return false;
        }
        return true;
    }

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
public:
    static bool collisionPlaneLine(QVector3D& result, const QVector3D& planeNormal, float planeD,
                            const QVector3D& linePoint, const QVector3D& lineDir, float& tLine);

public:
    static bool collisionLines(QVector2D& result, float& tLineA, float& tLineB, const QVector2D& pointA, const QVector2D& dirA,
                                                             const QVector2D& pointB, const QVector2D& dirB);

public:
    static bool circumCircle(QVector2D& resultPosition, const QVector2D& p1, const QVector2D& p2,
                             const QVector2D& p3);
    static bool circumSphere(QVector3D& resultPosition, const QVector3D& p1, const QVector3D& p2,
                             const QVector3D& p3, const QVector3D& p4);


public:
    static bool nearestPoint(QVector3D& result, const QVector3D& point1, QVector3D& point2,
                             QVector3D* simplex, int& n);
    static bool nearestPoint(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                             const QVector3D& point3, QVector3D *simplex, int& n);
    static bool nearestPoint(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                             const QVector3D& point3, const QVector3D& point4,
                             QVector3D* simplex, int& n);

private:
    static void _nearestPoint_edge(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                   QVector3D& vec12, QVector3D* simplex, int& n);
    static void _nearestPoint_2edges(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                     const QVector3D &point3, QVector3D& vec12, QVector3D& vec23,
                                     QVector3D* simplex, int& n);
    static void _nearestPoint_triangle(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                       const QVector3D& point3, QVector3D& vec12, QVector3D& vec23,
                                       QVector3D& vec31, QVector3D& dir, QVector3D *simplex, int& n);
    static void _nearestPoint_2triangles(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                         const QVector3D& point3, const QVector3D& point4, QVector3D& vec12,
                                         QVector3D& vec23, QVector3D& vec31, QVector3D& vec24,
                                         QVector3D& vec43, QVector3D& dir1, QVector3D& dir2,
                                         QVector3D* simplex, int& n);
    static void _nearestPoint_2triangles(QVector3D& result, const QVector3D& point1, const QVector3D& point2,
                                         const QVector3D& point3, const QVector3D& point4, QVector3D& vec12,
                                         QVector3D& vec23, QVector3D& vec24, QVector3D& dir1,
                                         QVector3D& dir2, QVector3D *simplex, int& n);
    static void _nearestPoint_3triangles(QVector3D& result, const QVector3D &point1, const QVector3D &point2,
                                         const QVector3D &point3, const QVector3D &point4, QVector3D &vec12,
                                         QVector3D& vec23, QVector3D& vec31, QVector3D& vec24,
                                         QVector3D& vec43, QVector3D& vec14, QVector3D& dir1,
                                         QVector3D& dir2, QVector3D& dir3,
                                         QVector3D* simplex, int& n);
};

}
#endif
