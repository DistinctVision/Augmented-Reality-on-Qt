#include "Camera.h"
#include <TMath/TMath.h>
#include <vector>

namespace AR {

const TMath::TVectord Camera::defaultCameraParameters = TMath::TVectord::create(1.0, 1.33, 0.5, 0.5, 0.0);

Camera::Camera()
{
    m_cameraParameters = defaultCameraParameters;
    m_imageSize.set(640.0f, 480.0f);
    _refreshCameraParameters();
}

Camera::Camera(const TMath::TVectord& cameraParameters, const Point2d& imageSize)
{
    setParameters(cameraParameters, imageSize);
}

void Camera::setImageSize(const Point2i& imageSize)
{
    setImageSize(Point2d((double)imageSize.x, (double)imageSize.y));
}

void Camera::setImageSize(const Point2d& imageSize)
{
    m_imageSize = imageSize;
    _refreshCameraParameters();
}

Point2d Camera::imageSize() const
{
     return m_imageSize;
}

double Camera::imageWidth() const
{
    return m_imageSize.x;
}

double Camera::imageHeight() const
{
    return m_imageSize.y;
}

void Camera::setCameraParameters(const TMath::TVectord& cameraParameters)
{
    TMath_assert(cameraParameters.size() == 5);
    m_cameraParameters = cameraParameters;
    _refreshCameraParameters();
}

void Camera::setParameters(const TMath::TVectord& cameraParameters, const Point2d& imageSize)
{
    TMath_assert(cameraParameters.size() == 5);
    m_imageSize = imageSize;
    m_cameraParameters = cameraParameters;
    _refreshCameraParameters();
}

TMath::TVectord Camera::cameraParameters() const
{
    return m_cameraParameters;
}

double Camera::largestRadiusInImage() const
{
    return m_largestRadius;
}

double Camera::onePixelDist() const
{
    return m_onePixelDist;
}

Point2d Camera::pixelCenter() const
{
    return m_pixelCenter;
}

Point2d Camera::pixelFocalLength() const
{
    return m_pixelFocal;
}


void Camera::_refreshCameraParameters()
{
    // This updates internal member variables according to the current camera parameters,
    // and the currently selected target image size.
    //

    // First: Focal length and image center in pixel coordinates
    m_pixelFocal.x = m_imageSize.x * m_cameraParameters(0);
    m_pixelFocal.y = m_imageSize.y * m_cameraParameters(1);
    m_pixelCenter.x = m_imageSize.x * m_cameraParameters(2) - 0.5;
    m_pixelCenter.y = m_imageSize.y * m_cameraParameters(3) - 0.5;

    // One over focal length
    m_invPixelFocal.x = 1.0 / m_pixelFocal.x;
    m_invPixelFocal.y = 1.0 / m_pixelFocal.y;

    // Some radial distortion parameters..
    m_W = m_cameraParameters(4);
    if (m_W != 0.0) {
        m_2Tan = 2.0 * std::tan(m_W * 0.5);
        m_oneOver2Tan = 1.0 / m_2Tan;
        m_invW = 1.0 / m_W;
        m_distortionEnabled = 1.0;
    } else {
        m_invW = 0.0;
        m_2Tan = 0.0;
        m_distortionEnabled = 0.0;
    }

    // work out biggest radius in image
    Point2d v;
    v.x = std::max(m_cameraParameters(2), 1.0 - m_cameraParameters(2)) / m_cameraParameters(0);
    v.y = std::max(m_cameraParameters(3), 1.0 - m_cameraParameters(3)) / m_cameraParameters(1);
    m_largestRadius = _inv_rtrans_factor(v.length());

    // At what stage does the model become invalid?
    m_maxRadius = 1.5 * m_largestRadius; // (pretty arbitrary)

    // work out world radius of one pixel
    // (This only really makes sense for square-ish pixels)
    {
        Point2d center = unproject(m_imageSize * 0.5);
        Point2d rootTwoAway = unproject(m_imageSize * 0.5 + Point2d(1.0, 1.0));
        m_onePixelDist = (center - rootTwoAway).length() / std::sqrt(2.0);
    }

    // Work out the linear projection values for the UFB
    {
        // First: Find out how big the linear bounding rectangle must be
        std::vector<Point2d> vertices;
        vertices.push_back(unproject(Point2d(- 0.5, - 0.5)));
        vertices.push_back(unproject(Point2d(m_imageSize.x - 0.5, - 0.5)));
        vertices.push_back(unproject(Point2d(m_imageSize.x - 0.5, m_imageSize.y - 0.5)));
        vertices.push_back(unproject(Point2d(- 0.5, m_imageSize.y - 0.5)));
        Point2d min = vertices[0];
        Point2d max = vertices[0];
        for (int i=0; i<4; ++i) {
          if (vertices[i].x < min.x)
              min.x = vertices[i].x;
          if (vertices[i].x > max.x)
              max.x = vertices[i].x;

          if (vertices[i].y < min.y)
              min.y = vertices[i].y;
          if (vertices[i].y > max.y)
              max.y = vertices[i].y;
        }
        m_implaneTopLeft = min;
        m_implaneBottomRight = max;

        // Store projection parameters to fill this bounding box
        m_UFBLinearInvFocal = max - min;
        m_UFBLinearFocal.x = 1.0 / m_UFBLinearInvFocal.x;
        m_UFBLinearFocal.y = 1.0 / m_UFBLinearInvFocal.y;
        m_UFBLinearCenter.x = - min.x * m_UFBLinearFocal.x;
        m_UFBLinearCenter.y = - min.y * m_UFBLinearFocal.y;
    }
}

// Project from the camera z=1 plane to image pixels,
// while storing intermediate calculation results in member variables
Point2d Camera::project(const Point2d& camPoint, ProjectionInfo& projectionInfo) const
{
    projectionInfo.camPoint = camPoint;
    projectionInfo.radius = camPoint.length();
    projectionInfo.invalid = (projectionInfo.radius > m_maxRadius);
    projectionInfo.factor = _rtrans_factor(projectionInfo.radius);
    projectionInfo.distortedRadius = projectionInfo.radius * projectionInfo.factor;
    projectionInfo.distortedCamPoint = camPoint * projectionInfo.factor;

    projectionInfo.imagePoint.x = m_pixelCenter.x + m_pixelFocal.x * projectionInfo.distortedCamPoint.x;
    projectionInfo.imagePoint.y = m_pixelCenter.y + m_pixelFocal.y * projectionInfo.distortedCamPoint.y;

    return projectionInfo.imagePoint;
}

// Un-project from image pixel coords to the camera z=1 plane
// while storing intermediate calculation results in member variables
Point2d Camera::unproject(const Point2d& imagePoint, ProjectionInfo& projectionInfo) const
{
    projectionInfo.imagePoint = imagePoint;
    projectionInfo.distortedCamPoint.x = (imagePoint.x - m_pixelCenter.x) * m_invPixelFocal.x;
    projectionInfo.distortedCamPoint.y = (imagePoint.y - m_pixelCenter.y) * m_invPixelFocal.y;
    projectionInfo.distortedRadius = projectionInfo.distortedCamPoint.length();
    projectionInfo.radius = _inv_rtrans_factor(projectionInfo.distortedRadius);
    double dFactor = (projectionInfo.distortedRadius > 0.001) ?
                (projectionInfo.radius / projectionInfo.distortedRadius) :
                1.0;
    projectionInfo.factor = 1.0 / dFactor;
    projectionInfo.camPoint = projectionInfo.distortedCamPoint * dFactor;
    return projectionInfo.camPoint;
}

// Project from the camera z=1 plane to image pixels,
// while storing intermediate calculation results in member variables
Point2d Camera::project(const Point2d& camPoint) const
{
    Point2d distortedCamPoint = camPoint * _rtrans_factor(camPoint.length());
    return Point2d(m_pixelCenter.x + m_pixelFocal.x * distortedCamPoint.x,
                   m_pixelCenter.y + m_pixelFocal.y * distortedCamPoint.y);
}

// Un-project from image pixel coords to the camera z=1 plane
// while storing intermediate calculation results in member variables
Point2d Camera::unproject(const Point2d& imagePoint) const
{
    Point2d distortedCamPoint((imagePoint.x - m_pixelCenter.x) * m_invPixelFocal.x,
                              (imagePoint.y - m_pixelCenter.y) * m_invPixelFocal.y);
    double distortedRadius = distortedCamPoint.length();
    double dFactor = (distortedRadius > 0.001) ? (_inv_rtrans_factor(distortedRadius) / distortedRadius) : 1.0;
    return distortedCamPoint * dFactor;
}

Point2d Camera::project(const Point2f& camPoint, ProjectionInfo& projectionInfo) const
{
    return project(camPoint.cast<double>(), projectionInfo);
}

Point2d Camera::unproject(const Point2f& imagePoint, ProjectionInfo& projectionInfo) const
{
    return unproject(imagePoint.cast<double>(), projectionInfo);
}

Point2d Camera::project(const Point2f& camPoint) const
{
    return project(camPoint.cast<double>());
}

Point2d Camera::unproject(const Point2f& imagePoint) const
{
    return unproject(imagePoint.cast<double>());
}

Point2d Camera::project(const Point2i& camPoint, ProjectionInfo& projectionInfo) const
{
    return project(camPoint.cast<double>(), projectionInfo);
}

Point2d Camera::unproject(const Point2i& imagePoint, ProjectionInfo& projectionInfo) const
{
    return unproject(imagePoint.cast<double>(), projectionInfo);
}

Point2d Camera::project(const Point2i& camPoint) const
{
    return project(camPoint.cast<double>());
}

Point2d Camera::unproject(const Point2i& imagePoint) const
{
    return unproject(imagePoint.cast<double>());
}

Point2d Camera::distort(const Point2d& point) const
{
    return point * _rtrans_factor(point.length());
}

Point2d Camera::undistort(const Point2d& point) const
{
    double distortedRadius = point.length();
    if (distortedRadius <= 0.001)
        return point;
    return point * (_inv_rtrans_factor(distortedRadius) / distortedRadius);
}

TMath::TMatrixd Camera::getProjectionDerivatives(const ProjectionInfo& projectionInfo) const
{
    // get the derivative of image frame wrt camera z=1 frame at the last computed projection
    // in the form (d im1/d cam1, d im1/d cam2)
    //             (d im2/d cam1, d im2/d cam2)

    double fracBydx;
    double fracBydy;

    const double& k = m_2Tan;
    const double& x = projectionInfo.camPoint.x;
    const double& y = projectionInfo.camPoint.y;
    double r = projectionInfo.radius * m_distortionEnabled;

    if (r < 0.001) {
        fracBydx = 0.0;
        fracBydy = 0.0;
    } else {
        double r_squared = r * r;
        double a = (m_invW * k) / (r_squared * (1.0 + (k * k) * r_squared)) - projectionInfo.factor / r_squared;
        fracBydx = x * a;
        fracBydy = y * a;
    }

    TMath::TMatrixd derivs(2, 2);
    derivs(0, 0) = m_pixelFocal.x * (fracBydx * x + projectionInfo.factor);
    derivs(1, 0) = m_pixelFocal.y * (fracBydx * y);
    derivs(0, 1) = m_pixelFocal.x * (fracBydy * x);
    derivs(1, 1) = m_pixelFocal.y * (fracBydy * y + projectionInfo.factor);
    return derivs;
}

TMath::TMatrixd Camera::getCameraParametersDerivatives(const Point2d& camPoint) const
{
    // Differentials wrt to the camera parameters
    // Use these to calibrate the camera
    // No need for this to be quick, so do them numerically

    TMath::TMatrixd numDerivs(2, 5);
    TMath::TVectord normal = m_cameraParameters;
    Camera c;
    Point2d out = project(camPoint);
    for (int i=0; i<5; i++) {
        TMath::TVectord update = normal;
        update(i) += 0.001;
        c.setParameters(update, m_imageSize);
        Point2d out_B = c.project(camPoint);
        numDerivs(0, i) = (out_B.x - out.x) / 0.001;
        numDerivs(1, i) = (out_B.y - out.y) / 0.001;
    }
    return numDerivs;
}

TMath::TMatrixd Camera::getCameraParametersDerivatives(const Point2d& camPoint, const TMath::TVector<bool>& fixedCameraParamters) const
{
    TMath_assert(fixedCameraParamters.size() == 5);
    TMath::TMatrixd numDerivs(2, 5);
    TMath::TVectord normal = m_cameraParameters;
    Camera c;
    Point2d out = project(camPoint);
    for (int i=0; i<5; ++i) {
        if (fixedCameraParamters(i)) {
            numDerivs(0, i) = 0.0;
            numDerivs(1, i) = 0.0;
        } else {
            TMath::TVectord update = normal;
            update(i) += 0.001;
            c.setParameters(update, m_imageSize);
            Point2d out_B = c.project(camPoint);
            numDerivs(0, i) = (out_B.x - out.x) / 0.001;
            numDerivs(1, i) = (out_B.y - out.y) / 0.001;
        }
    }
    return numDerivs;
}

bool Camera::radialDistortionIsEnabled() const
{
    return (m_W != 0.0);
}

void Camera::disableRadialDistortion()
{
    // Set the radial distortion parameter to zero
    // This disables radial distortion and also disables its differentials
    m_cameraParameters(4) = 0.0;
    _refreshCameraParameters();
}

} // namespace AR
