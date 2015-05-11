#include "AR/OF_ImageLevel.h"
#include "AR/ARSystem.h"
#include <algorithm>

namespace AR {

OF_ImageLevel::OF_ImageLevel()
{
    _gaussian = nullptr;
    _tempInfo1D = nullptr;
    _tempInfo2D = nullptr;
    _invMaxError = 0.0f;
    _sizeCursor.set(10, 10);
    _sigmaGaussian = 3.33f;
    _barrier = 0;
    resize(_sizeCursor * 4);
    setSizeCursor(_sizeCursor);
    _regionForCorners_begin.set(3, 3);
    _regionForCorners_end = _curImage.size() - _regionForCorners_begin;
}

OF_ImageLevel::~OF_ImageLevel()
{
#if (FASTFEATURE_USE_NONMAXSUP == 1)
    _rawCorners.clear();
#endif
    _candidateCorners.clear();
    if (_gaussian)
        delete[] _gaussian;
    if (_tempInfo1D)
        delete[] _tempInfo1D;
    if (_tempInfo2D)
        delete[] _tempInfo2D;
}

void OF_ImageLevel::resize(const IPoint& size)
{
    if (_curImage.size() != size) {
        _curImage.resize(size);
        _prevImage.resize(size);
    }
    _windowBegin = _sizeCursor + IPoint(1, 1);
    _windowEnd = _curImage.size() - _windowBegin;
}

void OF_ImageLevel::setSizeCursor(const IPoint& sizeCursor)
{
    _sizeCursor = sizeCursor;
    _sigmaGaussian = qMax(sizeCursor.x, sizeCursor.y) / 3.0f;
    int memSize = (_sizeCursor.y * 2 + 1) * (_sizeCursor.x * 2 + 1);
    _gaussian = static_cast<float*>(realloc(_gaussian, sizeof(float) * memSize));
    _tempInfo1D = static_cast<OpticalFlowInfo1D*>(realloc(_tempInfo1D, sizeof(OpticalFlowInfo1D) * memSize));
    _tempInfo2D = static_cast<OpticalFlowInfo2D*>(realloc(_tempInfo2D, sizeof(OpticalFlowInfo2D) * memSize));
    resize(_curImage.size());
    _solveGaussian();
}

void OF_ImageLevel::setSigmaGaussian(float sigma)
{
    _sigmaGaussian = sigma;
    _solveGaussian();
}

void OF_ImageLevel::_solveGaussian()
{
    _invMaxError = 0.0f;
    const float sigmaSquare = _sigmaGaussian * _sigmaGaussian;
    const float k1 = 1.0f / (2.0f * M_PI * sigmaSquare);
    const float invK2 = 1.0f / (2.0f * sigmaSquare);
    IPoint p;
    int k = 0;
    for (p.y=-_sizeCursor.y; p.y<=_sizeCursor.y; ++p.y) {
        for (p.x=-_sizeCursor.x; p.x<=_sizeCursor.x; ++p.x, ++k) {
            _gaussian[k] = k1 * qExp(- p.lengthSquared() * invK2);
            _invMaxError += _gaussian[k] * 255.0f * _gaussian[k] * 255.0f;
        }
    }
    _invMaxError = 1.0f / qSqrt(_invMaxError);
}

float OF_ImageLevel::getErrorValue(const IPointF& currentPos)
{
    const IPoint coordCur(qFloor(currentPos.x), qFloor(currentPos.y));
    const IPointF subPixelDelta(currentPos.x - coordCur.x, currentPos.y - coordCur.y);
    const IPointF invSubPixelDelta(1.0f - subPixelDelta.x, 1.0f - subPixelDelta.y);
    IPoint begin = coordCur - _sizeCursor;
    if (begin.x < 0)
        begin.x = 0;
    if (begin.y < 0)
        begin.y = 0;
    IPoint end = coordCur + _sizeCursor;
    if (end.x >= _curImage.width())
        end.x = _curImage.width() - 1;
    if (end.y >= _curImage.height())
        end.y = _curImage.height() - 1;

    int sk = 0, k;
    float error = 0.0f;
    IPoint p;
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[begin.y * _curImage.width()];
    for (p.y=begin.y; p.y<=end.y; ++p.y) {
        cstrA = cstrB;
        cstrB = &_curImage.data()[(p.y + 1) * _curImage.width()];
        k = sk;
        for (p.x=begin.x; p.x<=end.x; ++p.x, ++k) {
            const OpticalFlowInfo2D& temp = _tempInfo2D[k];
            const float d = (getSubPixelValue(cstrA, cstrB, p.x, subPixelDelta, invSubPixelDelta) - temp.value) * _gaussian[k];
            error += d * d;
        }
        sk += _sizeCursor.y * 2 + 1;
    }
    error = qSqrt(error) * _invMaxError;
    return error;
}

void OF_ImageLevel::_sortingCandidateCorners(int maxSize)
{
    int size = _candidateCorners.size();
    if (maxSize > size)
        maxSize = size;
    int i, j, indexMax;
    for (i=0; i<maxSize; ++i) {
        indexMax = i;
        for (j=i+1; j<size; ++j) {
            if (_candidateCorners[j].scoreShiTomasi > _candidateCorners[indexMax].scoreShiTomasi)
                indexMax = j;
        }
        std::swap(_candidateCorners[indexMax], _candidateCorners[i]);
    }
    _candidateCorners.resize(maxSize);
}

void OF_ImageLevel::findCandidatePoints()
{
    _candidateCorners.clear();
#if (FASTFEATURE_USE_NONMAXSUP == 1)
    _rawCorners.clear();
    FastFeature::fast_corner_detect_10(_curImage, _regionForCorners_begin, _regionForCorners_end,
                                       _rawCorners, _barrier, ARSystem::currentParameters.shiTomasiScoreThreshold);
    FastFeature::fastNonmaxSuppression(_rawCorners, _candidateCorners);
#else
    FastFeature::fast_corner_detect_10(_curImage, _regionForCorners_begin, _regionForCorners_end,
                                       _candidateCorners, _barrier, _shiTomasiScoreThreshold);
#endif
    std::sort(_candidateCorners.begin(), _candidateCorners.end(), FastFeature::comp);
    //_sortingCandidateCorners(maxSize);
}

bool OF_ImageLevel::startTrackPoint(const IPointF& point, const IPointF& pointOnPrevImage)
{
    const IPoint coordPrev(qFloor(pointOnPrevImage.x), qFloor(pointOnPrevImage.y));
    if (pointOutOfWindow(coordPrev))
        return false;
    const IPoint coordCur(qFloor(point.x), qFloor(point.y));
    if (pointOutOfWindow(coordCur))
        return false;
    const IPointF subPixelDelta_prev(pointOnPrevImage.x - coordPrev.x, pointOnPrevImage.y - coordPrev.y);
    const IPointF invSubPixelDelta_prev(1.0f - subPixelDelta_prev.x, 1.0f - subPixelDelta_prev.y);
    const IPointF subPixelDelta_cur(point.x - coordCur.x, point.y - coordCur.y);
    const IPointF invSubPixelDelta_cur(1.0f - subPixelDelta_cur.x, 1.0f - subPixelDelta_cur.y);

    _localVelocity = IPointF::ZERO;
    float Dxx = 0.0f, Dxy = 0.0f, Dyy = 0.0f;
    IPointF B(0.0f, 0.0f);
    const IPoint beginPrev = coordPrev - _sizeCursor;
    const IPoint endPrev = coordPrev + _sizeCursor;
    const IPoint beginCur = coordCur - _sizeCursor;

    uchar* strA;
    uchar* strB = &_prevImage.data()[_prevImage.width() * (beginPrev.y - 1)];
    uchar* strC = &strB[_prevImage.width()];
    uchar* strD = &strC[_prevImage.width()];
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[beginCur.y * _curImage.width()];
    IPoint p, pCur;
    int k = 0;
    for (p.y=beginPrev.y, pCur.y=beginCur.y; p.y<=endPrev.y; ++p.y, ++pCur.y) {
        strA = strB;
        strB = strC;
        strC = strD;
        strD = &strD[_prevImage.width()];
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=beginPrev.x, pCur.x=beginCur.x; p.x<=endPrev.x; ++p.x, ++pCur.x, ++k) {
            _tempInfo2D[k].d.x = getSubPixelValue(strA, strB, p.x+1, subPixelDelta_prev, invSubPixelDelta_prev) -
                    getSubPixelValue(strA, strB, p.x-1, subPixelDelta_prev, invSubPixelDelta_prev);
            _tempInfo2D[k].d.y = getSubPixelValue(strC, strD, p.x, subPixelDelta_prev, invSubPixelDelta_prev) -
                    getSubPixelValue(strA, strB, p.x, subPixelDelta_prev, invSubPixelDelta_prev);
            _tempInfo2D[k].value = getSubPixelValue(strB, strC, p.x, subPixelDelta_prev, invSubPixelDelta_prev);
            float dt = getSubPixelValue(cstrA, cstrB, pCur.x, subPixelDelta_cur, invSubPixelDelta_cur) -
                    _tempInfo2D[k].value;
            IPointF gd = _tempInfo2D[k].d * _gaussian[k];
            Dxx += gd.x * _tempInfo2D[k].d.x;
            Dxy += gd.x * _tempInfo2D[k].d.y;
            Dyy += gd.y * _tempInfo2D[k].d.y;
            B += gd * dt;
            _tempInfo2D[k].d = gd;
        }
    }

