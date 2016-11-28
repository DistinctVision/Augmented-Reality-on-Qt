#include "ARSceneDebugMap.h"
#include "ARTracker.h"
#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include <QMutexLocker>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QSharedPointer>
#include <QDebug>
#include "TMath/TTools.h"
#include "AR/MapResourceLocker.h"
#include <algorithm>

void ARSceneDebugMap::initScene()
{
    using namespace QScrollEngine;

    QMutexLocker ml(&m_mutex);
    m_resourceOfMap = std::shared_ptr<ResourcesOfMap>(new ResourcesOfMap());
    m_createdKeyFrames.resize(0);
    m_deletedKeyFrames.resize(0);
    m_createdMapPoints.resize(0);
    m_deletedMapPoints.resize(0);
    connect(scene(), &QScene::deleting, this, [this]() {
        QMutexLocker ml(&m_mutex);
        m_resourceOfMap.reset();
        m_activedMapPoints.clear();
        ARTracker * arTracker = this->arTracker();
        if (arTracker != nullptr)
            arTracker->arSystem().map()->resetListener();
    }, Qt::DirectConnection);
    QMesh * meshCam = new QMesh(scene());
    meshCam->setCountVertices(5);
    meshCam->setVertexPosition(0, QVector3D(0.0f, 0.0f, 0.0f));
    meshCam->setVertexPosition(1, QVector3D(-0.5f, 0.5f, 0.5f));
    meshCam->setVertexPosition(2, QVector3D(0.5f, 0.5f, 0.5f));
    meshCam->setVertexPosition(3, QVector3D(0.5f, -0.5f, 0.5f));
    meshCam->setVertexPosition(4, QVector3D(-0.5f, -0.5f, 0.5f));
    meshCam->setSizeOfELement(2);
    meshCam->setCountLines(8);
    meshCam->setLine(0, 0, 1);
    meshCam->setLine(1, 0, 2);
    meshCam->setLine(2, 0, 3);
    meshCam->setLine(3, 0, 4);
    meshCam->setLine(4, 1, 2);
    meshCam->setLine(5, 2, 3);
    meshCam->setLine(6, 3, 4);
    meshCam->setLine(7, 4, 1);
    meshCam->applyChanges();
    meshCam->updateLocalBoundingBox();
    m_cameraSample = new QEntity(scene());
    QEntity::Part * part = m_cameraSample->addPart(meshCam, QShPtr(new QSh_Color(QColor(0, 155, 255))));
    part->setDrawMode(GL_LINES);
    m_cameraSample->setVisibled(false);
    m_mapPointSample = new QSprite(scene());
    QOpenGLTexture * texture = scene()->parentContext()->texture("MapPoint");
    if (texture == nullptr)
        texture = scene()->parentContext()->loadTexture("MapPoint", ":/Data/Glow.png");
    m_mapPointSample->setShader(QShPtr(new QSh_Texture(texture)));
    m_mapPointSample->setScale(0.1f);
    m_mapPointSample->setAlpha(true);
    m_mapPointSample->setVisibled(false);
}

void ARSceneDebugMap::onCreateKeyFrame(const std::shared_ptr<AR::KeyFrame> & keyFrame)
{
    QMutexLocker ml(&m_mutex);
    m_createdKeyFrames.push_back(keyFrame);
}

void ARSceneDebugMap::onDeleteKeyFrame(const std::shared_ptr<AR::KeyFrame> & keyFrame)
{
    QMutexLocker ml(&m_mutex);
    m_deletedKeyFrames.push_back(keyFrame);
}

void ARSceneDebugMap::onCreateMapPoint(const std::shared_ptr<AR::MapPoint> & mapPoint)
{
    QMutexLocker ml(&m_mutex);
    m_createdMapPoints.push_back(mapPoint);
}

void ARSceneDebugMap::onDeleteMapPoint(const std::shared_ptr<AR::MapPoint> & mapPoint)
{
    QMutexLocker ml(&m_mutex);
    m_deletedMapPoints.push_back(mapPoint);
}

void ARSceneDebugMap::onTransformMap(const TMath::TMatrixd & rotation, const TMath::TVectord & translation)
{
    Q_UNUSED(rotation);
    Q_UNUSED(translation);
    QMutexLocker ml(&m_mutex);
    _updateMap();
}

