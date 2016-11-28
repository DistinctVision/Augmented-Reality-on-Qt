#include "ARSceneDebugTracker.h"
#include "ARTracker.h"
#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include <QMutexLocker>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QSharedPointer>
#include <QDebug>
#include "TMath/TTools.h"

void ARSceneDebugTracker::initScene()
{
    using namespace QScrollEngine;

    QMutexLocker ml(&m_mutex);
    scene()->setOrder(9999);
    connect(scene(), &QScene::beginDrawing, this, &ARSceneDebugTracker::drawTrackedMatches, Qt::DirectConnection);
}

void ARSceneDebugTracker::_endUpdate()
{
    using namespace QScrollEngine;
    using namespace AR;
    using namespace TMath;

    QMutexLocker ml(&m_mutex);

    ARTracker* arTracker = this->arTracker();
    m_imageSize = arTracker->imageSize();
    m_textureMatrix = arTracker->textureMatrix();
    m_matches = arTracker->debugTrackedMatches();
}

void ARSceneDebugTracker::drawTrackedMatches()
{
    using namespace QScrollEngine;

    QMutexLocker ml(&m_mutex);

    if (scene() == nullptr)
        return;

    if (m_matches.empty() || (m_imageSize == AR::Point2i(0, 0)))
        return;

    QScrollEngineContext* context = scene()->parentContext();

    context->glDepthMask(GL_FALSE);
    context->glDisable(GL_DEPTH_TEST);
    context->glDisable(GL_BLEND);

    QMatrix4x4 textureMatrix;
    textureMatrix.fill(0.0f);
    for (int i=0; i<2; ++i) {
        textureMatrix(i, 0) = m_textureMatrix(i, 0);
        textureMatrix(i, 1) = m_textureMatrix(i, 1);
        textureMatrix(i, 3) = m_textureMatrix(i, 2);
    }
    textureMatrix(3, 0) = m_textureMatrix(2, 0);
    textureMatrix(3, 1) = m_textureMatrix(2, 1);
    textureMatrix(3, 3) = m_textureMatrix(2, 2);

    QMatrix4x4 t;
    t.setToIdentity();
    t(0, 0) = 0.5f;
    t(1, 1) = -0.5f;
    t(0, 3) = 0.5f;
    t(1, 3) = 0.5f;

    QMatrix4x4 transformMatrix;
    transformMatrix.ortho(0.0f, (float)(m_imageSize.x),
                          0.0f, (float)(m_imageSize.y), 0.0f, 1.0f);
    transformMatrix = t.inverted() * textureMatrix * t * transformMatrix;

    context->glLineWidth(4.0f);

    int countLineVertices = (int)(m_matches.size() * 2);
    /*QVector2D* lineVertices = new QVector2D[countLineVertices];
    for (int i=0; i<(int)m_matches.size(); ++i) {
        std::pair<AR::Point2f, AR::Point2f>& match = m_matches[i];
        lineVertices[i * 2].setX(match.first.x);
        lineVertices[i * 2].setY(match.first.y);
        lineVertices[i * 2 + 1].setX(match.second.x);
        lineVertices[i * 2 + 1].setY(match.second.y);
    }
    context->drawLines(lineVertices, countLineVertices, QColor(55, 155, 255), transformMatrix);
    delete[] lineVertices;*/
    context->drawLines(reinterpret_cast<const QVector2D*>(m_matches.data()), countLineVertices,
                QColor(55, 155, 255), transformMatrix);
}
