CONFIG += c++11

SOURCES += \
    $$PWD/Camera.cpp \
    $$PWD/ImageProcessing.cpp \
    $$PWD/faster_corner_10.cxx \
    $$PWD/OpticalFlow.cpp \
    $$PWD/OpticalFlowCalculator.cpp \
    $$PWD/FeatureDetector.cpp \
    $$PWD/FastCorner.cpp \
    $$PWD/Frame.cpp \
    $$PWD/PerformanceMonitor.cpp \
    $$PWD/RotationTracker.cpp

HEADERS += \
    $$PWD/Camera.h \
    $$PWD/Image.h \
    $$PWD/ImageProcessing.h \
    $$PWD/OpticalFlow.h \
    $$PWD/OpticalFlowCalculator.h \
    $$PWD/FeatureDetector.h \
    $$PWD/Point2.h \
    $$PWD/Painter.h \
    $$PWD/FastCorner.h \ \
    $$PWD/TukeyRobustCost.h \
    $$PWD/Frame.h \
    $$PWD/ImageProcessing.h \
    $$PWD/Camera.h \
    $$PWD/Image.h \
    $$PWD/ZMSSD.h \
    $$PWD/PerformanceMonitor.h \
    $$PWD/RotationTracker.h

