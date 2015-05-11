#ifndef ARSYSTEM_H
#define ARSYSTEM_H

#include "QScrollEngine/QCamera3D.h"

#include <QtMath>
#include <QImage>
#include <QVector3D>
#include <QSize>

#include <vector>
#include <vector>
#include <utility>

#include <AR/Image.h>
#include "AR/HomographyInitializer.h"
#include "AR/Pose3DEstimator.h"
#include "AR/OF_System.h"
#include "AR/Map.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QFrustum.h"
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include "AR/FrameProvider.h"

namespace AR  {

class ColorConverter;
class Reconstructor3D;

class ARSystem
{
    friend class ColorConverter;

public:
    typedef struct TrackingParameters
    {
        IPoint resolution;
        IPoint sizeCursor;
        int countLevels;
        int checkLevel;
        int stepOfLevelOfFeature;
        int beginLevelForFeature;
        int endLevelForFeature;
        int countTrackingPoints;
        int sizeBlur;
        int maxCountIterationForTrackingPoints;
        float maxLocalVelocity;
        float epsDeltaStep;
        float detThreshold;
        float errorThreshold;
        int barrierOfCorners;
        float shiTomasiScoreThreshold;
        float minSquareDistancePointsBig;
        float minSquareDistancePointsSmall;
        float cutOffProjections_mapPoint;
        float distanceSquare_mapPoint;
        int countProjections_newMapPoint;
        int countProjections_oldMapPoint;
        float cutOff_nextFrame;
        float distanceSquared_nextFrame;
        float limitAvgErrorSquared;
        float limitMaxErrorSquared;
        IPoint sizeImageOfMapPoint;
        IPoint sizeBorder;
        IPoint sizeSmallImage;
        float limitErrorForGoodTracking;
        float cutOff_record;
        float distanceSquare_record;

        TrackingParameters()
        {
            resolution.set(480, 360);
            sizeCursor.set(10, 10);
            countLevels = 5;
            checkLevel = 1;
            stepOfLevelOfFeature = 2;
            beginLevelForFeature = 1;
            endLevelForFeature = 3;
            countTrackingPoints = 40;
            sizeBlur = 2;
            maxCountIterationForTrackingPoints = 15;
            maxLocalVelocity = 10.0f;
            epsDeltaStep = 0.01f;
            detThreshold = 0.1f;
            errorThreshold = 0.5f;
            barrierOfCorners = 5;
            shiTomasiScoreThreshold = 200.0f;
            minSquareDistancePointsBig = 15.0f * 15.0f;
            minSquareDistancePointsSmall = 7.0f * 7.0f;
            cutOffProjections_mapPoint = qCos(4.0f * (M_PI / 180.0f));
            distanceSquare_mapPoint = 1.0f * 1.0f;
            countProjections_newMapPoint = 4;
            countProjections_oldMapPoint = 2;
            cutOff_nextFrame = qCos(7.0f * (M_PI / 180.0f));
            distanceSquared_nextFrame = 3.0f * 3.0f;
            sizeSmallImage.set(80, 60);
            limitAvgErrorSquared = 0.015f;
            limitMaxErrorSquared = 0.03f;
            sizeImageOfMapPoint.set(7, 7);
            sizeBorder.set(10, 10);
            limitErrorForGoodTracking = 0.4f;
            cutOff_record = qCos(3.0f * (M_PI / 180.0f));
            distanceSquare_record = 0.3f * 0.3f;
        }
    } TrackingParameters;

    enum State: int
    {
        NotTracking,
        TrackingBegin,
        TrackingNow,
        Reconstruction3D,
        LostTracking,
        LostReconstruction3D
        //ViewDepthMap
    };

public:
    ARSystem();
    ~ARSystem();
    TrackingParameters trackingParameters() const { return _parameters; }
    void setTrackingParameters(const TrackingParameters& parameters);
    const Reconstructor3D* reconstructor3D() const { return _reconstructor3D; }
    Reconstructor3D* reconstructor3D() { return _reconstructor3D; }

    void drawResultOnGL(QScrollEngine::QScrollEngineContext* context, const QMatrix2x2& transform);

