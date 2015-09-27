#include "QScrollEngine/Tools/PlanarShadows.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QBoundingBox.h"
#include "QScrollEngine/Shaders/QSh_Texture1.h"

namespace QScrollEngine {

PlanarShadows::PlanarShadows()
{
    _light = nullptr;
    _scene = nullptr;
    _planePos = QVector3D(0.0f, 0.0f, 0.0f);
    _planeNormal = QVector3D(0.0f, 0.0f, 1.0f);
    _colorShadows.setRgb(0, 0, 0, 100);
    _isAlpha = false;
}

void PlanarShadows::addEntity(QEntity* entity)
{
    assert(entity != nullptr);
    _entities.insert(entity);
    connect(entity, SIGNAL(deleting()), this, SLOT(deletingObject()));
}

void PlanarShadows::deletingObject()
{
    deleteEntity(static_cast<QEntity*>(sender()));
}

void PlanarShadows::deleteEntity(QEntity* entity)
{
    assert(entity != nullptr);
    _entities.erase(entity);
    disconnect(entity, SIGNAL(deleting()), this, SLOT(deletingObject()));
}

void PlanarShadows::setLight(QLight* light)
{
    if (_light)
        disconnect(_light, SIGNAL(deleting()), this, SLOT(deletingLight()));
    _light = light;
    if (_light)
        connect(_light, SIGNAL(deleting()), this, SLOT(deletingLight()));
}

void PlanarShadows::deletingLight()
{
    if (static_cast<QLight*>(sender()) == _light) {
        _light = nullptr;
    }
}

void PlanarShadows::clear()
{
    for (std::set<QEntity*>::iterator it = _entities.begin(); it != _entities.end(); ++it) {
        disconnect(*it, SIGNAL(deleting()), this, SLOT(deletingObject()));
    }
    _entities.clear();
}

void PlanarShadows::setScene(QScene* scene)
{
    _scene = scene;
    disconnect(this);
    if (_scene)
        connect(_scene, SIGNAL(beginDrawingAlphaObjects()), this, SLOT(draw()), Qt::DirectConnection);
}

void PlanarShadows::_calculateMatrixShadow()
{
    float planeD = - QVector3D::dotProduct(_planePos, _planeNormal);
    QVector3D lightPosition = _light->position();
    float dot = QVector3D::dotProduct(lightPosition, _planeNormal) + planeD;

    _matrixShadow(0, 0) = dot - _planeNormal.x() * lightPosition.x();
    _matrixShadow(0, 1) =     - _planeNormal.y() * lightPosition.x();
    _matrixShadow(0, 2) =     - _planeNormal.z() * lightPosition.x();
    _matrixShadow(0, 3) =     - planeD * lightPosition.x();

    _matrixShadow(1, 0) =     - _planeNormal.x() * lightPosition.y();
    _matrixShadow(1, 1) = dot - _planeNormal.y() * lightPosition.y();
    _matrixShadow(1, 2) =     - _planeNormal.z() * lightPosition.y();
    _matrixShadow(1, 3) =     - planeD * lightPosition.y();

    _matrixShadow(2, 0) =     - _planeNormal.x() * lightPosition.z();
    _matrixShadow(2, 1) =     - _planeNormal.y() * lightPosition.z();
    _matrixShadow(2, 2) = dot - _planeNormal.z() * lightPosition.z();
    _matrixShadow(2, 3) =     - planeD * lightPosition.z();

    _matrixShadow(3, 0) =     - _planeNormal.x();
    _matrixShadow(3, 1) =     - _planeNormal.y();
    _matrixShadow(3, 2) =     - _planeNormal.z();
    _matrixShadow(3, 3) = dot - planeD;
}

void PlanarShadows::draw()
{
    if (_light == nullptr)
        return;
    if (_scene == nullptr)
        return;
    _parentContext = _scene->parentContext();
    if (_parentContext == nullptr)
        return;
    QOpenGLShaderProgram* program = _parentContext->shaderProgram(QSh::Color);
    if (!program->bind())
        return;
    program->enableAttributeArray(0);
    QCamera3D* camera = _parentContext->camera;
    camera->setScene(_scene);
    camera->update();
    _calculateMatrixShadow();
    if (_isAlpha) {
        _parentContext->glDepthMask(GL_FALSE);
        _parentContext->glEnable(GL_STENCIL_TEST);
        _parentContext->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        _parentContext->glStencilFunc(GL_ALWAYS, 0, 1);
        _parentContext->glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        _parentContext->glClearStencil(0);
        _parentContext->glClear(GL_STENCIL_BUFFER_BIT);
        _parentContext->glStencilFunc(GL_ALWAYS, 1, 1);
        _parentContext->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        program->setUniformValue(QSh_Color::locationColor, QColor(0, 0, 0, 255));
        for (std::set<QEntity*>::iterator it = _entities.begin(); it != _entities.end(); ++it)
            _drawShadowOfEntity(program, camera, *it);
        _parentContext->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //_parentContext->glDepthMask(GL_FALSE);
        _parentContext->glDisable(GL_DEPTH_TEST);
        _parentContext->glEnable(GL_BLEND);
        _parentContext->glStencilFunc(GL_NOTEQUAL, 0, 1);
        _parentContext->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        program->setUniformValue(QSh_Color::locationColor, _colorShadows);
        program->setUniformValue(QSh_Color::locationMatrixWVP, _parentContext->screenQuadMatrix());
        _parentContext->drawMesh(GL_TRIANGLES, _parentContext->screenQuad(), program);
        _parentContext->glEnable(GL_DEPTH_TEST);
        _parentContext->glDisable(GL_STENCIL_TEST);
        _parentContext->glDepthMask(GL_TRUE);
    } else {
        program->setUniformValue(QSh_Color::locationColor, QColor(_colorShadows.red(), _colorShadows.green(), _colorShadows.blue(), 255));
        for (std::set<QEntity*>::iterator it = _entities.begin(); it != _entities.end(); ++it)
            _drawShadowOfEntity(program, camera, *it);
    }
    program->disableAttributeArray(0);
    program->release();
}

void PlanarShadows::_drawShadowOfEntity(QOpenGLShaderProgram* program, QCamera3D* camera, QEntity* entity)
{
    entity->updateTransform();
    //if (camera->frustum.boundingBoxInFrustum(entity->boundingBox().transform(_matrixShadow))) {
        program->setUniformValue(QSh_Color::locationMatrixWVP, camera->matrixViewProj() * _matrixShadow * entity->matrixWorld());
        unsigned int k;
        QEntity::QPartEntity* part;
        for (k=0; k<entity->countParts(); ++k) {
            part = entity->part(k);
            _parentContext->drawMesh(part->drawMode(), part->mesh(), program);
        }
        for (k=0; k<entity->countEntityChilds(); ++k)
            _drawShadowOfEntity(program, camera, entity->childEntity(k));
    //}
}

}