void ARSceneDebugMap::onScaleMap(double scale)
{
    Q_UNUSED(scale);
    QMutexLocker ml(&m_mutex);
    _updateMap();
}

void ARSceneDebugMap::_updateMap()
{
    using namespace QScrollEngine;
    using namespace AR;
    using namespace TMath;

    if (m_resourceOfMap) {
        QQuaternion q;
        for (auto it = m_resourceOfMap->keyFrames.begin(); it != m_resourceOfMap->keyFrames.end(); ++it) {
            std::shared_ptr<KeyFrame> keyFrame = it->first;
            MapResourceLocker lockerKeyFrame(&m_mapResourceManager, keyFrame.get()); (void)lockerKeyFrame;
            QOtherMathFunctions::matrixToQuaternion(TTools::convert<QMatrix3x3>(keyFrame->rotation()).transposed(), q);
            it->second->setOrientation(q);
            it->second->setPosition(TTools::convert<QVector3D>(keyFrame->worldPosition()));
            Point2d imageSize = keyFrame->camera()->imageSize();
            it->second->setScale(1.0f, (float)(imageSize.y / imageSize.x), 1.0f);
        }
        for (auto it = m_resourceOfMap->mapPoints.begin(); it != m_resourceOfMap->mapPoints.end(); ++it) {
            std::shared_ptr<MapPoint> mapPoint = it->first;
            m_mapResourceManager.lock(mapPoint.get());
            it->second->setPosition(TTools::convert<QVector3D>(mapPoint->position()));
            m_mapResourceManager.unlock(mapPoint.get());
        }
    }
    m_resourceOfMap = std::shared_ptr<ResourcesOfMap>(new ResourcesOfMap());
}

void ARSceneDebugMap::onResetMap()
{
    QMutexLocker ml(&m_mutex);
    m_createdKeyFrames.clear();
    m_createdMapPoints.clear();
    for (auto it = m_resourceOfMap->keyFrames.begin(); it != m_resourceOfMap->keyFrames.end(); ++it)
        m_deletedKeyFrames.push_back(it->first);
    for (auto it = m_resourceOfMap->mapPoints.begin(); it != m_resourceOfMap->mapPoints.end(); ++it)
        m_deletedMapPoints.push_back(it->first);
    m_activedMapPoints.clear();
}

