#ifndef MAP_H
#define MAP_H

#include <vector>
#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>

#include "AR/Image.h"
#include "AR/OF_ImageLevel.h"
#include "AR/TMath.h"

#include "QScrollEngine/QFrustum.h"

namespace AR {

class ARSystem;
class HomographyInitializer;

class Map: public QObject
{
    Q_OBJECT

    friend class ARSystem;
    friend class HomographyInitializer;

public:
    typedef struct KeyFrame
    {
        Image<uchar> image;
        Image<uchar> smallImage;
        std::vector<IPointF> projectedPoints;
        std::vector<int> indicesMapPoints;
        QMatrix4x4 matrixWorld;
    } KeyFrame;

    typedef struct RecordFrame
    {
        Image<Rgba> colorImage;
        Image<uchar> image;
        QMatrix4x4 matrixWorld;
    } RecordFrame;

signals:
    void signalOfNewMapPoint(int index);
    void signalOfDeletingMapPoint(int index);
    void signalOfChangePositionMapPoint(int index);
    void signalOfResetMap();

public:
    void resetAll()
    {
        _mapPoints.clear();
        _keyFrames.clear();
        _recordFrames.clear();
        _deletedMapPoints.clear();
        _activeKeyFrame = -1;
        emit signalOfResetMap();
    }
    void deleteAllMapPointsAndSaveProjections();

    Map();

    int countKeyFrames() const { return static_cast<int>(_keyFrames.size()); }
    KeyFrame& keyFrame(int index) { return _keyFrames[index]; }
    const KeyFrame& keyFrame(int index) const { return _keyFrames[index]; }
    KeyFrame& lastKeyFrame() { return _keyFrames[_keyFrames.size() - 1]; }
    const KeyFrame& lastKeyFrame() const { return _keyFrames[_keyFrames.size() - 1]; }
    void addKeyFrame()
    {
        _keyFrames.resize(_keyFrames.size() + 1);
        KeyFrame& last = lastKeyFrame();
        last.indicesMapPoints.clear();
        last.projectedPoints.clear();
        last.image = Image<uchar>();
        last.smallImage = Image<uchar>();
    }
    void setActiveKeyFrame(int indexKeyFrame);
    int countMapPoints() const { return static_cast<int>(_mapPoints.size()); }
    QVector3D mapPoint(int index) const { return _mapPoints[index].worldPoint; }
    float errorOfMapPoint(int index) const { return _mapPoints[index].getGlobalError(); }
    bool pointIsDeleted(int indexMapPoint) const { return _mapPoints[indexMapPoint].isDeleted; }
    int addMapPoint(const QVector3D& point, Image<OF_ImageLevel::OpticalFlowInfo2D>& OFImage,
                    float sumErrors, int countErrors, const QVector3D& currentDir)
    {
        int index;
        if (_deletedMapPoints.size() > 0) {
            index = _deletedMapPoints[_deletedMapPoints.size() - 1];
            _deletedMapPoints.pop_back();
            _mapPoints[index] = MapPoint(point, OFImage, sumErrors, countErrors, currentDir);
        } else {
            _mapPoints.push_back(MapPoint(point, OFImage, sumErrors, countErrors, currentDir));
            index = _mapPoints.size() - 1;
        }
        emit signalOfNewMapPoint(index);
        return index;
    }
    void addMapPointToKeyFrame(int indexMapPoint, int indexKeyFrame, const IPointF& projection)
    {
        _keyFrames[indexKeyFrame].projectedPoints.push_back(projection);
        _keyFrames[indexKeyFrame].indicesMapPoints.push_back(indexMapPoint);
        _mapPoints[indexMapPoint].indexKeyFrame.push_back(indexKeyFrame);
        _mapPoints[indexMapPoint].isActived = (indexKeyFrame == _activeKeyFrame);
    }
    void deleteMapPoint(int indexMapPoint) {
        MapPoint& mapPoint = _mapPoints[indexMapPoint];
        mapPoint.isDeleted = true;
        for (unsigned int i=0; i<mapPoint.indexKeyFrame.size(); ++i)
            _deleteMapPointFromKeyFrame(indexMapPoint, mapPoint.indexKeyFrame[i]);
        mapPoint.indexKeyFrame.clear();
        _deletedMapPoints.push_back(indexMapPoint);
        emit signalOfDeletingMapPoint(indexMapPoint);
    }
    void addErrorOfMapPoint(int indexMapPoint, float error, const QVector3D& currentDir)
    {
        _mapPoints[indexMapPoint].addError(error, currentDir, _cutOffProjections_mapPoints);
        if (_mapPoints[indexMapPoint].getGlobalError() > _limitError)
            deleteMapPoint(indexMapPoint);
    }

