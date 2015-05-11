#ifndef OF_SYSTEM_H
#define OF_SYSTEM_H

#include <assert.h>
#include <vector>
#include "AR/Image.h"
#include "AR/OF_ImageLevel.h"

namespace AR {

class OF_System
{

public:
    OF_System() {_imageLevels = new OF_ImageLevel[1]; _countImageLevels = 1; _hashCellSize = 0.0f;
                 _checkLevel = 0; resize(IPoint(100, 100)); }
    ~OF_System() { delete[] _imageLevels; }

    int countImageLevels() const { return _countImageLevels; }
    const OF_ImageLevel& imageLevel(int index) const { return _imageLevels[index]; }
    OF_ImageLevel& imageLevel(int index) { return _imageLevels[index]; }
    void setCountImageLevels(int count)
    {
        assert(count > 0); delete[] _imageLevels;
        _countImageLevels = count;
        _imageLevels = new OF_ImageLevel[_countImageLevels];
        resize(size());
    }

    void setCurrentImage(const Image<uchar>& image)
    {
        _imageLevels[0].setCurrentImage(image);
        for (int i=1; i<_countImageLevels; ++i)
            _imageLevels[i].setCurrentImageFromHalfSample(_imageLevels[i-1].currentImage());
    }

    void setCurrentImageToPrev()
    {
        _imageLevels[0].copyCurrentImageToPrev();
        for (int i=1; i<_countImageLevels; ++i)
            _imageLevels[i].swapCurrentImageToPrev();
    }
    void setPrevImage(const Image<uchar>& image)
    {
        _imageLevels[0].setPrevImage(image);
        for (int i=1; i<_countImageLevels; ++i)
            _imageLevels[i].setPrevImageFromHalfSample(_imageLevels[i-1].prevImage());
    }
    IPoint size() const { return _imageLevels[0].size(); }
    void resize(const IPoint& size)
    {
        _imageLevels[0].resize(size);
        for (int i=1; i<_countImageLevels; ++i)
            _imageLevels[i].resize(_imageLevels[i-1].size() / 2);
        _updateSizeOfCasheImage();
        setRegionForCorners(regionForCorners_begin(), regionForCorners_end());
        _sourceWidth = _imageLevels[0].currentImage().width();
        _checkScale = _imageLevels[_checkLevel].currentImage().width() / static_cast<float>(_sourceWidth);
    }
    int maxCountIterationForTrackingPoints() const { return _maxCountIterationForTrackingPoints; }
    void setMaxCountIterationForTrackingPoints(int maxCount) { _maxCountIterationForTrackingPoints = maxCount; }
    float maxLocalVelocity() const { return _maxLocalVelocity; }
    void setMaxLocalVelocity(float velocity) { _maxLocalVelocity = velocity; }
    float epsDeltaStep() const { return _epsDeltaStep; }
    void setEpsDeltaStep(float epsDeltaStep) { _epsDeltaStep = epsDeltaStep; }
    float detThreshold() const { return _imageLevels[0].detThreshold(); }
    void setDetThreshold(float detThreshold)
    {
        for (int i=0; i<_countImageLevels; ++i)
            _imageLevels[i].setDetThreshold(detThreshold);
    }
    float errorThreshold() const { return _errorThreshold; }
    void setErrorThreshold(float errorThreshold) { _errorThreshold = errorThreshold; }
    int barrierOfCorners() const { return _imageLevels[0].barrier(); }
    void setBarrierOfCorners(int barrier)
    {
        for (int i=0; i<_countImageLevels; ++i)
            _imageLevels[i].setBarrier(barrier);
    }
    float shiTomasiScoreThreshold() const { return _imageLevels[0].shiTomasiScoreThreshold(); }
    void setShiTomasiScoreThreshold(float threshold)
    {
        for (int i=0; i<_countImageLevels; ++i)
            _imageLevels[i].setShiTomasiScoreThreshold(threshold);
    }
    IPoint sizeCursor() const { return _imageLevels[0].sizeCursor(); }
    void setSizeCursor(const IPoint& sizeCursor)
    {
        for (int i=0; i<_countImageLevels; ++i)
            _imageLevels[i].setSizeCursor(sizeCursor);
        setRegionForCorners(regionForCorners_begin(), regionForCorners_end());
    }
    float minSquareDistancePointsBig() const { return _minSquareDistancePointsBig; }
    void setMinSquareDistancePointsBig(float value) { _minSquareDistancePointsBig = value; _updateSizeOfCasheImage(); }
    float minSquareDistancePointsSmall() const { return _minSquareDistancePointsSmall; }
    void setMinSquareDistancePointsSmall(float value) { _minSquareDistancePointsSmall = value; }

    IPoint regionForCorners_begin() const { return _imageLevels[0].regionForCorners_begin(); }
    IPoint regionForCorners_end() const { return _imageLevels[0].regionForCorners_end(); }
    void setRegionForCorners(IPoint begin, IPoint end);

    int checkLevel() const { return _checkLevel; }
    void setCheckLevel(int level)
    {
        _checkLevel = level;
        _checkScale = _imageLevels[_checkLevel].currentImage().width() / static_cast<float>(_sourceWidth);
    }
    bool getSubOFImage(Image<OF_ImageLevel::OpticalFlowInfo2D>& outSubOFImage, const IPointF& position) const;
    bool trackingSubOFImage(IPointF& position, const Image<OF_ImageLevel::OpticalFlowInfo2D>& subOFImage, float maxDeltaSquare) const;

    void clearCashe() { _clearCasheImage(); }
    bool trackingPoint(IPointF& point);

    bool addPoint(const IPointF& point);

    std::vector<Image<IPointF>> allocDiffPyramid() const;
    void getDiffPyramid(std::vector<Image<IPointF>>& out) const;
    bool trackingPoint(IPointF& point, const std::vector<Image<IPointF>>& diffPyramid);

    bool trackingPointOnLine(IPointF& point, const IPointF& lineDir, const IPointF& pointOnPrevImage);

private:
    OF_ImageLevel* _imageLevels;
    int _countImageLevels;

    int _maxCountIterationForTrackingPoints;
    float _maxLocalVelocity;
    float _epsDeltaStep;
    float _errorThreshold;
    float _minSquareDistancePointsBig;
    float _minSquareDistancePointsSmall;
    int _checkLevel;

    int _sourceWidth;
    float _checkScale;

    float _hashCellSize;
    Image<std::vector<IPointF>> _casheImage;

    void _updateSizeOfCasheImage();
    void _clearCasheImage();
    IPoint _getCashePos(const IPointF& point);
    inline bool _existNeighborsInVector(const IPointF& pos, const std::vector<IPointF>& vector, float radiusSquare) const;
    inline bool _existNeighbors(const IPointF& pos, const IPoint& cashePos, float radiusSquare);
};

}

#endif // OF_SYSTEM_H
