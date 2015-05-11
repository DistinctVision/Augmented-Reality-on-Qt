#ifndef POSE3DESTIMATOR_H
#define POSE3DESTIMATOR_H

#include <vector>
#include <QVector3D>
#include <QMatrix4x4>

#include "AR/Image.h"
#include "AR/TMath.h"
#include "AR/TSVD.h"
#include "AR/TWLS.h"

#include <vector>
#include <cassert>

namespace AR {

class Pose3DEstimator
{
public:
    typedef struct ProjectionPoint {
        IPointF projection;
        QMatrix4x4 matrixWorld;

        ProjectionPoint() {}
        ProjectionPoint(const IPointF& point, const QMatrix4x4& matrix):projection(point), matrixWorld(matrix) {}
    } ProjectionPoint;

public:
    Pose3DEstimator();
    ~Pose3DEstimator();

    int countPoints() const { return _points.size(); }
    IPointF imagePoint(unsigned int index) const { return _pointsUV[index]; }
    void resetBeginTransform();
    void setBeginTransform(const QMatrix4x4& matrixWorld);
    void beginEstimatePose();
    void estimatePose();
    void addPoint(const QVector3D& worldPoint, const IPointF& imagePoint, float weight);
    static void getCameraParameters(IPointF& focalLength, IPointF& opticalCenter, const QMatrix4x4& matrixProjection)
    {
        opticalCenter.set(0.5f, 0.5f);
        focalLength.x = - ((matrixProjection(0, 0) * opticalCenter.x) / matrixProjection(3, 2));
        focalLength.y = - ((matrixProjection(1, 1) * opticalCenter.y) / matrixProjection(3, 2));
    }
    void setCameraParameters(const IPoint& screenResolution, const IPointF& focalLength, const IPointF& opticalCenter,
                                    bool invertX, bool invertY)
    {
        _screenResolution = screenResolution;
        IPointF pixelFocalLength(_screenResolution.x * focalLength.x * (invertX ? - 1.0f : 1.0f),
                                 _screenResolution.y * focalLength.y * (invertY ? - 1.0f : 1.0f));
        _pixelOpticalCenter.set(_screenResolution.x * opticalCenter.x, _screenResolution.y * opticalCenter.y);
        _invPixelFocalLength.x = 1.0f / pixelFocalLength.x;
        _invPixelFocalLength.y = 1.0f / pixelFocalLength.y;
    }
    void setLensDistortionParameters(int index, float P, float K) { _distortion_P[index] = P; _distortion_K[index] = K; }
    float lensDistortionP(int index) { return _distortion_P[index]; }
    float lensDistortionK(int index) { return _distortion_K[index]; }
    bool invertX() const { return (_invPixelFocalLength.x < 0.0f); }
    bool invertY() const { return (_invPixelFocalLength.y < 0.0f); }
    IPointF invPixelFocalLength() const { return _invPixelFocalLength; }
    IPointF pixelOpticalCenter() const { return _pixelOpticalCenter; }
    IPoint screenResolution() const { return _screenResolution; }
    IPointF getFocalLength() const
    {
        return IPointF(1.0f / (_invPixelFocalLength.x * _screenResolution.x),
                       1.0f / (_invPixelFocalLength.y * _screenResolution.y));
    }
    IPointF getOpticalCenter() const
    {
        return IPointF(_pixelOpticalCenter.x / static_cast<float>(_screenResolution.x),
                       _pixelOpticalCenter.y / static_cast<float>(_screenResolution.y));
    }
    IPointF getPixelFocalLengthForResolution(const IPoint& resolution) const
    {
        IPointF focalLength = getFocalLength();
        return IPointF(focalLength.x * resolution.x, focalLength.y * resolution.y);
    }
    IPointF getPixelOpticalCenterForResolution(const IPoint& resolution) const
    {
        IPointF opticalCenter = getOpticalCenter();
        return IPointF(opticalCenter.x * resolution.x, opticalCenter.y * resolution.y);
    }
    IPointF unprojectPoint(const IPointF& point) const
    {
        return IPointF((point.x - _pixelOpticalCenter.x) * _invPixelFocalLength.x,
                       (point.y - _pixelOpticalCenter.y) * _invPixelFocalLength.y);
    }
    IPointF projectPoint(const IPointF& point) const
    {
        return IPointF(point.x / _invPixelFocalLength.x + _pixelOpticalCenter.x,
                       point.y / _invPixelFocalLength.y + _pixelOpticalCenter.y);
    }
    IPointF undistorsionPoint(IPointF point) const
    {
        assert(false);
        for (int i=0; i<10; ++i) {
            float xx = point.x * point.x;
            float yy = point.y * point.y;
            float r2 = xx + yy;
            float cdist = 1.0f + (_distortion_K[1] * r2 + _distortion_K[0]) * r2;
            float t = 2.0f * point.x * point.y;
            float deltaX = t * _distortion_P[0] + (r2 + 2.0f * xx) * _distortion_P[1];
            float deltaY = (r2 + 2.0f * yy) * _distortion_P[0] + t * _distortion_P[1];
            point.x = (point.x - deltaX) / cdist;
            point.y = (point.y - deltaY) / cdist;
        }
        return point;
    }
    IPointF distorsionPoint(IPointF point) const
    {
        Q_UNUSED(point);
        assert(false);
        return IPointF();
    }
    bool pointInWindow(const IPointF& point) const
    {
        if (point.x < 0.0f)
            return false;
        if (point.y < 0.0f)
            return false;
        if (point.x >= _screenResolution.x)
            return false;
        if (point.y >= _screenResolution.y)
            return false;
        return true;
    }
    float epsilon() const { return _stop_epsilon; }
    void setEpsilon(float epsilon) { TMath_assert(epsilon >= FLT_EPSILON); _stop_epsilon = epsilon; }
    bool isCoplanar() const { return _isCoplanar; }
    QVector3D planeLocalX() const { return _planeLocalX; }
    QVector3D planeLocalY() const { return _planeLocalY; }
    QVector3D planeLocalZ() const { return _planeLocalZ; }
    QVector3D planePos() const { return _planePos; }

