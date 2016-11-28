#include "ARSceneCubes.h"
#include "ARSceneShip.h"
#include <QMutexLocker>

class RayCastCallback:
        public q3QueryCallback
{
public:
    q3RaycastData* rayCastData;
    float minToi = std::numeric_limits<float>::max();
    q3Box* resultBox = nullptr;

    bool ReportShape(q3Box* box) override
    {
        if (rayCastData->toi < minToi) {
            minToi = rayCastData->toi;
            resultBox = box;
        }
        return true;
    }
};

ARSceneCubes::ARSceneCubes():
    ARScene()
{
    m_physicsScene = nullptr;
    m_grid = nullptr;
    m_colorSelected = QColor(50, 150, 250);
    m_colorPressed = QColor(150, 50, 250);
    m_shaderForSelected.setColor(m_colorSelected);
}

void ARSceneCubes::initScene()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    scene()->setAmbientColor(50, 50, 50);

    m_planarShadows.setScene(scene());
    m_planarShadows.setColorShadows(QColor(0, 0, 0, 55));
    m_planarShadows.setAlpha(true);
    m_planarShadows.setPlanePos(QVector3D(0.0f, 0.0f, 0.0f));
    m_planarShadows.setPlaneDir(QVector3D(0.0f, 0.0f, 1.0f));

    connect(scene(), &QScene::deleting, this, &ARSceneCubes::_clear, Qt::DirectConnection);
    connect(scene(), &QScene::endDrawing, this, &ARSceneCubes::_drawSelected, Qt::DirectConnection);;

    m_grid = ARSceneShip::createGrid(scene(), QVector2D(5.0f, 5.0f), QSize(10, 10));
    m_grid->setVisibled(false);

    m_main = new QEntity(scene());
    m_cameraPivot = new QEntity(m_main);
    m_physicsScene = new q3Scene(1.0f / 30.0f, q3Vec3(0.0f, 0.0f, -9.8f), 5);

    QLight* light = new QLight(m_main);
    light->setPosition(0.0f, 0.0f, 30.0f);
    light->setRadius(100.0f);
    m_planarShadows.setLight(light);

    QScrollEngineContext* context = scene()->parentContext();

    QEntity* sampleCube = context->loadEntity(":/Data/cube.3DS");
    sampleCube->part(0)->mesh()->moveVertices(0.0f, 0.0f, -0.5f);
    sampleCube->part(0)->mesh()->applyChangesOfVertexPositions();
    sampleCube->part(0)->mesh()->updateLocalBoundingBox();
    sampleCube->setScale(0.9f);
    sampleCube->setParentEntity(m_main);
    QOpenGLTexture* sampleCubeTexture = context->texture("cubeTexture");
    if (sampleCubeTexture == nullptr)
        sampleCubeTexture = context->loadTexture("cubeTexture", ":/Data/CompanionCube.jpg");
    dynamic_cast<QSh_Texture*>(sampleCube->part(0)->shader().data())->setTexture(sampleCubeTexture);

    q3BodyDef bodyDef;
    bodyDef.bodyType = eDynamicBody;
    bodyDef.active = true;
    bodyDef.allowSleep = true;
    bodyDef.awake = true;
    bodyDef.position.Set(0.0f, 0.0f, 0.0f);
    bodyDef.axis.Set(0.0f, 0.0f, 1.0f);
    bodyDef.angle = 0.0f;
    q3BoxDef boxDef;
    boxDef.Set({ q3Vec3(0.0f, 0.0f, 0.0f),
                 q3Mat3(1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f) },
               q3Vec3(1.0f, 1.0f, 1.0f));

    int size = 2;
    int i, j;
    qsrand(QTime::currentTime().msec());
    for (int level = size - 1; level >= 0; --level) {
        float halfSize = ((level + 1.0f) * 0.5f);
        for (i = 0; i <= level; ++i) {
            for (j = 0; j <= level; ++j) {
                bodyDef.position.Set((i - halfSize), (j - halfSize), (size - level - 0.5f) + 0.1f);
                bodyDef.angle = (qrand() % 4) * (float)(M_PI) * 0.5f;
                q3Body* body = m_physicsScene->CreateBody(bodyDef);
                body->AddBox(boxDef);
                QEntity* entity = sampleCube->clone();
                m_planarShadows.addEntity(entity);
                m_physicsEntities.push_back({ body, entity });
            }
        }
    }
    delete sampleCube;

    bodyDef.bodyType = eStaticBody;
    bodyDef.position.Set(0.0f, 0.0f, -0.5f);
    bodyDef.axis.Set(0.0f, 0.0f, 1.0f);
    bodyDef.angle = 0.0f;
    boxDef.Set({ q3Vec3(0.0f, 0.0f, 0.0f),
                 q3Mat3(1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f) },
               q3Vec3(100.0f, 100.0f, 1.0f));
    m_physicsScene->CreateBody(bodyDef)->AddBox(boxDef);

    m_state = 0.0f;
    m_speedOfChangeState = 0.05f;

    m_selectedBody = nullptr;
    m_selectedEntity = nullptr;
    m_pressed = false;

    _updatePhysics();
}

