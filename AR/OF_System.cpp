#include "OF_System.h"

namespace AR {

bool OF_System::trackingPoint(IPointF& point)
{
    bool success = false;
    IPointF velocity = IPointF::ZERO;
    for (int j=_countImageLevels-1; j>=0; --j) {
        const int scaleFactor = _sourceWidth / _imageLevels[j].currentImage().width();
        const float invScaleFactor = 1.0f / static_cast<float>(scaleFactor);
        const IPointF localPrevPos = point * invScaleFactor;
        const IPointF localCurrentPos = (point + velocity) * invScaleFactor;
        if (!_imageLevels[j].startTrackPoint(localCurrentPos, localPrevPos))
            continue;
        if (!_imageLevels[j].velocityInRange(_maxLocalVelocity))
            return false;
        success = true;
        int iteration = 1;
        while (_imageLevels[j].needToNextIteration(_epsDeltaStep)) {
            if (!_imageLevels[j].iterationTrackPoint(localCurrentPos))
                break;
            if (!_imageLevels[j].velocityInRange(_maxLocalVelocity))
                return false;
            ++iteration;
            if (iteration > _maxCountIterationForTrackingPoints)
                break;
        }
        velocity += _imageLevels[j].localVelocity() * scaleFactor;
    }
    if (!success)
        return false;
    point += velocity;
    if (_imageLevels[0].pointOutOfRange(point)) {
        return false;
    } else {
        float error = _imageLevels[_checkLevel].getErrorValue(point * _checkScale);
        if (error > _errorThreshold) {
            return false;
        } else {
            IPoint cashePos = _getCashePos(point);
            if (_existNeighbors(point, cashePos, _minSquareDistancePointsSmall)) {
                return false;
            } else {
                //success
                _casheImage(cashePos).push_back(point);
            }
        }
    }
    return true;
}


bool OF_System::getSubOFImage(Image<OF_ImageLevel::OpticalFlowInfo2D>& outSubOFImage, const IPointF& position) const
{
    return _imageLevels[_checkLevel].getSubOpticalFlowInfo(outSubOFImage, position / qPow(2.0f, static_cast<float>(_checkLevel)));
}

bool OF_System::trackingSubOFImage(IPointF& position, const Image<OF_ImageLevel::OpticalFlowInfo2D>& subOFImage, float maxDeltaSquare) const
{
    float scaled = _imageLevels[0]._curImage.width() / static_cast<float>(_imageLevels[_checkLevel]._curImage.width());
    position /= scaled;
    maxDeltaSquare /= scaled;
    float eps = _epsDeltaStep / scaled;
    OF_ImageLevel& iLevel = _imageLevels[_checkLevel];
    IPointF startPosition = position, delta;
    for (int i=0; i<_maxCountIterationForTrackingPoints; ++i) {
        if (iLevel.iterationSubImage(delta, position, subOFImage) == false)
            return false;
        position += delta;
        if ((startPosition - position).lengthSquared() > maxDeltaSquare)
            return false;
        if ((qAbs(delta.x) < eps) && (qAbs(delta.y) < eps))
            break;
    }
    position *= scaled;
    return true;
}

void OF_System::setRegionForCorners(IPoint begin, IPoint end)
{
    IPoint cBegin = sizeCursor() + IPoint(1, 1);
    IPoint cEnd = size() - IPoint(2, 2) - cBegin;
    if (cEnd.x < cBegin.x)
        cEnd.x = cBegin.x;
    if (cEnd.y < cBegin.y)
        cEnd.y = cBegin.y;
    if (begin.x < cBegin.x)
        begin.x = cBegin.x;
    if (begin.y < cBegin.y)
        begin.y = cBegin.y;
    if (end.x > cEnd.x)
        end.x = cEnd.x;
    if (end.y > cEnd.y)
        end.y = cEnd.y;
    for (int i=0; i<_countImageLevels; ++i) {
        _imageLevels[i].setRegionForCorners(begin, end);
        begin /= 2;
        end /= 2;
    }
}

bool OF_System::addPoint(const IPointF& point)
{
    IPoint cashePos = _getCashePos(point);
    if (_existNeighbors(point, cashePos, _minSquareDistancePointsBig))
        return false;
    _casheImage(cashePos).push_back(point);
    return true;
}

IPoint OF_System::_getCashePos(const IPointF& point)
{
    return IPoint(qFloor(point.x / _hashCellSize), qFloor(point.y / _hashCellSize));
}

void OF_System::_clearCasheImage()
{
    int countElements = _casheImage.area();
    std::vector<IPointF>* data = _casheImage.data();
    for (int i=0; i<countElements; ++i)
        data[i].resize(0);
}

void OF_System::_updateSizeOfCasheImage()
{
    _hashCellSize = qSqrt(_minSquareDistancePointsBig);
    IPoint size(qCeil(_imageLevels[0].currentImage().width() / _hashCellSize),
                qCeil(_imageLevels[0].currentImage().height() / _hashCellSize));
    _casheImage.resize(size);
}

bool OF_System::_existNeighborsInVector(const IPointF& pos, const std::vector<IPointF>& vector, float radiusSquare) const
{
    for (std::vector<IPointF>::const_iterator it = vector.begin(); it != vector.end(); ++it) {
        if ((pos - *it).lengthSquared() < radiusSquare)
            return true;
    }
    return false;
}

bool OF_System::_existNeighbors(const IPointF& pos, const IPoint& cashePos, float radiusSquare)
{
    if (_existNeighborsInVector(pos, _casheImage(cashePos), radiusSquare))
        return true;
    if (cashePos.x > 0) {
        if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, cashePos.y), radiusSquare))
            return true;
        if (cashePos.x + 1 < _casheImage.width()) {
            if (_existNeighborsInVector(pos, _casheImage(cashePos.x + 1, cashePos.y), radiusSquare))
                return true;
            if (cashePos.y > 0) {
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, cashePos.y - 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     cashePos.y - 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x + 1, cashePos.y - 1), radiusSquare))
                    return true;
                if (cashePos.y + 1 < _casheImage.height()) {
                    if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, cashePos.y + 1), radiusSquare))
                        return true;
                    if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     cashePos.y + 1), radiusSquare))
                        return true;
                    if (_existNeighborsInVector(pos, _casheImage(cashePos.x + 1, cashePos.y + 1), radiusSquare))
                        return true;
                }
            } else {
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x + 1, 1), radiusSquare))
                    return true;
            }
        } else {
            if (cashePos.y > 0) {
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, cashePos.y - 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     cashePos.y - 1), radiusSquare))
                    return true;
                if (cashePos.y + 1 < _casheImage.height()) {
                    if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, cashePos.y + 1), radiusSquare))
                        return true;
                    if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     cashePos.y + 1), radiusSquare))
                        return true;
                }
            } else {
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x - 1, 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(cashePos.x,     1), radiusSquare))
                    return true;
            }
        }
    } else {
        if (_existNeighborsInVector(pos, _casheImage(1, cashePos.y), radiusSquare))
            return true;
        if (cashePos.y > 0) {
            if (_existNeighborsInVector(pos, _casheImage(0, cashePos.y - 1), radiusSquare))
                return true;
            if (_existNeighborsInVector(pos, _casheImage(1, cashePos.y - 1), radiusSquare))
                return true;
            if (cashePos.y + 1 < _casheImage.height()) {
                if (_existNeighborsInVector(pos, _casheImage(0, cashePos.y + 1), radiusSquare))
                    return true;
                if (_existNeighborsInVector(pos, _casheImage(1, cashePos.y + 1), radiusSquare))
                    return true;
            }
        } else {
            if (_existNeighborsInVector(pos, _casheImage(0, 1), radiusSquare))
                return true;
            if (_existNeighborsInVector(pos, _casheImage(1, 1), radiusSquare))
                return true;
        }
    }
    return false;
}

