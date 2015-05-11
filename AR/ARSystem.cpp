#include "AR/ARSystem.h"
#include "AR/ImageProcessing.h"
#include "AR/OF_ImageLevel.h"
#include "AR/TMath.h"
#include "AR/TSVD.h"
#include <QTime>
#include <algorithm>
#include <functional>
#include <cfloat>
#include "QScrollEngine/QOtherMathFunctions.h"
#include "AR/OtherEstimators.h"
#include "AR/Reconstructor3D.h"

namespace AR {

ARSystem::ARSystem():
    _state(NotTracking)
{
    _reconstructor3D = new Reconstructor3D();
    _reconstructor3D->setMap(&_map);
    setTrackingParameters(_parameters);
    reset();
}

ARSystem::~ARSystem()
{
    delete _reconstructor3D;
}

void ARSystem::reset()
{
    _state = NotTracking;
    _map.resetAll();
    _map.addKeyFrame();
    _map.keyFrame(0).projectedPoints.clear();
    _map.keyFrame(0).indicesMapPoints.clear();
    _map.keyFrame(0).matrixWorld.setToIdentity();
    _currentAddedNewPoints.clear();
    _infoAddedNewPoints.clear();
    _currentAddedOldPoints.clear();
    _infoAddedOldPoints.clear();
}

QScrollEngine::QEntity* ARSystem::entity()
{
    return _reconstructor3D->entity();
}

void ARSystem::setEntity(QScrollEngine::QEntity* entity)
{
    _reconstructor3D->setEntity(entity);
}

QScrollEngine::QScrollEngineContext* ARSystem::context()
{
    return _reconstructor3D->context();
}

void ARSystem::setContext(QScrollEngine::QScrollEngineContext* context)
{
    _reconstructor3D->setContext(context);
}

void ARSystem::setTrackingParameters(const TrackingParameters& parameters)
{
    _parameters = parameters;
    if (_parameters.endLevelForFeature < 1)
        _parameters.endLevelForFeature = 1;
    if (_parameters.countLevels < _parameters.endLevelForFeature)
        _parameters.countLevels = _parameters.endLevelForFeature;
    if (_parameters.sizeBorder.x < _parameters.sizeCursor.x + 1)
        _parameters.sizeBorder.x = _parameters.sizeCursor.x + 1;
    if (_parameters.sizeBorder.x < _parameters.sizeImageOfMapPoint.x + 1)
        _parameters.sizeBorder.x = _parameters.sizeImageOfMapPoint.x + 1;
    if (_parameters.sizeBorder.y < _parameters.sizeCursor.y + 1)
        _parameters.sizeBorder.y = _parameters.sizeCursor.y + 1;
    if (_parameters.sizeBorder.y < _parameters.sizeImageOfMapPoint.y + 1)
        _parameters.sizeBorder.y = _parameters.sizeImageOfMapPoint.y + 1;
    if (_parameters.checkLevel >= _parameters.countLevels)
        _parameters.checkLevel = _parameters.countLevels / 2;
    if (_parameters.sizeBlur < 1)
        _parameters.sizeBlur = 1;
    if (_parameters.resolution.x < 3)
        _parameters.resolution.x = 3;
    if (_parameters.resolution.y < 3)
        _parameters.resolution.y = 3;
    if (_parameters.resolution.x < _parameters.sizeCursor.x * 2)
        _parameters.resolution.x = _parameters.sizeCursor.x * 2;
    if (_parameters.resolution.y < _parameters.sizeCursor.y * 2)
        _parameters.resolution.y = _parameters.sizeCursor.y * 2;
    _opticalFlow.setCountImageLevels(_parameters.countLevels);
    _bwImage.resize(_parameters.resolution);
    _opticalFlow.resize(_parameters.resolution);
    _opticalFlow.setRegionForCorners(_opticalFlow.sizeCursor() * 2, _opticalFlow.size() - _opticalFlow.sizeCursor() * 2);
    IPoint smallSize = _opticalFlow.size();
    for (_countItersForSmallImage = 0; ; ++_countItersForSmallImage) {
        if ((smallSize.x < _parameters.sizeSmallImage.x) && (smallSize.y < _parameters.sizeSmallImage.y))
            break;
        smallSize /= 2;
    }
    _parameters.sizeSmallImage = smallSize;
    _opticalFlow.setCheckLevel(_parameters.checkLevel);
    _opticalFlow.setBarrierOfCorners(_parameters.barrierOfCorners);
    _opticalFlow.setShiTomasiScoreThreshold(_parameters.shiTomasiScoreThreshold);
    _opticalFlow.setSizeCursor(_parameters.sizeCursor);
    _opticalFlow.setDetThreshold(_parameters.detThreshold);
    _opticalFlow.setErrorThreshold(_parameters.errorThreshold);
    _opticalFlow.setEpsDeltaStep(_parameters.epsDeltaStep);
    _opticalFlow.setMaxCountIterationForTrackingPoints(_parameters.maxCountIterationForTrackingPoints);
    _opticalFlow.setMaxLocalVelocity(_parameters.maxLocalVelocity);
    _opticalFlow.setSizeCursor(_parameters.sizeCursor);
    _opticalFlow.setMinSquareDistancePointsBig(_parameters.minSquareDistancePointsBig);
    _opticalFlow.setMinSquareDistancePointsSmall(_parameters.minSquareDistancePointsSmall);
    _opticalFlow.setRegionForCorners(_parameters.sizeBorder, _opticalFlow.size() - _parameters.sizeBorder);
    _map.setLimitError(_parameters.limitAvgErrorSquared);
    _map.setCutOffProjections_mapPoints(_parameters.cutOffProjections_mapPoint);
    _map.setCutOff_nextFrame(_parameters.cutOff_nextFrame);
    _map.setDistanceSquared_nextFrame(_parameters.distanceSquared_nextFrame);
}

void ARSystem::drawResultOnGL(QScrollEngine::QScrollEngineContext* context, const QMatrix2x2& transform)
{
    QVector2D r, v;
    std::vector<QVector2D> lines;
    IPointF scale(2.0f / static_cast<float>(_bwImage.width()),
                  2.0f / static_cast<float>(_bwImage.height()));
    if (_state == TrackingBegin) {
        const Map::KeyFrame& firstFrame = _map.keyFrame(0);
        const Map::KeyFrame& secondFrame = _map.keyFrame(1);
        for (unsigned int i=0; i<firstFrame.projectedPoints.size(); ++i) {
            const IPointF& firstF = firstFrame.projectedPoints[i];
            v.setX(firstF.x * scale.x - 1.0f);
            v.setY(firstF.y * scale.y - 1.0f);
            r.setX(transform(0, 0) * v.x() + transform(0, 1) * v.y());
            r.setY(transform(1, 0) * v.x() + transform(1, 1) * v.y());
            lines.push_back(r);
            const IPointF& secondF = secondFrame.projectedPoints[i];
            v.setX(secondF.x * scale.x - 1.0f);
            v.setY(secondF.y * scale.y - 1.0f);
            r.setX(transform(0, 0) * v.x() + transform(0, 1) * v.y());
            r.setY(transform(1, 0) * v.x() + transform(1, 1) * v.y());
            lines.push_back(r);

        }
    } else if (_state == TrackingNow) {
        /*TMath::TMatrix<float> camProj = _pose3DEstimator.cameraProjection();
        const Map::KeyFrame& currentFrame = _map.lastKeyFrame();
        for (unsigned int i=0; i<currentFrame.projectedPoints.size(); ++i) {
            IPointF& currentF = currentFrame.projectedPoints[i];
            IPoint current(qFloor(scale.x * currentF.x), qFloor(scale.y * currentF.y));
            QVector3D v = currentFrame.matrixWorld * _map.mapPoint(currentFrame.indicesMapPoints[i]);
            TMath::TVector<float> r = camProj * TMath::TVector<float>(v.x(), v.y(), v.z());
            IPoint cur((int)(r(0) / r(2)), (int)(r(1) / r(2)));
            cur.x *= scale.x;
            cur.y *= scale.y;
            Painter::drawLine(_sourceImage, current, cur, Rgb(255, 255, 0));
        }*/
    } else {
        if (_map.countKeyFrames() == 0)
            return;
        float crossDeltaA = 5.0f;
        QVector2D crossDelta(crossDeltaA * scale.x, crossDeltaA * scale.y);
        crossDelta = QVector2D(transform(0, 0) * crossDelta.x() + transform(0, 1) * crossDelta.y(),
                               transform(1, 0) * crossDelta.x() + transform(1, 1) * crossDelta.y());
        const std::vector<IPointF>& points = _currentAddedNewPoints;
        for (unsigned int i=0; i<points.size(); ++i) {
            const IPointF& currentF = points[i];
            v.setX(currentF.x * scale.x - 1.0f);
            v.setY(currentF.y * scale.y - 1.0f);
            r.setX(transform(0, 0) * v.x() + transform(0, 1) * v.y());
            r.setY(transform(1, 0) * v.x() + transform(1, 1) * v.y());
            lines.push_back(r + crossDelta);
            lines.push_back(r - crossDelta);
            lines.push_back(r + QVector2D(crossDelta.x(), - crossDelta.y()));
            lines.push_back(r + QVector2D(- crossDelta.x(), crossDelta.y()));
        }
    }
    context->drawLines(lines.data(), lines.size(), QColor(255, 0, 0));
}

void ARSystem::_scaleAndBlurImage(Image<uchar>& sourceBWImage)
{
    if ((sourceBWImage.width() < 3) || (sourceBWImage.height() < 3))
        return;
    if (_integralImage.size() != sourceBWImage.size())
        _integralImage.resize(sourceBWImage.size());
    int* strIntegral = _integralImage.data();
    uchar* strBW = sourceBWImage.data();
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
    IPointF scale(sourceBWImage.width() / static_cast<float>(_bwImage.width()),
                  sourceBWImage.height() / static_cast<float>(_bwImage.height()));
    IPoint sourceSizeBlur(qFloor(_parameters.sizeBlur * scale.x), qFloor(_parameters.sizeBlur * scale.y));
    IPoint endPoint = _bwImage.size() - IPoint(1, 1);
    IPoint endSourcePoint = sourceBWImage.size() - IPoint(1, 1);
    IPointF sourcePointF;
    IPoint sourcePoint, invSourcePoint;
    float area;
    IPoint endPointOfArea = _bwImage.size() - IPoint(_parameters.sizeBlur, _parameters.sizeBlur);
    float invArea = 1.0f / static_cast<float>((sourceSizeBlur.x * 2) * (sourceSizeBlur.y * 2));
    strBW = &_bwImage.data()[_bwImage.width() * _parameters.sizeBlur];
    for (i=_parameters.sizeBlur; i<endPointOfArea.y; ++i) {
        sourcePoint.y = qFloor(scale.y * i);
        for (j=_parameters.sizeBlur; j<endPointOfArea.x; ++j) {
            sourcePoint.x = qFloor(scale.x * j);
            strBW[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x, sourcePoint.y-sourceSizeBlur.y) +
                                           _integralImage(sourcePoint.x+sourceSizeBlur.x, sourcePoint.y+sourceSizeBlur.y) -
                                          (_integralImage(sourcePoint.x-sourceSizeBlur.x, sourcePoint.y+sourceSizeBlur.y) +
                                           _integralImage(sourcePoint.x+sourceSizeBlur.x, sourcePoint.y-sourceSizeBlur.y))) * invArea);
        }
        strBW = &strBW[_bwImage.width()];
    }
    strBW = _bwImage.data();
    uchar* strBW_inv = &_bwImage.data()[_bwImage.width() * endPoint.y];
    float a = scale.x * _parameters.sizeBlur * 2.0f;
    for (i=0; i<_parameters.sizeBlur; ++i) {
        sourcePointF.y = scale.y * (i + _parameters.sizeBlur);
        sourcePoint.y = qFloor(sourcePointF.y);
        invSourcePoint.y = endSourcePoint.y - sourcePoint.y;
        invArea = 1.0f / (a * sourcePointF.y);
        for (j=_parameters.sizeBlur; j<endPointOfArea.x; ++j) {
            sourcePoint.x = qFloor(scale.x * j);
            strBW[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x, 0) +
                                           _integralImage(sourcePoint.x+sourceSizeBlur.x, sourcePoint.y) -
                                          (_integralImage(sourcePoint.x-sourceSizeBlur.x, sourcePoint.y) +
                                           _integralImage(sourcePoint.x+sourceSizeBlur.x, 0))) * invArea);
            strBW_inv[j] = static_cast<uchar>((_integralImage(sourcePoint.x-sourceSizeBlur.x, invSourcePoint.y) +
                                               _integralImage(sourcePoint.x+sourceSizeBlur.x, endSourcePoint.y) -
                                              (_integralImage(sourcePoint.x-sourceSizeBlur.x, endSourcePoint.y) +
                                               _integralImage(sourcePoint.x+sourceSizeBlur.x, invSourcePoint.y))) * invArea);
        }
        strBW = &strBW[_bwImage.width()];
        strBW_inv = &strBW_inv[-_bwImage.width()];
    }
    strBW = &_bwImage.data()[_bwImage.width() * _parameters.sizeBlur];
    for (i=_parameters.sizeBlur; i<endPointOfArea.y; ++i) {
        sourcePoint.y = qFloor(scale.y * i);
        for (j=0; j<_parameters.sizeBlur; ++j) {
            sourcePointF.x = scale.x * (j + _parameters.sizeBlur);
            sourcePoint.x = qFloor(sourcePointF.x);
            invSourcePoint.x = endSourcePoint.x - sourcePoint.x;
            area = (a * sourcePointF.x);
            strBW[j] = static_cast<uchar>((_integralImage(0, sourcePoint.y-sourceSizeBlur.y) +
                                           _integralImage(sourcePoint.x, sourcePoint.y+sourceSizeBlur.y) -
                                          (_integralImage(0, sourcePoint.y+sourceSizeBlur.y) +
                                           _integralImage(sourcePoint.x, sourcePoint.y-sourceSizeBlur.y))) / area);
            strBW[endPoint.x - j] = static_cast<uchar>((_integralImage(invSourcePoint.x, sourcePoint.y-sourceSizeBlur.y) +
                                                        _integralImage(endSourcePoint.x, sourcePoint.y+sourceSizeBlur.y) -
                                                       (_integralImage(invSourcePoint.x, sourcePoint.y+sourceSizeBlur.y) +
                                                        _integralImage(endSourcePoint.x, sourcePoint.y-sourceSizeBlur.y))) / area);
        }
        strBW = &strBW[_bwImage.width()];
    }
    int invJ;
    strBW = _bwImage.data();
    strBW_inv = &_bwImage.data()[_bwImage.width() * endPoint.y];
    for (i=0; i<_parameters.sizeBlur; ++i) {
        sourcePointF.y = scale.y * (i + _parameters.sizeBlur);
        sourcePoint.y = qFloor(sourcePointF.y);
        invSourcePoint.y = endSourcePoint.y - sourcePoint.y;
        for (j=0; j<_parameters.sizeBlur; ++j) {
            sourcePointF.x = scale.x * (j + _parameters.sizeBlur);
            sourcePoint.x = qFloor(sourcePointF.x);
            invSourcePoint.x = endSourcePoint.x - sourcePoint.x;
            area = sourcePointF.y * sourcePointF.x;
            strBW[j] = ((_integralImage(0, 0) +
                         _integralImage(sourcePoint)) -
                        (_integralImage(0, sourcePoint.y) +
                         _integralImage(sourcePoint.x, 0))) / area;
            invJ = endPoint.x - j;
            strBW[invJ] = ((_integralImage(invSourcePoint.x, 0) +
                            _integralImage(endSourcePoint.x, sourcePoint.y)) -
                            (_integralImage(invSourcePoint.x, sourcePoint.y) +
                             _integralImage(endSourcePoint.x, 0))) / area;
            strBW_inv[j] = ((_integralImage(0, invSourcePoint.y) +
                             _integralImage(sourcePoint.x, endSourcePoint.y)) -
                            (_integralImage(0, endSourcePoint.y) +
                             _integralImage(sourcePoint.x, invSourcePoint.y))) / area;
            strBW_inv[invJ] = ((_integralImage(invSourcePoint) +
                                _integralImage(endSourcePoint)) -
                               (_integralImage(invSourcePoint.x, endSourcePoint.y) +
                                _integralImage(endSourcePoint.x, invSourcePoint.y))) / area;
        }
        strBW = &strBW[_bwImage.width()];
        strBW_inv = &strBW_inv[-_bwImage.width()];
    }
}

