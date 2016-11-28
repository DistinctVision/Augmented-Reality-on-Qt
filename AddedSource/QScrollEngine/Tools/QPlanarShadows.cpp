#include "QScrollEngine/Tools/QPlanarShadows.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QBoundingBox.h"
#include "QScrollEngine/Shaders/QSh_Texture.h"

namespace QScrollEngine {

QPlanarShadows::QPlanarShadows(QObject* parent):
    QObject(parent)
{
    m_light = nullptr;
    m_scene = nullptr;
    m_planePos = QVector3D(0.0f, 0.0f, 0.0f);
    m_planeNormal = QVector3D(0.0f, 0.0f, 1.0f);
    m_colorShadows.setRgb(0, 0, 0, 100);
    m_isAlpha = false;
}

QPlanarShadows::~QPlanarShadows()
{
    clear();
}

void QPlanarShadows::addEntity(QEntity* entity)
{
    assert(entity != nullptr);
    m_entities.insert(entity);
    connect(entity, SIGNAL(onDelete(QSceneObject3D*)), this, SLOT(deletingObject(QSceneObject3D*)), Qt::DirectConnection);
}

void QPlanarShadows::deleteEntity(QEntity* entity)
{
    assert(entity != nullptr);
    m_entities.erase(entity);
    disconnect(entity, SIGNAL(onDelete(QSceneObject3D*)), this, SLOT(deletingObject(QSceneObject3D*)));
}

void QPlanarShadows::setLight(QLight* light)
{
    if (m_light)
        disconnect(m_light, SIGNAL(onDelete()), this, SLOT(deletingLight()));
    m_light = light;
    if (m_light)
        connect(m_light, SIGNAL(onDelete(QSceneObject3D*)), this, SLOT(deletingLight()), Qt::DirectConnection);
}

void QPlanarShadows::deletingObject(QSceneObject3D* object)
{
    QEntity* entity = static_cast<QEntity*>(object);
    if (entity != nullptr)
        deleteEntity(entity);
}

void QPlanarShadows::deletingLight()
{
    m_light = nullptr;
    disconnect(m_light, SIGNAL(onDelete(QSceneObject3D*)), this, SLOT(deletingLight()));
}

void QPlanarShadows::clear()
{
    for (std::set<QEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
        disconnect(*it, SIGNAL(onDelete(QSceneObject3D*)), this, SLOT(deletingObject(QSceneObject3D*)));
    }
    m_entities.clear();
}

void QPlanarShadows::setScene(QScene* scene)
{
    clear();
    m_scene = scene;
    if (m_scene) {
        connect(m_scene, &QScene::beginDrawingAlphaObjects, this, &QPlanarShadows::draw, Qt::DirectConnection);
    }
}

void QPlanarShadows::_calculateMatrixShadow()
{
    float planeD = - QVector3D::dotProduct(m_planePos, m_planeNormal);
    QVector3D lightPosition = m_light->position();
    float dot = QVector3D::dotProduct(lightPosition, m_planeNormal) + planeD;

    m_matrixShadow(0, 0) = dot - m_planeNormal.x() * lightPosition.x();
    m_matrixShadow(0, 1) =     - m_planeNormal.y() * lightPosition.x();
    m_matrixShadow(0, 2) =     - m_planeNormal.z() * lightPosition.x();
    m_matrixShadow(0, 3) =     - planeD * lightPosition.x();

    m_matrixShadow(1, 0) =     - m_planeNormal.x() * lightPosition.y();
    m_matrixShadow(1, 1) = dot - m_planeNormal.y() * lightPosition.y();
    m_matrixShadow(1, 2) =     - m_planeNormal.z() * lightPosition.y();
    m_matrixShadow(1, 3) =     - planeD * lightPosition.y();

    m_matrixShadow(2, 0) =     - m_planeNormal.x() * lightPosition.z();
    m_matrixShadow(2, 1) =     - m_planeNormal.y() * lightPosition.z();
    m_matrixShadow(2, 2) = dot - m_planeNormal.z() * lightPosition.z();
    m_matrixShadow(2, 3) =     - planeD * lightPosition.z();

    m_matrixShadow(3, 0) =     - m_planeNormal.x();
    m_matrixShadow(3, 1) =     - m_planeNormal.y();
    m_matrixShadow(3, 2) =     - m_planeNormal.z();
    m_matrixShadow(3, 3) = dot - planeD;
}

void QPlanarShadows::draw()
{
    if (m_light == nullptr)
        return;
    if (m_scene == nullptr)
        return;
    m_parentContext = m_scene->parentContext();
    if (m_parentContext == nullptr)
        return;
    m_attributes = m_parentContext->vertexAttributesOfShader(QSh::Type::Color);
    QOpenGLShaderProgram* program = m_parentContext->shaderProgram(QSh::Type::Color);
    if (!program->bind())
        return;
    program->enableAttributeArray(0);
    QCamera3D* camera = m_parentContext->camera;
    camera->setScene(m_scene);
    camera->update();
    _calculateMatrixShadow();
    if (m_isAlpha) {
        m_parentContext->glDepthMask(GL_FALSE);
        m_parentContext->glEnable(GL_STENCIL_TEST);
        m_parentContext->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        m_parentContext->glStencilFunc(GL_ALWAYS, 0, 1);
        m_parentContext->glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        m_parentContext->glClearStencil(0);
        m_parentContext->glClear(GL_STENCIL_BUFFER_BIT);
        m_parentContext->glStencilFunc(GL_ALWAYS, 1, 1);
        m_parentContext->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        program->setUniformValue("color", QColor(0, 0, 0, 255));
        for (std::set<QEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (_checkParentVisibled(*it))
                _drawShadowOfEntity(program, camera, *it);
        }
        m_parentContext->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //_parentContext->glDepthMask(GL_FALSE);
        m_parentContext->glDisable(GL_DEPTH_TEST);
        m_parentContext->glEnable(GL_BLEND);
        m_parentContext->glStencilFunc(GL_NOTEQUAL, 0, 1);
        m_parentContext->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        program->setUniformValue("color", m_colorShadows);
        program->setUniformValue("matrix_wvp", m_parentContext->screenQuadMatrix());
        m_parentContext->screenQuad()->bind(m_attributes);
        m_parentContext->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        m_parentContext->glEnable(GL_DEPTH_TEST);
        m_parentContext->glDisable(GL_STENCIL_TEST);
        m_parentContext->glDepthMask(GL_TRUE);
    } else {
        program->setUniformValue("color", QColor(m_colorShadows.red(),
                                                 m_colorShadows.green(),
                                                 m_colorShadows.blue(), 255));
        for (std::set<QEntity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (_checkParentVisibled(*it))
                _drawShadowOfEntity(program, camera, *it);
        }
    }
    program->disableAttributeArray(0);
    program->release();
}

bool QPlanarShadows::_checkParentVisibled(QEntity* entity) const
{
    QEntity* parent = entity->parentEntity();
    if (parent == nullptr)
        return true;
    if (!parent->visibled())
        return false;
    return _checkParentVisibled(parent);
}

void QPlanarShadows::_drawShadowOfEntity(QOpenGLShaderProgram* program, QCamera3D* camera, QEntity* entity)
{
    if (!entity->visibled())
        return;
    entity->updateTransform();
    //if (camera->frustum.boundingBoxInFrustum(entity->boundingBox().transform(_matrixShadow))) {
        program->setUniformValue("matrix_wvp", camera->matrixViewProj() * m_matrixShadow * entity->matrixWorld());
        std::size_t k;
        QEntity::Part* part;
        for (k=0; k<entity->countParts(); ++k) {
            part = entity->part(k);
            QMesh* mesh = part->mesh();
            mesh->bind(m_attributes);
            m_parentContext->glDrawElements(part->drawMode(), mesh->elements().size(), GL_UNSIGNED_INT, nullptr);
        }
        for (k=0; k<entity->countEntityChilds(); ++k)
            _drawShadowOfEntity(program, camera, entity->childEntity(k));
    //}
}

}
