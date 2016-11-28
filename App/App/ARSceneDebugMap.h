#ifndef ARSCENEDEBUGMAP_H
#define ARSCENEDEBUGMAP_H

#include "ARScene.h"
#include <QMutex>
#include "ARTracker.h"
#include "AR/MapResourcesManager.h"
#include "AR/Map.h"
#include <vector>
#include <utility>
#include "AR/Point2.h"
#include <QMatrix3x3>
#include "QScrollEngine/QEntity.h"

class ARSceneDebugMap: public ARScene, public AR::Map::MapListener
{
    Q_OBJECT

public:
    void onCreateKeyFrame(const std::shared_ptr<AR::KeyFrame> & keyFrame) override;
    void onDeleteKeyFrame(const std::shared_ptr<AR::KeyFrame> & keyFrame) override;

    void onCreateMapPoint(const std::shared_ptr<AR::MapPoint> & mapPoint) override;
    void onDeleteMapPoint(const std::shared_ptr<AR::MapPoint> & mapPoint) override;

    void onTransformMap(const TMath::TMatrixd & rotation, const TMath::TVectord & translation) override;
    void onScaleMap(double scale) override;
    void onResetMap() override;

protected:
    void initScene() override;
    void _endUpdate() override;

private:
    struct ResourcesOfMap {
        std::vector<std::pair<std::shared_ptr<AR::KeyFrame>, QScrollEngine::QEntity*>> keyFrames;
        std::vector<std::pair<std::shared_ptr<AR::MapPoint>, QScrollEngine::QSprite*>> mapPoints;
    };

    std::vector<std::shared_ptr<AR::KeyFrame>> m_createdKeyFrames;
    std::vector<std::shared_ptr<AR::KeyFrame>> m_deletedKeyFrames;
    std::vector<std::shared_ptr<AR::MapPoint>> m_createdMapPoints;
    std::vector<std::shared_ptr<AR::MapPoint>> m_deletedMapPoints;

    std::shared_ptr<ResourcesOfMap> m_resourceOfMap;
    QScrollEngine::QEntity * m_cameraSample;
    QScrollEngine::QSprite * m_mapPointSample;

    AR::MapResourcesManager m_mapResourceManager;

    QMutex m_mutex;
    std::vector<QScrollEngine::QSprite*> m_activedMapPoints;

    void _updateMap();
};

#endif // ARSCENEDEBUGMAP_H