    float det = Dxx * Dyy - Dxy * Dxy;
    if (qAbs(det) < _detThreshold)
        return false;
    _Dxx = Dyy / det;
    _Dxy = - Dxy / det;
    _Dyy = Dxx / det;
    B = - B;
    _localVelocity.set(_Dxx * B.x + _Dxy * B.y, _Dxy * B.x + _Dyy * B.y);
    _deltaStep.set(qAbs(_localVelocity.x), qAbs(_localVelocity.y));
    return true;
}

bool OF_ImageLevel::iterationTrackPoint(const IPointF& point)
{
    const IPointF currentPos = point + _localVelocity;
    const IPoint coordCur(qFloor(currentPos.x), qFloor(currentPos.y));
    if (pointOutOfWindow(coordCur))
        return false;
    const IPointF subPixelDelta(currentPos.x - coordCur.x, currentPos.y - coordCur.y);
    const IPointF invSubPixelDelta(1.0f - subPixelDelta.x, 1.0f - subPixelDelta.y);
    const IPoint begin = coordCur - _sizeCursor;
    const IPoint end = coordCur + _sizeCursor;

    IPointF B(0.0f, 0.0f);

    int k = 0;
    IPoint p;
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[begin.y * _curImage.width()];
    for (p.y=begin.y; p.y<=end.y; ++p.y) {
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=begin.x; p.x<=end.x; ++p.x, ++k) {
            const OpticalFlowInfo2D& temp = _tempInfo2D[k];
            const float dt = getSubPixelValue(cstrA, cstrB, p.x, subPixelDelta, invSubPixelDelta) - temp.value;
            B += temp.d * dt;
        }
    }

    B = - B;
    _deltaStep.set(_Dxx * B.x + _Dxy * B.y, _Dxy * B.x + _Dyy * B.y);
    _deltaStep *= 0.9f;
    _localVelocity += _deltaStep;
    _deltaStep.x = qAbs(_deltaStep.x);
    _deltaStep.y = qAbs(_deltaStep.y);
    return true;
}

