#ifndef RECONSTRUCTOR3D_H
#define RECONSTRUCTOR3D_H

#include <vector>
#include <QThread>
#include "AR/Image.h"
#include "AR/OF_System.h"
#include "AR/Pose3DEstimator.h"
#include "AR/HomographyInitializer.h"
#include "AR/Map.h"
#include "AR/ARSystem.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/Tools/QIsoSurface.h"

namespace AR {

class Reconstructor3D : public QThread
{
    Q_OBJECT
public:
    typedef struct ReconstructParameters
    {
        IPoint sizeDepthMap;
        IPoint sizeCursor;
        IPoint sizeBorder;
        int countLevels;
        int checkLevel;
        int maxCountIterationForTrackingPoints;
        float maxLocalVelocity;
        float epsDeltaStep;
        float detThreshold;
        float errorThreshold;
        float minSquareDistancePointsBig;
        float minSquareDistancePointsSmall;
        float cutOff_nextFrame;
        float distanceSquare_nextFrame;
        int minCountFrames;
        int maxCountFrames;
        QVector3D startPointOfReconstructRegion;
        QVector3D endPointOfReconstructRegion;
        float cellSizeOfReconstruct;
        float limitMaxErrorSquared;

        ReconstructParameters()
        {
            sizeDepthMap.set(160, 120);
            sizeCursor.set(10, 10);
            sizeBorder.set(30, 30);
            countLevels = 5;
            checkLevel = 0;
            maxCountIterationForTrackingPoints = 15;
            maxLocalVelocity = 10.0f;
            epsDeltaStep = 0.01f;
            detThreshold = 0.5f;
            errorThreshold = 0.5f;
            minSquareDistancePointsBig = 1.0f * 1.0f;
            minSquareDistancePointsSmall = 0.1f * 0.1f;
            cutOff_nextFrame = qCos(15.0f * (M_PI / 180.0f));
            distanceSquare_nextFrame = 4.0f * 4.0f;
            minCountFrames = 3;
            maxCountFrames = 6;
            startPointOfReconstructRegion = QVector3D(-7.0f, -7.0f, -7.0f);
            endPointOfReconstructRegion = QVector3D(7.0f, 7.0f, 7.0f);
            cellSizeOfReconstruct = 0.1f;
            limitMaxErrorSquared = 0.03f;
        }
    } ReconstructParameters;

    typedef struct ReconstructPoint
    {
        std::vector<Pose3DEstimator::ProjectionPoint> projections;
    } ReconstructPoint;

    typedef struct ReconstructFrame
    {
        Image<Rgba> colorImage;
        Image<float> depthMap;
        QMatrix4x4 matrixWorld;
    } ReconstructFrame;

    class ReconstructField:
            public QScrollEngine::QIsoSurface::ScalarField
    {
    public:
        ReconstructField();
        ~ReconstructField();

        float value(const QVector3D& point);

        void setDepthMapSize(const IPoint& size) { _depthMapSize = size; }
        IPoint depthMapSize() const { return _depthMapSize; }
        void setPixelFocalLength(const IPointF& pixelFocalLength) { _pixelFocalLength = pixelFocalLength; }
        IPointF pixelFocalLength() const { return _pixelFocalLength; }
        void setPixelOpticalCenter(const IPointF& pixelOpticalCenter) { _pixelOpticalCenter = pixelOpticalCenter; }
        IPointF pixelOpticalCenter() const { return _pixelOpticalCenter; }
        inline bool getLocalPointInDepthMap(IPointF& mapCoord, float& localZ, const QMatrix4x4& matrixWorld, const QVector3D& point) const;
        inline static float getSubPixelValue(const Image<float>& depthMap, const IPointF& point);
        void setReconstructFrames(const std::vector<ReconstructFrame>* frames) { _frames = frames; _vectorOfDeltaZ.reserve(frames->size());}
        const std::vector<ReconstructFrame>* reconstructFrames() const { return _frames; }
        float blurDeltaZ() const { return _blurDeltaZ; }
        void setBlurDeltaZ(float delta) { _blurDeltaZ = delta; }

    private:
        const std::vector<ReconstructFrame>* _frames;
        std::vector<float> _vectorOfDeltaZ;
        float _blurDeltaZ;
        IPoint _depthMapSize;
        IPointF _pixelOpticalCenter;
        IPointF _pixelFocalLength;
    };

