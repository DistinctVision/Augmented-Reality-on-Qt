android: {
    TEMPLATE = app
} else: {
    TEMPLATE = lib
}

QT       += widgets opengl
QT       += multimedia

CONFIG += c++11

INCLUDEPATH += $$PWD/include/
DEPENDPATH += $$PWD/include/

SOURCES += \
    $$PWD/src/QAnimation3D.cpp \
    $$PWD/src/QBoundingBox.cpp \
    $$PWD/src/QCamera3D.cpp \
    $$PWD/src/QDrawObject3D.cpp \
    $$PWD/src/QEntity.cpp \
    $$PWD/src/QFileLoad3DS.cpp \
    $$PWD/src/QFrustum.cpp \
    $$PWD/src/QGLPrimitiv.cpp \
    $$PWD/src/QLight.cpp \
    $$PWD/src/QMesh.cpp \
    $$PWD/src/QOtherMathFunctions.cpp \
    $$PWD/src/QScene.cpp \
    $$PWD/src/QSceneObject3D.cpp \
    $$PWD/src/QScrollEngineWidget.cpp \
    $$PWD/src/QShObject3D.cpp \
    $$PWD/src/QSpotLight.cpp \
    $$PWD/src/QSprite.cpp \
    $$PWD/src/Shaders/QSh__Bloom.cpp \
    $$PWD/src/Shaders/QSh__BloomMap.cpp \
    $$PWD/src/Shaders/QSh__Blur.cpp \
    $$PWD/src/Shaders/QSh_Color.cpp \
    $$PWD/src/Shaders/QSh_ColoredPart.cpp \
    $$PWD/src/Shaders/QSh_Light.cpp \
    $$PWD/src/Tools/QIsoSurface.cpp \
    $$PWD/src/Tools/QPlanarShadows.cpp \
    $$PWD/src/QFileSaveLoad3DS.cpp \
    $$PWD/src/QScrollEngineContext.cpp \
    $$PWD/src/Shaders/QSh_Refraction_FallOff.cpp \
    $$PWD/src/Shaders/QSh_Texture.cpp \
    $$PWD/src/Shaders/QSh_LightVC.cpp \
    $$PWD/src/QSkinnedMesh.cpp

HEADERS += \
    $$PWD/include/QScrollEngine/QAnimation3D.h \
    $$PWD/include/QScrollEngine/QBoundingBox.h \
    $$PWD/include/QScrollEngine/QCamera3D.h \
    $$PWD/include/QScrollEngine/QDrawObject3D.h \
    $$PWD/include/QScrollEngine/QEntity.h \
    $$PWD/include/QScrollEngine/QFileLoad3DS.h \
    $$PWD/include/QScrollEngine/QFrustum.h \
    $$PWD/include/QScrollEngine/QGLPrimitiv.h \
    $$PWD/include/QScrollEngine/QLight.h \
    $$PWD/include/QScrollEngine/QMesh.h \
    $$PWD/include/QScrollEngine/QOtherMathFunctions.h \
    $$PWD/include/QScrollEngine/QScene.h \
    $$PWD/include/QScrollEngine/QSceneObject3D.h \
    $$PWD/include/QScrollEngine/QScrollEngine.h \
    $$PWD/include/QScrollEngine/QShObject3D.h \
    $$PWD/include/QScrollEngine/QSpotLight.h \
    $$PWD/include/QScrollEngine/QSprite.h \
    $$PWD/include/QScrollEngine/Shaders/QSh.h \
    $$PWD/include/QScrollEngine/Shaders/QSh__Bloom.h \
    $$PWD/include/QScrollEngine/Shaders/QSh__BloomMap.h \
    $$PWD/include/QScrollEngine/Shaders/QSh__Blur.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_All.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_Color.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_ColoredPart.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_Light.h \
    $$PWD/include/QScrollEngine/Tools/QIsoSurface.h \
    $$PWD/include/QScrollEngine/QFileSaveLoad3DS.h \
    $$PWD/include/QScrollEngine/QScrollEngineContext.h \
    $$PWD/include/QScrollEngine/QScrollEngineWidget.h \
    $$PWD/include/QScrollEngine/Tools/QPlanarShadows.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_Refraction_FallOff.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_Texture.h \
    $$PWD/include/QScrollEngine/Shaders/QSh_LightVC.h \
    $$PWD/include/QScrollEngine/QSkinnedMesh.h

RESOURCES += \
    $$PWD/resources/shaders.qrc
