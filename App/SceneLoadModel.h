#ifndef SCENELOADMODEL_H
#define SCENELOADMODEL_H

#include <vector>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <vector>
#include "App/ARCameraQml.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Tools/PlanarShadows.h"


class SceneLoadModel:
        public SceneInterface
{
    Q_OBJECT
public:
    SceneLoadModel(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~SceneLoadModel();

    void beginUpdate() override { }
    void endUpdate() override { }

    void loadModel(const QString& path);

private:
    QScrollEngine::QEntity* _model;
    QScrollEngine::PlanarShadows _planarShadows;

    QString _findDirInPath(const QString& path) const;
};

#endif // SCENEWATER_H