bool OF_ImageLevel::getSubImage(Image<uchar>& outSubImage, const IPointF& position) const
{
    const IPointF halfSize(outSubImage.width() * 0.5f, outSubImage.height() * 0.5f);
    IPoint begin(qFloor(position.x - halfSize.x), qFloor(position.y - halfSize.y));
    if ((begin.x < 0) || (begin.y < 0))
        return false;
    IPoint end(qCeil(position.x + halfSize.x), qCeil(position.y + halfSize.y));
    if ((end.x + 1 >= _curImage.width()) || (end.y + 1 > _curImage.height()))
        return false;
    IPointF subPixelDelta = position  - IPointF(qFloor(position.x), qFloor(position.y));
    IPointF invSubPixelDelta = IPointF(1.0f, 1.0f) - subPixelDelta;
    uchar* outStr = outSubImage.data();
    const uchar* strCur = &_curImage.data()[begin.y * _curImage.width()];
    const uchar* strNext = &strCur[_curImage.width()];
    IPoint p;
    for (p.y=0; p.y<outSubImage.height(); ++p.y) {
        for (p.x=0; p.x<outSubImage.width(); ++p.x)
            outStr[p.x] = static_cast<uchar>(getSubPixelValue(strCur, strNext, p.x, subPixelDelta, invSubPixelDelta));
        outStr = &outStr[outSubImage.width()];
        strCur = strNext;
        strNext = &strNext[_curImage.width()];
    }
    return true;
}

