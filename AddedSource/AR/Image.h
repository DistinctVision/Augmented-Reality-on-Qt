#ifndef AR_IMAGE_H
#define AR_IMAGE_H

#include <iomanip>
#include <cassert>
#include <memory>
#include <functional>
#include <cmath>
#include <algorithm>

#include "Point2.h"

typedef unsigned char uchar;

namespace AR {

class Rgb
{
public:
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
        return Rgb((red + a.red), (green + a.green), (blue + a.blue));
    }
};

class Rgba
{
public:
    uchar red;
    uchar green;
    uchar blue;
    uchar alpha;

    Rgba() {}
    inline void set(uchar Red, uchar Green, uchar Blue, uchar Alpha = 255) {
        red = Red; green = Green; blue = Blue; alpha = Alpha; }
    inline Rgba(uchar Red, uchar Green, uchar Blue, uchar Alpha = 255) { set(Red, Green, Blue, Alpha); }
    inline Rgba operator * (float a) const { return Rgba(static_cast<uchar>(red * a),
                                                 static_cast<uchar>(green * a),
                                                 static_cast<uchar>(blue * a),
                                                 static_cast<uchar>(alpha * a)); }
    inline Rgba operator + (const Rgba& a) const
    {
        using namespace std;
        return Rgba((red + a.red), (green + a.green),
                    (blue + a.blue), (alpha + a.alpha));
    }
};

template <typename T>
class ImageRef;

template <typename T>
class ConstImage;

template <typename T>
class Image;

template <typename T>
class ImageRef
{
public:
    typedef T TypeValue;

public:
    inline bool equals(const ImageRef<T>& image) const
    {
        if (m_count_copies != nullptr)
            return (m_count_copies == image.m_count_copies);
        return (m_data == image.m_data) && (m_size == image.m_size);
    }

    inline const T* data() const { return m_data; }
    inline const T& operator () (int x, int y) const { return m_data[y * m_size.x + x]; }
    inline const T& operator () (const Point2i& point) const { return m_data[point.y * m_size.x + point.x]; }
    inline const T* pointer(int x, int y) const { return &m_data[y * m_size.x + x]; }
    inline const T* pointer(const Point2i& point) const { return &m_data[point.y * m_size.x + point.x]; }

    inline int area() const { return m_size.y * m_size.x; }
    inline int countBytes() const { return area() * sizeof(T); }
    inline Point2i size() const { return m_size; }
    inline int width() const { return m_size.x; }
    inline int height() const { return m_size.y; }
    inline bool autoDeleting() { return (m_count_copies != nullptr); }
    inline Image<T> copy() const;

    template<typename ConvertType>
    void convert(Image<ConvertType>& out,
                 const std::function<ConvertType(const Point2i& pos, const T& a)>& convertFunction) const;

    template<typename ConvertType>
    Image<ConvertType> convert(const std::function<ConvertType(const Point2i& pos, const T& a)>& convertFunction) const;

    inline bool pointInImageWithBorder(const Point2i& point, int border) const
    {
        return ((point.x > border) && (point.y > border) &&
                (point.x < (m_size.x - border)) && (point.y < (m_size.y - border)));
    }

    inline static void copyData(Image<T>& imageDst, const ImageRef<T>& imageScr);
    inline static void swap(ImageRef<T>& imageA, ImageRef<T>& imageB)
    {
        std::swap(imageA.m_data, imageB.m_data);
        std::swap(imageA.m_size, imageB.m_size);
        std::swap(imageA.m_count_copies, imageB.m_count_copies);
    }

protected:
    friend class ConstImage<T>;
    friend class Image<T>;

    T* m_data;
    int* m_count_copies;
    Point2i m_size;

    ImageRef() {}
    ~ImageRef() {}
    //void operator = (const ImageRef&) { }

    inline void _remove()
    {
        //m_size.set(0, 0);
        if (m_count_copies == nullptr) {
            //m_data = nullptr;
            return;
        }
        if (*m_count_copies <= 1) {
/*#if defined(_WIN32)
            _aligned_free(m_data);
#elif defined(__ANDROID__)
            free(m_data);
#endif*/
            delete[] m_data;
            delete m_count_copies;
        } else {
            --(*m_count_copies);
        }
        m_data = nullptr;
        m_count_copies = nullptr;
    }
    inline void _allocData()
    {
        //assert(m_size.x >= 0 && m_size.y >= 0);
/*#if defined(_WIN32)
        m_data = static_cast<T*>(_aligned_malloc(countBytes(), 16));
#elif defined(__ANDROID__)
        m_data = static_cast<T*>(memalign(16, countBytes());
#endif
        m_data = new (_data)T[area()];*/
        m_data = new T[area()];
    }
    inline void _copyRef(const ImageRef<T>& image)
    {
        m_data = image.m_data;
        m_size = image.m_size;
        if (image.m_count_copies != nullptr) {
            ++(*image.m_count_copies);
            m_count_copies = image.m_count_copies;
        } else {
            m_count_copies = nullptr;
        }
    }
    inline void _moveRef(ImageRef<T>&& image)
    {
        m_data = image.m_data;
        image.m_data = nullptr;
        m_size = image.m_size;
        image.m_size.set(0, 0);
        m_count_copies = image.m_count_copies;
        image.m_count_copies = nullptr;
    }
};

template <typename T>
class Image;