void ARSceneDebugMap::_endUpdate()
{
    using namespace QScrollEngine;
    using namespace AR;
    using namespace TMath;

    QMutexLocker ml(&m_mutex);

    for (auto it = m_activedMapPoints.begin(); it != m_activedMapPoints.end(); ++it) {
        QSh_Color * sh = static_cast<QSh_Color*>((*it)->shader().data());
        sh->setColor(255, 255, 255);
    }
    m_activedMapPoints.resize(0);

    bool enabled = false;
    if (m_resourceOfMap) {
        auto copyOfKeyFrames = m_resourceOfMap->keyFrames; Q_UNUSED(copyOfKeyFrames);
        auto copyOfMapPoints = m_resourceOfMap->mapPoints; Q_UNUSED(copyOfMapPoints);
        ARTracker* arTracker = this->arTracker();
        if (arTracker) {
            if (arTracker->trackingState() == ARTracker::Tracking)
                enabled = true;
            if (arTracker->arSystem().map()->listener() != this) {
                m_createdKeyFrames.clear();
                m_createdMapPoints.clear();
                for (auto it = m_resourceOfMap->keyFrames.begin(); it != m_resourceOfMap->keyFrames.end(); ++it)
                    m_deletedKeyFrames.push_back(it->first);
                for (auto it = m_resourceOfMap->mapPoints.begin(); it != m_resourceOfMap->mapPoints.end(); ++it)
                    m_deletedMapPoints.push_back(it->first);
                m_activedMapPoints.clear();
                Map * map = arTracker->arSystem().map();
                map->setListener(this);
                std::size_t i;
                for (i = 0; i < map->countMapPoints(); ++i) {
                    m_createdMapPoints.push_back(map->mapPoint(i));
                }
                for (i = 0; i < map->countKeyFrames(); ++i) {
                    m_createdKeyFrames.push_back(map->keyFrame(i));
                }
                connect(this, &ARScene::arTrackerChanged, [map, arTracker]() {
                    map->resetListener();
                });
            }
        }
        QQuaternion q;
        for (auto it = m_createdKeyFrames.begin(); it != m_createdKeyFrames.end(); ++it) {
            std::shared_ptr<KeyFrame> keyFrame = *it;
            MapResourceLocker locker(&m_mapResourceManager, keyFrame.get()); (void)locker;
            QEntity * e = m_cameraSample->clone();
            QOtherMathFunctions::matrixToQuaternion(TTools::convert<QMatrix3x3>(keyFrame->rotation()).transposed(), q);
            e->setOrientation(q);
            e->setPosition(TTools::convert<QVector3D>(keyFrame->worldPosition()));
            Point2d imageSize = keyFrame->camera()->imageSize();
            e->setScale(1.0f, (float)(imageSize.y / imageSize.x), 1.0f);
            e->setVisibled(true);
            m_resourceOfMap->keyFrames.push_back({ keyFrame, e });
        }
        m_createdKeyFrames.resize(0);
        for (auto it = m_deletedKeyFrames.begin(); it != m_deletedKeyFrames.end(); ++it) {
            auto it_deleted = m_resourceOfMap->keyFrames.begin();
            while ((it_deleted != m_resourceOfMap->keyFrames.end()) && (it_deleted->first != *it))
                ++it_deleted;
            if (it_deleted != m_resourceOfMap->keyFrames.end()) {
                delete it_deleted->second;
                m_resourceOfMap->keyFrames.erase(it_deleted);
            } else {
                qDebug() << Q_FUNC_INFO << "Unregistred keyFrame!";
            }
        }
        m_deletedKeyFrames.resize(0);
        for (auto it = m_createdMapPoints.begin(); it != m_createdMapPoints.end(); ++it) {
            QSprite * e = m_mapPointSample->copy();
            std::shared_ptr<AR::MapPoint> mapPoint = *it;
            MapResourceLocker locker(&m_mapResourceManager, mapPoint.get()); (void)locker;
            e->setPosition(TTools::convert<QVector3D>(mapPoint->position()));
            e->setVisibled(true);
            m_resourceOfMap->mapPoints.push_back({ mapPoint, e });
        }
        m_createdMapPoints.resize(0);
        for (auto it = m_deletedMapPoints.begin(); it != m_deletedMapPoints.end(); ++it) {
            auto it_deleted = m_resourceOfMap->mapPoints.begin();
            while ((it_deleted != m_resourceOfMap->mapPoints.end()) && (it_deleted->first != *it))
                ++it_deleted;
            if (it_deleted != m_resourceOfMap->mapPoints.end()) {
                delete it_deleted->second;
                m_resourceOfMap->mapPoints.erase(it_deleted);
            } else {
                qDebug() << Q_FUNC_INFO << "Unregistred mapPont!";
            }
        }
        m_deletedMapPoints.resize(0);
        const std::vector<PreviewFrame::PreviewFeature>& features = arTracker->arSystem().currentFeatures();
        for (auto it = features.begin(); it != features.end(); ++it) {
            auto it_s = m_resourceOfMap->mapPoints.begin();
            while ((it_s != m_resourceOfMap->mapPoints.end()) && (it_s->first != it->mapPoint))
                ++it_s;
            if (it_s != m_resourceOfMap->mapPoints.end()) {
                QSprite * s = it_s->second;
                std::shared_ptr<AR::MapPoint> mapPoint = it_s->first;
                MapResourceLocker lockerMapPoint(&m_mapResourceManager, mapPoint.get()); (void)lockerMapPoint;
                s->setPosition(TTools::convert<QVector3D>(mapPoint->position()));
                QSh_Color * sh = static_cast<QSh_Color*>(s->shader().data());
                sh->setColor(255, 150, 100);
                m_activedMapPoints.push_back(s);
            } else {
                qDebug() << Q_FUNC_INFO  << "Unregistred feature!";
            }
        }
    }
    scene()->setEnabled(enabled);
}
