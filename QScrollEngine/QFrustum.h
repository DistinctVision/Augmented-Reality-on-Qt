#ifndef QFRUSTUM
#define QFRUSTUM

#include <QVector3D>
#include <QMatrix4x4>

#include "QScrollEngine/QBoundingBox.h"

namespace QScrollEngine {

class QFrustum
{
public:
    // для обозначения сторон:
    enum FrustumSide:int
    {
        RIGHT   = 0,        // Правая сторона пирамиды
        LEFT    = 1,        // Левая сторона пирамиды
        BOTTOM  = 2,        // Нижняя сторона пирамиды
        TOP     = 3,        // Верхняя сторона пирамиды
        BACK    = 4,        // Задняя сторона пирамиды
        FRONT   = 5         // Передняя
    };

    // для обозначения переменных плоскости:
    enum PlaneData:int
    {
        A = 0,              // Значение X нормали плоскости
        B = 1,              // Значение Y нормали плоскости
        C = 2,              // Значение Z нормали плоскости
        D = 3               // Расстояние плоскости от начала координат
    };

public:
    QFrustum();

    // Вызывается каждый раз при движении камеры, чтобы обновить пирамиду
    void calculate(const QMatrix4x4& matrix);

    // Принимает 3D точку и возвращает TRUE, если она находится внутри frustum'a
    bool pointInFrustum(const QVector3D& point) const;

    // Принимает точку и радиус и возвращает TRUE, если она находится внутри frustum'a
    bool sphereInFrustum(const QVector3D& position, float radius) const;

    // Принимает центр и половину длинны куба
    bool boxInFrustum(const QVector3D& position, const QVector3D& halfSize) const;

    bool boundingBoxInFrustum(const QBoundingBox& boundingBox) const;

private:
    // Хранит A B C и D переменные для каждой стороны пирамиды.
    float _frustum[6][4];

    void normalizePlane(int side);
};

}
#endif
