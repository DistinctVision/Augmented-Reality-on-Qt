#ifndef AR_IMAGERESIZER_H
#define AR_IMAGERESIZER_H

#include <cmath>
#include "Image.h"

namespace AR {

class ImageResizer
{
public:
    void inputImage(const ImageRef<uchar>& input)
    {
        if ((input.width() < 3) || (input.height() < 3))
            return;
        if (_integralImage.size() != input.size())
            _integralImage = Image<int>(input.size());
        int* strIntegral = _integralImage.data();
        const uchar* strBW = input.data();
        int i, j;
        int rs = 0;
        for (j=0; j<_integralImage.width(); ++j) {
            rs += strBW[j];
            strIntegral[j] = rs;
        }
        const int stepIntegral = _integralImage.width();
        for (i=1; i<_integralImage.height(); ++i) {
            strIntegral = &strIntegral[stepIntegral];
            strBW = &strBW[stepIntegral];
            rs = 0;
            for (j=0; j<stepIntegral; ++j) {
                rs += strBW[j];
                strIntegral[j] = rs + strIntegral[j - stepIntegral];
            }
        }
    }

    void scaleImage(Image<uchar>& out, int sizeBlur)
    {
        if ((out.width() < 3) || (out.height() < 3))
            return;
        if ((_integralImage.width() < 3) || (_integralImage.height() < 3))
            return;
        int i, j;
        IPointF scale(_integralImage.width() / static_cast<float>(out.width()),
                      _integralImage.height() / static_cast<float>(out.height()));
        IPoint sourceSizeBlur((int)std::floor(sizeBlur * scale.x), (int)std::floor(sizeBlur * scale.y));
        IPoint endPoint = out.size() - IPoint(1, 1);
        IPoint endSourcePoint = _integralImage.size() - IPoint(1, 1);
        IPointF sourcePointF;
        IPoint sourcePoint, invSourcePoint;
        float area;
        IPoint endPointOfArea = out.size() - IPoint(sizeBlur, sizeBlur);
        float invArea = 1.0f / static_cast<float>((sourceSizeBlur.x * 2) * (sourceSizeBlur.y * 2));
        uchar* strBW = &out.data()[out.width() * sizeBlur];
        for (i=sizeBlur; i<endPointOfArea.y; ++i) {
            sourcePoint.y = (int)std::floor(scale.y * i);
            for (j=sizeBlur; j<endPointOfArea.x; ++j) {
                sourcePoint.x = (int)std::floor(scale.x * j);
                strBW[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x,
                                                              sourcePoint.y-sourceSizeBlur.y) +
                                            _integralImage(sourcePoint.x+sourceSizeBlur.x,
                                                           sourcePoint.y+sourceSizeBlur.y) -
                                           (_integralImage(sourcePoint.x-sourceSizeBlur.x,
                                                           sourcePoint.y+sourceSizeBlur.y) +
                                            _integralImage(sourcePoint.x+sourceSizeBlur.x,
                                                           sourcePoint.y-sourceSizeBlur.y))
                                            ) * invArea);
            }
            strBW = &strBW[out.width()];
        }
        strBW = out.data();
        uchar* strBW_inv = &out.data()[out.width() * endPoint.y];
        float a = scale.x * sizeBlur * 2.0f;
        for (i=0; i<sizeBlur; ++i) {
            sourcePointF.y = scale.y * (i + sizeBlur);
            sourcePoint.y = (int)std::floor(sourcePointF.y);
            invSourcePoint.y = endSourcePoint.y - sourcePoint.y;
            invArea = 1.0f / (a * sourcePointF.y);
            for (j=sizeBlur; j<endPointOfArea.x; ++j) {
                sourcePoint.x = (int)std::floor(scale.x * j);
                strBW[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x, 0) +
                                               _integralImage(sourcePoint.x+sourceSizeBlur.x, sourcePoint.y) -
                                              (_integralImage(sourcePoint.x-sourceSizeBlur.x, sourcePoint.y) +
                                               _integralImage(sourcePoint.x+sourceSizeBlur.x, 0))) * invArea);
                strBW_inv[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x, invSourcePoint.y) +
                                                   _integralImage(sourcePoint.x+sourceSizeBlur.x, endSourcePoint.y) -
                                                  (_integralImage(sourcePoint.x-sourceSizeBlur.x, endSourcePoint.y) +
                                                   _integralImage(sourcePoint.x+sourceSizeBlur.x, invSourcePoint.y))
                                                   ) * invArea);
            }
            strBW = &strBW[out.width()];
            strBW_inv = &strBW_inv[-out.width()];
        }
        strBW = &out.data()[out.width() * sizeBlur];
        for (i=sizeBlur; i<endPointOfArea.y; ++i) {
            sourcePoint.y = (int)std::floor(scale.y * i);
            for (j=0; j<sizeBlur; ++j) {
                sourcePointF.x = scale.x * (j + sizeBlur);
                sourcePoint.x = (int)std::floor(sourcePointF.x);
                invSourcePoint.x = endSourcePoint.x - sourcePoint.x;
                area = (a * sourcePointF.x);
                strBW[j] = static_cast<uchar>((_integralImage(0, sourcePoint.y-sourceSizeBlur.y) +
                                               _integralImage(sourcePoint.x, sourcePoint.y+sourceSizeBlur.y) -
                                              (_integralImage(0, sourcePoint.y+sourceSizeBlur.y) +
                                               _integralImage(sourcePoint.x, sourcePoint.y-sourceSizeBlur.y))) / area);
                strBW[endPoint.x - j] = static_cast<uchar>((_integralImage(invSourcePoint.x,
                                                                           sourcePoint.y-sourceSizeBlur.y) +
                                                            _integralImage(endSourcePoint.x,
                                                                           sourcePoint.y+sourceSizeBlur.y) -
                                                           (_integralImage(invSourcePoint.x,
                                                                           sourcePoint.y+sourceSizeBlur.y) +
                                                            _integralImage(endSourcePoint.x,
                                                                           sourcePoint.y-sourceSizeBlur.y))) / area);
            }
            strBW = &strBW[out.width()];
        }
        int invJ;
        strBW = out.data();
        strBW_inv = &out.data()[out.width() * endPoint.y];
        for (i=0; i<sizeBlur; ++i) {
            sourcePointF.y = scale.y * (i + sizeBlur);
            sourcePoint.y = (int)std::floor(sourcePointF.y);
            invSourcePoint.y = endSourcePoint.y - sourcePoint.y;
            for (j=0; j<sizeBlur; ++j) {
                sourcePointF.x = scale.x * (j + sizeBlur);
                sourcePoint.x = (int)std::floor(sourcePointF.x);
                invSourcePoint.x = endSourcePoint.x - sourcePoint.x;
                area = sourcePointF.y * sourcePointF.x;
                strBW[j] = static_cast<uchar>(((_integralImage(0, 0) +
                             _integralImage(sourcePoint)) -
                            (_integralImage(0, sourcePoint.y) +
                             _integralImage(sourcePoint.x, 0))) / area);
                invJ = endPoint.x - j;
                strBW[invJ] = static_cast<uchar>(((_integralImage(invSourcePoint.x, 0) +
                                _integralImage(endSourcePoint.x, sourcePoint.y)) -
                                (_integralImage(invSourcePoint.x, sourcePoint.y) +
                                 _integralImage(endSourcePoint.x, 0))) / area);
                strBW_inv[j] = static_cast<uchar>(((_integralImage(0, invSourcePoint.y) +
                                 _integralImage(sourcePoint.x, endSourcePoint.y)) -
                                (_integralImage(0, endSourcePoint.y) +
                                 _integralImage(sourcePoint.x, invSourcePoint.y))) / area);
                strBW_inv[invJ] = static_cast<uchar>(((_integralImage(invSourcePoint) +
                                    _integralImage(endSourcePoint)) -
                                   (_integralImage(invSourcePoint.x, endSourcePoint.y) +
                                    _integralImage(endSourcePoint.x, invSourcePoint.y))) / area);
            }
            strBW = &strBW[out.width()];
            strBW_inv = &strBW_inv[-out.width()];
        }
    }

private:
    Image<int> _integralImage;
};

}
#endif // AR_IMAGERESIZER_H