void ARSystem::_addNewPointsToVector(int maxSize)
{
    if (static_cast<int>(_currentAddedNewPoints.size()) >= maxSize)
        return;
    QMatrix4x4 currentMatrixWorld = _map.lastKeyFrame().matrixWorld;
    QVector3D currentPos3D(currentMatrixWorld(0, 3), currentMatrixWorld(1, 3), currentMatrixWorld(2, 3));
    float minValue;
    int i, minIndex;
    for (i=_parameters.beginLevelForFeature; i<=_parameters.endLevelForFeature; i+=_parameters.stepOfLevelOfFeature)
        _opticalFlow.imageLevel(i).findCandidatePoints();
    int* currentIndex = new int[_parameters.endLevelForFeature + 1];
    float* scaleLevel = new float[_parameters.endLevelForFeature + 1];
    for (i=_parameters.beginLevelForFeature; i<=_parameters.endLevelForFeature; i+=_parameters.stepOfLevelOfFeature) {
        currentIndex[i] = 0;
        scaleLevel[i] = _bwImage.width() / static_cast<float>(_opticalFlow.imageLevel(i).size().x);
    }
    for(;;) {
        minIndex = -1;
        minValue = FLT_MAX;
        for (i=_parameters.beginLevelForFeature; i<=_parameters.endLevelForFeature; i+=_parameters.stepOfLevelOfFeature) {
            std::vector<FastFeature::FastCorner>& corners = _opticalFlow.imageLevel(i).candidateCorners();
            if (currentIndex[i] >= static_cast<int>(corners.size()))
                break;
            if (minValue > corners[currentIndex[i]].scoreShiTomasi) {
                minValue = corners[currentIndex[i]].scoreShiTomasi;
                minIndex = i;
            }
        }
        if (minIndex < 0)
            break;
        IPoint& pos = _opticalFlow.imageLevel(minIndex).candidateCorners()[currentIndex[minIndex]].pos;
        IPointF posF(static_cast<float>(pos.x * scaleLevel[minIndex]), static_cast<float>(pos.y * scaleLevel[minIndex]));
        if (_opticalFlow.addPoint(posF)) {
            _currentAddedNewPoints.push_back(posF);
            _infoAddedNewPoints.resize(_infoAddedNewPoints.size() + 1);
            InfoForNewMapPoint& info = _infoAddedNewPoints[_infoAddedNewPoints.size() - 1];
            info.projections.resize(1);
            info.projections[0].matrixWorld = currentMatrixWorld;
            info.projections[0].projection = posF;
            info.lastDir = _pose3DEstimator.getDirFromScreen(currentMatrixWorld, posF);
            info.lastPos = currentPos3D;
            if (static_cast<int>(_currentAddedNewPoints.size()) >= maxSize)
                break;
        }
        ++currentIndex[minIndex];
    };
    delete[] currentIndex;
    delete[] scaleLevel;
}