bool OF_ImageLevel::getSubOpticalFlowInfo(Image<OpticalFlowInfo2D>& outSubImage, const IPointF& position) const
{
    const IPointF centerSubImage(outSubImage.width() * 0.5f, outSubImage.height() * 0.5f);
    IPoint begin(qFloor(position.x - centerSubImage.x), qFloor(position.y - centerSubImage.y));
    if ((begin.x < 1) || (begin.y < 1))
        return false;
    IPoint end(qCeil(position.x + centerSubImage.x), qCeil(position.y + centerSubImage.y));
    if ((end.x + 2 >= _curImage.width()) || (end.y + 2 > _curImage.height()))
        return false;
    IPointF subPixelDelta = position - IPointF(qFloor(position.x), qFloor(position.y));
    IPointF invSubPixelDelta = IPointF(1.0f, 1.0f) - subPixelDelta;
    OpticalFlowInfo2D* outStr = outSubImage.data();
    const uchar* strPrev = &_curImage.data()[(begin.y - 1) * _curImage.width()];
    const uchar* strCur = &strPrev[_curImage.width()];
    const uchar* strNext = &strCur[_curImage.width()];
    const uchar* strNext1 = &strNext[_curImage.width()];
    IPoint p;
    int x1;
    for (p.y=0; p.y<outSubImage.height(); ++p.y) {
        for (p.x=0; p.x<outSubImage.width(); ++p.x) {
            x1 = begin.x + p.x;
            OpticalFlowInfo2D& cur = outStr[p.x];
            cur.value = getSubPixelValue(strCur, strNext, x1, subPixelDelta, invSubPixelDelta);
            cur.d.x = getSubPixelValue(strCur, strNext, x1 + 1, subPixelDelta, invSubPixelDelta) -
                    getSubPixelValue(strCur, strNext, x1 - 1, subPixelDelta, invSubPixelDelta);
            cur.d.y = getSubPixelValue(strNext, strNext1, x1, subPixelDelta, invSubPixelDelta) -
                    getSubPixelValue(strPrev, strCur, x1, subPixelDelta, invSubPixelDelta);
        }
        outStr = &outStr[outSubImage.width()];
        strPrev = strCur;
        strCur = strNext;
        strNext = strNext1;
        strNext1 = &strNext1[_curImage.width()];
    }
    return true;
}

