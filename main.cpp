#include <QApplication>

#include "QScrollEngine/QScrollEngine.h"

#include <QtQml/QQmlEngine>
#include <QQuickView>
#include <QScreen>
#include "App/ARCameraQml.h"

#include <QDebug>
#include <QtQml/QQmlContext>

int main(int argc, char *argv[])
{
    using namespace QScrollEngine;
    QApplication app(argc, argv);
    app.setApplicationName("qScrollEngine");
    app.setApplicationVersion("0.4a");
    /*QScrollEngine::QScrollEngineWidget widget;//движок - это виджет, наследуемый от QGLWidget
    widget.show();
    new AppEvent(&widget);//нужно делать отдельный объект для управления миром*/
    //объект соединяется с виджетом с помощью сигналов*/
    ARCameraQml::declareQml();
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    // Qt.quit() called in embedded .qml by default only emits
    // quit() signal, so do this (optionally use Qt.exit()).
    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    view.setSource(QUrl("qrc:///main.qml"));
    view.resize(800, 480);
    view.setClearBeforeRendering(true);
    view.show();
    //view.resetOpenGLState();

    return app.exec();
}