void ARSystem::_addOldPointsToVector(int maxSize)
{
    if (static_cast<int>(_infoAddedOldPoints.size()) >= maxSize)
        return;
    IPointF pixelFocalLength = _pose3DEstimator.invPixelFocalLength();
    pixelFocalLength.set(1.0f / pixelFocalLength.x, 1.0f / pixelFocalLength.y);
    float maxDeltaSquare = _parameters.limitMaxErrorSquared * qMax(pixelFocalLength.x, pixelFocalLength.y);
    QMatrix4x4 currentMatrixWorld = _map.lastKeyFrame().matrixWorld;
    QVector3D currentPos(currentMatrixWorld(0, 3), currentMatrixWorld(1, 3), currentMatrixWorld(2, 3));
    _frustum.calculate(_matrixProj * currentMatrixWorld);
    int indexMapPoint = 0;
    std::vector<Map::MapPoint>& mapPoints = _map._mapPoints;
    for (std::vector<Map::MapPoint>::iterator it=mapPoints.begin(); it!=mapPoints.end(); ++it, ++indexMapPoint) {
        if (it->isDeleted || it->isActived)
            continue;
        if (!_frustum.pointInFrustum(it->worldPoint))
            continue;
        QVector3D v = QScrollEngine::QOtherMathFunctions::transform(currentMatrixWorld, it->worldPoint);
        IPointF begin(- v.x() / v.z(), - v.y() / v.z());
        IPointF projection = _pose3DEstimator.projectPoint(begin);
        if (!_opticalFlow.trackingSubOFImage(projection, it->OFImage, maxDeltaSquare))
            continue;
        IPointF end = _pose3DEstimator.unprojectPoint(projection);
        float errorSquared = (end - begin).lengthSquared();
        QVector3D dir = _pose3DEstimator.getDirFromScreen(currentMatrixWorld, projection);
        if (errorSquared > _parameters.limitMaxErrorSquared * 10.0f) {
            _map.addErrorOfMapPoint(indexMapPoint, errorSquared, dir);
        }
        _currentAddedOldPoints.push_back(projection);
        _infoAddedOldPoints.resize(_infoAddedOldPoints.size() + 1);
        InfoForOldMapPoint& info = _infoAddedOldPoints[_infoAddedOldPoints.size() - 1];
        info.lastDir = dir;
        info.lastPos = currentPos;
        info.countOfVisibles = 1;
        info.indexMapPoint = indexMapPoint;
        it->isActived = true;
        if (static_cast<int>(_currentAddedOldPoints.size()) > maxSize)
            return;
    }
}

