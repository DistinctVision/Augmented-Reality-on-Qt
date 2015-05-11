#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>

#include <cfloat>
#include <cassert>
#include <memory>
#include <qmath.h>
#include <memory>
#include <functional>

namespace AR {

typedef struct Rgb
{
    uchar red;
    uchar green;
    uchar blue;

    Rgb() {}
    inline void set(uchar Red, uchar Green, uchar Blue) { red = Red; green = Green; blue = Blue; }
    inline Rgb(uchar Red, uchar Green, uchar Blue) { set(Red, Green, Blue); }
    inline Rgb operator * (float a) const { return Rgb(static_cast<uchar>(red * a),
                                                 static_cast<uchar>(green * a),
                                                 static_cast<uchar>(blue * a)); }
    inline Rgb operator + (const Rgb& a) const
    {
        using namespace std;
        return Rgb(max(red + a.red, 255), max(green + a.green, 255), max(blue + a.blue, 255));
    }
} Rgb;

typedef struct Rgba
{
    uchar red;
    uchar green;
    uchar blue;
    uchar alpha;

    Rgba() {}
    inline void set(uchar Red, uchar Green, uchar Blue, uchar Alpha = 255) { red = Red; green = Green; blue = Blue; alpha = Alpha; }
    inline Rgba(uchar Red, uchar Green, uchar Blue, uchar Alpha = 255) { set(Red, Green, Blue, Alpha); }
    inline Rgba operator * (float a) const { return Rgba(static_cast<uchar>(red * a),
                                                 static_cast<uchar>(green * a),
                                                 static_cast<uchar>(blue * a),
                                                 static_cast<uchar>(alpha * a)); }
    inline Rgba operator + (const Rgba& a) const
    {
        using namespace std;
        return Rgba(max(red + a.red, 255), max(green + a.green, 255), max(blue + a.blue, 255), max(alpha + a.alpha, 255));
    }
} Rgba;

typedef struct IPoint
{
    static const IPoint ZERO;

    int x, y;
    IPoint() {}
    inline void set(int X, int Y) { x = X; y = Y; }
    inline IPoint(int X, int Y) { set(X, Y); }
    inline IPoint operator + (const IPoint& p) const { return IPoint(x+p.x, y+p.y); }
    inline IPoint operator - (const IPoint& p) const { return IPoint(x-p.x, y-p.y); }
    inline IPoint operator - () const { return IPoint(-x, -y); }
    inline void operator += (const IPoint& p) { x+= p.x; y+=p.y; }
    inline void operator -= (const IPoint& p) { x-= p.x; y-=p.y; }
    inline IPoint operator * (const int a) const { return IPoint(x*a, y*a); }
    inline IPoint operator / (const int a) const { return IPoint(x/a, y/a); }
    inline void operator *= (const int a) { x*=a; y*=a; }
    inline void operator /= (const int a) { x/=a; y/=a; }
    inline bool operator == (const IPoint& p) const
    {
        if (p.x != x) return false; if (p.y != y) return false;
        return true;
    }
    inline bool operator != (const IPoint& p) const
    {
        if (p.x != x) return true;
        if (p.y != y) return true;
        return false;
    }
    inline float lengthSquared() const { return static_cast<float>(x * x + y * y); }
    inline float length() const { return qSqrt(lengthSquared()); }
    inline static int dot(const IPoint& a, const IPoint& b)
    {
        return (a.x * b.x + a.y * b.y);
    }
} IPoint;

typedef struct IPointF
{
    static const IPointF ZERO;

    float x, y;
    IPointF() {}
    inline void set(float X, float Y) { x = X; y = Y; }
    inline IPointF(float X, float Y) { set(X, Y); }
    inline IPointF operator + (const IPointF& p) const { return IPointF(x + p.x, y + p.y); }
    inline IPointF operator - (const IPointF& p) const { return IPointF(x - p.x, y - p.y); }
    inline void operator += (const IPointF& p) { x += p.x; y += p.y; }
    inline void operator -= (const IPointF& p) { x -= p.x; y -= p.y; }
    inline IPointF operator - () const { return IPointF(- x, - y); }
    inline IPointF operator * (const int a) const { return IPointF(x * a, y * a); }
    inline IPointF operator * (const float a) const { return IPointF(x * a, y * a); }
    inline IPointF operator * (const double a) const { return IPointF(static_cast<float>(x * a), static_cast<float>(y * a)); }
    inline IPointF operator / (const float a) const { return IPointF(x / a, y / a); }
    inline IPointF operator / (const double a) const { return IPointF(static_cast<float>(x / a), static_cast<float>(y / a)); }
    inline void operator *= (const int a) { x *= a; y *= a; }
    inline void operator *= (const float a) { x *= a; y *= a; }
    inline void operator *= (const double a) { x *= a; y *= a; }
    inline void operator /= (const float a) { x /= a; y /= a; }
    inline void operator /= (const double a) { x /= a; y /= a; }
    inline float lengthSquared() const { return (x * x + y * y); }
    inline float length() const { return qSqrt(lengthSquared()); }
    inline float normalize()
    {
        float l = length();
        if (l > FLT_EPSILON) {
            x /= l;
            y /= l;
        } else {
            x = y = 0.0f;
            return 0.0f;
        }
        return l;
    }
    inline static float dot(const IPointF& a, const IPointF& b)
    {
        return (a.x * b.x + a.y * b.y);
    }
} IPointF;

template <typename T>
class Image
{
public:
    typedef T TypeValue;

public:
    inline Image() { _data = nullptr; _count_copies = new int(1); _size = IPoint::ZERO; }
    inline Image(const Image& image) { _data = image._data; _size = image._size;
                                       ++(*image._count_copies); _count_copies = image._count_copies; }
    inline Image(Image&& image) {
        _data = image._data; image._data = nullptr;
        _size = image._size; image._size.set(0, 0);
        _count_copies = image._count_copies; image._count_copies = new int(0);
    }
    inline Image(const IPoint& size) { _size = size; _allocData(); _count_copies = new int(1); }
    inline ~Image() { _remove(); }
    inline void operator = (const Image& image) { _remove(); _data = image._data; _size = image._size;
                                                  ++(*image._count_copies); _count_copies = image._count_copies; }
    inline const T* data() const { return _data; }
    inline T* data() { return _data; }
    inline T& operator () (int x, int y) { return _data[y * _size.x + x]; }
    inline const T& operator () (const IPoint& point) const { return _data[point.y * _size.x + point.x]; }
    inline T& operator () (const IPoint& point) { return _data[point.y * _size.x + point.x]; }
    inline T* pointer(int x, int y) { return &_data[y * _size.x + x]; }
    inline const T* pointer(int x, int y) const { return &_data[y * _size.x + x]; }
    inline T* pointer(const IPoint& point) { return &_data[point.y * _size.x + point.x]; }
    inline const T* pointer(const IPoint& point) const { return &_data[point.y * _size.x + point.x]; }