bool OF_ImageLevel::iterationSubImage(IPointF& delta, const IPointF& pos, const Image<OpticalFlowInfo2D>& subImage) const
{
    IPointF centerOfSubImage(subImage.width() * 0.5f, subImage.height() * 0.5f);
    IPoint coordCur(qFloor(pos.x), qFloor(pos.y));
    IPoint begin(qFloor(pos.x - centerOfSubImage.x), qFloor(pos.y - centerOfSubImage.y));
    if ((begin.x < 1) || (begin.y < 1))
        return false;
    IPoint end(qCeil(pos.x + centerOfSubImage.x), qCeil(pos.y + centerOfSubImage.y));
    if ((end.x + 2 >= _curImage.width()) || (end.y + 2 > _curImage.height()))
        return false;
    IPointF subPixelDelta(pos.x - coordCur.x, pos.y - coordCur.y);
    IPointF invSubPixelDelta(1.0f - subPixelDelta.x, 1.0f - subPixelDelta.y);

    IPointF B(0.0f, 0.0f);
    float Dxx = 0.0f, Dxy = 0.0f, Dyy = 0.0f;

    const float sigmaSquare = _sigmaGaussian * _sigmaGaussian;
    const float k1 = 1.0f / (2.0f * M_PI * sigmaSquare);
    const float invK2 = 1.0f / (2.0f * sigmaSquare);
    IPoint p;
    const OpticalFlowInfo2D* strI = subImage.data();
    const uchar* cstrA;
    const uchar* cstrB = &_curImage.data()[begin.y * _curImage.width()];
    for (p.y=0; p.y<subImage.height(); ++p.y) {
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=0; p.x<subImage.width(); ++p.x) {
            const OpticalFlowInfo2D& temp = strI[p.x];
            float dt = temp.value - getSubPixelValue(cstrA, cstrB, begin.x + p.x, subPixelDelta, invSubPixelDelta);
            float g = (k1 * qExp(- (IPointF(p.x, p.y) - centerOfSubImage).lengthSquared() * invK2));
            IPointF gd = temp.d * g;
            Dxx += gd.x * temp.d.x;
            Dxy += gd.x * temp.d.y;
            Dyy += gd.y * temp.d.y;
            B += gd * dt;
        }
        strI = &strI[subImage.width()];
    }

    float det = Dxx * Dyy - Dxy * Dxy;
    if (qAbs(det) < 0.0001f)
        return false;
    Dyy = Dyy / det;
    Dxy = - Dxy / det;
    Dxx = Dxx / det;
    //B = - B;
    delta.set(Dyy * B.x + Dxy * B.y, Dxy * B.x + Dxx * B.y);
    delta *= 0.9f;
    return true;
}

void OF_ImageLevel::getDiffImage(Image<IPointF>& out) const
{
    const uchar* strA = _prevImage.data();
    const uchar* strB = &strA[_prevImage.width()];
    const uchar* strC = &strB[_prevImage.width()];
    IPointF* outStr = out.data();
    IPoint p;
    for (p.y=1; p.y<_prevImage.height()-1; ++p.y) {
        for (p.x=1; p.x<_prevImage.width()-1; ++p.x) {
            outStr[p.x-1].set(strB[p.x + 1] - strB[p.x - 1], strC[p.x] - strA[p.x]);
        }
        strA = strB;
        strB = strC;
        strC = &strC[_prevImage.width()];
        outStr = &outStr[out.width()];
    }
}

