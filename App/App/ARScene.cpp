#include "ARScene.h"
#include "ARTracker.h"

ARScene::ARScene()
{
    m_enabled = true;
    m_scene = nullptr;
    m_arTracker = nullptr;
}

void ARScene::beginUpdate()
{
    if ((m_arTracker != nullptr) && (m_scene != nullptr)) {
        _beginUpdate();
    }
}

void ARScene::endUpdate()
{
    if ((m_arTracker != nullptr) && (m_scene != nullptr)) {
        m_scene->orientation = m_arTracker->orientation();
        m_scene->position = m_arTracker->position();
        _endUpdate();
    }
}

void ARScene::_beginUpdate()
{
}

void ARScene::_endUpdate()
{
}

void ARScene::onPressed(int id, float x, float y)
{
    Q_UNUSED(id);
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void ARScene::onReleased(int id)
{
    Q_UNUSED(id);
}

void ARScene::setFrameTexture(GLuint textureId, const QMatrix3x3& textureMatrix, bool egl)
{
    Q_UNUSED(textureId);
    Q_UNUSED(egl);
    Q_UNUSED(textureMatrix);
}

QScrollEngine::QScene* ARScene::scene()
{
    return m_scene;
}

const QScrollEngine::QScene* ARScene::scene() const
{
    return m_scene;
}

QScrollEngine::QScene* ARScene::createScene(QScrollEngine::QScrollEngineContext* context)
{
    using namespace QScrollEngine;
    m_scene = new QScene(context);
    initScene();
    return m_scene;
}

void ARScene::freeScene()
{
    m_scene = nullptr;
}

bool ARScene::enabled() const
{
    return m_enabled;
}

void ARScene::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}

ARTracker* ARScene::arTracker()
{
    return m_arTracker;
}
const ARTracker* ARScene::arTracker() const
{
    return m_arTracker;
}

void ARScene::setArTracker(ARTracker* arTracker)
{
    if (m_arTracker != arTracker) {
        m_arTracker = arTracker;
        emit arTrackerChanged();
    }
}

void ARScene::initScene()
{
}
