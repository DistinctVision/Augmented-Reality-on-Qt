#include "QScrollEngine/QScrollEngineWidget.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"
#include "QScrollEngine/QGLPrimitiv.h"
#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QFileLoad3DS.h"
#include "QScrollEngine/QCamera3D.h"
#include <qmath.h>
#include <locale.h>
#include <QTime>

namespace QScrollEngine {

QScrollEngineWidget::QScrollEngineWidget(QWidget* parent) :
    QOpenGLWidget(parent), QScrollEngineContext(nullptr)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer.setSingleShot(true);
#if (SUPPORT_TOUCHSCREEN == 1)
    setAttribute(Qt::WA_AcceptTouchEvents);
#endif
}

QScrollEngineWidget::~QScrollEngineWidget()
{
}

bool QScrollEngineWidget::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Paint:
        {
            return QOpenGLWidget::event(event);
        }
#if (SUPPORT_MOUSE == 1)
    case QEvent::MouseButtonPress:
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QTouchEvent::TouchPoint touchPoint;
            touchPoint.setId(-1);
            touchPoint.setPos(mouseEvent->pos());
            emit touchPressed(touchPoint);
            return true;
        }
    case QEvent::MouseMove:
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QTouchEvent::TouchPoint touchPoint;
            touchPoint.setId(-1);
            touchPoint.setPos(mouseEvent->pos());
            emit touchMoved(touchPoint);
            return true;
        }
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QTouchEvent::TouchPoint touchPoint;
            touchPoint.setId(-1);
            touchPoint.setPos(mouseEvent->pos());
            emit touchReleased(touchPoint);
            return true;
        }
#endif
#if (SUPPORT_KEYBOARD == 1)
    case QEvent::KeyPress:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        emit keyPress(keyEvent->key());
        return true;
    }
    case QEvent::KeyRelease:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        emit keyRelease(keyEvent->key());
        return true;
    }
#endif
#if (SUPPORT_TOUCHSCREEN == 1)
    case QEvent::TouchUpdate:
        //return QWidget::event(event);
        break;
    case QEvent::TouchBegin:
        //return QWidget::event(event);
        break;
    case QEvent::TouchEnd:
        break;
    case QEvent::TouchCancel:
        //return QWidget::event(event);
        break;
#endif
    default:
        return QOpenGLWidget::event(event);
    }
#if (SUPPORT_TOUCHSCREEN == 1)
    QList<QTouchEvent::TouchPoint> listTouchs = static_cast<QTouchEvent *>(event)->touchPoints();
    foreach (const QTouchEvent::TouchPoint& touchPoint, listTouchs) {
        switch (touchPoint.state()) {
        case Qt::TouchPointStationary:
            {
                continue;
            }
        case Qt::TouchPointMoved:
            {
                emit touchMoved(touchPoint);
                break;
            }
        case Qt::TouchPointPressed:
            {
                emit touchPressed(touchPoint);
                break;
            }
        case Qt::TouchPointReleased:
            {
                emit touchReleased(touchPoint);
                break;
            }\
        }
    }
#endif
    return true;
}

void QScrollEngineWidget::initializeGL()
{
    setOpenGLContext(context());
    m_timer.start(30);
}

void QScrollEngineWidget::resizeGL(int width, int height)
{
    resizeContext(width, height);
    emit afterResized();
}


void QScrollEngineWidget::paintGL()
{
    emit prevDraw();
    paintContext();
    emit afterDraw();
    m_timer.start(10);
}

}