float ARSystem::_trackingPose()
{
    int currentIndexKeyFrame = _map.countKeyFrames() - 1;
    Map::KeyFrame& currentKeyFrame = _map.keyFrame(currentIndexKeyFrame);
    unsigned int i, ci = 0;
    for (i=0; i<currentKeyFrame.projectedPoints.size(); ++i) {
        if (_opticalFlow.trackingPoint(currentKeyFrame.projectedPoints[i])) {
            currentKeyFrame.projectedPoints[ci] = currentKeyFrame.projectedPoints[i];
            currentKeyFrame.indicesMapPoints[ci] = currentKeyFrame.indicesMapPoints[i];
            ++ci;
        } else {
            _map._mapPoints[currentKeyFrame.indicesMapPoints[i]].isActived = false;
        }
    }
    currentKeyFrame.projectedPoints.resize(ci);
    currentKeyFrame.indicesMapPoints.resize(ci);
    std::vector<float> vectorOfErrorSquared;
    bool successTracking;
    do {
        if (currentKeyFrame.projectedPoints.size() < 6) {
            successTracking = false;
            break;
        }
        successTracking = true;
        _pose3DEstimator.beginEstimatePose();
        vectorOfErrorSquared.resize(currentKeyFrame.projectedPoints.size());
        for (i=0; i<vectorOfErrorSquared.size(); ++i) {
            vectorOfErrorSquared[i] = _map.errorOfMapPoint(currentKeyFrame.indicesMapPoints[i]);
            vectorOfErrorSquared[i] *= vectorOfErrorSquared[i];
        }
        std::vector<float> copyVector = vectorOfErrorSquared;
        float sigmaSquaredOfError = TMath::Tukey<float>::findSigmaSquared(copyVector);
        for (i=0; i<currentKeyFrame.projectedPoints.size(); ++i)
            _pose3DEstimator.addPoint(_map.mapPoint(currentKeyFrame.indicesMapPoints[i]), currentKeyFrame.projectedPoints[i],
                                            TMath::Tukey<float>::weight(vectorOfErrorSquared[i], sigmaSquaredOfError));
        _pose3DEstimator.estimatePose();
        unsigned int j = 0, k;
        for (int i=0; i<_pose3DEstimator.countPoints(); ++i) {
            if (_pose3DEstimator.getCurrentErrorSquared(i) > _parameters.limitMaxErrorSquared) {
                successTracking = false;
                Map::MapPoint& mapPoint = _map._mapPoints[currentKeyFrame.indicesMapPoints[j]];
                for (k=0; k<mapPoint.indexKeyFrame.size(); ++k) {
                    if (mapPoint.indexKeyFrame[k] == currentIndexKeyFrame) {
                        mapPoint.indexKeyFrame.erase(mapPoint.indexKeyFrame.begin() + k);
                        break;
                    }
                }
                currentKeyFrame.indicesMapPoints.erase(currentKeyFrame.indicesMapPoints.begin() + j);
                currentKeyFrame.projectedPoints.erase(currentKeyFrame.projectedPoints.begin() + j);
                mapPoint.isActived = false;
            } else {
                ++j;
            }
        }
    } while(!successTracking);
    if (!successTracking)
        return - 1.0f;
    _pose3DEstimator.getResultMatrixWorld(currentKeyFrame.matrixWorld);
    float sumError = 0.0f;
    for (i=0; i<currentKeyFrame.projectedPoints.size(); ++i) {
        QVector3D currentDir = _pose3DEstimator.getDirFromScreen(currentKeyFrame.matrixWorld, currentKeyFrame.projectedPoints[i]);
        float errorSquared = _pose3DEstimator.getCurrentErrorSquared(i);
        _map.addErrorOfMapPoint(currentKeyFrame.indicesMapPoints[i], errorSquared, currentDir);
        sumError += errorSquared;
    }
    return sumError / static_cast<float>(currentKeyFrame.projectedPoints.size());
}

void ARSystem::_trackingAddedOldPoints(bool goodTracking)
{
    int currentIndexKeyFrame = _map.countKeyFrames() - 1;
    QMatrix4x4 currentMatrixWorld = _map.lastKeyFrame().matrixWorld;
    QVector3D currentPos(currentMatrixWorld(0, 3), currentMatrixWorld(1, 3), currentMatrixWorld(2, 3));
    unsigned int i, ci = 0;
    if (goodTracking) {
        for (i=0; i<_currentAddedOldPoints.size(); ++i) {
            Map::MapPoint& mapPoint = _map._mapPoints[_infoAddedOldPoints[i].indexMapPoint];
            if (!mapPoint.isDeleted) {
                if (_opticalFlow.trackingPoint(_currentAddedOldPoints[i])) {
                    float error = _pose3DEstimator.getErrorSquared(currentMatrixWorld, mapPoint.worldPoint, _currentAddedOldPoints[i]);
                    QVector3D currentDir = _pose3DEstimator.getDirFromScreen(currentMatrixWorld, _currentAddedOldPoints[i]);
                    if (error < _parameters.limitMaxErrorSquared * 10.0f) {
                        if ((QVector3D::dotProduct(currentDir, _infoAddedOldPoints[i].lastDir) < _parameters.cutOffProjections_mapPoint) ||
                                ((currentPos - _infoAddedOldPoints[i].lastPos).lengthSquared() > _parameters.distanceSquare_mapPoint)) {
                            _infoAddedOldPoints[i].lastDir = currentDir;
                            _infoAddedOldPoints[i].lastPos = currentPos;
                            ++_infoAddedOldPoints[i].countOfVisibles;
                            if (_infoAddedOldPoints[i].countOfVisibles >= _parameters.countProjections_oldMapPoint) {
                                _map.addMapPointToKeyFrame(_infoAddedOldPoints[i].indexMapPoint, currentIndexKeyFrame,
                                                           _currentAddedOldPoints[i]);
                                qDebug() << qrand();
                                continue;
                            }
                        }
                        _currentAddedOldPoints[ci] = _currentAddedOldPoints[i];
                        _infoAddedOldPoints[ci] = _infoAddedOldPoints[i];
                        ++ci;
                        continue;
                    } else {
                        _map.addErrorOfMapPoint(_infoAddedOldPoints[i].indexMapPoint, error, currentDir);
                    }
                }
            }
            mapPoint.isActived = false;
        }
        _currentAddedOldPoints.resize(ci);
        _infoAddedOldPoints.resize(ci);
        _addOldPointsToVector(_parameters.countTrackingPoints - _map.lastKeyFrame().projectedPoints.size());
    } else {
        for (i=0; i<_currentAddedOldPoints.size(); ++i) {
            Map::MapPoint& mapPoint = _map._mapPoints[_infoAddedOldPoints[i].indexMapPoint];
            if (!mapPoint.isDeleted) {
                if (_opticalFlow.trackingPoint(_currentAddedOldPoints[i])) {
                    if (_pose3DEstimator.getErrorSquared(currentMatrixWorld, _map._mapPoints[_infoAddedOldPoints[i].indexMapPoint].worldPoint,
                                                         _currentAddedOldPoints[i]) < _parameters.limitMaxErrorSquared * 10.0f) {
                        _currentAddedOldPoints[ci] = _currentAddedOldPoints[i];
                        _infoAddedOldPoints[ci] = _infoAddedOldPoints[i];
                        ++ci;
                        continue;
                    }
                }
            }
            _map._mapPoints[_infoAddedOldPoints[i].indexMapPoint].isActived = false;
        }
        _currentAddedOldPoints.resize(ci);
        _infoAddedOldPoints.resize(ci);
    }
}

