QT       += widgets opengl
QT       += multimedia

CONFIG += c++11

SOURCES += \
    $$PWD/QScrollEngine/QAnimation3D.cpp \
    $$PWD/QScrollEngine/QBoundingBox.cpp \
    $$PWD/QScrollEngine/QCamera3D.cpp \
    $$PWD/QScrollEngine/QDrawObject3D.cpp \
    $$PWD/QScrollEngine/QEntity.cpp \
    $$PWD/QScrollEngine/QFileLoad3DS.cpp \
    $$PWD/QScrollEngine/QFrustum.cpp \
    $$PWD/QScrollEngine/QGLPrimitiv.cpp \
    $$PWD/QScrollEngine/QLight.cpp \
    $$PWD/QScrollEngine/QMesh.cpp \
    $$PWD/QScrollEngine/QOtherMathFunctions.cpp \
    $$PWD/QScrollEngine/QScene.cpp \
    $$PWD/QScrollEngine/QSceneObject3D.cpp \
    $$PWD/QScrollEngine/QScrollEngineWidget.cpp \
    $$PWD/QScrollEngine/QShObject3D.cpp \
    $$PWD/QScrollEngine/QSpotLight.cpp \
    $$PWD/QScrollEngine/QSprite.cpp \
    $$PWD/QScrollEngine/Shaders/QSh.cpp \
    $$PWD/QScrollEngine/Shaders/QSh__Bloom.cpp \
    $$PWD/QScrollEngine/Shaders/QSh__BloomMap.cpp \
    $$PWD/QScrollEngine/Shaders/QSh__Blur.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Color.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_ColoredPart.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Light.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Texture1.cpp \
    $$PWD/QScrollEngine/Tools/QIsoSurface.cpp \
    $$PWD/QScrollEngine/Tools/PlanarShadows.cpp \
    $$PWD/QScrollEngine/QFileSaveLoad3DS.cpp \
    $$PWD/QScrollEngine/QScrollEngineContext.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Refraction_FallOff.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Sphere_Light.cpp \
    $$PWD/QScrollEngine/Shaders/QSh_Sphere_Texture1.cpp

HEADERS += \
    $$PWD/QScrollEngine/QAnimation3D.h \
    $$PWD/QScrollEngine/QBoundingBox.h \
    $$PWD/QScrollEngine/QCamera3D.h \
    $$PWD/QScrollEngine/QDrawObject3D.h \
    $$PWD/QScrollEngine/QEntity.h \
    $$PWD/QScrollEngine/QFileLoad3DS.h \
    $$PWD/QScrollEngine/QFrustum.h \
    $$PWD/QScrollEngine/QGLPrimitiv.h \
    $$PWD/QScrollEngine/QLight.h \
    $$PWD/QScrollEngine/QMesh.h \
    $$PWD/QScrollEngine/QOtherMathFunctions.h \
    $$PWD/QScrollEngine/QScene.h \
    $$PWD/QScrollEngine/QSceneObject3D.h \
    $$PWD/QScrollEngine/QScrollEngine.h \
    $$PWD/QScrollEngine/QShObject3D.h \
    $$PWD/QScrollEngine/QSpotLight.h \
    $$PWD/QScrollEngine/QSprite.h \
    $$PWD/QScrollEngine/Shaders/QSh.h \
    $$PWD/QScrollEngine/Shaders/QSh__Bloom.h \
    $$PWD/QScrollEngine/Shaders/QSh__BloomMap.h \
    $$PWD/QScrollEngine/Shaders/QSh__Blur.h \
    $$PWD/QScrollEngine/Shaders/QSh_All.h \
    $$PWD/QScrollEngine/Shaders/QSh_Color.h \
    $$PWD/QScrollEngine/Shaders/QSh_ColoredPart.h \
    $$PWD/QScrollEngine/Shaders/QSh_Light.h \
    $$PWD/QScrollEngine/Shaders/QSh_Texture1.h \
    $$PWD/QScrollEngine/Tools/QIsoSurface.h \
    $$PWD/QScrollEngine/QFileSaveLoad3DS.h \
    $$PWD/QScrollEngine/QScrollEngineContext.h \
    $$PWD/QScrollEngine/QScrollEngineWidget.h \
    $$PWD/QScrollEngine/Tools/PlanarShadows.h \
    $$PWD/QScrollEngine/Shaders/QSh_Refraction_FallOff.h \
    $$PWD/QScrollEngine/Shaders/QSh_Sphere_Light.h \
    $$PWD/QScrollEngine/Shaders/QSh_Sphere_Texture1.h

INCLUDEPATH += .

RESOURCES += \
    $$PWD/QScrollEngine/shaders.qrc
