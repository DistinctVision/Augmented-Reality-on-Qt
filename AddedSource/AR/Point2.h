#ifndef AR_POINT2_H
#define AR_POINT2_H

#include <limits>
#include <climits>
#include <cmath>

namespace AR {

template <typename T>
class Point2
{
public:
    T x, y;

    Point2() {}
    inline void set(T x, T y) { this->x = x; this->y = y; }
    inline void setZero() { set((T)0, (T)0); }
    inline Point2(T x, T y) { set(x, y); }
    inline Point2 operator + (const Point2& p) const { return Point2(x + p.x, y + p.y); }
    inline Point2 operator - (const Point2& p) const { return Point2(x - p.x, y - p.y); }
    inline void operator += (const Point2& p) { x += p.x; y += p.y; }
    inline void operator -= (const Point2& p) { x -= p.x; y -= p.y; }
    inline Point2 operator - () const { return Point2(- x, - y); }
    template <typename TB>
    inline Point2 operator * (TB a) const { return Point2((T)(x * a), (T)(y * a)); }
    template <typename TB>
    inline Point2 operator / (TB a) const { return Point2((T)(x / a), (T)(y / a)); }
    template <typename TB>
    inline void operator *= (TB a) { x *= a; y *= a; }
    template <typename TB>
    inline void operator /= (TB a) { x /= a; y /= a; }
    inline bool operator == (const Point2<T>& p) const
    {
        return ((x == p.x) && (y == p.y));
    }
    inline bool operator != (const Point2<T>& p) const
    {
        return ((x != p.x) || (y != p.y));
    }
    inline T lengthSquared() const { return (x * x + y * y); }
    inline T length() const { return std::sqrt(lengthSquared()); }
    inline T normalize()
    {
        T l = length();
        if (l > std::numeric_limits<T>::epsilon()) {
            x /= l;
            y /= l;
        } else {
            x = y = 0.0f;
            return (T)0;
        }
        return l;
    }
    template <typename CastType>
    inline Point2<CastType> cast() const
    {
        return Point2<CastType>(static_cast<CastType>(x), static_cast<CastType>(y));
    }
    inline static T dot(const Point2& a, const Point2& b)
    {
        return (a.x * b.x + a.y * b.y);
    }
};

template <typename T, typename TB>
static Point2<T> operator * (TB a, const Point2<T>& b)
{
    return Point2<T>((T)(b.x * a), (T)(b.y * a));
}

typedef Point2<int> Point2i;
typedef Point2<float> Point2f;
typedef Point2<double> Point2d;

}

#endif // AR_POINT2_H
