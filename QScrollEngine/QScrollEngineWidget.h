#ifndef QSCROLLENGINEWIDGET_H
#define QSCROLLENGINEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QQuaternion>
#include <QList>
#include <vector>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTouchEvent>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMessageBox>
#include <QString>
#include <QMap>
#include <QImage>
#include <QMouseEvent>
#include <QTimer>
#include <QMutex>

#include "QScrollEngine/QScrollEngineContext.h"

#define SUPPORT_MOUSE 1
#define SUPPORT_KEYBOARD 1
#define SUPPORT_TOUCHSCREEN 1

namespace QScrollEngine {

class QScrollEngineWidget :
        public QOpenGLWidget, public QScrollEngineContext
{
    Q_OBJECT
public:
    explicit QScrollEngineWidget(QWidget* parent = 0);
    ~QScrollEngineWidget();

signals:
    void prevDraw();
    void afterDraw();
    void afterResized();
#if ((SUPPORT_MOUSE == 1) || (SUPPORT_TOUCHSCREEN == 1))
    void touchPressed(const QTouchEvent::TouchPoint& );
    void touchMoved(const QTouchEvent::TouchPoint& );
    void touchReleased(const QTouchEvent::TouchPoint& );
#endif
#if (SUPPORT_KEYBOARD == 1)
    void keyPress(int );
    void keyRelease(int );
#endif

protected:
    bool event(QEvent* event);

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    QTimer _timer;
};

}

#endif