    inline int area() const { return _size.y * _size.x; }
    inline int countBytes() const { return area() * sizeof(T); }
    inline IPoint size() const { return _size; }
    inline int width() const { return _size.x; }
    inline int height() const { return _size.y; }
    inline void resize(const IPoint& size) { _remove(); _size = size; _allocData();
                                     _count_copies = new int; *_count_copies = 1; }
    inline static void copyData(Image& imageDst, const Image& imageScr)
    {
        memcpy(imageDst.data(), imageScr.data(), imageScr.countBytes());
    }
    inline Image<T> copy() const
    {
        Image<T> image(_size);
        memcpy(image.data(), _data, countBytes());
        return image;
    }
    inline static void swap(Image& imageA, Image& imageB) { std::swap(imageA._data, imageB._data);
                                                            std::swap(imageA._size, imageB._size);
                                                            std::swap(imageA._count_copies, imageB._count_copies); }

    inline void fill(const T& val) { int sz = _size.y * _size.x; for (int i=0; i<sz; _data[i] = val, ++i); }
    template<typename ConvertType>
    inline Image<ConvertType> convert(const std::function<ConvertType(const T& a)>& convertFunction)
    {
        Image<ConvertType> image(_size);
        int count = area();
        for (int i=0; i<count; ++i)
            image.data()[i] = convertFunction(_data[i]);
        return image;
    }
    inline bool pointInImageWithBorder(const IPoint& point, int border) { return ((point.x > border) &&
                                                                                  (point.y > border) &&
                                                                                  (point.x < _size.x - border) &&
                                                                                  (point.y < _size.y - border)); }
    static void halfSample(Image& out, const Image& in)
    {
        assert((in.size() / 2) == out.size());
        IPoint inP, outP;
        T* outStr;
        unsigned int value;
        for (inP.y = outP.y = 0; outP.y < out._size.y; ++outP.y) {
            const TypeValue* inStr1 = &in._data[inP.y * in._size.x];
            ++inP.y;
            const TypeValue* inStr2 = &in._data[inP.y * in._size.x];
            ++inP.y;
            outStr = &out._data[outP.y * out._size.x];
            for (inP.x = outP.x = 0; outP.x < out.width(); ++outP.x) {
                value = inStr1[inP.x] + inStr2[inP.x];
                ++inP.x;
                outStr[outP.x] = (value + inStr1[inP.x] + inStr2[inP.x]) / 4;
                ++inP.x;
            }
        }
    }

protected:
    T* _data;
    int* _count_copies;
    IPoint _size;

