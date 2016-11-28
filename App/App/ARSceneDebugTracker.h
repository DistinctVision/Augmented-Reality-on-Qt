#ifndef ARSCENETRACKERDEBUG_H
#define ARSCENETRACKERDEBUG_H

#include "ARScene.h"
#include <QMutex>
#include "ARTracker.h"
#include "AR/Point2.h"
#include <QMatrix3x3>

class ARSceneDebugTracker: public ARScene
{
    Q_OBJECT

public slots:
    void drawTrackedMatches();

protected:
    void initScene() override;
    void _endUpdate() override;

private:
    QScrollEngine::QEntity* m_cameraSample;
    QScrollEngine::QSprite* m_mapPointSample;

    QMutex m_mutex;
    std::vector<std::pair<AR::Point2f, AR::Point2f>> m_matches;
    AR::Point2i m_imageSize;
    QMatrix3x3 m_textureMatrix;

    void _updateMap();
};

#endif // ARSCENETRACKERDEBUG_H
