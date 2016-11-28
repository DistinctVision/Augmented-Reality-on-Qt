#ifndef AR_IMAGEPROCESSING_H
#define AR_IMAGEPROCESSING_H

#include "Image.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include <cmath>
#include <cassert>

#if QT_MULTIMEDIA_LIB
#include <QImage>
#endif


namespace AR {

class ImageProcessing
{
public:
    static void sobel(Image<bool>& out, Image<int>& outIntegral, const ImageRef<uchar>& in, const uchar threshold);
    static void erode(Image<bool>& out, const ImageRef<int>& integral, int size, float k);
#if QT_MULTIMEDIA_LIB
    static Image<Rgba> convertQImage(const QImage& image);
#endif
    static Image<uchar> convertToBW(const ImageRef<Rgb>& image);

    static void gaussianBlurX(Image<uchar>& out, const ImageRef<uchar>& in, int halfSizeBlur, float sigma)
    {
        gaussianBlurX<float, uchar>(out, in, halfSizeBlur, sigma);
    }

    static void gaussianBlurY(Image<uchar>& out, const ImageRef<uchar>& in, int halfSizeBlur, float sigma)
    {
        gaussianBlurY<float, uchar>(out, in, halfSizeBlur, sigma);
    }

    static void gaussianBlur(Image<uchar>& out, const ImageRef<uchar>& in, int halfSizeBlur, float sigma)
    {
        gaussianBlur<float, uchar>(out, in, halfSizeBlur, sigma);
    }

    static void halfSample(Image<uchar>& out, const ImageRef<uchar>& in)
    {
        halfSample<unsigned int, uchar>(out, in);
    }