void ARSceneCubes::onPressed(int id, float x, float y)
{
    Q_UNUSED(id);
    Q_UNUSED(x);
    Q_UNUSED(y);
    QMutexLocker ml(&m_mutex);
    m_shaderForSelected.setColor(m_colorPressed);
    m_pressed = true;
}

void ARSceneCubes::onReleased(int id)
{
    Q_UNUSED(id);
    QMutexLocker ml(&m_mutex);
    m_shaderForSelected.setColor(m_colorSelected);
    m_pressed = false;
}

void ARSceneCubes::_beginUpdate()
{
}

void ARSceneCubes::_endUpdate()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    scene()->setEnabled((arTracker()->trackingState() == ARTracker::Tracking));
    if (!scene()->enabled())
        return;
    scene()->parentContext()->glLineWidth(2.0f);
    if (m_state < 1.0f) {
        m_grid->setVisibled(true);
        m_main->setVisibled(true);
        QSh_ColoredPart* sh = dynamic_cast<QSh_ColoredPart*>(m_grid->part(0)->shader().data());
        QColor color = sh->backColor();
        if (m_state < 0.5f) {
            sh->setLimit(1.0f - m_state * 2.0f);
            color.setAlphaF(1.0f);
            sh->setBackColor(color);
        } else {
            sh->setLimit(0.0f);
            color.setAlphaF((1.0f - m_state) * 2.0f);
            sh->setBackColor(color);
        }
        m_state += m_speedOfChangeState;
    } else {
        m_grid->setVisibled(false);
        m_main->setVisibled(true);
        _updatePhysics();
        _updateSelected();
    }
}

void ARSceneCubes::_clear()
{
    QMutexLocker ml(&m_mutex);
    if (m_physicsScene) {
        delete m_physicsScene;
        m_physicsEntities.clear();
    }
}

