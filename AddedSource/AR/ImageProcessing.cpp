#include "ImageProcessing.h"
#include <cmath>
#include <cassert>
#include "TMath/TTools.h"

namespace AR {

void ImageProcessing::sobel(Image<bool>& out, Image<int>& outIntegral, const ImageRef<uchar>& in, const uchar threshold)
{
    assert((in.size() == out.size()) && (out.size() == outIntegral.size()));
    const signed char op[2][3] = { {1, 2, 1}, {-1, -2, -1} };
    const uchar* strPrev;
    const uchar* strCur = in.data();
    const uchar* strNext = &strCur[in.width()];
    bool* strOut;
    int* strPrevOutIntegral;
    int* strOutIntegral = outIntegral.data();
    int Gx, Gy;
    int w = in.width() - 1;
    int h = in.height() - 1;
    int rs;
    Point2i p;
    for (p.y=1; p.y<h; ++p.y) {
        strPrev = strCur;
        strCur = strNext;
        const int delta = in.width() * p.y;
        strNext = &in.data()[delta];
        strOut = &out.data()[delta];
        strPrevOutIntegral = strOutIntegral;
        strOutIntegral = &outIntegral.data()[delta];
        rs = 0;
        for (p.x=1; p.x<w; ++p.x) {
            Gx = strPrev[p.x-1] * op[0][0] + strPrev[p.x] * op[0][1] + strPrev[p.x+1] * op[0][2];
            Gx += strNext[p.x-1] * op[1][0] + strNext[p.x] * op[1][1] + strNext[p.x+1] * op[1][2];
            Gy = strPrev[p.x-1] * op[0][0] + strCur[p.x-1] * op[0][1] + strPrev[p.x-1] * op[0][2];
            Gy += strPrev[p.x+1] * op[1][0] + strCur[p.x+1] * op[1][1] + strPrev[p.x+1] * op[1][2];
            const bool val = ((((Gx > 0) ? Gx : -Gx) + ((Gy > 0) ? Gy : -Gy)) > threshold);
            strOut[p.x] = val;
            if (val)
                ++rs;
            strOutIntegral[p.x] = rs + strPrevOutIntegral[p.x];
        }
    }
}

void ImageProcessing::erode(Image<bool>& out, const ImageRef<int>& integral, int size, float k)
{
    assert(out.size() == integral.size());
    const int minCountPixels = static_cast<int>((size * 2 + 1) * (size * 2 + 1) * k);
    bool* strOut;
    Point2i p;
    int w = integral.width() - size;
    int h = integral.height() - size;
    for (p.y=size; p.y<h; ++p.y) {
        strOut = &out.data()[p.y * out.width()];
        const int* strA = &integral.data()[(p.y - size) * integral.width()];
        const int* strB = &integral.data()[(p.y + size) * integral.width()];
        for (p.x=size; p.x<w; ++p.x) {
            strOut[p.x] = ((strB[p.x + size] + strA[p.x - size] - (strB[p.x - size] + strA[p.x + size])) > minCountPixels);
        }
    }
}

#if QT_MULTIMEDIA_LIB
Image<Rgba> ImageProcessing::convertQImage(const QImage& image)
{
    Image<Rgba> result(Point2i(image.width(), image.height()));
    Point2i p;
    if (image.allGray()) {
        for (p.y=0; p.y<result.height(); ++p.y) {
            Rgba* resultStr = &result.data()[result.width() * p.y];
            for (p.x=0; p.x<result.width(); ++p.x) {
                int gray = qGray(image.pixel(p.x, p.y));
                resultStr[p.x].set(gray, gray, gray, 255);
            }
        }
    } else {
        for (p.y=0; p.y<result.height(); ++p.y) {
            Rgba* resultStr = &result.data()[result.width() * p.y];
            for (p.x=0; p.x<result.width(); ++p.x) {
                QRgb rgba = image.pixel(p.x, p.y);
                resultStr[p.x].set(static_cast<uchar>(qRed(rgba)), static_cast<uchar>(qGreen(rgba)),
                                   static_cast<uchar>(qBlue(rgba)), static_cast<uchar>(qAlpha(rgba)));
            }
        }
    }
    return result;
}
#endif

Image<uchar> ImageProcessing::convertToBW(const ImageRef<Rgb>& image)
{
    Image<uchar> result(image.size());
    Point2i p;
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

}
