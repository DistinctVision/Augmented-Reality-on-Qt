#ifndef ARSCENECUBES_H
#define ARSCENECUBES_H

#include <QMutex>
#include <QList>
#include <QColor>
#include "ARScene.h"
#include "qu3e/q3.h"
#include "QScrollEngine/QScrollEngine.h"

class ARSceneCubes:
        public ARScene
{
    Q_OBJECT

public:
    ARSceneCubes();

    void onPressed(int id, float x, float y) override;
    void onReleased(int id) override;

protected:
    void initScene() override;

    void _beginUpdate();
    void _endUpdate();

private slots:
    void _clear();
    void _drawSelected();

private:
    QMutex m_mutex;
    QScrollEngine::QPlanarShadows m_planarShadows;
    q3Scene* m_physicsScene;
    QList<QPair<q3Body*, QScrollEngine::QEntity*>> m_physicsEntities;
    QScrollEngine::QEntity* m_grid;
    QScrollEngine::QEntity* m_main;
    QScrollEngine::QEntity* m_cameraPivot;
    QColor m_colorPressed;
    QColor m_colorSelected;
    bool m_pressed;
    QScrollEngine::QEntity* m_selectedEntity;
    q3Body* m_selectedBody;
    QVector3D m_selectedLocalPosition;
    QScrollEngine::QSh_Color m_shaderForSelected;
    float m_state;
    float m_speedOfChangeState;

    void _updatePhysics();
    void _updateSelected();
    void _drawEntity(const QScrollEngine::QEntity* entity);

};

#endif // ARSCENECUBES_H