    void _remove()
    {
        if (*_count_copies <= 1) {
/*#if defined(_WIN32)
            _aligned_free(_data);
#elif defined(__ANDROID__)
            free(_data);
#endif*/
            delete[] _data;
            delete _count_copies;
        } else {
            --(*_count_copies);
        }
        _data = nullptr;
        _count_copies = nullptr;
        _size = IPoint::ZERO;
    }
    void _allocData()
    {
        //assert(_size.x >= 0 && _size.y >= 0);
/*#if defined(_WIN32)
        _data = static_cast<T*>(_aligned_malloc(countBytes(), 16));
#elif defined(__ANDROID__)
        _data = static_cast<T*>(memalign(16, countBytes());
#endif
        _data = new (_data)T[area()];*/
        _data = new T[area()];
    }
};

class Painter
{
public:
    template <typename T>
    static void drawLine(Image<T>& image, float x1, float y1, float x2, float y2, const T& c)
    {
        float dx = x2-x1;
        float dy = y2-y1;
        int w = image.width();
        int h = image.height();
        float len = qAbs(dx)+qAbs(dy);
        for(int t=0;t<=len;t++) {
            int x = (int)(x1 + t/(len)*dx+0.5f);
            int y = (int)(y1 + t/(len)*dy+0.5f);
            if (x >=0 && x<w && y>=0 && y<h)
                image(x, y) = c;
        }
    }
    template <typename T>
    inline static void drawLine(Image<T>& image, const IPoint& p1, const IPoint& p2, const T& c) {
                    drawLine<T>(image, static_cast<float>(p1.x), static_cast<float>(p1.y),
                                    static_cast<float>(p2.x), static_cast<float>(p2.y), c); }
    template <typename T>
    inline static void drawLine(Image<T>& image, const IPointF& p1, const IPointF& p2, const T& c) {
                    drawLine<T>(image, p1.x, p1.y, p2.x, p2.y, c); }

    template <typename T>
    static void drawBox(Image<T>& image, float upperleftX, float upperleftY, float lowerrightX, float lowerrightY,
                        const T& c)
    {
        drawLine<T>(image, upperleftX, upperleftY, upperleftX, lowerrightY, c);
        drawLine<T>(image, upperleftX, upperleftY, lowerrightX, upperleftY, c);
        drawLine<T>(image, upperleftX, lowerrightY, lowerrightX, lowerrightY, c);
        drawLine<T>(image, lowerrightX, upperleftY, lowerrightX, lowerrightY, c);
    }
    template <typename T>
    inline static void drawBox(Image<T>& image, const IPoint& upperleft, const IPoint& lowerright, const T& c) {
                    drawBox<T>(image, static_cast<float>(upperleft.x), static_cast<float>(upperleft.y),
                                   static_cast<float>(lowerright.x), static_cast<float>(lowerright.y), c); }
    template <typename T>
    inline static void drawBox(Image<T>& image, const IPointF& upperleft, const IPointF& lowerright, const T& c) {
                    drawBox<T>(image, upperleft.x, upperleft.y, lowerright.x, lowerright.y, c); }

