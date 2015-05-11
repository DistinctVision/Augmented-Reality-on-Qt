#ifndef SCENEPIGPETR_H
#define SCENEPIGPETR_H

#include <QList>
#include <QVector3D>
#include "QScrollEngine/QScene.h"
#include "App/ARCameraQml.h"
#include "QScrollEngine/QMesh.h"

class ScenePigPetr:
        public SceneInterface
{
    Q_OBJECT
public:
    ScenePigPetr(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~ScenePigPetr();

    void beginUpdate() override;
    void endUpdate() override;

public slots:
    void slotBeginDrawing();
    void slotEndDrawing();

private:
    QScrollEngine::QEntity* _all;
    QScrollEngine::QEntity* _parent;
    QScrollEngine::QEntity* _pathViewQuad;
    QScrollEngine::QEntity* /*fucked*/_tracktor;
    QScrollEngine::QEntity* _path[2];
    QScrollEngine::QEntity* _elka;
    QScrollEngine::QEntity* _tree;
    QList<QScrollEngine::QEntity*> _elkas;
    QList<QScrollEngine::QEntity*> _trees;
    QScrollEngine::QBoundingBox _bbNew;
    QScrollEngine::QBoundingBox _bbView;
    float _deadLineX;

    void _updateTrees();
    void _setAlphaOfEntity(QScrollEngine::QEntity* entity, float alpha);
};

#endif // SCENEWATER_H