void ARSystem::_trackingAddedNewPoints(bool goodTracking)
{
    if (goodTracking) {
        int currentIndexKeyFrame = _map.countKeyFrames() - 1;
        QMatrix4x4 currentMatrixWorld = _map.lastKeyFrame().matrixWorld;
        QVector3D currentPos(currentMatrixWorld(0, 3), currentMatrixWorld(1, 3), currentMatrixWorld(2, 3));
        unsigned int i, ci = 0;
        for (i=0; i<_currentAddedNewPoints.size(); ++i) {
            if (_opticalFlow.trackingPoint(_currentAddedNewPoints[i])) {
                QVector3D currentDir = _pose3DEstimator.getDirFromScreen(currentMatrixWorld, _currentAddedNewPoints[i]);
                if ((QVector3D::dotProduct(currentDir, _infoAddedNewPoints[i].lastDir) < _parameters.cutOffProjections_mapPoint) ||
                        ((currentPos - _infoAddedNewPoints[i].lastPos).lengthSquared() > _parameters.distanceSquare_mapPoint)) {
                    _infoAddedNewPoints[i].projections.push_back(
                                Pose3DEstimator::ProjectionPoint(_currentAddedNewPoints[i], currentMatrixWorld));
                    _infoAddedNewPoints[i].lastDir = currentDir;
                    _infoAddedNewPoints[i].lastPos = currentPos;
                }
                if ((static_cast<int>(_infoAddedNewPoints[i].projections.size()) >= _parameters.countProjections_newMapPoint)) {
                    QVector3D resultPoint;
                    if (_pose3DEstimator.getPointInWorld(resultPoint, _infoAddedNewPoints[i].projections)) {
                        float sumErrors = _pose3DEstimator.getSumErrorSquared(resultPoint, _infoAddedNewPoints[i].projections);
                        if (sumErrors  < _parameters.limitMaxErrorSquared * _infoAddedNewPoints[i].projections.size()) {
                            Image<OF_ImageLevel::OpticalFlowInfo2D> OFImage(_parameters.sizeImageOfMapPoint);
                            if (_opticalFlow.getSubOFImage(OFImage, _currentAddedNewPoints[i])) {
                                int indexMapPoint = _map.addMapPoint(resultPoint, OFImage, sumErrors,
                                                                     _infoAddedNewPoints[i].projections.size(), currentDir);
                                _map.addMapPointToKeyFrame(indexMapPoint, currentIndexKeyFrame, _currentAddedNewPoints[i]);
                            }
                        }
                    }
                } else {
                    _currentAddedNewPoints[ci] = _currentAddedNewPoints[i];
                    _infoAddedNewPoints[ci] = _infoAddedNewPoints[i];
                    ++ci;
                }
            }
        }
        _currentAddedNewPoints.resize(ci);
        _infoAddedNewPoints.resize(ci);
        _addNewPointsToVector(_parameters.countTrackingPoints - _map.lastKeyFrame().projectedPoints.size());
    } else {
        _currentAddedNewPoints.clear();
        _infoAddedNewPoints.clear();
    }
}

Image<uchar> ARSystem::_createSmallImage() const
{
    if (_countItersForSmallImage < _parameters.countLevels)
        return _opticalFlow.imageLevel(_countItersForSmallImage).currentImage().copy();
    int countDeltaIters = _countItersForSmallImage - _parameters.countLevels + 1;
    Image<uchar> cur = _opticalFlow.imageLevel(_parameters.countLevels - 1).currentImage().copy();
    Image<uchar> t;
    while (countDeltaIters > 0) {
        t.resize(cur.size() / 2);
        Image<uchar>::halfSample(t, cur);
        Image<uchar>::swap(t, cur);
        --countDeltaIters;
    }
    return cur;
}

void ARSystem::_clearCurrentTracking()
{
    _map.setActiveKeyFrame(-1);
    std::vector<Map::MapPoint>& mapPoints = _map._mapPoints;
    unsigned int i;
    for (i=0; i<_infoAddedOldPoints.size(); ++i)
        mapPoints[_infoAddedOldPoints[i].indexMapPoint].isActived = false;
    _infoAddedOldPoints.clear();
    _currentAddedOldPoints.clear();
    _infoAddedNewPoints.clear();
    _currentAddedNewPoints.clear();
    int currentIndex = _map.countKeyFrames() - 1;
    Map::KeyFrame& current = _map.lastKeyFrame();
    for (i=0; i<current.indicesMapPoints.size(); ++i)
        _map._deleteMapPointFromKeyFrame(current.indicesMapPoints[i], currentIndex);
    current.indicesMapPoints.clear();
    current.projectedPoints.clear();
}

int ARSystem::_findKeyFrame()
{
    int indexLast = _map.countKeyFrames() - 1;
    Map::KeyFrame& current = _map._keyFrames[indexLast];
    Image<uchar> smallImage = _createSmallImage();
    int sizeImage = _parameters.sizeSmallImage.x * _parameters.sizeSmallImage.y;
    int bestKeyFrame = -1;
    float bestScore = FLT_MAX, score;
    for (int i=0; i<indexLast; ++i) {
        score = Map::diffImage(smallImage, _map.keyFrame(i).smallImage, sizeImage);
        if (score < bestScore) {
            bestKeyFrame = i;
            bestScore = score;
        }
    }
    if (bestKeyFrame < 0)
        return -1.0f;
    Map::KeyFrame& nearestKeyFrame = _map.keyFrame(bestKeyFrame);
    current.indicesMapPoints = nearestKeyFrame.indicesMapPoints;
    for (std::vector<int>::iterator it = current.indicesMapPoints.begin(); it!=current.indicesMapPoints.end(); ++it)
        _map._mapPoints[*it].indexKeyFrame.push_back(indexLast);
    current.projectedPoints = nearestKeyFrame.projectedPoints;
    current.matrixWorld = nearestKeyFrame.matrixWorld;
    _pose3DEstimator.setBeginTransform(current.matrixWorld);
    _map.setActiveKeyFrame(indexLast);
    return bestKeyFrame;
}

void ARSystem::_createNextKeyFrame()
{
    _map.addKeyFrame();
    int indexKeyFrame = _map.countKeyFrames() - 1;
    Map::KeyFrame& prev = _map.keyFrame(indexKeyFrame - 1);
    prev.image = _bwImage.copy();
    prev.smallImage = _createSmallImage();
    Map::KeyFrame& next = _map.keyFrame(indexKeyFrame);
    next.indicesMapPoints = prev.indicesMapPoints;
    for (std::vector<int>::iterator it = next.indicesMapPoints.begin(); it!=next.indicesMapPoints.end(); ++it)
        _map._mapPoints[*it].indexKeyFrame.push_back(indexKeyFrame);
    next.projectedPoints = prev.projectedPoints;
    next.matrixWorld = prev.matrixWorld;
    _map.setActiveKeyFrame(indexKeyFrame);
}

void ARSystem::research()
{
    switch (_state) {
    case TrackingNow: {
        _state = LostTracking;
    }
        break;
    case Reconstruction3D: {
        _state = LostReconstruction3D;
    }
        break;
    default:
        break;
    }
}

