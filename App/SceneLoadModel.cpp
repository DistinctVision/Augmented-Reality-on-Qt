#include "App/SceneLoadModel.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <random>
#include <cfloat>
#include <cmath>
#include <QDebug>
#include "QScrollEngine/QEntity.h"

SceneLoadModel::SceneLoadModel(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    setAmbientColor(70, 70, 70);
    context->glLineWidth(3.0f);
    QLight* light = new QLight(this);
    light->setPosition(4.0f, 0.0f, 10.0f);
    light->setRadius(20.0f);
    _planarShadows.setScene(this);
    _planarShadows.setLight(light);
    _planarShadows.setPlanePos(QVector3D(0.0f, 0.0f, 0.0f));
    _planarShadows.setPlaneDir(QVector3D(0.0f, 0.0f, 1.0f));
    _planarShadows.setAlpha(true);
    _planarShadows.setColorShadows(QColor(0, 0, 0, 100));
}

SceneLoadModel::~SceneLoadModel()
{
}

QString SceneLoadModel::_findDirInPath(const QString& path) const
{
    int index = path.indexOf("/", 0), i;
    for (;;) {
        i = path.indexOf("/", index + 1);
        if (i < 0)
            break;
        index = i;
    }
    if (index < 0)
        return "";
    return path.left(index + 1);
}

void SceneLoadModel::loadModel(const QString& path)
{
    using namespace QScrollEngine;
    parentContext()->deleteAllTextures();
    while (this->countEntities() > 0)
        delete this->entity(0);
    QEntity* entity = parentContext()->loadEntity(path, _findDirInPath(path));
    if (entity == nullptr) {
        qWarning("Failed to load entity.");
        return;
    }
    entity->setParentScene(this);
    entity->updateTransform();
    _planarShadows.addEntity(entity);
    QBoundingBox bb = entity->boundingBox();
    QVector3D delta = bb.max() - bb.min();
    float scale = qMin(qMin(3.0f / delta.x(), 3.0f / delta.y()), 3.0f / delta.z());
    entity->setPosition((entity->position() - bb.getCenter()) * scale);
    entity->setScale(entity->scale() * scale);
    entity->updateTransform();
    bb = entity->boundingBox();
    entity->setPosition(entity->position() - QVector3D(0.0f, 0.0f, bb.min().z()));
}
