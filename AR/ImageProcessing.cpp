#include "AR/ImageProcessing.h"
#include <assert.h>

namespace AR {

void ImageProcessing::sobel(Image<uchar>& in, Image<bool> &out, Image<int>& outIntegral, const uchar threshold)
{
    assert(in.size() == out.size() && out.size() == outIntegral.size());
    const signed char op[2][3] = { {1, 2, 1}, {-1, -2, -1} };
    uchar* strPrev;
    uchar* strCur = in.data();
    uchar* strNext = &in.data()[in.width()];
    bool* strOut;
    int* strPrevOutIntegral;
    int* strOutIntegral = outIntegral.data();
    int Gx, Gy;
    int w = in.width() - 1;
    int h = in.height() - 1;
    int rs;
    IPoint p;
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
            const bool val = ((qAbs(Gx) + qAbs(Gy)) > threshold);
            strOut[p.x] = val;
            if (val)
                ++rs;
            strOutIntegral[p.x] = rs + strPrevOutIntegral[p.x];
        }
    }
}

void ImageProcessing::erode(Image<int>& integral, Image<bool>& out, int size, float k)
{
    assert(out.size() == integral.size());
    const int minCountPixels = static_cast<int>((size * 2 + 1) * (size * 2 + 1) * k);
    bool* strOut;
    IPoint p;
    int w = integral.width() - size;
    int h = integral.height() - size;
    for (p.y=size; p.y<h; ++p.y) {
        strOut = &out.data()[p.y * out.width()];
        const int* strA = &integral.data()[(p.y - size) * integral.width()];
        const int* strB = &integral.data()[(p.y + size) * integral.width()];
        for (p.x=size; p.x<w; ++p.x) {
            const int r = ((strB[p.x + size] + strA[p.x - size] - (strB[p.x - size] + strA[p.x + size])) > minCountPixels);
            strOut[p.x] = r;
        }
    }
}

}