void ARSystem::nextState()
{
    if (_reconstructor3D->isRunning())
        return;
    switch (_state) {
    case NotTracking: {
        _map.keyFrame(0).image.resize(_bwImage.size());
        Image<uchar>::copyData(_map.keyFrame(0).image, _bwImage);
        _map.keyFrame(0).smallImage = _createSmallImage();
        _map.addKeyFrame();
        _map.keyFrame(0).projectedPoints = _currentAddedNewPoints;
        _map.keyFrame(1).projectedPoints = _map.keyFrame(0).projectedPoints;
        _currentAddedNewPoints.clear();
        _state = TrackingBegin;
    }
        break;
    case TrackingBegin: {
        _map.keyFrame(1).image.resize(_bwImage.size());
        Image<uchar>::copyData(_map.keyFrame(1).image, _bwImage);
        _map.keyFrame(1).smallImage = _createSmallImage();
        if (!_homographyInitializer.initializeMap(_pose3DEstimator, _map, _opticalFlow, _parameters.sizeImageOfMapPoint)) {
            reset();
            break;
        }
        _map.addKeyFrame();
        int currentIndexKeyFrame = _map.countKeyFrames() - 1;
        Map::KeyFrame& currentFrame = _map.keyFrame(currentIndexKeyFrame);
        currentFrame.matrixWorld = _map.keyFrame(1).matrixWorld;
        currentFrame.projectedPoints.resize(_map.keyFrame(1).projectedPoints.size());
        currentFrame.indicesMapPoints.resize(_map.keyFrame(1).projectedPoints.size());
        for (unsigned int i=0; i<_map.keyFrame(1).projectedPoints.size(); ++i) {
            currentFrame.projectedPoints[i] = _map.keyFrame(1).projectedPoints[i];
            currentFrame.indicesMapPoints[i] = _map.keyFrame(1).indicesMapPoints[i];
            _map._mapPoints[currentFrame.indicesMapPoints[i]].indexKeyFrame.push_back(currentIndexKeyFrame);
        }
        _map.setActiveKeyFrame(currentIndexKeyFrame);
        _pose3DEstimator.setBeginTransform(currentFrame.matrixWorld);
        _currentAddedNewPoints.clear();
        _infoAddedNewPoints.clear();
        _currentAddedOldPoints.clear();
        _infoAddedOldPoints.clear();
        _state = TrackingNow;
    }
        break;
    case TrackingNow: {
        _state = Reconstruction3D;
        //reset();
    }
        break;
    case Reconstruction3D: {
        _clearCurrentTracking();
        _reconstructor3D->setPose3DEstimator(_pose3DEstimator);
        _reconstructor3D->start();
        //_state = ViewDepthMap;
        //iter = 0;
        _state = LostTracking;
    }
        break;
    case LostTracking: {
        reset();
    }
        break;
    case LostReconstruction3D: {
        _clearCurrentTracking();
        _reconstructor3D->setPose3DEstimator(_pose3DEstimator);
        _reconstructor3D->start();
        //_state = ViewDepthMap;
        //iter = 0;
        _state = LostTracking;
    }
        break;
    /*case ViewDepthMap: {
        iter = iter + 1;
        if (iter >= _reconstructor3D->ss.size()) {
            iter = 0;
            _state = LostTracking;
        }
    }
        break;*/
    }
}