template <typename T>
class ConstImage final: public ImageRef<T>
{
public:
    ConstImage()
    {
        this->m_data = nullptr;
        this->m_count_copies = nullptr;
        this->m_size.setZero();
    }
    ConstImage(const ImageRef<T>& image)
    {
        this->_copyRef(image);
    }
    ConstImage(const ConstImage<T>& image)
    {
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
    }
    ConstImage(const Image<T>& image)
    {
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
    }
    ConstImage(ImageRef<T>&& image)
    {
        this->_moveRef(image);
    }
    ConstImage(ConstImage<T>&& image)
    {
        this->_copyRef(static_cast<ImageRef<T>&&>(image));
    }
    ConstImage(Image<T>&& image)
    {
        this->_copyRef(static_cast<ImageRef<T>&&>(image));
    }
    ConstImage(const Point2i& size, const T* data, bool autoDeleting = true)
    {
        this->m_size = size;
        this->m_data = const_cast<T*>(data);
        this->m_count_copies = (autoDeleting) ? new int(1) : nullptr;
    }
    ~ConstImage()
    {
        this->_remove();
    }
    ConstImage& operator = (const ImageRef<T>& image)
    {
        this->_remove();
        this->_copyRef(image);
        return (*this);
    }
    ConstImage& operator = (const ConstImage<T>& image)
    {
        this->_remove();
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
        return (*this);
    }
    ConstImage& operator = (const Image<T>& image)
    {
        this->_remove();
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
        return (*this);
    }
    ConstImage& operator = (ImageRef<T>&& image)
    {
        this->_remove();
        this->_moveRef(image);
        return (*this);
    }
    ConstImage& operator = (ConstImage<T>&& image)
    {
        this->_remove();
        this->_moveRef(static_cast<ImageRef<T>&&>(image));
        return (*this);
    }
    ConstImage& operator = (Image<T>&& image)
    {
        this->_remove();
        this->_moveRef(static_cast<ImageRef<T>&&>(image));
        return (*this);
    }
};

template <typename T>
class Image: public ImageRef<T>
{
public:
    Image()
    {
        this->m_data = nullptr;
        this->m_count_copies = nullptr;
        this->m_size.setZero();
    }
    Image(const Image<T>& image)
    {
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
    }
    Image(Image<T>&& image)
    {
        this->_moveRef(static_cast<ImageRef<T>&&>(image));
    }
    Image(const Point2i& size)
    {
        this->m_size = size;
        this->_allocData();
        this->m_count_copies = new int(1);
    }
    Image(const Point2i& size, T* data, bool autoDeleting = true)
    {
        this->m_size = size;
        this->m_data = const_cast<T*>(data);
        this->m_count_copies = (autoDeleting) ? new int(1) : nullptr;
    }
    ~Image()
    {
        this->_remove();
    }
    operator ConstImage<T>()
    {
        return ConstImage<T>(*this);
    }

    Image<T>& operator = (const Image<T>& image)
    {
        this->_remove();
        this->_copyRef(static_cast<const ImageRef<T>&>(image));
        return (*this);
    }
    Image<T>& operator = (Image<T>&& image)
    {
        this->_remove();
        this->_moveRef(static_cast<ImageRef<T>&&>(image));
        return (*this);
    }

    inline T* data() { return this->m_data; }
    inline const T* data() const { return this->m_data; }
    inline T& operator () (int x, int y) { return this->m_data[y * this->m_size.x + x]; }
    inline const T& operator () (int x, int y) const { return this->m_data[y * this->m_size.x + x]; }
    inline T& operator () (const Point2i& point) { return this->m_data[point.y * this->m_size.x + point.x]; }
    inline T& operator () (const Point2i& point) const { return this->m_data[point.y * this->m_size.x + point.x]; }
    inline T* pointer(int x, int y) { return &this->m_data[y * this->m_size.x + x]; }
    inline const T* pointer(int x, int y) const { return &this->m_data[y * this->m_size.x + x]; }
    inline T* pointer(const Point2i& point) { return &this->m_data[point.y * this->m_size.x + point.x]; }
    inline const T* pointer(const Point2i& point) const { return &this->m_data[point.y * this->m_size.x + point.x]; }

    void fill(const T& val)
    {
        int sz = this->m_size.y * this->m_size.x;
        for (int i = 0; i < sz; this->m_data[i] = val, ++i);
    }
};

template <typename T>
Image<T> ImageRef<T>::copy() const
{
    Image<T> image(m_size);
    memcpy(image.data(), m_data, countBytes());
    return image;
}

template <typename T>
template <typename ConvertType>
void ImageRef<T>::convert(Image<ConvertType>& out,
                          const std::function<ConvertType(const Point2i& pos, const T& a)>& convertFunction) const
{
    assert(this->m_size == out.size());
    T* str = this->m_data;
    ConvertType* outStr = out.data();
    Point2i p;
    for (p.y = 0; p.y < this->m_size.y; ++p.y) {
        for (p.x = 0; p.x < this->m_size.x; ++p.x) {
            outStr[p.x] = convertFunction(p, str[p.x]);
        }
        str = &str[this->m_size.x];
        outStr = &outStr[this->m_size.x];
    }
}

template <typename T>
template <typename ConvertType>
Image<ConvertType> ImageRef<T>::convert(const std::function<ConvertType(const Point2i& pos, const T& a)>& convertFunction) const
{
    Image<ConvertType> image(m_size);
    convert<ConvertType>(image, convertFunction);
    return image;
}

template <typename T>
void ImageRef<T>::copyData(Image<T>& imageDst, const ImageRef<T>& imageScr)
{
    assert(imageDst.data() != nullptr);
    assert(imageDst.size() == imageScr.size());
    memcpy(imageDst.data(), imageScr.data(), imageScr.countBytes());
}

}

#endif // IMAGE_H