bool OF_ImageLevel::startTrackPoint(const Image<IPointF>& diffImage, const IPointF& point, const IPoint& pointOnPrevImage)
{
    if (pointOutOfWindow(pointOnPrevImage))
        return false;
    const IPoint coordCur(qFloor(point.x), qFloor(point.y));
    if (pointOutOfWindow(coordCur))
        return false;
    const IPointF subPixelDelta_cur(point.x - coordCur.x, point.y - coordCur.y);
    const IPointF invSubPixelDelta_cur(1.0f - subPixelDelta_cur.x, 1.0f - subPixelDelta_cur.y);

    _localVelocity = IPointF::ZERO;
    float Dxx = 0.0f, Dxy = 0.0f, Dyy = 0.0f;
    IPointF B(0.0f, 0.0f);
    const IPoint beginPrev = pointOnPrevImage - _sizeCursor;
    const IPoint endPrev = pointOnPrevImage + _sizeCursor;
    const IPoint beginCur = coordCur - _sizeCursor;

    uchar* strPrev = &_prevImage.data()[_prevImage.width() * (beginPrev.y - 1)];
    const IPointF* strDiff = &diffImage.data()[diffImage.width() * (beginPrev.y - 2)];
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[beginCur.y * _curImage.width()];
    IPoint p, pCur;
    int k = 0;
    for (p.y=beginPrev.y, pCur.y=beginCur.y; p.y<=endPrev.y; ++p.y, ++pCur.y) {
        strPrev = &strPrev[_prevImage.width()];
        strDiff = &strDiff[diffImage.width()];
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=beginPrev.x, pCur.x=beginCur.x; p.x<=endPrev.x; ++p.x, ++pCur.x, ++k) {
            _tempInfo2D[k].value = static_cast<float>(strPrev[p.x]);
            float dt = getSubPixelValue(cstrA, cstrB, pCur.x, subPixelDelta_cur, invSubPixelDelta_cur) -
                    _tempInfo2D[k].value;
            const IPointF& diff = strDiff[p.x - 1];
            _tempInfo2D[k].d = diff * _gaussian[k];
            Dxx += _tempInfo2D[k].d.x * diff.x;
            Dxy += _tempInfo2D[k].d.x * diff.y;
            Dyy += _tempInfo2D[k].d.y * diff.y;
            B += _tempInfo2D[k].d * dt;
        }
    }

    float det = Dxx * Dyy - Dxy * Dxy;
    if (qAbs(det) < _detThreshold)
        return false;
    det = 1.0f / det;
    _Dxx = Dyy * det;
    _Dxy = - Dxy * det;
    _Dyy = Dxx * det;
    B = - B;
    _localVelocity.set(_Dxx * B.x + _Dxy * B.y, _Dxy * B.x + _Dyy * B.y);
    _deltaStep.set(qAbs(_localVelocity.x), qAbs(_localVelocity.y));
    return true;
}