std::vector<Image<IPointF>> OF_System::allocDiffPyramid() const
{
    std::vector<Image<IPointF>> pyramid;
    pyramid.resize(_countImageLevels);
    for (unsigned int i=0; i<pyramid.size(); ++i)
        pyramid[i] = Image<IPointF>(_imageLevels[i].size() - IPoint(2, 2));
    return pyramid;
}

void OF_System::getDiffPyramid(std::vector<Image<IPointF>>& out) const
{
    for (int i=0; i<_countImageLevels; ++i)
        _imageLevels[i].getDiffImage(out[i]);
}

bool OF_System::trackingPoint(IPointF& point, const std::vector<Image<IPointF>>& diffPyramid)
{
    bool success = false;
    IPointF velocity = IPointF::ZERO;
    for (int j=_countImageLevels-1; j>=0; --j) {
        const int scaleFactor = _sourceWidth / _imageLevels[j].currentImage().width();
        const float invScaleFactor = 1.0f / static_cast<float>(scaleFactor);
        const IPointF localPrevPos = point * invScaleFactor;
        const IPoint pLocalPrevPos(qFloor(localPrevPos.x), qFloor(localPrevPos.y));
        const IPointF localCurrentPos = (point + velocity) * invScaleFactor;
        if (!_imageLevels[j].startTrackPoint(diffPyramid[j], localCurrentPos, pLocalPrevPos))
            continue;
        if (!_imageLevels[j].velocityInRange(_maxLocalVelocity))
            return false;
        success = true;
        int iteration = 1;
        while (_imageLevels[j].needToNextIteration(_epsDeltaStep)) {
            if (!_imageLevels[j].iterationTrackPoint(localCurrentPos))
                break;
            if (!_imageLevels[j].velocityInRange(_maxLocalVelocity))
                return false;
            ++iteration;
            if (iteration > _maxCountIterationForTrackingPoints)
                break;
        }
        velocity += (_imageLevels[j].localVelocity()) * scaleFactor;
    }
    if (!success)
        return false;
    point += velocity;
    if (_imageLevels[0].pointOutOfRange(point)) {
        return false;
    } else {
        float error = _imageLevels[_checkLevel].getErrorValue(point * _checkScale);
        if (error > _errorThreshold) {
            return false;
        } else {
            IPoint hashPos = _getCashePos(point);
            if (_existNeighbors(point, hashPos, _minSquareDistancePointsSmall)) {
                return false;
            } else {
                //success
                _casheImage(hashPos).push_back(point);
            }
        }
    }
    return true;
}

