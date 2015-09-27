#include "App/ScenePigPetr.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <random>
#include <cfloat>
#include <cmath>
#include <QDebug>
#include <QTime>
#include "QScrollEngine/QEntity.h"

ScenePigPetr::ScenePigPetr(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    connect(this, SIGNAL(beginDrawing()), this, SLOT(slotBeginDrawing()));
    connect(this, SIGNAL(endDrawing()), this, SLOT(slotEndDrawing()));
    setAmbientColor(100, 100, 100);
    //scale = QVector3D(20.0f, 20.0f, 20.0f);
    _parent = new QScrollEngine::QEntity(this);
    _parent->setScale(0.01f);
    _pathViewQuad = new QEntity(_parent);
    _pathViewQuad->addPart(context, QGLPrimitiv::Quad, new QSh_Color(QColor(255, 255, 255, 255)));
    _pathViewQuad->setScale(500.0f);
    _pathViewQuad->setVisible(false);
    context->loadTexture("EYE.JPG", ":/GameData/eye.jpg");
    _all = context->loadEntity(":/GameData/petr.3DS", "");
    _tracktor = _all->findChild("tracktor");
    _tracktor->setParentEntity(_parent);
    _all->setParentScene(nullptr);
    _path[0] = _all->findChild("Path1");
    _path[0]->part(0)->setVisible(false);
    _path[0]->setParentEntity(_parent);
    _path[0]->updateTransform();
    _path[1] = _all->findChild("Path2");
    _path[1]->part(0)->setVisible(false);
    _path[1]->setParentEntity(_parent);
    _path[1]->updateTransform();
    _elka = _all->findChild("elka");
    _elka->setParentEntity(_parent);
    _elka->setVisible(false);
    _elka->updateTransform();
    _tree = _all->findChild("derevo");
    _tree->setParentEntity(_parent);
    _tree->setVisible(false);
    _tree->updateTransform();
    _parent->updateTransform();
    _pathViewQuad->updateTransform();
    _bbView = _pathViewQuad->boundingBox();
    _bbNew = _path[0]->boundingBox();
    _bbNew.merge(_path[1]->boundingBox());
    QVector3D minNew = _bbNew.min();
    _bbNew.addPoint(QVector3D(_bbNew.max().x() + 3.0f, 3.0f, minNew.z()));
    _bbNew.addPoint(QVector3D(minNew.x(), -3.0f, minNew.z()));
    minNew = _bbNew.min();
    minNew.setX(_bbView.max().x() + 1.0f);
    _bbNew.toPoints(_bbNew.max(), minNew);
    _deadLineX = _bbView.min().x();
    (new QLight(_parent))->setPosition(0.0f, 0.0f, 1000.0f);
    light(0)->setRadius(2000.0f);
    context->setAnimationSpeed(2.0f);
}

ScenePigPetr::~ScenePigPetr()
{
    delete _all;
}

void ScenePigPetr::_setAlphaOfEntity(QScrollEngine::QEntity* entity, float alpha)
{
    using namespace QScrollEngine;
    unsigned int i;
    for (i=0; i<entity->countParts(); ++i) {
        QEntity::QPartEntity* part = entity->part(i);
        QSh_Light* sh = dynamic_cast<QSh_Light*>(part->shader());
        if (sh) {
            QColor color = sh->color();
            sh->setColorF(color.redF(), color.greenF(), color.blueF(), alpha);
            sh->setSpecularIntensity(alpha);
            part->setAlpha(alpha < 1.0f);
        }
    }
    for (i=0; i<entity->countEntityChilds(); ++i) {
        _setAlphaOfEntity(entity->childEntity(i), alpha);
    }
}