void ARSceneCubes::_updatePhysics()
{
    m_cameraPivot->updateTransform();
    QVector3D globalSelectedPosition = m_cameraPivot->transformPoint(m_selectedLocalPosition);
    if (m_selectedBody) {
        if (m_pressed) {
            m_selectedBody->SetToAwake();
            m_selectedBody->SetTransform(q3Vec3(globalSelectedPosition.x(),
                                                globalSelectedPosition.y(),
                                                globalSelectedPosition.z()));
            m_selectedBody->SetLinearVelocity(q3Vec3(0.0f, 0.0f, 0.0f));
            m_selectedBody->SetAngularVelocity(q3Vec3(0.0f, 0.0f, 0.0f));
        }
    } else {
        m_pressed = false;
        m_shaderForSelected.setColor(m_colorSelected);
    }
    m_physicsScene->Step();
    for (auto it = m_physicsEntities.begin(); it != m_physicsEntities.end(); ++it) {
        q3Vec3 position = it->first->GetWorldPoint(q3Vec3(0.0f, 0.0f, 0.0f));
        q3Quaternion orientation = it->first->GetQuaternion();
        it->second->setPosition(position.x, position.y, position.z);
        it->second->setOrientation(QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    }
    if (m_pressed) {
        m_selectedEntity->setPosition(globalSelectedPosition);
        m_selectedBody->SetTransform(q3Vec3(globalSelectedPosition.x(),
                                            globalSelectedPosition.y(),
                                            globalSelectedPosition.z()));
    }
}

void ARSceneCubes::_updateSelected()
{
    using namespace QScrollEngine;

    QEntity* prevSelectedEntity = m_selectedEntity;

    m_selectedBody = nullptr;
    m_selectedEntity = nullptr;

    scene()->updateCameraInfo(scene()->parentContext()->camera);
    QScene::CameraInfo cameraInfo = scene()->cameraInfo();
    m_cameraPivot->setPosition(cameraInfo.position);
    m_cameraPivot->setOrientation(cameraInfo.orientation);

    q3RaycastData rayCastData;
    rayCastData.dir.Set(- cameraInfo.localZ.x(), - cameraInfo.localZ.y(), - cameraInfo.localZ.z());
    rayCastData.start.Set(cameraInfo.position.x(), cameraInfo.position.y(), cameraInfo.position.z());
    rayCastData.t = 30.0f;
    rayCastData.toi = 0.0f;
    rayCastData.normal.SetAll(0.0f);
    RayCastCallback callback;
    callback.rayCastData = &rayCastData;
    m_physicsScene->RayCast(&callback, rayCastData);
    if (callback.resultBox == nullptr) {
        m_pressed = false;
        m_shaderForSelected.setColor(m_colorSelected);
        return;
    }
    for (auto it = m_physicsEntities.begin(); it != m_physicsEntities.end(); ++it) {
        if (it->first == callback.resultBox->body) {
            m_selectedBody = it->first;
            m_selectedEntity = it->second;
            break;
        }
    }
    if (m_selectedEntity == nullptr) {
        m_pressed = false;
        m_shaderForSelected.setColor(m_colorSelected);
        return;
    }
    if (prevSelectedEntity == nullptr) {
        m_selectedLocalPosition = m_cameraPivot->inverseTransformPoint(m_selectedEntity->globalPosition());
    } else if (prevSelectedEntity != m_selectedEntity) {
        m_selectedEntity = nullptr;
        m_selectedBody = nullptr;
        m_pressed = false;
        m_shaderForSelected.setColor(m_colorSelected);
    }
}

void ARSceneCubes::_drawEntity(const QScrollEngine::QEntity* entity)
{
    using namespace QScrollEngine;
    QScrollEngineContext* context = scene()->parentContext();
    std::size_t i;
    for (i = 0; i < entity->countParts(); ++i) {
        const QEntity::QPartEntity* part = entity->part(i);
        context->drawMesh(part->mesh(), part, &m_shaderForSelected);
    }
    for (i = 0; i < entity->countEntityChilds(); ++i) {
        _drawEntity(entity->childEntity(i));
    }
}
void ARSceneCubes::_drawSelected()
{
    using namespace QScrollEngine;

    QMutexLocker ml(&m_mutex);
    if (m_selectedEntity == nullptr)
        return;
    if (scene() == nullptr)
        return;
    QScrollEngineContext* context = scene()->parentContext();

    context->glDisable(GL_BLEND);
    context->glDisable(GL_DEPTH_TEST);
    context->glDepthMask(GL_FALSE);
    context->glEnable(GL_STENCIL_TEST);
    context->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    context->glStencilFunc(GL_ALWAYS, 0, 1);
    context->glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    context->glClearStencil(0);
    context->glClear(GL_STENCIL_BUFFER_BIT);
    context->glStencilFunc(GL_ALWAYS, 1, 1);
    context->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    QVector3D scale = m_selectedEntity->scale();
    m_selectedEntity->setScale(scale * 0.9f);
    m_selectedEntity->updateTransform();
    m_selectedEntity->updateMatrxWorldViewProj(context->camera->matrixViewProj());
    _drawEntity(m_selectedEntity);
    context->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //context->glDepthMask(GL_FALSE);
    context->glStencilFunc(GL_NOTEQUAL, 1, 1);
    context->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    m_selectedEntity->setScale(scale);
    m_selectedEntity->updateTransform();
    m_selectedEntity->updateMatrxWorldViewProj(context->camera->matrixViewProj());
    _drawEntity(m_selectedEntity);
    context->glEnable(GL_DEPTH_TEST);
    context->glDisable(GL_STENCIL_TEST);
    context->glDepthMask(GL_TRUE);
}
