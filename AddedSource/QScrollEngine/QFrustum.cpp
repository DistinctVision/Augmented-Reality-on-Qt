#include "QScrollEngine/QFrustum.h"
#include <qmath.h>

namespace QScrollEngine {

QFrustum::QFrustum()
{
    //entity = new QEntity(NULL, QGLPrimitiv::Quad);
}

void QFrustum::normalizePlane(int side)
{
    // Вычисляем величину нормали плоскости (точку A B C)
    // Помните, что (A, B, C) плоскости - то же самое, что (X, Y, Z) для нормали.
    // Чтобы вычислить величину, используем формулу: sqrt(x^2 + y^2 + z^2)
    float magnitude = (float)sqrtf(m_frustum[side][A] * m_frustum[side][A] +
                                  m_frustum[side][B] * m_frustum[side][B] +
                                  m_frustum[side][C] * m_frustum[side][C] );

    // Затем делим значения плоскости на её величину.
    // После этого с плоскостью будет легче работать.
    m_frustum[side][A] /= magnitude;
    m_frustum[side][B] /= magnitude;
    m_frustum[side][C] /= magnitude;
    m_frustum[side][D] /= magnitude;
}

void QFrustum::calculate(const QMatrix4x4& matrix)
{
    //matrix = matrix.inverted();
    // Нам нужно получить стороны frustum'а. Чтобы сделать это,
    // возьмём обрезающие плоскости, полученные из matrix (матрица проекции умноженная на мировую), и из них получим стороны.

    // получаем ПРАВУЮ сторону frustum'а
    m_frustum[RIGHT][A] = matrix(3, 0) - matrix(0, 0);
    m_frustum[RIGHT][B] = matrix(3, 1) - matrix(0, 1);
    m_frustum[RIGHT][C] = matrix(3, 2) - matrix(0, 2);
    m_frustum[RIGHT][D] = matrix(3, 3) - matrix(0, 3);

    // Теперь, имея нормаль (A,B,C) и расстояние (D) к плоскости,
    // нам нужно нормализовать нормаль и дистанцию

    // Нормализуем правую сторону
    normalizePlane(RIGHT);

    // получаем ЛЕВУЮ сторону frustum'а
    m_frustum[LEFT][A] = matrix(3, 0) + matrix(0, 0);
    m_frustum[LEFT][B] = matrix(3, 1) + matrix(0, 1);
    m_frustum[LEFT][C] = matrix(3, 2) + matrix(0, 2);
    m_frustum[LEFT][D] = matrix(3, 3) + matrix(0, 3);
    normalizePlane(LEFT);

    // получаем нижнюю сторону frustum'а
    m_frustum[BOTTOM][A] = matrix(3, 0) - matrix(1, 0);
    m_frustum[BOTTOM][B] = matrix(3, 1) - matrix(1, 1);
    m_frustum[BOTTOM][C] = matrix(3, 2) - matrix(1, 2);
    m_frustum[BOTTOM][D] = matrix(3, 3) - matrix(1, 3);
    normalizePlane(BOTTOM);

    // получаем верхнюю сторону frustum'а
    m_frustum[TOP][A] = matrix(3, 0) + matrix(1, 0);
    m_frustum[TOP][B] = matrix(3, 1) + matrix(1, 1);
    m_frustum[TOP][C] = matrix(3, 2) + matrix(1, 2);
    m_frustum[TOP][D] = matrix(3, 3) + matrix(1, 3);
    normalizePlane(TOP);

    // получаем заднюю сторону frustum'а
    m_frustum[BACK][A] = matrix(3, 0) - matrix(2, 0);
    m_frustum[BACK][B] = matrix(3, 1) - matrix(2, 1);
    m_frustum[BACK][C] = matrix(3, 2) - matrix(2, 2);
    m_frustum[BACK][D] = matrix(3, 3) - matrix(2, 3);
    normalizePlane(BACK);

    // получаем переднюю сторону frustum'а
    m_frustum[FRONT][A] = matrix(3, 0) + matrix(2, 0);
    m_frustum[FRONT][B] = matrix(3, 1) + matrix(2, 1);
    m_frustum[FRONT][C] = matrix(3, 2) + matrix(2, 2);
    m_frustum[FRONT][D] = matrix(3, 3) + matrix(2, 3);
    normalizePlane(FRONT);
}

bool QFrustum::pointInFrustum(const QVector3D& point) const
{
    // (A,B,C) - это (X,Y,Z) нормали плоскости. Формула равна нулю, если точка лежит НА
    // плоскости. Если точка ПОЗАДИ плоскости, значение будет отрицательным, если же
    // ПЕРЕД плоскостью - значение будет положительным. Нам нужно проверить, находится
    // ли точка ПЕРЕД плоскостью, так что всё, что нам нужно сделать - пройти через
    // каждую точку и применить к ней формулу плоскости. Результат будет расстоянием
    // до плоскости.

    // Проходим через все стороны пирамиды.
    for(int i = 0; i < 6; i++ )
    {
        // Применяем формулу плоскости и проверяем, находится ли точка позади плоскости.
        // Если она позади хотя бы одной плоскости из всех, можно возвращать false.
        if(m_frustum[i][A] * point.x() + m_frustum[i][B] * point.y() + m_frustum[i][C] * point.z() + m_frustum[i][D] <= 0.0f)
        {
            // Точка находится позади стороны, так что она НЕ внутри пирамиды
            return false;
        }
    }

    // Иначе точка находится внутри пирамиды, возвращаем true
    return true;
}

bool QFrustum::sphereInFrustum(const QVector3D& position, float radius) const
{
    // Эта функция почти идентична предыдущей. Отличие в том, что деперь нам придется
    // учесть ещё и радиус вокруг точки. Например, одно то, что центр сферы находится вне
    // пирамиды, ещё не значит, что и все её точки находятся снаружи. Поэтому вместо проверки,
    // меньше ли результат формулы нуля (<=0), нужно прибавить к нулю отрицательный радиус сферы.

    // Проходим через все стороны пирамиды
    for(int i = 0; i < 6; i++ )
    {
        // Если центр сферы дальше от плоскости, чем её радиус
        if(m_frustum[i][A] * position.x() + m_frustum[i][B] * position.y() + m_frustum[i][C] * position.z() +
                m_frustum[i][D] <= -radius)
        {
            // То и вся сфера снаружи, возвращаем false
            return false;
        }
    }

    // Иначе сфера внутри
    return true;
}

bool QFrustum::boxInFrustum(const QVector3D& position, const QVector3D& halSize) const
{
    // Тут работы немного больше, но тоже не слишком много.
    // Нам передаётся центр куба и половина его длинны. Думайте о длинне так же,
    // как о радиусе для сферы. Мы проверяем каждую точку куба на нахождение внутри
    // пирамиды. Если точка находится перед стороной, переходим к следующей сторону.

    for(int i = 0; i < 6; i++ )
    {
        if (m_frustum[i][A] * (position.x() - halSize.x()) + m_frustum[i][B] * (position.y() - halSize.y()) +
            m_frustum[i][C] * (position.z() - halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() + halSize.x()) + m_frustum[i][B] * (position.y() - halSize.y()) +
            m_frustum[i][C] * (position.z() - halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() - halSize.x()) + m_frustum[i][B] * (position.y() + halSize.y()) +
            m_frustum[i][C] * (position.z() - halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() + halSize.x()) + m_frustum[i][B] * (position.y() + halSize.y()) +
            m_frustum[i][C] * (position.z() - halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() - halSize.x()) + m_frustum[i][B] * (position.y() - halSize.y()) +
            m_frustum[i][C] * (position.z() + halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() + halSize.x()) + m_frustum[i][B] * (position.y() - halSize.y()) +
            m_frustum[i][C] * (position.z() + halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() - halSize.x()) + m_frustum[i][B] * (position.y() + halSize.y()) +
            m_frustum[i][C] * (position.z() + halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;
        if (m_frustum[i][A] * (position.x() + halSize.x()) + m_frustum[i][B] * (position.y() + halSize.y()) +
            m_frustum[i][C] * (position.z() + halSize.z()) + m_frustum[i][D] > 0.0f)
           continue;

        // Если мы дошли досюда, куб не внутри пирамиды.
        return false;
    }

    return true;
}

bool QFrustum::boundingBoxInFrustum(const QBoundingBox& boundingBox) const
{
    QVector3D center = boundingBox.getCenter();
    return boxInFrustum(center, boundingBox.max() - center);
}

}