    void tracking(Image<Rgba>& sourceImage, Image<uchar>& sourceBWImage);
    void tracking(FrameProvider* frameProvider, QOpenGLFunctions* glFunctions,
                  QOpenGLFramebufferObject* FBO_color, QOpenGLFramebufferObject* FBO_luminance);
    int countTrackingPoints() const { return ((_map.countKeyFrames() > 0) ?
                                                  static_cast<int>(_map.lastKeyFrame().projectedPoints.size()) : 0); }

    void reset();

    State state() const { return _state; }
    void nextState();
    void research();

    const QMatrix4x4 matrixWorld() const { if (_map.countKeyFrames() > 0) return _map.lastKeyFrame().matrixWorld; return QMatrix4x4(); }
    const Map& map() const { return _map; }

    void setCameraParameters(const QScrollEngine::QCamera3D* camera3d, bool invertX, bool invertY)
    {
        IPointF focalLength, opticalCenter;
        Pose3DEstimator::getCameraParameters(focalLength, opticalCenter, camera3d->matrixProj());
        setCameraParameters(focalLength, opticalCenter, invertX, invertY);
    }
    inline void setCameraParameters(const IPointF& focalLength, const IPointF& opticalCenter, bool invertX, bool invertY)
    {
        _pose3DEstimator.setCameraParameters(_bwImage.size(), focalLength, opticalCenter, invertX, invertY);
        _matrixProj.perspective(_pose3DEstimator.getVerticalAngle(), _bwImage.width() / static_cast<float>(_bwImage.height()), 1.0f, 100.0f);
    }
    inline void setLensDistortionParameters(int index, float P, float K)
    {
        _pose3DEstimator.setLensDistortionParameters(index, P, K);
    }
    inline float lensDistortionP(int index) { return _pose3DEstimator.lensDistortionP(index); }
    inline float lensDistortionK(int index) { return _pose3DEstimator.lensDistortionK(index); }
    inline bool invertX() const { return _pose3DEstimator.invertX(); }
    inline bool invertY() const { return _pose3DEstimator.invertY(); }
    inline IPointF invPixelFocalLength() const { return _pose3DEstimator.invPixelFocalLength(); }
    inline IPointF pixelOpticalCenter() const { return _pose3DEstimator.pixelOpticalCenter(); }
    inline float getHorizontalAngle() const { return _pose3DEstimator.getHorizontalAngle(); }
    inline float getVerticalAngle() const { return _pose3DEstimator.getVerticalAngle(); }

    QScrollEngine::QEntity* entity();
    void setEntity(QScrollEngine::QEntity* entity);
    QScrollEngine::QScrollEngineContext* context();
    void setContext(QScrollEngine::QScrollEngineContext* context);

    IPoint resolution() { return _parameters.resolution; }
protected:
    typedef struct InfoForNewMapPoint {
        std::vector<Pose3DEstimator::ProjectionPoint> projections;
        QVector3D lastDir;
        QVector3D lastPos;
    } InfoForNewMapPoint;

    typedef struct InfoForOldMapPoint {
        int indexMapPoint;
        int countOfVisibles;
        QVector3D lastDir;
        QVector3D lastPos;
    } InfoForOldMapPoint;

    TrackingParameters _parameters;
    int _countItersForSmallImage;

    Image<uchar> _bwImage;
    Image<int> _integralImage;

    HomographyInitializer _homographyInitializer;
    Pose3DEstimator _pose3DEstimator;
    Map _map;
    Reconstructor3D* _reconstructor3D;

    QVector3D _currentFrameBeginDir;
    QVector3D _currentFrameBeginPos;

    std::vector<InfoForNewMapPoint> _infoAddedNewPoints;
    std::vector<IPointF> _currentAddedNewPoints;
    std::vector<InfoForOldMapPoint> _infoAddedOldPoints;
    std::vector<IPointF> _currentAddedOldPoints;
    OF_System _opticalFlow;

    QMatrix4x4 _matrixProj;
    QScrollEngine::QFrustum _frustum;
    volatile State _state;
    //int iter;

    void _scaleAndBlurImage(Image<uchar>& sourceBWImage);
    float _trackingPose();
    void _trackingAddedNewPoints(bool goodTracking);
    void _trackingAddedOldPoints(bool goodTracking);
    void _addNewPointsToVector(int maxSize);
    void _addOldPointsToVector(int maxSize);
    Image<uchar> _createSmallImage() const;
    void _createNextKeyFrame();
    void _clearCurrentTracking();
    int _findKeyFrame();
};

}

#endif // ARSYSTEM_H