bool OF_System::trackingPointOnLine(IPointF& point, const IPointF& lineDir, const IPointF& pointOnPrevImage)
{
    bool success = false;
    IPointF velocity = IPointF::ZERO;
    for (int j=_countImageLevels-1; j>=0; --j) {
        const int scaleFactor = _sourceWidth / _imageLevels[j].currentImage().width();
        const float invScaleFactor = 1.0f / static_cast<float>(scaleFactor);
        const IPointF localCurrentPos = (point + velocity) * invScaleFactor;
        if (!_imageLevels[j].startTrackPointOnLine(localCurrentPos, lineDir, pointOnPrevImage * invScaleFactor))
            continue;
        success = true;
        int iteration = 1;
        while (_imageLevels[j].needToNextIteration(_epsDeltaStep)) {
            if (!_imageLevels[j].iterationTrackPointOnLine(localCurrentPos, lineDir))
                break;
            ++iteration;
            if (iteration > _maxCountIterationForTrackingPoints)
                break;
        }
        velocity += (_imageLevels[j].localVelocity()) * scaleFactor;
    }
    if (!success)
        return false;
    point += velocity;
    if (_imageLevels[0].pointOutOfRange(point)) {
        return false;
    } else {
        float error = _imageLevels[_checkLevel].getErrorValue(point * _checkScale);
        if (error > _errorThreshold) {
            return false;
        }
        //success
    }
    return true;
}

}
