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

class PlanarShadows : public QObject
{
    Q_OBJECT
public:
    PlanarShadows();

    void addEntity(QEntity* entity);
    void deleteEntity(QEntity* entity);
    bool isExist(QEntity* entity) const { return (_entities.find(entity) != _entities.end()); }
    int countEntities() const { return _entities.size(); }
    QLight* light() { return _light; }
    void setLight(QLight* light);
    QScene* scene() { return _scene; }
    void setScene(QScene* scene);
    QVector3D planePos() const { return _planePos; }
    void setPlanePos(const QVector3D& planePos) { _planePos = planePos; }
    QVector3D planeNormal() const { return _planeNormal; }
    void setPlaneDir(const QVector3D& dir) { _planeNormal = dir.normalized(); }
    QColor colorShadow() const { return _colorShadows; }
    void setColorShadows(const QColor& color) { _colorShadows = color; }
    bool isAlpha() const { return _isAlpha; }
    void setAlpha(bool enable) { _isAlpha = enable; }
    void clear();

private slots:
    void draw();
    void deletingObject();
    void deletingLight();

private:
    bool _isAlpha;
    QScene* _scene;
    QLight* _light;
    QScrollEngineContext* _parentContext;
    std::set<QEntity*> _entities;
    QVector3D _planePos;
    QVector3D _planeNormal;
    QMatrix4x4 _matrixShadow;
    QColor _colorShadows;

    void _calculateMatrixShadow();
    void _drawShadowOfEntity(QOpenGLShaderProgram* program, QCamera3D* camera, QEntity* entity);
};

}

#endif // PLANASSHADOWS_H