    class DepthMapProcessor
    {
    public:
        DepthMapProcessor();
        void setDepthMap(const Image<float>& depthMap) { _depthMap = depthMap; }
        Image<float> depthMap() { return _depthMap; }
        int countIterations() const { return _countIterations; }
        void setCountIterations(int count) { _countIterations = count; }
        int minCountNeighbors() const { return _minArea; }
        void setMinCountNeighbors(int count) { _minArea = count; }
        float maxDetlaZ() const { return _maxDeltaZ; }
        void setMaxDeltaZ(float delta) { _maxDeltaZ = delta; }
        IPoint sizeCursor() const { return _sizeCursor; }
        void setSizeCursor(const IPoint& sizeCursor) { _sizeCursor = sizeCursor; }
        float blurSigma() const { return std::sqrt(_blurSigmaSquared); }
        void setBlurSigma(float sigma) { _blurSigmaSquared = sigma * sigma; }
        float blurDeltaZ() const { return _blurDeltaZ; }
        void setBlurDeltaZ(float deltaZ) { _blurDeltaZ = deltaZ; }
        void process();

    private:
        typedef struct Info
        {
            IPointF d;
            int iteration;
        } Info;

        Image<float> _depthMap;
        Image<Info> _infoImage;
        int _countIterations;
        int _iteration;
        IPoint _sizeCursor;
        int _minArea;
        float _maxDeltaZ;
        float _blurSigmaSquared;
        float _blurDeltaZ;

        void _clearNoise();
        void _calculateInfoImage();
        void _fillElement(const IPoint& c);
        float _getWeight(const IPointF& delta, const IPointF& diff) const
        {
            float l = delta.lengthSquared();
            if (l < 0.0001f)
                return 1000.0f;
            //return 1.0f / l;
            if (diff.x < 1.0f) {
                float disA = std::fabs(delta.x * diff.x + delta.y * diff.y);
                float disB = std::sqrt(l + disA * disA);
                return std::max(1.0f / (disA + disB), 1000.0f);
            }
            return std::max(2.0f / std::sqrt(l), 1000.0f);
        }
        bool _updateMinMax(float& min, float& max, const float& value) const
        {
            if (value <= 0.0f)
                return false;
            if (value < min)
                min = value;
            if (value > max)
                max = value;
            return true;
        }
        inline float _getBluredPoint(const Image<float>& depthMap, float kernelValue, const IPoint& point,
                                     const float& k1, const float& invK2);
        void _blurMap();
    };

public:
    Reconstructor3D(QObject* parent = 0);
    //std::vector<Image<Rgb>> ss;

    void setMap(Map* map) { _map = map; }
    void setPose3DEstimator(const Pose3DEstimator& pose3DEstimator);
    void setParameters(const ReconstructParameters& parameters);
    void setContext(QScrollEngine::QScrollEngineContext* context) { _context = context; }
    QScrollEngine::QScrollEngineContext* context() { return _context; }

    void setEntity(QScrollEngine::QEntity* entity) { _entity = entity; }
    QScrollEngine::QEntity* entity() { return _entity; }

    void run();
    Image<Rgb> depthMap() const { return _depthMapToImageRgb(_reconstructFrames[0].depthMap, 1.0f, 12.0f); }

    bool rgbSwap() const { return _rgbSwap; }
    void setRgbSwap(bool enable) { _rgbSwap = enable; }

    void createFinishEntity();

signals:
    void updateProgress(float progress);

protected:
    ReconstructParameters _parameters;
    Map* _map;
    Pose3DEstimator _pose3DEstimator;
    IPointF _opticalCenter;
    IPointF _focalLength;
    HomographyInitializer _homographyInitializer;
    OF_System _opticalFlow;
    std::vector<Image<IPointF>> _diffPyramid;
    std::vector<ReconstructFrame> _reconstructFrames;
    Image<ReconstructPoint> _reconstructImage;
    bool _rgbSwap;
    std::vector<QVector3D> _vertices;
    std::vector<QVector3D> _normals;
    std::vector<GLuint> _triangles;

    QScrollEngine::QIsoSurface _isoSurface;
    ReconstructField _reconstructField;
    DepthMapProcessor _depthMapProcessor;
    QScrollEngine::QScrollEngineContext* _context;
    QScrollEngine::QEntity* _entity;
    void _createCurrentReconstructFrame(Map::RecordFrame& recordFrame);
    void _solveCurrentRecordFrame(Map::RecordFrame& recordFrame);
    void _finishSolveReconstructFrame(ReconstructFrame& reconstructFrame);
    void _pointToFrames(const ReconstructPoint& point, ReconstructFrame& frame);
    void _createReconstructEntity(const std::vector<QVector3D>& vertices, const std::vector<QVector3D>& normals,
                                  const std::vector<GLuint>& triangles);
    Image<Rgb> _depthMapToImageRgb(const Image<float>& depthMap, float nearDistance, float farDistance) const;
    Image<Rgb> _bwToRgb(const Image<uchar>& image) const;
};

}

#endif // RECONSTRUCTOR3D_H
