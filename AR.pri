QT       += multimedia
CONFIG += c++11

SOURCES += \
    $$PWD/AR/ARSystem.cpp \
    $$PWD/AR/Image.cpp \
    $$PWD/AR/faster_corner_10.cxx \
    $$PWD/AR/FastFeature.cpp \
    $$PWD/AR/ImageProcessing.cpp \  
    $$PWD/AR/TMath.cpp \
    $$PWD/AR/HomographyInitializer.cpp \
    $$PWD/AR/OF_ImageLevel.cpp \
    $$PWD/AR/OF_System.cpp \
    $$PWD/AR/Map.cpp \
    $$PWD/AR/Pose3DEstimator.cpp \
    $$PWD/AR/Reconstructor3D.cpp \
    $$PWD/AR/FrameProvider.cpp 

HEADERS += \
    $$PWD/AR/ARSystem.h \
    $$PWD/AR/Image.h \
    $$PWD/AR/FastFeature.h \
    $$PWD/AR/ImageProcessing.h \
    $$PWD/AR/TMath.h \
    $$PWD/AR/TSVD.h \
    $$PWD/AR/TCholesky.h \
    $$PWD/AR/TWLS.h \
    $$PWD/AR/OtherEstimators.h \
    $$PWD/AR/HomographyInitializer.h \
    $$PWD/AR/OF_ImageLevel.h \
    $$PWD/AR/OF_System.h \
    $$PWD/AR/Map.h \
    $$PWD/AR/Pose3DEstimator.h \
    $$PWD/AR/Reconstructor3D.h \
    $$PWD/AR/FrameProvider.h 

INCLUDEPATH += .