    TMath::TMatrix<float> cameraProjection() const
    {
        TMath::TMatrix<float> camProj(3, 3);
        IPointF pixelFocalLength(1.0f / _invPixelFocalLength.x, 1.0f / _invPixelFocalLength.y);
        camProj(0, 0) = pixelFocalLength.x;
        camProj(0, 1) = 0.0f;
        camProj(0, 2) = - _pixelOpticalCenter.x;
        camProj(1, 0) = 0.0f;
        camProj(1, 1) = pixelFocalLength.y;
        camProj(1, 2) = - _pixelOpticalCenter.y;
        camProj(2, 0) = 0.0f;
        camProj(2, 1) = 0.0f;
        camProj(2, 2) = - 1.0f;
        return camProj;
    }
    void getResultMatrixWorld(QMatrix4x4& matrixWorld) const;
    float getCurrentErrorSquared(int indexPoint) const;

    bool getPointInWorld(QVector3D& result, const std::vector<ProjectionPoint>& projections);
    float getErrorSquared(const QMatrix4x4& matrixWorld, const QVector3D& worldPoint, const IPointF& projection) const;
    float getSumErrorSquared(const QVector3D& worldPoint, const std::vector<ProjectionPoint>& projections) const;
    QVector3D getDirFromScreen(const QMatrix4x4& matrixWorld, const IPointF& point) const;
    float getHorizontalAngle() const { return qAtan(qAbs(_pixelOpticalCenter.x * _invPixelFocalLength.x)) * 2.0f; }
    float getVerticalAngle() const { return qAtan(qAbs(_pixelOpticalCenter.y * _invPixelFocalLength.y)) * 2.0f; }

protected:
    float _stop_epsilon;

    IPoint _screenResolution;
    IPointF _invPixelFocalLength;
    IPointF _pixelOpticalCenter;
    float _distortion_K[2];
    float _distortion_P[2];

    std::vector<IPointF> _pointsUV;
    std::vector<QVector4D> _points;

    bool _isCoplanar;
    QVector3D _planeLocalX;
    QVector3D _planeLocalY;
    QVector3D _planeLocalZ;
    QVector3D _planePos;

    TMath::TSVD<float> _svd;
    TMath::TWLS<float> _wls;

    TMath::TMatrix<float> _transform;


    inline QVector3D _transformPoint(const QVector4D& point) const
    {
        return QVector3D(
            _transform(0, 0) * point.x() + _transform(0, 1) * point.y() + _transform(0, 2) * point.z() + _transform(0, 3),
            _transform(1, 0) * point.x() + _transform(1, 1) * point.y() + _transform(1, 2) * point.z() + _transform(1, 3),
            _transform(2, 0) * point.x() + _transform(2, 1) * point.y() + _transform(2, 2) * point.z() + _transform(2, 3));
    }
    inline IPointF _projectToUV(const QVector3D& point) const
    {
        return IPointF(point.x() / point.z(), point.y() / point.z());
    }
    void _getCoordSystemFromPlaneCoords(QVector3D& nX, QVector3D& nY, QVector3D& nZ, QVector3D& pos) const;
    void _getPlaneCoords();
    void _computeFirstPoseForCoplanarPoints();
    void _computeFirstPoseForNoncoplanarPoints();
    void _refinePose();
};

}

#endif // POSE3DESTIMATION_H