    template <typename T>
    static void drawCross(Image<T>& image, float x, float y, float len, const T& c)
    {
        drawLine<T>(image, x-len, y, x+len, y, c);
        drawLine<T>(image, x, y-len, x, y+len, c);
    }
    template <typename T>
    inline static void drawCross(Image<T>& image, const IPoint& p, float len, const T& c) {
                    drawCross<T>(image, static_cast<float>(p.x), static_cast<float>(p.y), len, c); }
    template <typename T>
    inline static void drawCross(Image<T>& image, const IPointF& p, float len, const T& c) {
                    drawCross<T>(image, p.x, p.y, len, c); }
    template <typename T>
    static void drawImage(Image<T>& canvasImage, const Image<T>& image, const IPointF& pos, const IPointF& size)
    {
        assert((image.width() > 0) && (image.height() > 0));
        assert((size.x > 0.0005f) && (size.y > 0.0005f));
        IPoint cStart(qMax(qFloor(pos.x), 0), qMax(qFloor(pos.y), 0));
        IPoint cEnd(qMin(cStart.x + qCeil(size.x), canvasImage.width() - 1),
                    qMin(cStart.y + qCeil(size.y), canvasImage.height() - 1));
        IPointF scale(image.width() / size.x, image.height() / size.y);
        IPoint p;
        for (p.y=cStart.y; p.y<cEnd.y; ++p.y) {
            T* canvasStr = &canvasImage.data()[canvasImage.width() * p.y];
            float subY = (p.y - pos.y) * scale.y;
            int y = qFloor(subY);
            subY = subY - y;
            const T* strA = &image.data()[image.width() * y];
            const T* strB = &image.data()[image.width() * qMin(y + 1, image.height() - 1)];
            for (p.x=cStart.x; p.x<cEnd.x; ++p.x) {
                float subX = (p.x - pos.x) * scale.x;
                int x = qFloor(subX);
                subX = subX - x;
                int nextX = qMin(x + 1, canvasImage.width() - 1);
                canvasStr[p.x] = static_cast<T>(strA[x] * ((1.0f - subX) * (1.0f - subY)) + strA[nextX] * (subX * (1.0f - subY)) +
                        strB[x] * ((1.0f - subX) * subY) + strB[nextX] * (subX * subY));
            }
        }
    }
    static Image<Rgb> convertQImage(const QImage& image)
    {
        Image<Rgb> result;
        result.resize(IPoint(image.width(), image.height()));
        IPoint p;
        for (p.y=0; p.y<result.height(); ++p.y) {
            Rgb* resultStr = &result.data()[result.width() * p.y];
            for (p.x=0; p.x<result.width(); ++p.x) {
                QRgb rgb = image.pixel(p.x, p.y);
                resultStr[p.x].set(static_cast<uchar>(qRed(rgb)), static_cast<uchar>(qGreen(rgb)), static_cast<uchar>(qBlue(rgb)));
            }
        }
        return result;
    }
    static Image<uchar> convertToBW(const Image<Rgb>& image)
    {
        Image<uchar> result;
        result.resize(image.size());
        IPoint p;
        for (p.y=0; p.y<result.height(); ++p.y) {
            uchar* resultStr = &result.data()[result.width() * p.y];
            const Rgb* imageStr = &image.data()[image.width() * p.y];
            for (p.x=0; p.x<result.width(); ++p.x) {
                const Rgb& rgb = imageStr[p.x];
                resultStr[p.x] = static_cast<uchar>((rgb.red + rgb.green + rgb.blue) / 3);
            }
        }
        return result;
    }
};

}

#endif // IMAGE_H
