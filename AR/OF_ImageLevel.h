#ifndef OF_OF_ImageLevel_H
#define OF_OF_ImageLevel_H

#include <vector>

#include "AR/Image.h"
#include "AR/FastFeature.h"

namespace AR {

class OF_System;

class OF_ImageLevel
{
    friend class OF_System;

public:
    typedef struct OpticalFlowInfo1D
    {
        float d;
        float value;
    } OpticalFlowInfo1D;
    typedef struct OpticalFlowInfo2D
    {
        IPointF d;
        float value;
    } OpticalFlowInfo2D;

public:
    OF_ImageLevel();
    ~OF_ImageLevel();

    IPoint size() const { return _curImage.size(); }
    void resize(const IPoint& size);
    IPoint sizeCursor() const { return _sizeCursor; }
    void setSizeCursor(const IPoint& sizeCursor);
    float sigmaGaussian() const { return _sigmaGaussian; }
    void setSigmaGaussian(float sigma);
    IPoint windowBegin() const { return _windowBegin; }
    IPoint windowEnd() const { return _windowEnd; }
    void copyCurrentImageToPrev() { Image<uchar>::copyData(_prevImage, _curImage); }
    void swapCurrentImageToPrev() { Image<uchar>::swap(_prevImage, _curImage); }
    void setCurrentImage(const Image<uchar>& image) { Image<uchar>::copyData(_curImage, image); }
    void setPrevImage(const Image<uchar>& image) { Image<uchar>::copyData(_prevImage, image); }
    void setCurrentImageFromHalfSample(const Image<uchar>& image) { Image<uchar>::halfSample(_curImage, image); }
    void setPrevImageFromHalfSample(const Image<uchar>& image) { Image<uchar>::halfSample(_prevImage, image); }
    inline Image<uchar>& currentImage() { return _curImage; }
    inline const Image<uchar>& currentImage() const { return _curImage; }
    inline Image<uchar>& prevImage() { return _prevImage; }
    inline const Image<uchar>& prevImage() const { return _prevImage; }
    inline int barrier() const { return _barrier; }
    inline void setBarrier(int barrier) { _barrier = barrier; }
    inline float shiTomasiScoreThreshold() const { return _shiTomasiScoreThreshold; }
    inline void setShiTomasiScoreThreshold(float threshold) { _shiTomasiScoreThreshold = threshold; }
    inline IPointF localVelocity() const { return _localVelocity; }
    inline IPointF deltaStep() const { return _deltaStep; }

    inline bool needToNextIteration(float epsDeltaStep) const { return (_deltaStep.x > epsDeltaStep || _deltaStep.y > epsDeltaStep); }
    inline bool pointOutOfRange(const IPointF& point) const
    {
        return (point.x < 0.0f || point.x >= _curImage.width() || point.y < 0.0f || point.y >= _curImage.height());
    }
    inline bool velocityInRange(float range) const { return (qAbs(_localVelocity.x) < range && qAbs(_localVelocity.y) < range); }
    inline bool pointOutOfWindow(const IPoint& point) const
    {
        if ((point.x < _windowBegin.x) || (point.x >= _windowEnd.x))
            return true;
        if ((point.y < _windowBegin.y) || (point.y >= _windowEnd.y))
            return true;
        return false;
    }
    inline bool pointOutOfWindow(const IPointF& point) const
    {
        if ((point.x < _windowBegin.x) || (point.x >= _windowEnd.x))
            return true;
        if ((point.y < _windowBegin.y) || (point.y >= _windowEnd.y))
            return true;
        return false;
    }

    float getErrorValue(const IPointF& currentPos);

    void findCandidatePoints();
    inline std::vector<FastFeature::FastCorner>& candidateCorners() { return  _candidateCorners; }
    float detThreshold() const { return _detThreshold; }
    void setDetThreshold(float threshold) { _detThreshold = threshold; }

    IPoint regionForCorners_begin() const { return _regionForCorners_begin; }
    IPoint regionForCorners_end() const { return _regionForCorners_end; }
    void setRegionForCorners(const IPoint& begin, const IPoint& end) { _regionForCorners_begin = begin; _regionForCorners_end = end; }

    bool getSubImage(Image<uchar>& outSubImage, const IPointF& position) const;
    bool getSubOpticalFlowInfo(Image<OpticalFlowInfo2D>& outSubImage, const IPointF& position) const;
    bool iterationSubImage(IPointF& delta, const IPointF& pos, const Image<OpticalFlowInfo2D>& subImage) const;

    inline static float getSubPixelValue(const uchar* strCurrent, const uchar* strNext, const int x,
                                   const IPointF& subPixelDelta, const IPointF& invSubPixelDelta)
    {
        return ((strCurrent[x] * invSubPixelDelta.x * invSubPixelDelta.y) +
                (strCurrent[x+1] * subPixelDelta.x * invSubPixelDelta.y) +
                (strNext[x] * invSubPixelDelta.x * subPixelDelta.y) +
                (strNext[x+1] * subPixelDelta.x * subPixelDelta.y));
    }
    void getDiffImage(Image<IPointF>& out) const;
    bool startTrackPoint(const IPointF& point, const IPointF& pointOnPrevImage);
    bool iterationTrackPoint(const IPointF& point);
    bool startTrackPoint(const Image<IPointF>& diffImage, const IPointF& point, const IPoint& pointOnPrevImage);
    bool startTrackPointOnLine(const IPointF& point, const IPointF& dir, const IPointF& pointOnPrevImage);
    bool iterationTrackPointOnLine(const IPointF& point, const IPointF& dir);

private:
    Image<uchar> _curImage;
    Image<uchar> _prevImage;
    int _barrier;
    float _shiTomasiScoreThreshold;
#if (FASTFEATURE_USE_NONMAXSUP == 1)
    std::vector<FastFeature::FastCorner> _rawCorners;
#endif
    std::vector<FastFeature::FastCorner> _candidateCorners;

    float* _gaussian;
    float _sigmaGaussian;
    float _invMaxError;
    float _detThreshold;
    OpticalFlowInfo2D* _tempInfo2D;
    OpticalFlowInfo1D* _tempInfo1D;
    IPoint _sizeCursor;
    IPoint _windowBegin;
    IPoint _windowEnd;
    IPoint _regionForCorners_begin;
    IPoint _regionForCorners_end;
    float _Dxx;
    float _Dxy;
    float _Dyy;
    IPointF _deltaStep;
    IPointF _localVelocity;

    void _solveGaussian();
    void _sortingCandidateCorners(int maxSize);
};

}

#endif // OF_OF_ImageLevel_H