    float cutOffProjections_mapPoints() const { return _cutOffProjections_mapPoints; }
    void setCutOffProjections_mapPoints(float cutOffProjections) { _cutOffProjections_mapPoints = cutOffProjections; }
    void setCutOff_nextFrame(float cutOff_nextFrame) { _cutOff_nextFrame = cutOff_nextFrame; }
    float cutOff_nextFrame() const { return _cutOff_nextFrame; }
    void setDistanceSquared_nextFrame(float distanceSquared) { _distanceSquared_nextFrame = distanceSquared; }
    float distanceSquared_nextFrame() const { return _distanceSquared_nextFrame; }
    void transformMap(const QMatrix4x4& transform);
    void transformMapPoint(int index, const QMatrix4x4& transform)
    {
        _mapPoints[index].worldPoint = transform * _mapPoints[index].worldPoint;
        _mapPoints[index].lastDir = transform.mapVector(_mapPoints[index].lastDir);
        emit signalOfChangePositionMapPoint(index);
    }
    void scaleMap(float scale);
    float limitErrror() const { return _limitError; }
    void setLimitError(float limit) { _limitError = limit; }
    bool existNearestKeyFrame() const;
    static float cutOffProjection(const QMatrix4x4& matrixWorldA, const QMatrix4x4& matrixWorldB)
    {
        return qMin(matrixWorldA(2, 0) * matrixWorldB(2, 0) +
                    matrixWorldA(2, 1) * matrixWorldB(2, 1) +
                    matrixWorldA(2, 2) * matrixWorldB(2, 2),
                    matrixWorldA(0, 0) * matrixWorldB(0, 0) +
                    matrixWorldA(0, 1) * matrixWorldB(0, 1) +
                    matrixWorldA(0, 2) * matrixWorldB(0, 2));
    }
    static float distanceSquared(const QMatrix4x4& matrixWorldA, const QMatrix4x4& matrixWorldB)
    {
        QVector3D delta(matrixWorldB(0, 3) - matrixWorldA(0, 3),
                        matrixWorldB(1, 3) - matrixWorldA(1, 3),
                        matrixWorldB(2, 3) - matrixWorldA(2, 3));
        return delta.lengthSquared();
    }
    static float diffImage(const Image<uchar>& imageA, const Image<uchar>& imageB, int size)
    {
        const uchar* dataA = imageA.data();
        const uchar* dataB = imageB.data();
        float diffSquared = 0.0f;
        for (int i=0; i<size; ++i) {
            float diff = dataB[i] - dataA[i];
            diffSquared += diff * diff;
        }
        return diffSquared;
    }
    int countRecordFrames() const { return static_cast<int>(_recordFrames.size()); }
    RecordFrame& recordFrame(int index) { return _recordFrames[index]; }
    const RecordFrame& recordFrame(int index) const { return _recordFrames[index]; }
    RecordFrame& lastRecordFrame() { return _recordFrames[_recordFrames.size() - 1]; }
    const RecordFrame& lastRecordFrame() const { return _recordFrames[_recordFrames.size() - 1]; }
    void addRecordFrame() { _recordFrames.resize(_recordFrames.size() + 1); }

private:
    typedef struct MapPoint
    {
        QVector3D worldPoint;
        float sumGlobalErrors;
        int countGlobalErrors;
        float sumCurrentErrors;
        int countCurrentErrors;
        QVector3D lastDir;
        bool isDeleted;
        bool isActived;
        std::vector<int> indexKeyFrame;
        Image<OF_ImageLevel::OpticalFlowInfo2D> OFImage;

        MapPoint() { isDeleted = false; isActived = false; }
        MapPoint(const QVector3D& point, Image<OF_ImageLevel::OpticalFlowInfo2D>& OFImageFromPoint,
                 float sumErrors, int countErrors, const QVector3D& currentDir)
        {
            TMath_assert(countErrors >= 0);
            worldPoint = point;
            OFImage = OFImageFromPoint;
            sumGlobalErrors = 0.0f;
            countGlobalErrors = 0;
            sumCurrentErrors = sumErrors;
            countCurrentErrors = countErrors;
            lastDir = currentDir;
            isDeleted = false;
            isActived = false;
        }
        float getGlobalError() const {
            return ((sumGlobalErrors + sumCurrentErrors / (float)countCurrentErrors) / (float)(countGlobalErrors + 1)); }
        void addError(float error, const QVector3D& currentDir, float cutOffProjections)
        {
            if ((sumCurrentErrors > 1000000.0f) || (QVector3D::dotProduct(lastDir, currentDir) < cutOffProjections)) {
                sumGlobalErrors += sumCurrentErrors / (float)countCurrentErrors;
                ++countGlobalErrors;
                sumCurrentErrors = error;
                countCurrentErrors = 1;
                lastDir = currentDir;
            } else {
                sumCurrentErrors += error;
                ++countCurrentErrors;
            }
        }
    } MapPoint;

    std::vector<MapPoint> _mapPoints;
    std::vector<KeyFrame> _keyFrames;
    std::vector<int> _deletedMapPoints;
    std::vector<RecordFrame> _recordFrames;

    int _activeKeyFrame;
    float _cutOffProjections_mapPoints;
    float _limitError;
    float _cutOff_nextFrame;
    float _distanceSquared_nextFrame;

    void _deleteMapPointFromKeyFrame(int indexMapPoint, int indexKeyFrame)
    {
        KeyFrame& keyFrame = _keyFrames[indexKeyFrame];
        for (unsigned int i=0; i<keyFrame.indicesMapPoints.size(); ++i) {
            if (keyFrame.indicesMapPoints[i] == indexMapPoint) {
                keyFrame.indicesMapPoints.erase(keyFrame.indicesMapPoints.begin() + i);
                keyFrame.projectedPoints.erase(keyFrame.projectedPoints.begin() + i);
                return;
            }
        }
    }

};

}

#endif // MAP_H
