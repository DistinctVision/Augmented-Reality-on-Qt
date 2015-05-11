#ifndef SCENEGRID_H
#define SCENEGRID_H

#include <vector>
#include <QVector3D>
#include "QScrollEngine/QScene.h"
#include "App/ARCameraQml.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "QScrollEngine/QMesh.h"
#include <vector>
#include "QScrollEngine/Shaders/QSh.h"


class SceneGrid:
        public SceneInterface
{
    Q_OBJECT
public:
    SceneGrid(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~SceneGrid();

    void beginUpdate() override { }
    void endUpdate() override { }

private:
    QScrollEngine::QEntity* _createHexGrid(float cellSize, int countX, int countY);
    QScrollEngine::QMesh* _createArrow(float length, float lengthArrow);
};

#endif // SCENEWATER_H
