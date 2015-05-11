#include "AR/Reconstructor3D.h"
#include "AR/OF_ImageLevel.h"
#include <QOpenGLTexture>
#include <QVector2D>
#include <QDebug>
#include <QTime>
#include <QStandardPaths>
#include <cfloat>
#include <limits>
#include <cmath>
#include <functional>
#include <algorithm>
#include <map>

namespace AR {

Reconstructor3D::ReconstructField::ReconstructField():
    _frames(nullptr)
{
    _depthMapSize.set(160, 120);
    _blurDeltaZ = 0.9f;
}

Reconstructor3D::ReconstructField::~ReconstructField()
{
}

bool Reconstructor3D::ReconstructField::getLocalPointInDepthMap(IPointF& mapCoord, float& localZ,
                                                                const QMatrix4x4& matrixWorld, const QVector3D& point) const
{
    localZ = - (matrixWorld(2, 0) * point.x() + matrixWorld(2, 1) * point.y() +
                matrixWorld(2, 2) * point.z() + matrixWorld(2, 3));
    if ((localZ < 0.0005f) && (localZ < 100.0f))
        return false;
    mapCoord.x = (matrixWorld(0, 0) * point.x() + matrixWorld(0, 1) * point.y() +
                  matrixWorld(0, 2) * point.z() + matrixWorld(0, 3));
    mapCoord.x /= localZ;
    mapCoord.y = (matrixWorld(1, 0) * point.x() + matrixWorld(1, 1) * point.y() +
                  matrixWorld(1, 2) * point.z() + matrixWorld(1, 3));
    mapCoord.y /= localZ;
    mapCoord.set((mapCoord.x * _pixelFocalLength.x) + _pixelOpticalCenter.x,
                 (mapCoord.y * _pixelFocalLength.y) + _pixelOpticalCenter.y);
    if ((mapCoord.x < 0.0f) || (mapCoord.y < 0.0f) || (mapCoord.x >= _depthMapSize.x) || (mapCoord.y >= _depthMapSize.y))
        return false;
    return true;
}

float Reconstructor3D::ReconstructField::getSubPixelValue(const Image<float>& depthMap, const IPointF& point)
{
    IPoint fPoint(qFloor(point.x), qFloor(point.y));
    IPointF subDelta(point.x - fPoint.x, point.y - fPoint.y);
    IPointF invSubDelta(1.0f - subDelta.x, 1.0f - subDelta.y);
    const float* strA = &depthMap.data()[depthMap.width() * fPoint.y];
    const float* strB = &strA[depthMap.width()];
    if (strA[fPoint.x] > 0.0f) {
        if (strA[fPoint.x+1] > 0.0f) {
            if (strB[fPoint.x] > 0.0f) {
                if (strB[fPoint.x+1] > 0.0f) {
                    return ((strA[fPoint.x] * invSubDelta.x * invSubDelta.y) +
                            (strA[fPoint.x+1] * subDelta.x * invSubDelta.y) +
                            (strB[fPoint.x] * invSubDelta.x * subDelta.y) +
                            (strB[fPoint.x+1] * subDelta.x * subDelta.y));
                } else {
                    float dis1 = std::sqrt(subDelta.x * subDelta.x + subDelta.y * subDelta.y);
                    float dis2 = std::sqrt(invSubDelta.x * invSubDelta.x + subDelta.y * subDelta.y);
                    float dis3 = std::sqrt(subDelta.x * subDelta.x + invSubDelta.y * invSubDelta.y);
                    return (strA[fPoint.x] * dis1 + strA[fPoint.x] * dis2 + strB[fPoint.x] * dis3) / (dis1 + dis2 + dis3);
                }
            } else {
                if (strB[fPoint.x+1] > 0.0f) {
                    float dis1 = std::sqrt(subDelta.x * subDelta.x + subDelta.y * subDelta.y);
                    float dis2 = std::sqrt(invSubDelta.x * invSubDelta.x + subDelta.y * subDelta.y);
                    float dis3 = std::sqrt(invSubDelta.x * invSubDelta.x + invSubDelta.y * invSubDelta.y);
                    return (strA[fPoint.x] * dis1 + strA[fPoint.x] * dis2 + strB[fPoint.x+1] * dis3) / (dis1 + dis2 + dis3);
                }
            }
        } else {
            if (strB[fPoint.x] > 0.0f) {
                if (strB[fPoint.x+1] > 0.0f) {
                    float dis1 = std::sqrt(subDelta.x * subDelta.x + subDelta.y * subDelta.y);
                    float dis2 = std::sqrt(subDelta.x * subDelta.x + invSubDelta.y * invSubDelta.y);
                    float dis3 = std::sqrt(invSubDelta.x * invSubDelta.x + invSubDelta.y * invSubDelta.y);
                    return (strA[fPoint.x] * dis1 + strB[fPoint.x] * dis2 + strB[fPoint.x+1] * dis3) / (dis1 + dis2 + dis3);
                }
            }
        }
    } else {
        if (strA[fPoint.x+1] > 0.0f) {
            if (strB[fPoint.x] > 0.0f) {
                if (strB[fPoint.x+1] > 0.0f) {
                    float dis1 = std::sqrt(invSubDelta.x * invSubDelta.x + subDelta.y * subDelta.y);
                    float dis2 = std::sqrt(subDelta.x * subDelta.x + invSubDelta.y * invSubDelta.y);
                    float dis3 = std::sqrt(invSubDelta.x * invSubDelta.x + invSubDelta.y * invSubDelta.y);
                    return (strA[fPoint.x+1] * dis1 + strB[fPoint.x] * dis2 + strB[fPoint.x+1] * dis3) / (dis1 + dis2 + dis3);
                }
            }
        }
    }
    return 0.0f;
}

float Reconstructor3D::ReconstructField::value(const QVector3D& point)
{
    _vectorOfDeltaZ.resize(0);
    IPointF mapCoord;
    float localDepth, depth;
    float deltaDepth, score, resultDeltaDepth, minScore = FLT_MAX;
    bool finded = false;
    const std::vector<ReconstructFrame>& frames = *_frames;
    for (std::vector<ReconstructFrame>::const_iterator it = frames.begin(); it!=frames.end(); ++it) {
        if (getLocalPointInDepthMap(mapCoord, localDepth, it->matrixWorld, point)) {
            depth = getSubPixelValue(it->depthMap, mapCoord);
            if (depth > 0.0f) {
                deltaDepth = localDepth - depth;
                score = std::fabs(localDepth);
                _vectorOfDeltaZ.push_back(deltaDepth);
                if (score < minScore) {
                    minScore = score;
                    resultDeltaDepth = deltaDepth;
                    finded = true;
                }
            }
        }
    }
    if (!finded)
        return FLT_MAX;
    float sum = 0.0f;
    float weight = 0.0f;
    for (std::vector<float>::const_iterator it = _vectorOfDeltaZ.begin(); it != _vectorOfDeltaZ.end(); ++it) {
        deltaDepth = std::fabs(*it - resultDeltaDepth);
        if (deltaDepth < _blurDeltaZ) {
            localDepth = 1.0f - deltaDepth / _blurDeltaZ;
            sum += *it * localDepth;
            weight += localDepth;
        }
    }
    return sum / weight;
}

Reconstructor3D::DepthMapProcessor::DepthMapProcessor()
{
    _countIterations = 10;
    _sizeCursor = IPoint(6, 6);
    _minArea = 10;
    _maxDeltaZ = 0.07f;
    _blurSigmaSquared = 2.0f * 2.0f;
    _blurDeltaZ = 0.9f;
}

void Reconstructor3D::DepthMapProcessor::_clearNoise()
{
    Image<IPoint> parentImage(_depthMap.size());
    std::function<IPoint(int, int)> getParent = [&parentImage](int x, int y) -> IPoint
    {
        IPoint child(x, y);
        IPoint parent = parentImage(child);
        while (parent.x >= 0) {
            child = parent;
            parent = parentImage(child);
        }
        return child;
    };
    float* strA;
    float* strB = _depthMap.data();
    IPoint* strParent = parentImage.data();
    IPoint p, parent, parentB;
    p.y = 0;
    strParent[0].set(-1,  1);
    for (p.x=1; p.x<_depthMap.width(); ++p.x) {
        if (strB[p.x] == 0.0f)
            continue;
        if ((strB[p.x-1] > 0.0f) && (std::fabs(strB[p.x-1] - strB[p.x]) < _maxDeltaZ)) {
            parent = getParent(p.x - 1, 0);
            ++parentImage(parent).y;
            strParent[p.x] = parent;
        } else {
            strParent[p.x].set(-1, 1);
        }
    }
    for (p.y=1; p.y<_depthMap.height(); ++p.y) {
        strA = strB;
        strB = &strB[_depthMap.width()];
        strParent = &strParent[_depthMap.width()];
        if (strB[0] > 0.0f) {
            if ((strA[0] > 0.0f) && (std::fabs(strA[0] - strB[0]) < _maxDeltaZ)) {
                parent = getParent(0, p.y - 1);
                ++parentImage(parent).y;
                strParent[0] = parent;
            } else {
                strParent[0].set(-1, 1);
            }
        }
        for (p.x=1; p.x<_depthMap.width(); ++p.x) {
            if (strB[p.x] == 0.0f)
                continue;
            if ((strB[p.x-1] > 0.0f) && (std::fabs(strB[p.x-1] - strB[p.x]) < _maxDeltaZ)) {
                if ((strA[p.x] > 0.0f) && (std::fabs(strA[p.x] - strB[p.x]) < _maxDeltaZ)) {
                    parent = getParent(p.x-1, p.y);
                    strParent[p.x] = parent;
                    parentB = getParent(p.x, p.y-1);
                    if (parent == parentB) {
                        ++parentImage(parent).y;
                    } else {
                        IPoint& parentB_ptrValue = parentImage(parentB);
                        parentImage(parent).y += parentB_ptrValue.y + 1;
                        parentB_ptrValue = parent;
                    }
                } else {
                    parent = getParent(p.x-1, p.y);
                    ++parentImage(parent).y;
                    strParent[p.x] = parent;
                }
            } else if ((strA[p.x] > 0.0f) && (std::fabs(strA[p.x] - strB[p.x]) < _maxDeltaZ)) {
                parent = getParent(p.x, p.y-1);
                ++parentImage(parent).y;
                strParent[p.x] = parent;
            } else {
                strParent[p.x].set(-1, 1);
            }
        }
    }
    static const IPoint neightbors[8] = {IPoint(-1, -1), IPoint(0, -1), IPoint( 1, -1), IPoint( 1, 0),
                                         IPoint( 1,  1), IPoint(0,  1), IPoint(-1,  1), IPoint(-1, 0) };
    int i, j;
    IPoint pA, pB;
    float zA, zB;
    strA = _depthMap.data();
    for (p.y=1; p.y<_depthMap.height()-1; ++p.y) {
        strA = &strA[_depthMap.width()];
        for (p.x=1; p.x<_depthMap.width()-1; ++p.x) {
            if (strA[p.x] == 0.0f) {
                for (i=0; i<8; ++i) {
                    pA = p + neightbors[i];
                    zA = _depthMap(pA);
                    if (zA > 0.0f) {
                        parent = getParent(pA.x, pA.y);
                        for (j=i+1; j<8; ++j) {
                            pB = p + neightbors[j];
                            zB = _depthMap(pB);
                            if (zB > 0.0f) {
                                if (std::fabs(zA - zB) < _maxDeltaZ) {
                                    parentB = getParent(pB.x, pB.y);
                                    if (parent != parentB) {
                                        IPoint& parentB_ptrValue = parentImage(parentB);
                                        parentImage(parent).y += parentB_ptrValue.y;
                                        parentB_ptrValue = parent;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    strA = _depthMap.data();
    strParent = parentImage.data();
    for (p.y=0; p.y<_depthMap.height(); ++p.y) {
        for (p.x=0; p.x<_depthMap.width(); ++p.x) {
            if (strA[p.x] == 0.0f)
                continue;
            if (parentImage(getParent(p.x, p.y)).y < _minArea) {
                strA[p.x] = 0.0f;
            }
        }
        strParent = &strParent[_depthMap.width()];
        strA = &strA[_depthMap.width()];
    }
}

void Reconstructor3D::DepthMapProcessor::_calculateInfoImage()
{
    _infoImage = Image<Info>(_depthMap.size() - IPoint(2, 2));
    const float* strA = _depthMap.data();
    const float* strB = &strA[_depthMap.width()];
    const float* strC = &strB[_depthMap.width()];
    Info* strInfo = _infoImage.data();
    IPoint p;
    for (p.y=1; p.y<=_infoImage.height(); ++p.y) {
        for (p.x=1; p.x<=_infoImage.width(); ++p.x) {
            Info& info = strInfo[p.x-1];
            if ((strB[p.x-1] > 0.0f) && (strB[p.x+1] > 0.0f) && (strA[p.x] > 0.0f) && (strC[p.x] > 0.0f)) {
                info.d.set(strB[p.x+1] - strB[p.x-1], strC[p.x] - strA[p.x]);
                if (info.d.normalize() == 0.0f)
                    info.d.x = 10.0f;
            } else {
                info.d.x = 10.0f;
            }
            info.iteration = (strB[p.x] > 0.0f) ? 0 : -1;
        }
        strA = strB;
        strB = strC;
        strC = &strC[_depthMap.width()];
        strInfo = &strInfo[_infoImage.width()];
    }
}

float Reconstructor3D::DepthMapProcessor::_getBluredPoint(const Image<float>& depthMap, float kernelValue, const IPoint& point,
                                                          const float& k1, const float& invK2)
{
    IPoint begin(qMax(point.x - _sizeCursor.x, 0), qMax(point.y - _sizeCursor.y, 0));
    IPoint end(qMin(point.x + _sizeCursor.x, depthMap.width()), qMin(point.y + _sizeCursor.y, depthMap.height()));
    IPoint p;
    float sumValue = 0.0f;
    float sumWeight = 0.0f;
    const float* str = &depthMap.data()[depthMap.width() * begin.y];
    for (p.y=begin.y; p.y<end.y; ++p.y) {
        for (p.x=begin.x; p.x<end.x; ++p.x) {
            if (str[p.x] == 0.0f)
                continue;
            if (std::fabs(str[p.x] - kernelValue) > _blurDeltaZ)
                continue;
            float weight = k1 * std::exp(- (p - point).lengthSquared() * invK2);
            sumValue += str[p.x] * weight;
            sumWeight += weight;
        }
        str = &str[depthMap.width()];
    }
    return sumValue / sumWeight;
}

void Reconstructor3D::DepthMapProcessor::_blurMap()
{
    Image<float> copyDepthMap = _depthMap.copy();
    float k1 = 1.0f / (2.0f * M_PI * _blurSigmaSquared);
    float invK2 = 1.0f / (2.0f * _blurSigmaSquared);
    IPoint p;
    float* str = _depthMap.data();
    for (p.y=0; p.y<_depthMap.height(); ++p.y) {
        for (p.x=0; p.x<_depthMap.width(); ++p.x) {
            if (str[p.x] > 0.0f)
                str[p.x] = _getBluredPoint(copyDepthMap, str[p.x], p, k1, invK2);
        }
        str = &str[_depthMap.width()];
    }
}

void Reconstructor3D::DepthMapProcessor::process()
{
    TMath_assert((_depthMap.width() >= 3) && (_depthMap.height() >= 3));
    _clearNoise();
    _calculateInfoImage();
    IPoint p;
    for (_iteration = 0; _iteration<_countIterations; ++_iteration) {
        const Info* strA = _infoImage.data();
        const Info* strB = &strA[_infoImage.width()];
        const Info* strC = &strB[_infoImage.width()];
        for (p.y=2; p.y<_infoImage.height(); ++p.y) {
            for (p.x=2; p.x<_infoImage.width(); ++p.x) {
                if (strB[p.x-1].iteration < 0) {
                    if ((strB[p.x-2].iteration == _iteration) || (strB[p.x].iteration == _iteration) ||
                            (strA[p.x-1].iteration == _iteration) || (strC[p.x-1].iteration == _iteration)) {
                        _fillElement(p);
                    }
                }
            }
            strA = strB;
            strB = strC;
            strC = &strC[_infoImage.width()];
        }
    }
    _blurMap();
}

void Reconstructor3D::DepthMapProcessor::_fillElement(const IPoint& c)
{
    IPoint begin = c - _sizeCursor;
    if (begin.x < 2)
        begin.x = 2;
    if (begin.y < 2)
        begin.y = 2;
    IPoint end = c + _sizeCursor;
    if (end.x > _depthMap.width() - 3)
        end.x = _depthMap.width() - 3;
    if (end.y > _depthMap.height() - 3)
        end.y = _depthMap.height() - 3;
    Info* strInfo = &_infoImage.data()[(begin.y - 1) * _infoImage.width()];
    float* strMap = &_depthMap.data()[begin.y * _depthMap.width()];
    IPoint p;
    int count = 0;
    float sumWValue = 0.0f, sumWeight = 0.0f;
    for (p.y=begin.y; p.y<c.y; ++p.y) {
        for (p.x=begin.x; p.x<=end.x; ++p.x) {
            Info& info = strInfo[p.x-1];
            if (info.iteration < 0)
                continue;
            float weight = _getWeight(IPointF(p.x - c.x, p.y - c.y), info.d);
            sumWValue += strMap[p.x] * weight;
            sumWeight += weight;
            ++count;
        }
        strInfo = &strInfo[_infoImage.width()];
        strMap = &strMap[_depthMap.width()];
    }
    for (p.x=begin.x; p.x<c.x; ++p.x) {
        Info& info = strInfo[p.x-1];
        if (info.iteration < 0)
            continue;
        float weight = _getWeight(IPointF(p.x - c.x, p.y - c.y), info.d);
        sumWValue += strMap[p.x] * weight;
        sumWeight += weight;
        ++count;
    }
    for (p.x=c.x+1; p.x<=end.x; ++p.x) {
        Info& info = strInfo[p.x-1];
        if (info.iteration < 0)
            continue;
        float weight = _getWeight(IPointF(p.x - c.x, p.y - c.y), info.d);
        sumWValue += strMap[p.x] * weight;
        sumWeight += weight;
        ++count;
    }
    strInfo = &strInfo[_infoImage.width()];
    strMap = &strMap[_depthMap.width()];
    for (p.y=c.y+1; p.y<=end.y; ++p.y) {
        for (p.x=begin.x; p.x<=end.x; ++p.x) {
            Info& info = strInfo[p.x-1];
            if (info.iteration < 0)
                continue;
            float weight = _getWeight(IPointF(p.x - c.x, p.y - c.y), info.d);
            sumWValue += strMap[p.x] * weight;
            sumWeight += weight;
            ++count;
        }
        strInfo = &strInfo[_infoImage.width()];
        strMap = &strMap[_depthMap.width()];
    }
    if (count < _minArea)
        return;
    sumWValue /= sumWeight;
    _depthMap(c) = sumWValue;
    IPoint c0(c.x - 1, c.y - 1);
    _infoImage(c0).iteration = _iteration + 1;
    const float& r0 = _depthMap(c.x-1, c.y-1);
    const float& r1 = _depthMap(c.x+1, c.y-1);
    const float& r2 = _depthMap(c.x+1, c.y+1);
    const float& r3 = _depthMap(c.x-1, c.y+1);
    IPointF tDiff;
    if (r0 > 0.0f) {
        if (r1 > 0.0f) {
            if (_depthMap(c.x, c.y-2) > 0.0f) {
               tDiff.set(r1 - r0, sumWValue - _depthMap(c.x, c.y-2));
               if (tDiff.normalize() > 0.0f)
                    _infoImage(c0.x, c0.y-1).d = tDiff;
            }
        }
        if (r3 > 0.0f) {
            if (_depthMap(c.x-2, c.y) > 0.0f) {
                tDiff.set(sumWValue - _depthMap(c.x-2, c.y), r3 - r0);
                if (tDiff.normalize() > 0.0f)
                    _infoImage(c0.x-1, c0.y).d = tDiff;
            }
        }
    }
    if (r2 > 0.0f) {
        if (r1 > 0.0f) {
            if (_depthMap(c.x+2, c.y) > 0.0f) {
               tDiff.set(_depthMap(c.x+2, c.y) - sumWValue, r2 - r1);
               if (tDiff.normalize() > 0.0f)
                    _infoImage(c0.x+1, c0.y).d = tDiff;
            }
        }
        if (r3 > 0.0f) {
            if (_depthMap(c.x, c.y+2) > 0.0f) {
                tDiff.set(r2 - r3, _depthMap(c.x, c.y+2) - sumWValue);
                if (tDiff.normalize() > 0.0f)
                     _infoImage(c.x, c.y+1).d = tDiff;
            }
        }
    }
}

Reconstructor3D::Reconstructor3D(QObject* parent) :
    QThread(parent)
{
    _opticalFlow.resize(IPoint(100, 100));
    _map = nullptr;
    _entity = nullptr;
    setParameters(_parameters);
    setPose3DEstimator(_pose3DEstimator);
    _rgbSwap = false;
}

void Reconstructor3D::setPose3DEstimator(const Pose3DEstimator& pose3DEstimator)
{
    _opticalCenter = pose3DEstimator.getOpticalCenter();
    _focalLength = pose3DEstimator.getFocalLength();
}

void Reconstructor3D::setParameters(const ReconstructParameters& parameters)
{
    _parameters = parameters;
    _opticalFlow.setCountImageLevels(_parameters.countLevels);
    _opticalFlow.setSizeCursor(_parameters.sizeCursor);
    _opticalFlow.setDetThreshold(_parameters.detThreshold);
    _opticalFlow.setErrorThreshold(_parameters.errorThreshold);
    _opticalFlow.setEpsDeltaStep(_parameters.epsDeltaStep);
    _opticalFlow.setMaxCountIterationForTrackingPoints(_parameters.maxCountIterationForTrackingPoints);
    _opticalFlow.setMaxLocalVelocity(_parameters.maxLocalVelocity);
    _opticalFlow.setSizeCursor(_parameters.sizeCursor);
    _opticalFlow.setMinSquareDistancePointsBig(_parameters.minSquareDistancePointsBig);
    _opticalFlow.setMinSquareDistancePointsSmall(_parameters.minSquareDistancePointsSmall);
    _reconstructField.setDepthMapSize(_parameters.sizeDepthMap);
}

void Reconstructor3D::_createCurrentReconstructFrame(Map::RecordFrame& recordFrame)
{
    _reconstructFrames.resize(_reconstructFrames.size() + 1);
    ReconstructFrame& reconstructFrame = _reconstructFrames[_reconstructFrames.size() - 1];
    reconstructFrame.colorImage = recordFrame.colorImage;
    reconstructFrame.matrixWorld = recordFrame.matrixWorld;
    reconstructFrame.depthMap = Image<float>(_reconstructImage.size());
    //reconstructFrame.depthMap.fill(0.0f);
    _opticalFlow.setPrevImage(recordFrame.image);
    IPoint windowBegin = _opticalFlow.imageLevel(0).windowBegin();
    IPoint windowEnd = _opticalFlow.imageLevel(0).windowEnd();
    IPoint windowSize = windowEnd - windowBegin;
    IPointF scale(windowSize.x / static_cast<float>(_reconstructImage.width()),
                  windowSize.y / static_cast<float>(_reconstructImage.height()));
    //Image<Rgb> s = _bwToRgb(recordFrame.image);
    IPoint p;
    IPointF pointInImage;
    ReconstructPoint* str = _reconstructImage.data();
    for (p.y=0; p.y<_reconstructImage.height(); ++p.y) {
        for (p.x=0; p.x<_reconstructImage.width(); ++p.x) {
            pointInImage.set(p.x * scale.x + windowBegin.x, p.y * scale.y + windowBegin.y);
            str[p.x].projections.resize(1);
            str[p.x].projections[0].projection = pointInImage;
            str[p.x].projections[0].matrixWorld = recordFrame.matrixWorld;
            //Painter::drawCross(s, pointInImage, 4.0f, Rgb(255,0,0));
        }
        str = &str[_reconstructImage.width()];
    }
    //ss.push_back(s);
}

void Reconstructor3D::_solveCurrentRecordFrame(Map::RecordFrame& recordFrame)
{
    _opticalFlow.clearCashe();
    _opticalFlow.getDiffPyramid(_diffPyramid);
    _opticalFlow.setCurrentImage(recordFrame.image);
    /*IPoint windowBegin = _opticalFlow.imageLevel(0).windowBegin();
    IPoint windowEnd = _opticalFlow.imageLevel(0).windowEnd();
    IPoint windowSize = windowEnd - windowBegin;
    IPointF scale(windowSize.x / static_cast<float>(_reconstructField.depthMapSize().x),
                  windowSize.y / static_cast<float>(_reconstructField.depthMapSize().y));*/
    //Image<Rgb> s = _bwToRgb(recordFrame.image);
    IPoint p;
    IPointF pointInImage;
    ReconstructPoint* str = _reconstructImage.data();
    for (p.y=0; p.y<_reconstructImage.height(); ++p.y) {
        for (p.x=0; p.x<_reconstructImage.width(); ++p.x) {
            //pointInImage.set(p.x * scale.x + windowBegin.x, p.y * scale.y + windowBegin.y);
            pointInImage = str[p.x].projections[0].projection;
            if (_opticalFlow.trackingPoint(pointInImage, _diffPyramid)) {
                str[p.x].projections.push_back(Pose3DEstimator::ProjectionPoint(pointInImage, recordFrame.matrixWorld));
                //Painter::drawCross(s, pointInImage, 4.0f, Rgb(255,0,0));
            }
        }
        str = &str[_reconstructImage.width()];
    }
    //ss.push_back(s);
}

void Reconstructor3D::_finishSolveReconstructFrame(ReconstructFrame& reconstructFrame)
{
    QVector3D result;
    IPoint p;
    reconstructFrame.depthMap.fill(0.0f);
    float* strD = reconstructFrame.depthMap.data();
    ReconstructPoint* str = _reconstructImage.data();
    for (p.y=0; p.y<_reconstructImage.height(); ++p.y) {
        for (p.x=0; p.x<_reconstructImage.width(); ++p.x) {
            if (str[p.x].projections.size() >= 2) {
                if (_pose3DEstimator.getPointInWorld(result, str[p.x].projections)) {
                    _pointToFrames(str[p.x], reconstructFrame);
                }
            }
        }
        str = &str[_reconstructImage.width()];
        strD = &strD[reconstructFrame.depthMap.width()];
    }
}

void Reconstructor3D::_createReconstructEntity(const std::vector<QVector3D>& vertices, const std::vector<QVector3D>& normals,
                                               const std::vector<GLuint>& triangles)
{
    using namespace QScrollEngine;
    std::vector<QMesh*> meshes;
    meshes.resize(_reconstructFrames.size());
    unsigned int i;
    _entity->deleteParts();
    for (i=0; i<_reconstructFrames.size(); ++i) {
        QMesh* mesh = new QMesh(_context);
        mesh->setSizeOfELement(3);
        mesh->setEnable_vertex_normal(true);
        Image<Rgba> colorImage = _reconstructFrames[i].colorImage.convert<Rgba>(
                    [] (const Rgba& a){ return Rgba(a.blue, a.green, a.red, a.alpha); } );
        QImage qtImage(reinterpret_cast<uchar*>(colorImage.data()), colorImage.width(), colorImage.height(), QImage::Format_RGB32);
        QOpenGLTexture* texture = new QOpenGLTexture(qtImage);
        QSh_Texture1* shader = new QSh_Texture1(texture);
        _entity->addPart(mesh, shader);
        meshes[i] = mesh;
    }
    std::vector<std::vector<GLuint>> mAssignedVertices;
    std::vector<GLuint>::iterator itVertex;
    mAssignedVertices.resize(_reconstructFrames.size());
    QVector3D normal, point;
    const GLuint* sourceTriangle;
    GLuint triangle[3];
    unsigned int indexFrame, indexVertex;
    float visible, maxVisible;
    IPointF projection;
    unsigned int countTriangles = triangles.size() / 3;
    for (unsigned int k = 0; k < countTriangles; ++k) {
        sourceTriangle = &triangles.at(k * 3);
        const QVector3D& v1 = vertices.at(sourceTriangle[0]);
        const QVector3D& v2 = vertices.at(sourceTriangle[1]);
        const QVector3D& v3 = vertices.at(sourceTriangle[2]);
        normal = QVector3D::crossProduct(v1 - v2, v3 - v2);
        normal.normalize();
        indexFrame = UINT_MAX;
        maxVisible = 10.0f;
        for (i=0; i<_reconstructFrames.size(); ++i) {
            ReconstructFrame& frame = _reconstructFrames[i];
            visible = frame.matrixWorld(2, 0) * normal.x() + frame.matrixWorld(2, 1) * normal.y() + frame.matrixWorld(2, 2) * normal.z();
            if (visible < maxVisible) {
                point = frame.matrixWorld * v1;
                point.setZ(- point.z());
                if (point.z() < 0.0005f)
                    continue;
                projection = _pose3DEstimator.projectPoint(IPointF(point.x() / point.z(), point.y() / point.z()));
                if (!_pose3DEstimator.pointInWindow(projection))
                    continue;
                point = frame.matrixWorld * v2;
                point.setZ(- point.z());
                if (point.z() < 0.0005f)
                    continue;
                projection = _pose3DEstimator.projectPoint(IPointF(point.x() / point.z(), point.y() / point.z()));
                if (!_pose3DEstimator.pointInWindow(projection))
                    continue;
                point = frame.matrixWorld * v3;
                point.setZ(- point.z());
                if (point.z() < 0.0005f)
                    continue;
                projection = _pose3DEstimator.projectPoint(IPointF(point.x() / point.z(), point.y() / point.z()));
                if (!_pose3DEstimator.pointInWindow(projection))
                    continue;
                maxVisible = visible;
                indexFrame = i;
            }
        }
        if (maxVisible > 0.0f)
            continue;
        //if (indexFrame == UINT_MAX)
        //    continue;
        for (i=0; i<3; ++i) {
            itVertex = std::find(mAssignedVertices.at(indexFrame).begin(), mAssignedVertices.at(indexFrame).end(), sourceTriangle[i]);
            if (itVertex == mAssignedVertices.at(indexFrame).end()) {
                triangle[i] = mAssignedVertices.at(indexFrame).size();
                mAssignedVertices.at(indexFrame).push_back(sourceTriangle[i]);
            } else {
                triangle[i] = (itVertex - mAssignedVertices.at(indexFrame).begin());
            }
        }
        meshes[indexFrame]->addTriangle(triangle[0], triangle[1], triangle[2]);
    }
    unsigned int j;
    for (i=0; i<mAssignedVertices.size(); ++i) {
        ReconstructFrame& frame = _reconstructFrames.at(i);
        std::vector<GLuint>& assignedVertices = mAssignedVertices.at(i);
        QMesh* mesh = meshes[i];
        mesh->setCountVertices(assignedVertices.size());
        for (j=0; j<mAssignedVertices[i].size(); ++j) {
            indexVertex = assignedVertices[j];
            mesh->setVertexPosition(j, vertices[indexVertex]);
            point = frame.matrixWorld * vertices[indexVertex];
            point.setZ(- point.z());
            projection = _pose3DEstimator.projectPoint(IPointF(point.x() / point.z(), point.y() / point.z()));
            projection.x /= static_cast<float>(frame.colorImage.width());
            projection.y /= static_cast<float>(frame.colorImage.height());
            mesh->setVertexTextureCoord(j, QVector2D(projection.x, projection.y));
            mesh->setVertexNormal(j, normals[indexVertex]);
        }
        mesh->applyChanges();
        mesh->updateLocalBoundingBox();
    }
    /*for (i=0; i<_entity->countParts(); ++i) {
        qDebug() << _entity->part(i)->mesh()->countVertices() << _entity->part(i)->mesh()->countTriangles();
    }*/
}

void Reconstructor3D::run()
{
    assert(_map != nullptr);
    assert(_context != nullptr);
    assert(_entity != nullptr);
    emit updateProgress(0.0f);
    _reconstructFrames.resize(0);
    Map& map = (*_map);
    qDebug() << '\n' << "Reconstruction: Start 3d reconstruction (count frames - " + QString::number(map.countRecordFrames()) + ")";
    if (map.countRecordFrames() < 3) {
        qDebug() << "Reconstruction: Fail. Need to more frames." << '\n';
        emit updateProgress(1.0f);
        return;
    }
    _reconstructField.setPixelFocalLength(IPointF(_focalLength.x * _parameters.sizeDepthMap.x,
                                                  _focalLength.y * _parameters.sizeDepthMap.y));
    _reconstructField.setPixelOpticalCenter(IPointF(_opticalCenter.x * _parameters.sizeDepthMap.x,
                                                    _opticalCenter.y * _parameters.sizeDepthMap.y));
    Map::RecordFrame& firstRecordFrame = map.recordFrame(0);
    _pose3DEstimator.setCameraParameters(firstRecordFrame.image.size(), _focalLength, _opticalCenter, false, false);
    if (_opticalFlow.size() != firstRecordFrame.image.size())
        _opticalFlow.resize(firstRecordFrame.image.size());
    _diffPyramid = _opticalFlow.allocDiffPyramid();
    _opticalFlow.setRegionForCorners(_parameters.sizeBorder, _opticalFlow.size() - _parameters.sizeBorder);
    _reconstructImage.resize(_reconstructField.depthMapSize());
    _createCurrentReconstructFrame(firstRecordFrame);
    float firstStepProcent = 0.9f;
    emit updateProgress(((1) / static_cast<float>(map.countRecordFrames())) * firstStepProcent);
    int currentCountFrames = 1;
    int startRecordFrame = 0;
    int i;
    for (i=1; i<map.countRecordFrames(); ++i) {
        Map::RecordFrame& recordFrame = map.recordFrame(i);
        if (recordFrame.image.size() != firstRecordFrame.image.size()) {
            qDebug() << "Reconstruction: Error of reconstruction. Size image is changed.";
            break;
        }
        _solveCurrentRecordFrame(recordFrame);
        qDebug() << "Reconstruction: Frame " + QString::number(i + 1) + " is computed (" +
                    QString::number(((i + 1) / static_cast<float>(map.countRecordFrames())) * 100.0f) + "%).";
        emit updateProgress(((i + 1) / static_cast<float>(map.countRecordFrames())) * firstStepProcent);
        ++currentCountFrames;
        if (currentCountFrames >= _parameters.minCountFrames) {
            if ((currentCountFrames >= _parameters.maxCountFrames) ||
                   (Map::cutOffProjection(map.recordFrame(startRecordFrame).matrixWorld, map.recordFrame(i).matrixWorld) <
                            _parameters.cutOff_nextFrame) ||
                   (Map::distanceSquared(map.recordFrame(startRecordFrame).matrixWorld, map.recordFrame(i).matrixWorld) >
                            _parameters.distanceSquare_nextFrame)) {
                _finishSolveReconstructFrame(_reconstructFrames[_reconstructFrames.size() - 1]);
                ++i;
                if (i >= map.countRecordFrames()) {
                    currentCountFrames = 0;
                    break;
                }
                Map::RecordFrame& recordFrame = map.recordFrame(i);
                _createCurrentReconstructFrame(recordFrame);
                startRecordFrame = i;
                currentCountFrames = 1;
            }
        }
    }
    if (currentCountFrames > 0) {
        if (currentCountFrames > 1)
            _finishSolveReconstructFrame(_reconstructFrames[_reconstructFrames.size() - 1]);
        else
            _reconstructFrames.pop_back();
    }
    emit updateProgress(firstStepProcent);
    for (unsigned int j=0; j<_reconstructFrames.size(); ++j) {
        _depthMapProcessor.setDepthMap(_reconstructFrames[j].depthMap);
        _depthMapProcessor.process();
    }
    _isoSurface.setRegion(_parameters.startPointOfReconstructRegion, _parameters.endPointOfReconstructRegion);
    _isoSurface.setCellSize(_parameters.cellSizeOfReconstruct);
    _isoSurface.setTValue(0.0f);
    _isoSurface.setEpsilon(0.002f);
    _isoSurface.setScalarField(&_reconstructField);
    _reconstructField.setReconstructFrames(&_reconstructFrames);
    _isoSurface.isoApproximate(_vertices, _normals, _triangles);
    emit updateProgress(0.95f);
}

void Reconstructor3D::createFinishEntity()
{
    qDebug() << "Reconstruction: Create finish entity.";
    _createReconstructEntity(_vertices, _normals, _triangles);
    emit updateProgress(1.0f);
    qDebug() << "Reconstruction: Finish entity is created.";
}

void Reconstructor3D::_pointToFrames(const ReconstructPoint& point, ReconstructFrame& frame)
{
    QVector3D worldPoint;
    if (point.projections.size() < 2)
        return;
    if (!_pose3DEstimator.getPointInWorld(worldPoint, point.projections))
        return;
    for (std::vector<Pose3DEstimator::ProjectionPoint>::const_iterator it = point.projections.begin(); it != point.projections.end(); ++it) {
        if (_pose3DEstimator.getErrorSquared(it->matrixWorld, worldPoint, it->projection) > _parameters.limitMaxErrorSquared) {
            return;
        }
    }
    IPointF mapCoord;
    IPoint mapCoord2;
    float localZ;
    if (_reconstructField.getLocalPointInDepthMap(mapCoord, localZ, frame.matrixWorld, worldPoint)) {
        mapCoord2.set(qFloor(mapCoord.x), qFloor(mapCoord.y));
        float prevz = frame.depthMap(mapCoord2);
        if ((prevz == 0.0f) || (prevz < localZ))
            frame.depthMap(mapCoord2) = localZ;
    }
}

Image<Rgb> Reconstructor3D::_depthMapToImageRgb(const Image<float>& depthMap, float nearDistance, float farDistance) const
{
    Image<Rgb> out(depthMap.size());
    float delta = farDistance - nearDistance;
    int size = out.area();
    const float* dataIn = depthMap.data();
    Rgb* dataOut = out.data();
    for (int i=0; i<size; ++i) {
        uchar val = 255 - static_cast<uchar>(std::min(std::max((dataIn[i] - nearDistance) / delta, 0.0f), 1.0f) * 255.0f);
        dataOut[i].set(val, val, val);
    }
    return out;
}

Image<Rgb> Reconstructor3D::_bwToRgb(const Image<uchar>& image) const
{
    Image<Rgb> result(image.size());
    int size = image.area();
    for (int i=0; i<size; ++i) {
        uchar val = image.data()[i];
        result.data()[i].set(val, val, val);
    }
    return result;
}

}