void ARSystem::tracking(Image<Rgba>& sourceImage, Image<uchar>& sourceBWImage)
{
    /*using namespace TMath;
    std::vector<QVector3D> mw;
    QMatrix4x4 mWorldA, mWorldB, mPlane;
    mPlane.translate(0.0f, 0.0f, - 5.0f);
    mPlane.rotate(QQuaternion::fromAxisAndAngle(2.0f, 1.0f, 3.0f, 30.0f));
    mWorldA.setToIdentity();
    QMatrix4x4 realMotion;
    realMotion.translate(-5.0f, 0.0f, - 10.0f);
    realMotion.rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -10.0f));
    mWorldB = realMotion * mWorldA;
    TMatrix<float> mProj = _pose3DEstimator.cameraProjection();
    _map.resetAll();
    _map.addKeyFrame();
    _map.addKeyFrame();
    srand(QTime::currentTime().msec());
    _pose3DEstimator.beginEstimatePose();
    std::vector<float> errors;
    errors.resize(30);
    for (unsigned int i=0; i<errors.size(); ++i) {
        errors[i] = (rand() % 20) * 0.1f;
        errors[i] *= errors[i];
    }
    std::vector<float> tErrors = errors;
    float sigmaSquared = Tukey<float>::findSigmaSquared(tErrors);
    for (int i=15; i<15; ++i) {
        QVector3D v((rand() % 100 - 50) * 0.05f, (rand() % 100 - 50) * 0.05f, 0.0f);
        v = mPlane * v;
        QVector3D vt = mWorldA * v;
        TVector<float> vf = (mProj * TVector<float>(vt.x(), vt.y(), vt.z())).projectAffine();
        IPointF f;
        f.set(vf(0), vf(1));
        _map.keyFrame(0).projectedPoints.push_back(f);
        vt = mWorldB * v;
        vf = (mProj * TVector<float>(vt.x(), vt.y(), vt.z())).projectAffine();
        f.set(vf(0), vf(1));
        float angle = (rand() % 360) * (M_PI / 180.0f);
        float error = qSqrt(errors[i]);
        //f.x += error * qCos(angle);
        //f.y += error * qSin(angle);
        _map.keyFrame(1).projectedPoints.push_back(f);
        float weight = Tukey<float>::weight(errors[i], sigmaSquared);
        _pose3DEstimator.addPoint(v, f, weight);
        mw.push_back(v);
    }
    float ea, eb;
    for (int i=15; i<30; ++i) {
        QVector3D v((rand() % 100 - 50) * 0.05f, (rand() % 100 - 50) * 0.05f, -(rand() % 100) * 0.05f);
        v = mPlane * v;
        QVector3D vt = mWorldA * v;
        IPointF f;
        TVector<float> vf = (mProj * TVector<float>(vt.x(), vt.y(), vt.z())).projectAffine();
        f.set(vf(0), vf(1));
        _map.keyFrame(0).projectedPoints.push_back(f);
        ea = _pose3DEstimator.getErrorSquared(mWorldA, v, f);
        vt = mWorldB * v;
        vf = (mProj * TVector<float>(vt.x(), vt.y(), vt.z())).projectAffine();
        f.set(vf(0), vf(1));
        float angle = (rand() % 360) * (M_PI / 180.0f);
        float error = qSqrt(errors[i]);
        //f.x += error * qCos(angle);
        //f.y += error * qSin(angle);
        _map.keyFrame(1).projectedPoints.push_back(f);
        eb = _pose3DEstimator.getErrorSquared(mWorldB, v, f);
        float weight = Tukey<float>::weight(errors[i], sigmaSquared);
        _pose3DEstimator.addPoint(v, f, weight);
        mw.push_back(v);
        ea = eb;
    }
    QMatrix4x4 t;
    _pose3DEstimator.resetBeginTransform();
    _pose3DEstimator.estimatePose();
    _pose3DEstimator.getResultMatrixWorld(t);
    QMatrix4x4 motion = _homographyInitializer.resultMotion();
    std::vector<Pose3DEstimator::ProjectionPoint> projections;
    projections.resize(2);
    projections[0].matrixWorld = mWorldA;
    projections[1].matrixWorld = mWorldB;
    QVector3D a, b;
    for (int i=0; i<_map.keyFrame(0).projectedPoints.size(); ++i) {
        projections[0].projection = _map.keyFrame(0).projectedPoints[i];
        projections[1].projection = _map.keyFrame(1).projectedPoints[i];
        if (_pose3DEstimator.getPointInWorld(a, projections)) {
            b = mw[i];
            ea = _pose3DEstimator.getSumErrorSquared(a, projections);
            eb = _pose3DEstimator.getSumErrorSquared(b, projections);
            i = i;
        }
    }
    _homographyInitializer.initializeMap(_pose3DEstimator, _map);
    return;*/
    assert(sourceImage.size() == sourceBWImage.size());
    _scaleAndBlurImage(sourceBWImage);
    _opticalFlow.setCurrentImage(_bwImage);
    switch (_state) {
    case NotTracking: {
        _opticalFlow.clearCashe();
        unsigned int i, ci = 0;
        for (i=0; i<_currentAddedNewPoints.size(); ++i) {
            if (_opticalFlow.trackingPoint(_currentAddedNewPoints[i])) {
                _currentAddedNewPoints[ci] = _currentAddedNewPoints[i];
                ++ci;
            }
        }
        _currentAddedNewPoints.resize(ci);
        _addNewPointsToVector(_parameters.countTrackingPoints);
    }
        break;
    case TrackingBegin: {
        std::vector<IPointF>& firstPoints = _map.keyFrame(0).projectedPoints;
        std::vector<IPointF>& secondPoints = _map.keyFrame(1).projectedPoints;
        if (firstPoints.size() < 6) {
            reset();
            break;
        }
        _opticalFlow.clearCashe();
        float avgDelta = 0.0f;
        unsigned int i, ci = 0;
        for (i=0; i<secondPoints.size(); ++i) {
            if (_opticalFlow.trackingPoint(secondPoints[i])) {
                firstPoints[ci] = firstPoints[i];
                secondPoints[ci] = secondPoints[i];
                IPointF d = secondPoints[ci] - firstPoints[ci];
                avgDelta += std::max(std::fabs(d.x), std::fabs(d.y));
                ++ci;
            }
        }
        avgDelta /= static_cast<float>(ci);
        firstPoints.resize(ci);
        secondPoints.resize(ci);
        if ((ci > 12) && (avgDelta > 40.0f)) {
            if (_homographyInitializer.initializeMap(_pose3DEstimator, _map, _opticalFlow, _parameters.sizeImageOfMapPoint)) {
                _map.keyFrame(1).image.resize(_bwImage.size());
                Image<uchar>::copyData(_map.keyFrame(1).image, _bwImage);
                _map.keyFrame(1).smallImage = _createSmallImage();
                _map.addKeyFrame();
                int currentIndexKeyFrame = _map.countKeyFrames() - 1;
                Map::KeyFrame& currentFrame = _map.keyFrame(currentIndexKeyFrame);
                currentFrame.matrixWorld = _map.keyFrame(1).matrixWorld;
                currentFrame.projectedPoints.resize(_map.keyFrame(1).projectedPoints.size());
                currentFrame.indicesMapPoints.resize(_map.keyFrame(1).projectedPoints.size());
                for (unsigned int i=0; i<_map.keyFrame(1).projectedPoints.size(); ++i) {
                    currentFrame.projectedPoints[i] = _map.keyFrame(1).projectedPoints[i];
                    currentFrame.indicesMapPoints[i] = _map.keyFrame(1).indicesMapPoints[i];
                    _map._mapPoints[currentFrame.indicesMapPoints[i]].indexKeyFrame.push_back(currentIndexKeyFrame);
                }
                _map.setActiveKeyFrame(currentIndexKeyFrame);
                _pose3DEstimator.setBeginTransform(currentFrame.matrixWorld);
                _currentAddedNewPoints.clear();
                _infoAddedNewPoints.clear();
                _currentAddedOldPoints.clear();
                _infoAddedOldPoints.clear();
                _state = TrackingNow;
            } else {
                _map.deleteAllMapPointsAndSaveProjections();
            }
        }
    }
        break;
    case TrackingNow: {
        _opticalFlow.clearCashe();
        float error = _trackingPose();
        if (error < 0.0f) {
            _clearCurrentTracking();
            _state = LostTracking;
            break;
        }
        bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
        _trackingAddedOldPoints(goodTracking);
        _trackingAddedNewPoints(goodTracking);
        if (goodTracking)
            if (!_map.existNearestKeyFrame())
                _createNextKeyFrame();
    }
        break;
    case Reconstruction3D: {
        _opticalFlow.clearCashe();
        float error = _trackingPose();
        if (error < 0.0f) {
            _clearCurrentTracking();
            _state = LostReconstruction3D;
            break;
        }
        bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
        _trackingAddedOldPoints(goodTracking);
        _trackingAddedNewPoints(goodTracking);
        if (goodTracking)
            if (!_map.existNearestKeyFrame())
                _createNextKeyFrame();
        Map::KeyFrame& lastKeyFrame = _map.lastKeyFrame();
        if (_map.countRecordFrames() > 0) {
            Map::RecordFrame& lastRecordFrame = _map.lastRecordFrame();
            if ((Map::cutOffProjection(lastKeyFrame.matrixWorld, lastRecordFrame.matrixWorld) < _parameters.cutOff_record) ||
                    (Map::distanceSquared(lastKeyFrame.matrixWorld, lastRecordFrame.matrixWorld) > _parameters.distanceSquare_record)) {
                _map.addRecordFrame();
                Map::RecordFrame& newRecordFrame = _map.lastRecordFrame();
                newRecordFrame.image = sourceBWImage.copy();
                newRecordFrame.colorImage = sourceImage.copy();
                newRecordFrame.matrixWorld = lastKeyFrame.matrixWorld;
            }
        } else {
            _map.addRecordFrame();
            Map::RecordFrame& newRecordFrame = _map.lastRecordFrame();
            newRecordFrame.image = sourceBWImage.copy();
            newRecordFrame.colorImage = sourceImage.copy();
            newRecordFrame.matrixWorld = lastKeyFrame.matrixWorld;
        }
    }
        break;
    case LostTracking: {
        int indexBestKeyFrame = _findKeyFrame();
        if (indexBestKeyFrame >= 0) {
            Map::KeyFrame& keyFrame = _map.keyFrame(indexBestKeyFrame);
            _opticalFlow.clearCashe();
            _opticalFlow.setPrevImage(keyFrame.image);
            float error = _trackingPose();
            bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
            if (error < 0.0f)
                goodTracking = false;
            if ((goodTracking)) {
                _state = TrackingNow;
                break;
            } else {
                _clearCurrentTracking();
                break;
            }
        }
    }
        break;
    case LostReconstruction3D: {
        int indexBestKeyFrame = _findKeyFrame();
        if (indexBestKeyFrame >= 0) {
            Map::KeyFrame& keyFrame = _map.keyFrame(indexBestKeyFrame);
            _opticalFlow.clearCashe();
            _opticalFlow.setPrevImage(keyFrame.image);
            float error = _trackingPose();
            bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
            if (error < 0.0f)
                goodTracking = false;
            if ((goodTracking)) {
                _state = Reconstruction3D;
                break;
            } else {
                _clearCurrentTracking();
                break;
            }
        }
    }
        break;
    /*case ViewDepthMap: {
        _sourceImage = _reconstructor3D->ss[iter].copy();
        Image<Rgb> depthMap = _reconstructor3D->depthMap();
        Painter::drawImage(_sourceImage, depthMap, IPointF::ZERO, IPointF(depthMap.width(), depthMap.height()) * 1.5f);
    }
        break;*/
    }
    _opticalFlow.setCurrentImageToPrev();
}