    template <typename RealSumType, typename Type>
    static void gaussianBlurX(Image<Type>& out, const ImageRef<Type>& in, int halfSizeBlur, float sigma)
    {
        assert(out.size() == in.size());
        assert(in.data() != out.data());
        assert((in.width() > halfSizeBlur * 2 + 1));
        Point2i sizeImage = in.size();
        int beginCenterRegionX = halfSizeBlur;
        int endCenterRegionX = sizeImage.x - halfSizeBlur;
        Point2i p;
        int k = 0;
        float sigmaSquare = sigma * sigma;
        int sizeKernel = halfSizeBlur * 2 + 1;
        float* g = new float[sizeKernel];
        float k1 = (float)(1.0f / (sigma * std::sqrt(2.0f * M_PI)));
        float invK2 = 1.0f / (2.0f * sigmaSquare);
        for (p.x=-halfSizeBlur; p.x<=halfSizeBlur; ++p.x, ++k)
            g[k] = k1 * std::exp(- p.x * p.x * invK2);
        int c;
        RealSumType sum;
        const Type* inStr = in.data();
        Type* outStr = out.data();
        for (p.y=0; p.y<sizeImage.y; ++p.y) {
            for (p.x=0; p.x<beginCenterRegionX; ++p.x) {
                sum = (RealSumType)(0);
                c = p.x - halfSizeBlur;
                for (k=0; c<=0; ++k, ++c)
                    sum += (RealSumType)(inStr[0] * g[k]);
                for (; k<sizeKernel; ++k, ++c)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            for (p.x=beginCenterRegionX; p.x<endCenterRegionX; ++p.x) {
                c = p.x - halfSizeBlur;
                sum = (RealSumType)(0);
                for (k=0; k<sizeKernel; ++k, ++c)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            for (p.x=endCenterRegionX; p.x<sizeImage.x; ++p.x) {
                sum = (RealSumType)(0);
                c = p.x - halfSizeBlur;
                for (k=0; c<sizeImage.x-1; ++k, ++c)
                    sum += (RealSumType)(inStr[c] * g[k]);
                for (; k<sizeKernel; ++k)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            inStr = &inStr[sizeImage.x];
            outStr = &outStr[sizeImage.x];
        }
        delete[] g;
    }

    template <typename OutType, typename T>
    inline static OutType interpolate(const ImageRef<T>& image, float x, float y)
    {
        int ix = (int)(std::floor(x)), iy = (int)(std::floor(y));
        float dx = x - ix, dy = y - iy;
        float idx = 1.0f - dx, idy = 1.0f - dy;
        const T* strA = &image.data()[iy * image.width() + ix];
        const T* strB = &strA[image.width()];
        return (OutType)((strA[0] * idx * idy) + (strA[1] * dx * idy) +
                         (strB[0] * idx * dy) + (strB[1] * dx * dy));
    }
    template <typename OutType, typename T>
    inline static OutType interpolate(const ImageRef<T>& image, const Point2f& point)
    {
        return interpolate<OutType, T>(image, point.x, point.y);
    }

    template <typename OutType, typename T>
    inline static OutType interpolate(const ImageRef<T>& image, double x, double y)
    {
        int ix = (int)(std::floor(x)), iy = (int)(std::floor(y));
        double dx = x - ix, dy = y - iy;
        double idx = 1.0 - dx, idy = 1.0 - dy;
        const T* strA = &image.data()[iy * image.width() + ix];
        const T* strB = &strA[image.width()];
        return (OutType)((strA[0] * idx * idy) + (strA[1] * dx * idy) +
                         (strB[0] * idx * dy) + (strB[1] * dx * dy));
    }
    template <typename OutType, typename T>
    inline static OutType interpolate(const ImageRef<T>& image, const Point2d& point)
    {
        return interpolate<OutType, T>(image, point.x, point.y);
    }

    template <typename RealSumType, typename Type>
    static void gaussianBlurY(Image<Type>& out, const ImageRef<Type>& in, int halfSizeBlur, float sigma)
    {
        assert(out.size() == in.size());
        assert(in.data() != out.data());
        assert((in.height() > halfSizeBlur * 2 + 1));
        Point2i sizeImage = in.size();
        int beginCenterRegionY = halfSizeBlur;
        int endCenterRegionY = sizeImage.y - halfSizeBlur;
        Point2i p;
        int sizeKernel = halfSizeBlur * 2 + 1;
        float sigmaSquare = sigma * sigma;
        float* g = new float[sizeKernel];
        float k1 = (float)(1.0f / (sigma * std::sqrt(2.0f * M_PI)));
        float invK2 = 1.0f / (2.0f * sigmaSquare);
        int k = 0;
        for (p.y=-halfSizeBlur; p.y<=halfSizeBlur; ++p.y, ++k)
            g[k] = k1 * std::exp(- p.y * p.y * invK2);
        RealSumType sum;
        int c;
        int c_offset = - halfSizeBlur * sizeImage.x;
        const Type* inStr = in.data();
        Type* outStr = out.data();
        for (p.y=0; p.y<beginCenterRegionY; ++p.y) {
            int c_limit = halfSizeBlur - p.y;
            for (p.x=0; p.x<sizeImage.x; ++p.x) {
                c = p.x + c_offset;
                sum = (RealSumType)(0);
                for (k=0; k<c_limit; ++k, c+=sizeImage.x)
                    sum += (RealSumType)(inStr[p.x] * g[k]);
                for (; k<sizeKernel; ++k, c+=sizeImage.x)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            inStr = &inStr[sizeImage.x];
            outStr = &outStr[sizeImage.x];
        }
        for (; p.y<endCenterRegionY; ++p.y) {
            for (p.x=0; p.x<sizeImage.x; ++p.x) {
                c = p.x + c_offset;
                sum = (RealSumType)(0);
                for (k=0; k<sizeKernel; ++k, c+=sizeImage.x)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            inStr = &inStr[sizeImage.x];
            outStr = &outStr[sizeImage.x];
        }
        for (; p.y<sizeImage.y; ++p.y) {
            int c_limit = sizeKernel - (p.y - endCenterRegionY + 2);
            for (p.x=0; p.x<sizeImage.x; ++p.x) {
                c = p.x + c_offset;
                sum = (RealSumType)(0);
                for (k=0; k<c_limit; ++k, c+=sizeImage.x)
                    sum += (RealSumType)(inStr[c] * g[k]);
                for (; k<sizeKernel; ++k)
                    sum += (RealSumType)(inStr[c] * g[k]);
                outStr[p.x] = (Type)(sum);
            }
            inStr = &inStr[sizeImage.x];
            outStr = &outStr[sizeImage.x];
        }
        delete[] g;
    }

    template <typename RealSumType, typename Type>
    static void gaussianBlur(Image<Type>& out, const ImageRef<Type>& in, int halfSizeBlur, float sigma)
    {
        Image<Type> temp(in.size());
        gaussianBlurX<RealSumType, Type>(temp, in, halfSizeBlur, sigma);
        gaussianBlurY<RealSumType, Type>(out, temp, halfSizeBlur, sigma);
    }

    template <typename SumType, typename Type>
    static void halfSample(Image<Type>& out, const ImageRef<Type>& in)
    {
        assert((in.size() / 2) == out.size());
        Point2i outP;
        int inPx = 0;
        Type* outStr = out.data();
        const Type* inStrA = in.data();
        const Type* inStrB = &inStrA[in.width()];
        SumType value;
        for (outP.y = 0; outP.y < out.height(); ++outP.y) {
            for (inPx = outP.x = 0; outP.x < out.width(); ++outP.x) {
                value = (SumType)(inStrA[inPx] + inStrA[inPx + 1]);
                outStr[outP.x] = (Type)((value + inStrB[inPx] + inStrB[inPx + 1]) / 4);
                inPx += 2;
            }
            outStr = &outStr[out.width()];
            inStrA = &inStrB[in.width()];
            inStrB = &inStrA[in.width()];
        }
    }

    template <typename T, typename S, typename P>
    static int transform2x2(Image<T>& out, const ImageRef<S>& in, const TMath::TMatrix<P>& M,
                            const Point2<P>& outOrigin, const Point2<P>& inOrigin,
                            const T defaultValue = T())
    {
        TMath_assert((M.rows() == 2) && (M.cols() == 2));
        int outWidth = out.width(), outHeight = out.height(), inWidth = in.width(), inHeight = in.height();
        Point2<P> across(M(0, 0), M(1, 0));
        Point2<P> down(M(0, 1), M(1, 1));

        Point2<P> p0 = inOrigin - Point2<P>(M(0, 0) * outOrigin.x + M(0, 1) * outOrigin.y,
                                            M(1, 0) * outOrigin.x + M(1, 1) * outOrigin.y);

        Point2<P> min = p0, max = p0;

        if (across.x < 0)
            min.x += outWidth * across.x;
        else
            max.x += outWidth * across.x;
        if (down.x < 0)
            min.x += outHeight * down.x;
        else
            max.x += outHeight * down.x;
        if (across.y < 0)
            min.y += outWidth * across.y;
        else
            max.y += outWidth * across.y;
        if (down.y < 0)
            min.y += outHeight * down.y;
        else
            max.y += outHeight * down.y;

        Point2<P> carriage_return = down - across * outWidth;

        Point2<P> p = p0;
        S* outStr = out.data();

        if ((min.x >= 0) && (min.y >= 0) && (max.x < inWidth-1) && (max.y < inHeight-1)) {
            for (int i=0; i<outHeight; ++i, p += carriage_return) {
                for (int j=0; j<outWidth; ++j, p += across)
                    outStr[j] = interpolate<T>(in, p);
                outStr = &outStr[outWidth];
            }
            return 0;
        }

        Point2<P> bound((P)(inWidth - 1), (P)(inHeight - 1));
        int count = 0;
        for (int i=0; i<outHeight; ++i, p += carriage_return) {
            for (int j=0; j<outWidth; ++j, p += across) {
                if (((P)(0) <= p.x) && ((P)(0) <= p.y) &&  (p.x < bound.x) && (p.y < bound.y)) {
                    outStr[j] = interpolate<T>(in, p);
                } else {
                    outStr[j] = defaultValue;
                    ++count;
                }
            }
            outStr = &outStr[outWidth];
        }
        return count;
    }

    template <typename Type>
    static void copyFragment(Image<Type>& out, const ImageRef<Type>& scr, const Point2i& sizeFragment,
                             const Point2i& beginInOut, const Point2i& beginInScr)
    {
        assert((beginInOut.x >= 0) && (beginInOut.y >= 0) && (beginInScr.x >= 0) && (beginInScr.y >= 0));
        assert((sizeFragment.x >= 0) && (sizeFragment.y >= 0));
        assert((out.width() >= (beginInOut.x + sizeFragment.x)) && (out.height() >= (beginInOut.y + sizeFragment.y)));
        assert((scr.width() >= (beginInScr.x + sizeFragment.x)) && (scr.height() >= (beginInScr.y + sizeFragment.y)));
        Type* outStr = out.pointer(beginInOut);
        const Type* scrStr = scr.pointer(beginInScr);
        Point2i p;
        for (p.y = 0; p.y < sizeFragment.y; ++p.y) {
            for (p.x = 0; p.x < sizeFragment.x; ++p.x) {
                outStr[p.x] = scrStr[p.x];
            }
            outStr = &outStr[out.width()];
            scrStr = &scrStr[scr.width()];
        }
    }
};

}

#endif // AR_IMAGEPROCESSING_H
