#ifndef PLANARSHADOWS_H
#define PLANARSHADOWS_H

#include "QScrollEngine/QScrollEngine.h"
#include <cassert>
#include <set>
#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QCamera3D.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/Shaders/QSh_Color.h"
#include <QOpenGLShaderProgram>

namespace QScrollEngine {

class QPlanarShadows:
        public QObject
{
    Q_OBJECT
public:
    QPlanarShadows(QObject* parent = 0);
    ~QPlanarShadows();

    void addEntity(QEntity* entity);
    void deleteEntity(QEntity* entity);
    bool isExist(QEntity* entity) const { return (m_entities.find(entity) != m_entities.end()); }
    int countEntities() const { return m_entities.size(); }
    QLight* light() { return m_light; }
    void setLight(QLight* light);
    QScene* scene() { return m_scene; }
    void setScene(QScene* scene);
    QVector3D planePos() const { return m_planePos; }
    void setPlanePos(const QVector3D& planePos) { m_planePos = planePos; }
    QVector3D planeNormal() const { return m_planeNormal; }
    void setPlaneDir(const QVector3D& dir) { m_planeNormal = dir.normalized(); }
    QColor colorShadow() const { return m_colorShadows; }
    void setColorShadows(const QColor& color) { m_colorShadows = color; }
    bool isAlpha() const { return m_isAlpha; }
    void setAlpha(bool enable) { m_isAlpha = enable; }
    void clear();

private slots:
    void draw();
    void deletingObject(QSceneObject3D* object);
    void deletingLight();

private:
    bool m_isAlpha;
    QScene* m_scene;
    QLight* m_light;
    QScrollEngineContext* m_parentContext;
    std::set<QEntity*> m_entities;
    QVector3D m_planePos;
    QVector3D m_planeNormal;
    QMatrix4x4 m_matrixShadow;
    QColor m_colorShadows;
    std::vector<QSh::VertexAttributes> m_attributes;

    bool _checkParentVisibled(QEntity* entity) const;
    void _calculateMatrixShadow();
    void _drawShadowOfEntity(QOpenGLShaderProgram* program, QCamera3D* camera, QEntity* entity);
};

}

#endif // PLANASSHADOWS_H
