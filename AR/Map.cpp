#include "AR/Map.h"
#include "AR/TMath.h"
#include "AR/HomographyInitializer.h"
#include "AR/ARSystem.h"
#include "QScrollEngine/QCamera3D.h"
#include <utility>
#include <algorithm>

namespace AR {

Map::Map()
{
    _cutOffProjections_mapPoints = qCos(5.0f * (M_PI / 180.0f));
    _limitError = 0.01f;
    resetAll();
    QScrollEngine::QCamera3D camera;
    //setMatrixProj(camera.matrixProj());
    _activeKeyFrame = -1;
}

void Map::deleteAllMapPointsAndSaveProjections()
{
    for (unsigned int i=0; i<_mapPoints.size(); ++i)
        emit signalOfDeletingMapPoint(i);
    _mapPoints.clear();
    for (std::vector<KeyFrame>::iterator it = _keyFrames.begin(); it != _keyFrames.end(); ++it) {
        it->indicesMapPoints.clear();
    }
}

void Map::setActiveKeyFrame(int indexKeyFrame)
{
    unsigned int i;
    if (_activeKeyFrame >= 0) {
        KeyFrame& keyFrame = _keyFrames[_activeKeyFrame];
        for (i=0; i<keyFrame.indicesMapPoints.size(); ++i)
            _mapPoints[keyFrame.indicesMapPoints[i]].isActived = false;
    }
    _activeKeyFrame = indexKeyFrame;
    if (_activeKeyFrame < 0)
        return;
    KeyFrame& keyFrame = _keyFrames[_activeKeyFrame];
    for (i=0; i<keyFrame.indicesMapPoints.size(); ++i)
        _mapPoints[keyFrame.indicesMapPoints[i]].isActived = true;
}

void Map::transformMap(const QMatrix4x4& transform)
{
    bool success;
    QMatrix4x4 invertedTransform = transform.inverted(&success);
    TMath_assert(success);
    unsigned int i;
    for (i=0; i<_mapPoints.size(); ++i) {
        MapPoint& mapPoint = _mapPoints[i];
        if (!mapPoint.isDeleted) {
            mapPoint.worldPoint = transform * mapPoint.worldPoint;
            mapPoint.lastDir = transform.mapVector(mapPoint.lastDir);
            emit signalOfChangePositionMapPoint(i);
        }
    }
    for (i=0; i<_keyFrames.size(); ++i) {
        KeyFrame& keyFrame = _keyFrames[i];
        keyFrame.matrixWorld *= invertedTransform;
    }
}

void Map::scaleMap(float scale)
{
    unsigned int i;
    for (i=0; i<_mapPoints.size(); ++i) {
        MapPoint& mapPoint = _mapPoints[i];
        if (!mapPoint.isDeleted) {
            mapPoint.worldPoint *= scale;
            emit signalOfChangePositionMapPoint(i);
        }
    }
    for (i=0; i<_keyFrames.size(); ++i) {
        KeyFrame& keyFrame = _keyFrames[i];
        keyFrame.matrixWorld(0, 3) *= scale;
        keyFrame.matrixWorld(1, 3) *= scale;
        keyFrame.matrixWorld(2, 3) *= scale;
    }
}

bool Map::existNearestKeyFrame() const
{
    if (_keyFrames.empty())
        return false;
    unsigned int size = _keyFrames.size() - 1;
    const KeyFrame& last = _keyFrames[size];
    for (unsigned int i=0; i<size; ++i) {
        const KeyFrame& keyFrame = _keyFrames[i];
        if ((distanceSquared(last.matrixWorld, keyFrame.matrixWorld) < _distanceSquared_nextFrame) &&
                cutOffProjection(last.matrixWorld, keyFrame.matrixWorld) > _cutOff_nextFrame)
            return true;
    }
    return false;
}

}