void ARSystem::tracking(FrameProvider* frameProvider, QOpenGLFunctions* glFunctions,
                        QOpenGLFramebufferObject* FBO_color, QOpenGLFramebufferObject* FBO_luminance)
{
    FBO_luminance->bind();
    frameProvider->bindLuminanceShader(glFunctions);
    frameProvider->getLuminanceImage(glFunctions, _bwImage);
    _opticalFlow.setCurrentImage(_bwImage);
    switch (_state) {
    case NotTracking: {
        _opticalFlow.clearCashe();
        unsigned int i, ci = 0;
        for (i=0; i<_currentAddedNewPoints.size(); ++i) {
            if (_opticalFlow.trackingPoint(_currentAddedNewPoints[i])) {
                _currentAddedNewPoints[ci] = _currentAddedNewPoints[i];
                ++ci;
            }
        }
        _currentAddedNewPoints.resize(ci);
        _addNewPointsToVector(_parameters.countTrackingPoints);
    }
        break;
    case TrackingBegin: {
        std::vector<IPointF>& firstPoints = _map.keyFrame(0).projectedPoints;
        std::vector<IPointF>& secondPoints = _map.keyFrame(1).projectedPoints;
        if (firstPoints.size() < 6) {
            reset();
            break;
        }
        _opticalFlow.clearCashe();
        float avgDelta = 0.0f;
        unsigned int i, ci = 0;
        for (i=0; i<secondPoints.size(); ++i) {
            if (_opticalFlow.trackingPoint(secondPoints[i])) {
                firstPoints[ci] = firstPoints[i];
                secondPoints[ci] = secondPoints[i];
                IPointF d = secondPoints[ci] - firstPoints[ci];
                avgDelta += std::max(std::fabs(d.x), std::fabs(d.y));
                ++ci;
            }
        }
        avgDelta /= static_cast<float>(ci);
        firstPoints.resize(ci);
        secondPoints.resize(ci);
        if ((ci > 12) && (avgDelta > 40.0f)) {
            if (_homographyInitializer.initializeMap(_pose3DEstimator, _map, _opticalFlow, _parameters.sizeImageOfMapPoint)) {
                _map.keyFrame(1).image.resize(_bwImage.size());
                Image<uchar>::copyData(_map.keyFrame(1).image, _bwImage);
                _map.keyFrame(1).smallImage = _createSmallImage();
                _map.addKeyFrame();
                int currentIndexKeyFrame = _map.countKeyFrames() - 1;
                Map::KeyFrame& currentFrame = _map.keyFrame(currentIndexKeyFrame);
                currentFrame.matrixWorld = _map.keyFrame(1).matrixWorld;
                currentFrame.projectedPoints.resize(_map.keyFrame(1).projectedPoints.size());
                currentFrame.indicesMapPoints.resize(_map.keyFrame(1).projectedPoints.size());
                for (unsigned int i=0; i<_map.keyFrame(1).projectedPoints.size(); ++i) {
                    currentFrame.projectedPoints[i] = _map.keyFrame(1).projectedPoints[i];
                    currentFrame.indicesMapPoints[i] = _map.keyFrame(1).indicesMapPoints[i];
                    _map._mapPoints[currentFrame.indicesMapPoints[i]].indexKeyFrame.push_back(currentIndexKeyFrame);
                }
                _map.setActiveKeyFrame(currentIndexKeyFrame);
                _pose3DEstimator.setBeginTransform(currentFrame.matrixWorld);
                _currentAddedNewPoints.clear();
                _infoAddedNewPoints.clear();
                _currentAddedOldPoints.clear();
                _infoAddedOldPoints.clear();
                _state = TrackingNow;
            } else {
                _map.deleteAllMapPointsAndSaveProjections();
            }
        }
    }
        break;
    case TrackingNow: {
        _opticalFlow.clearCashe();
        float error = _trackingPose();
        if (error < 0.0f) {
            _clearCurrentTracking();
            _state = LostTracking;
            break;
        }
        bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
        _trackingAddedOldPoints(goodTracking);
        _trackingAddedNewPoints(goodTracking);
        if (goodTracking)
            if (!_map.existNearestKeyFrame())
                _createNextKeyFrame();
    }
        break;
    case Reconstruction3D: {
        _opticalFlow.clearCashe();
        float error = _trackingPose();
        if (error < 0.0f) {
            _clearCurrentTracking();
            _state = LostReconstruction3D;
            break;
        }
        bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
        _trackingAddedOldPoints(goodTracking);
        _trackingAddedNewPoints(goodTracking);
        if (goodTracking)
            if (!_map.existNearestKeyFrame())
                _createNextKeyFrame();
        Image<Rgba> colorImage(_bwImage.size());
        FBO_color->bind();
        frameProvider->bindColorShader(glFunctions);
        frameProvider->getColorImage(glFunctions, colorImage);
        Map::KeyFrame& lastKeyFrame = _map.lastKeyFrame();
        if (_map.countRecordFrames() > 0) {
            Map::RecordFrame& lastRecordFrame = _map.lastRecordFrame();
            if ((Map::cutOffProjection(lastKeyFrame.matrixWorld, lastRecordFrame.matrixWorld) < _parameters.cutOff_record) ||
                    (Map::distanceSquared(lastKeyFrame.matrixWorld, lastRecordFrame.matrixWorld) > _parameters.distanceSquare_record)) {
                _map.addRecordFrame();
                Map::RecordFrame& newRecordFrame = _map.lastRecordFrame();
                newRecordFrame.image = _bwImage.copy();
                newRecordFrame.colorImage = colorImage;
                newRecordFrame.matrixWorld = lastKeyFrame.matrixWorld;
            }
        } else {
            _map.addRecordFrame();
            Map::RecordFrame& newRecordFrame = _map.lastRecordFrame();
            newRecordFrame.image = _bwImage.copy();
            newRecordFrame.colorImage = colorImage;
            newRecordFrame.matrixWorld = lastKeyFrame.matrixWorld;
        }
    }
        break;
    case LostTracking: {
        int indexBestKeyFrame = _findKeyFrame();
        if (indexBestKeyFrame >= 0) {
            Map::KeyFrame& keyFrame = _map.keyFrame(indexBestKeyFrame);
            _opticalFlow.clearCashe();
            _opticalFlow.setPrevImage(keyFrame.image);
            float error = _trackingPose();
            bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
            if (error < 0.0f)
                goodTracking = false;
            if ((goodTracking)) {
                _state = TrackingNow;
                break;
            } else {
                _clearCurrentTracking();
                break;
            }
        }
    }
        break;
    case LostReconstruction3D: {
        int indexBestKeyFrame = _findKeyFrame();
        if (indexBestKeyFrame >= 0) {
            Map::KeyFrame& keyFrame = _map.keyFrame(indexBestKeyFrame);
            _opticalFlow.clearCashe();
            _opticalFlow.setPrevImage(keyFrame.image);
            float error = _trackingPose();
            bool goodTracking = (error < _parameters.limitMaxErrorSquared * _parameters.limitErrorForGoodTracking);
            if (error < 0.0f)
                goodTracking = false;
            if ((goodTracking)) {
                _state = Reconstruction3D;
                break;
            } else {
                _clearCurrentTracking();
                break;
            }
        }
    }
        break;
    }
    _opticalFlow.setCurrentImageToPrev();
    //glFunctions->glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
}

}