void ScenePigPetr::_updateTrees()
{
    qsrand(QTime::currentTime().msec());
    using namespace QScrollEngine;
    for (int i=0; i<_trees.size(); ++i) {
        _trees[i]->updateTransform();
        if (_trees[i]->boundingBox().max().x() < _deadLineX) {
            delete _trees[i];
            _trees.removeAt(i);
            --i;
        }
    }
    _parent->updateTransform();
    _tree->updateTransform();
    _path[0]->updateTransform();
    _path[1]->updateTransform();
    QBoundingBox bbTree = _tree->boundingBox();
    QVector3D sizeTree = bbTree.max() - bbTree.min();
    _elka->updateTransform();
    QBoundingBox bbElka = _elka->boundingBox();
    QVector3D sizeElka = bbElka.max() - bbElka.min();
    QVector3D sizeNew = _bbNew.max() - _bbNew.min();
    QVector2D start, size;
    size.setX(sizeNew.x());
    size.setY(sizeNew.y());
    size.setY(size.y() * 0.6f);
    QVector2D r;
    QVector3D p;
    QEntity* newTree;
    QEntity* path = (_path[0]->globalPosition().x() > _path[1]->globalPosition().x()) ? _path[0] : _path[1];
    while (_trees.size() < 4) {
        if (qrand() % 2) {
            start.setX(_bbNew.min().x() - sizeTree.x() * 0.5f);
            start.setY(_bbNew.min().y());
            newTree = _tree->clone();
        } else {
            start.setX(_bbNew.min().x() - sizeElka.x() * 0.5f);
            start.setY(_bbNew.min().y());
            newTree = _elka->clone();
        }
        r.setX((qrand() % 1000) * 0.001f);
        r.setY((qrand() % 1000) * 0.001f);
        if (r.y() > 0.5f)
            r.setY(r.y() + (0.4f / 0.6f));
        p.setX(start.x() + size.x() * r.x());
        p.setY(start.y() + size.y() * r.y());
        p.setZ(_parent->getTransformPoint(_tree->position()).z());
        newTree->setParentEntity(path);
        newTree->setVisible(true);
        newTree->setPosition(path->getInverseTransformPoint(_parent->getInverseTransformPoint(p)));
        newTree->updateTransform();
        bool collision = false;
        QBoundingBox bb = newTree->boundingBox();
        for (QList<QEntity*>::iterator it = _trees.begin(); it != _trees.end(); ++it) {
            if (bb.collision((*it)->boundingBox())) {
                collision = true;
                break;
            }
        }
        if (collision) {
            delete newTree;
        } else {
            _trees.push_back(newTree);
        }
    }
}

void ScenePigPetr::beginUpdate()
{
    using namespace QScrollEngine;
    _updateTrees();
    QAnimation3D* animation = _path[0]->animation();
    float nextTime = animation->currentTime() + animation->animationSpeed();
    if (nextTime > static_cast<float>(animation->maxTimesKeys())) {
        int pathIndex = (_path[0]->globalPosition().x() > _path[1]->globalPosition().x()) ? 0 : 1;
        _path[pathIndex]->updateTransform();
        QEntity* entity;
        for (unsigned int i=0; i<_path[pathIndex]->countEntityChilds(); ++i) {
            entity = _path[pathIndex]->childEntity(i);
            entity->setPosition(_path[1 - pathIndex]->getInverseTransformPoint(
                        _path[pathIndex]->getTransformPoint(entity->position() + QVector3D(-2000.0f, 0.0f, 0.0f))));
            entity->setParentEntity(_path[1 - pathIndex]);
        }
        _path[0]->swapAnimation(_path[1]);
    }
    float x, alpha, minx = _bbView.min().x(), maxx = _bbView.max().x();
    for (int i=0; i<_trees.size(); ++i) {
        x = _trees[i]->globalPosition().x();
        alpha = qMin(qMin(qMax(x - minx, 0.0f), 0.5f), qMin(qMax(maxx - x, 0.0f), 0.5f)) / 0.5f;
        _setAlphaOfEntity(_trees[i], alpha);
    }
}

void ScenePigPetr::endUpdate()
{
}

void ScenePigPetr::slotBeginDrawing()
{
    using namespace QScrollEngine;
    QScrollEngineContext* context = parentContext();
    context->glEnable(GL_STENCIL_TEST);
    context->glStencilFunc(GL_ALWAYS, 0, 1);
    context->glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    context->glClearStencil(0);
    context->glClear(GL_STENCIL_BUFFER_BIT);
    context->glStencilFunc(GL_ALWAYS, 1, 1);
    context->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    context->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    context->glDepthMask(GL_FALSE);
    _pathViewQuad->updateTransform();
    _pathViewQuad->updateMatrxWorldViewProj(context->camera->matrixViewProj());
    _pathViewQuad->draw(context);
    context->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    context->glStencilFunc(GL_NOTEQUAL, 0, 1);
    context->glEnable(GL_DEPTH_TEST);
    context->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    context->glDepthMask(GL_TRUE);
    _path[0]->updateTransform();
    _path[0]->updateMatrxWorldViewProj(context->camera->matrixViewProj());
    _path[0]->part(0)->draw(context);
    _path[1]->updateTransform();
    _path[1]->updateMatrxWorldViewProj(context->camera->matrixViewProj());
    _path[1]->part(0)->draw(context);
    context->glDisable(GL_STENCIL_TEST);
}

void ScenePigPetr::slotEndDrawing()
{

}
