QT += qml quick widgets multimedia

CONFIG += c++11

TARGET = AR_Qt
TEMPLATE = app

include (QScrollEngine.pri)
include (AR.pri)

SOURCES += main.cpp \
    App/ARCameraQml.cpp \
    App/SceneShip.cpp \
    App/SceneSpace.cpp \
    App/SceneGrid.cpp \
    App/SceneBreakout.cpp \
    App/ScenePigPetr.cpp \
    App/SceneLoadModel.cpp

HEADERS += \
    App/ARCameraQml.h \
    App/SceneShip.h \
    App/SceneSpace.h \
    App/SceneGrid.h \
    App/SceneBreakout.h \
    App/ScenePigPetr.h \
    App/SceneLoadModel.h

INCLUDEPATH += .

RESOURCES += \
    mainresources.qrc \
    $$PWD/resources.qrc