bool OF_ImageLevel::startTrackPointOnLine(const IPointF& point, const IPointF& dir, const IPointF& pointOnPrevImage)
{
    const IPoint coordPrev(qFloor(pointOnPrevImage.x), qFloor(pointOnPrevImage.y));
    if (pointOutOfWindow(coordPrev))
        return false;
    const IPoint coordCur(qFloor(point.x), qFloor(point.y));
    if (pointOutOfWindow(coordCur))
        return false;
    IPointF subPixelDelta_prev(pointOnPrevImage.x - coordPrev.x, pointOnPrevImage.y - coordPrev.y);
    IPointF invSubPixelDelta_prev(1.0f - subPixelDelta_prev.x, 1.0f - subPixelDelta_prev.y);
    IPointF subPixelDelta_cur(point.x - coordCur.x, point.y - coordCur.y);
    IPointF invSubPixelDelta_cur(1.0f - subPixelDelta_cur.x, 1.0f - subPixelDelta_cur.y);
    IPointF subPixelDelta_start = subPixelDelta_cur - dir * 0.5f;
    IPointF subPixelDelta_end = subPixelDelta_cur + dir * 0.5f;
    int indexOffsetX_start, indexOffsetY_start;
    int indexOffsetX_end, indexOffsetY_end;
    if (subPixelDelta_start.x < 0.0f) {
        indexOffsetX_start = -1;
        subPixelDelta_start.x += 1.0f;
    } else {
        indexOffsetX_start = 0;
    }
    if (subPixelDelta_start.y < 0.0f) {
        indexOffsetY_start = 0;
        subPixelDelta_start.y += 1.0f;
    } else {
        indexOffsetY_start = 1;
    }
    if (subPixelDelta_end.x > 1.0f) {
        indexOffsetX_end = 1;
        subPixelDelta_end.x -= 1.0f;
    } else {
        indexOffsetX_end = 0;
    }
    if (subPixelDelta_end.y > 1.0f) {
        indexOffsetY_end = 2;
        subPixelDelta_end.y -= 1.0f;
    } else {
        indexOffsetY_end = 1;
    }
    IPointF invSubPixelDelta_start(1.0f - subPixelDelta_start.x, 1.0f - subPixelDelta_start.y);
    IPointF invSubPixelDelta_end(1.0f - subPixelDelta_end.x, 1.0f - subPixelDelta_end.y);

    _localVelocity = IPointF::ZERO;
    float D = 0.0f;
    float B = 0.0f;
    IPoint beginPrev = coordPrev - _sizeCursor;
    IPoint endPrev = coordPrev + _sizeCursor;
    IPoint beginCur = coordCur - _sizeCursor;

    uchar* strPrev[4];
    strPrev[1] = &_prevImage.data()[_prevImage.width() * (beginPrev.y - 1)];
    strPrev[2] = &strPrev[1][_prevImage.width()];
    strPrev[3] = &strPrev[2][_prevImage.width()];
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[beginCur.y * _curImage.width()];
    IPoint p, pCur;
    int k = 0;
    for (p.y=beginPrev.y, pCur.y=beginCur.y; p.y<=endPrev.y; ++p.y, ++pCur.y) {
        strPrev[0] = strPrev[1];
        strPrev[1] = strPrev[2];
        strPrev[2] = strPrev[3];
        strPrev[3] = &strPrev[3][_prevImage.width()];
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=beginPrev.x, pCur.x=beginCur.x; p.x<=endPrev.x; ++p.x, ++pCur.x, ++k) {
            _tempInfo1D[k].d = (getSubPixelValue(strPrev[indexOffsetY_end], strPrev[indexOffsetY_end+1], p.x+indexOffsetX_end,
                                subPixelDelta_end, invSubPixelDelta_end)-
                    getSubPixelValue(strPrev[indexOffsetY_start], strPrev[indexOffsetY_start+1], p.x+indexOffsetX_start,
                    subPixelDelta_start, invSubPixelDelta_start)) * _gaussian[k];
            _tempInfo1D[k].value = getSubPixelValue(strPrev[1], strPrev[2], p.x, subPixelDelta_prev, invSubPixelDelta_prev);
            D += _tempInfo1D[k].d;
            B += (getSubPixelValue(cstrA, cstrB, pCur.x, subPixelDelta_cur, invSubPixelDelta_cur) -
                    _tempInfo1D[k].value) * _gaussian[k];
        }
    }
    if (qAbs(D) < _detThreshold)
        return false;
    _Dxx = - 1.0f / D;
    _localVelocity = dir * (B * _Dxx);
    _deltaStep.set(qAbs(_localVelocity.x), qAbs(_localVelocity.y));
    return true;
}

bool OF_ImageLevel::iterationTrackPointOnLine(const IPointF& point, const IPointF& dir)
{
    const IPointF currentPos = point + _localVelocity;
    const IPoint coordCur(qFloor(currentPos.x), qFloor(currentPos.y));
    if (pointOutOfWindow(coordCur))
        return false;
    const IPointF subPixelDelta(currentPos.x - coordCur.x, currentPos.y - coordCur.y);
    const IPointF invSubPixelDelta(1.0f - subPixelDelta.x, 1.0f - subPixelDelta.y);
    const IPoint begin = coordCur - _sizeCursor;
    const IPoint end = coordCur + _sizeCursor;

    float B = 0.0f;

    int k = 0;
    IPoint p;
    uchar* cstrA;
    uchar* cstrB = &_curImage.data()[begin.y * _curImage.width()];
    for (p.y=begin.y; p.y<=end.y; ++p.y) {
        cstrA = cstrB;
        cstrB = &cstrB[_curImage.width()];
        for (p.x=begin.x; p.x<=end.x; ++p.x, ++k) {
            B += (getSubPixelValue(cstrA, cstrB, p.x, subPixelDelta, invSubPixelDelta) - _tempInfo1D[k].value) * _gaussian[k];
        }
    }

    _deltaStep = dir * (B * _Dxx);
    _deltaStep *= 0.9f;
    _localVelocity += _deltaStep;
    _deltaStep.x = qAbs(_deltaStep.x);
    _deltaStep.y = qAbs(_deltaStep.y);
    return true;
}

}
