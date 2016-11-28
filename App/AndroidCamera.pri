android: {
    QT += widgets opengl
    QT += multimedia
    QT += androidextras

    CONFIG += c++11

    #DEFINES += QT_OPENGL_ES_3

    #LIBS += -L$$NDK/platforms/android-19/arch-arm/usr/lib/ -lGLESv3
	
    SOURCES += \
        AndroidCamera/AndroidJniCamera.cpp \
        AndroidCamera/AndroidCameraView.cpp \
        AndroidCamera/AndroidJniSurfaceTexture.cpp \
        AndroidCamera/PixelsReader.cpp

    HEADERS += \
        AndroidCamera/AndroidJniCamera.h \
        AndroidCamera/AndroidJniSurfaceTexture.h \
        AndroidCamera/AndroidCameraView.h \
        AndroidCamera/PixelsReader.h

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/AndroidCamera/Java

    OTHER_FILES += \
        AndroidCamera/Java/src/ru/qtproject/artechproject86/ARCameraListener.java \
        AndroidCamera/Java/src/ru/qtproject/artechproject86/ARSurfaceTextureHolder.java \
        AndroidCamera/Java/src/ru/qtproject/artechproject86/ARSurfaceTextureListener.java \
        AndroidCamera/Java/AndroidManifest.xml

}
