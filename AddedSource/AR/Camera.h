#ifndef AR_CAMERA_H
#define AR_CAMERA_H

#include "AR/Image.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"

#include <cmath>

namespace AR {

// The parameters are:
// 0 - normalized x focal length
// 1 - normalized y focal length
// 2 - normalized x offset
// 3 - normalized y offset
// 4 - w (distortion parameter)

class Camera
{
public:
    struct ProjectionInfo {
        Point2d camPoint;          // Last z=1 coord
        Point2d imagePoint;        // Last image / UFB coord
        Point2d distortedCamPoint; // Last distorted z=1 coord
        double radius;             // Last z=1 radius
        double distortedRadius;    // Last z=1 distorted radius
        double factor;             // Last ratio of z=1 radii
        bool invalid;
    };

public:
    Camera();
    Camera(const TMath::TVectord& cameraParameters, const Point2d& imageSize);

    // Image size get/set: updates the internal projection params to that target image size.
    void setImageSize(const Point2d& imageSize);
    void setImageSize(const Point2i& imageSize);
    Point2d imageSize() const;
    double imageWidth() const;
    double imageHeight() const;

    void setCameraParameters(const TMath::TVectord& cameraParameters);
    void setParameters(const TMath::TVectord& cameraParameters, const Point2d& imageSize);
    TMath::TVectord cameraParameters() const;

    // Various projection functions
    // Projects from camera z=1 plane to pixel coordinates, with radial distortion
    Point2d project(const Point2d& camPoint) const;
    Point2d project(const Point2f& camPoint) const;
    Point2d project(const Point2i& camPoint) const;
    Point2d project(const Point2d& camPoint, ProjectionInfo& projectionInfo) const;
    Point2d project(const Point2f& camPoint, ProjectionInfo& projectionInfo) const;
    Point2d project(const Point2i& camPoint, ProjectionInfo& projectionInfo) const;
    // Inverse operation
    Point2d unproject(const Point2d& imagePoint) const;
    Point2d unproject(const Point2f& imagePoint) const;
    Point2d unproject(const Point2i& imagePoint) const;
    Point2d unproject(const Point2d& imagePoint, ProjectionInfo& projectionInfo) const;
    Point2d unproject(const Point2f& imagePoint, ProjectionInfo& projectionInfo) const;
    Point2d unproject(const Point2i& imagePoint, ProjectionInfo& projectionInfo) const;

    Point2d distort(const Point2d& point) const;
    Point2d undistort(const Point2d& point) const;

    TMath::TMatrixd getProjectionDerivatives(const ProjectionInfo& projectionInfo) const; // Projection jacobian
    TMath::TMatrixd getCameraParametersDerivatives(const Point2d& camPoint) const;
    TMath::TMatrixd getCameraParametersDerivatives(const Point2d& camPoint, const TMath::TVector<bool>& fixedCameraParamters) const;

    double largestRadiusInImage() const;
    double onePixelDist() const;

    double pixelAspectRatio();

    bool radialDistortionIsEnabled() const;
    void disableRadialDistortion();

    Point2d pixelCenter() const;
    Point2d pixelFocalLength() const;

    // Useful for gvar-related reasons (in case some external func tries to read the camera params gvar, and needs some defaults.)
    static const TMath::TVectord defaultCameraParameters;

private:
    friend class CameraCalibrator;   // friend declarations allow access to calibration jacobian and camera update function.
    friend class CalibImage;

    TMath::TVectord m_cameraParameters;

    // Cached from last RefreshParams:
    double m_largestRadius;      // Largest R in the image
    double m_maxRadius;          // Largest R for which we consider projection valid
    double m_onePixelDist;       // z=1 distance covered by a single pixel offset (a rough estimate!)
    double m_2Tan;               // distortion model coeff
    double m_oneOver2Tan;        // distortion model coeff
    double m_W;                  // distortion model coeff
    double m_invW;               // distortion model coeff
    double m_distortionEnabled;  // One or zero depending on if distortion is on or off.
    Point2d m_pixelCenter;        // Pixel projection center
    Point2d m_pixelFocal;         // Pixel focal length
    Point2d m_invPixelFocal;      // Inverse pixel focal length
    Point2d m_imageSize;
    Point2d m_UFBLinearFocal;
    Point2d m_UFBLinearInvFocal;
    Point2d m_UFBLinearCenter;
    Point2d m_implaneTopLeft;
    Point2d m_implaneBottomRight;

    void _refreshCameraParameters();

protected:

    // Radial distortion transformation factor: returns ration of distorted / undistorted radius.
    inline double _rtrans_factor(double r) const
    {
        return ((r < 0.001) || (m_W == 0.0)) ? 1.0 : (m_invW * std::atan(r * m_2Tan) / r);
    }

    // Inverse radial distortion: returns un-distorted radius from distorted.
    inline double _inv_rtrans_factor(double r) const
    {
        return (m_W == 0.0) ? r : (std::tan(r * m_W) * m_oneOver2Tan);
    }
};

} // namespace AR

#endif // AR_CAMERA_H
