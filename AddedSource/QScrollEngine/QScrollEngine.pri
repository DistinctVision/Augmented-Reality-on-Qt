CONFIG += c++11

QT       += widgets opengl
QT       += multimedia

CONFIG += c++11

INCLUDEPATH += .

SOURCES += \
    $$PWD/QAnimation3D.cpp \
    $$PWD/QBoundingBox.cpp \
    $$PWD/QCamera3D.cpp \
    $$PWD/QDrawObject3D.cpp \
    $$PWD/QEntity.cpp \
    $$PWD/QFileLoad3DS.cpp \
    $$PWD/QFrustum.cpp \
    $$PWD/QGLPrimitiv.cpp \
    $$PWD/QLight.cpp \
    $$PWD/QMesh.cpp \
    $$PWD/QOtherMathFunctions.cpp \
    $$PWD/QScene.cpp \
    $$PWD/QSceneObject3D.cpp \
    $$PWD/QScrollEngineWidget.cpp \
    $$PWD/QShObject3D.cpp \
    $$PWD/QSpotLight.cpp \
    $$PWD/QSprite.cpp \
    $$PWD/Shaders/QSh__Bloom.cpp \
    $$PWD/Shaders/QSh__BloomMap.cpp \
    $$PWD/Shaders/QSh__Blur.cpp \
    $$PWD/Shaders/QSh_Color.cpp \
    $$PWD/Shaders/QSh_ColoredPart.cpp \
    $$PWD/Shaders/QSh_Light.cpp \
    $$PWD/Tools/QIsoSurface.cpp \
    $$PWD/Tools/QPlanarShadows.cpp \
    $$PWD/QFileSaveLoad3DS.cpp \
    $$PWD/QScrollEngineContext.cpp \
    $$PWD/Shaders/QSh_Refraction_FallOff.cpp \
    $$PWD/Shaders/QSh_Texture.cpp \
    $$PWD/Shaders/QSh_LightVC.cpp \
    $$PWD/QSkinnedMesh.cpp

HEADERS += \
    $$PWD/QAnimation3D.h \
    $$PWD/QBoundingBox.h \
    $$PWD/QCamera3D.h \
    $$PWD/QDrawObject3D.h \
    $$PWD/QEntity.h \
    $$PWD/QFileLoad3DS.h \
    $$PWD/QFrustum.h \
    $$PWD/QGLPrimitiv.h \
    $$PWD/QLight.h \
    $$PWD/QMesh.h \
    $$PWD/QOtherMathFunctions.h \
    $$PWD/QScene.h \
    $$PWD/QSceneObject3D.h \
    $$PWD/QScrollEngine.h \
    $$PWD/QShObject3D.h \
    $$PWD/QSpotLight.h \
    $$PWD/QSprite.h \
    $$PWD/Shaders/QSh.h \
    $$PWD/Shaders/QSh__Bloom.h \
    $$PWD/Shaders/QSh__BloomMap.h \
    $$PWD/Shaders/QSh__Blur.h \
    $$PWD/Shaders/QSh_All.h \
    $$PWD/Shaders/QSh_Color.h \
    $$PWD/Shaders/QSh_ColoredPart.h \
    $$PWD/Shaders/QSh_Light.h \
    $$PWD/Tools/QIsoSurface.h \
    $$PWD/QFileSaveLoad3DS.h \
    $$PWD/QScrollEngineContext.h \
    $$PWD/QScrollEngineWidget.h \
    $$PWD/Tools/QPlanarShadows.h \
    $$PWD/Shaders/QSh_Refraction_FallOff.h \
    $$PWD/Shaders/QSh_Texture.h \
    $$PWD/Shaders/QSh_LightVC.h \
    $$PWD/QSkinnedMesh.h

RESOURCES += \
    $$PWD/shaders.qrc
