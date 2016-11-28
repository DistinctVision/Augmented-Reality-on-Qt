#include "Frame.h"
#include "TMath/TMath.h"
#include "ImageProcessing.h"
#include "OpticalFlow.h"
#include <cassert>

namespace AR {

Frame::Frame(const std::shared_ptr<const Camera> & camera,
             const std::vector<Image<uchar>> & imagePyramid,
             const TMath::TMatrixd & rotation,
             const TMath::TVectord & translation):
    m_camera(camera), m_rotation(rotation), m_translation(translation)
{
    TMath_assert(imagePyramid.size() > 0);
    TMath_assert((rotation.rows() == 3) && (rotation.cols() == 3));
    TMath_assert(translation.size() == 3);
    m_imagePyramid.resize(imagePyramid.size());
    m_imagePyramid[0] = imagePyramid[0];
    Point2i size = m_imagePyramid[0].size();
    std::size_t i = 1;
    for (; i < imagePyramid.size(); ++i) {
        size /= 2;
        m_imagePyramid[i] = imagePyramid[i];
        if (m_imagePyramid[i].size() != size) {
            assert(false);
        }
    }
}

Frame::Frame(const std::shared_ptr<const Camera> & camera,
             const std::vector<Image<uchar>> & imagePyramid):
    m_camera(camera), m_rotation(3, 3), m_translation(3)
{
    TMath_assert(imagePyramid.size() > 0);
    m_imagePyramid.resize(imagePyramid.size());
    m_imagePyramid[0] = imagePyramid[0];
    Point2i size = m_imagePyramid[0].size();
    std::size_t i = 1;
    for (; i < imagePyramid.size(); ++i) {
        size /= 2;
        m_imagePyramid[i] = imagePyramid[i];
        if (m_imagePyramid[i].size() != size) {
            assert(false);
        }
    }
}

Frame::Frame(const Frame & frame)
{
    m_camera = frame.m_camera;
    m_rotation = frame.m_rotation;
    m_translation = frame.m_translation;
    m_imagePyramid = frame.m_imagePyramid;
}

Frame::Frame(Frame && frame)
{
    m_camera = std::move(frame.m_camera);
    m_rotation = std::move(frame.m_rotation);
    m_translation = std::move(frame.m_translation);
    m_imagePyramid = std::move(frame.m_imagePyramid);
}

std::shared_ptr<const Camera> Frame::camera() const
{
    return m_camera;
}

int Frame::countImageLevels() const
{
    return (int)m_imagePyramid.size();
}

ConstImage<uchar> Frame::imageLevel(int level) const
{
    return m_imagePyramid[level];
}

Point2i Frame::imageSize() const
{
    return m_imagePyramid[0].size();
}

TMath::TMatrixd Frame::rotation() const
{
    return m_rotation;
}

void Frame::setRotation(const TMath::TMatrixd & rotation)
{
    TMath_assert((rotation.rows() == 3) && (rotation.cols() == 3));
    m_rotation = rotation;
}

TMath::TVectord Frame::translation() const
{
    return m_translation;
}

void Frame::setTranslation(const TMath::TVectord & translation)
{
    TMath_assert(translation.size() == 3);
    m_translation = translation;
}

TMath::TVectord Frame::worldPosition() const
{
    return - TMath::TTools::matrix3x3Inverted(m_rotation) * m_translation;
}

void Frame::transform(const TMath::TMatrixd & rotation, const TMath::TVectord & translation)
{
    m_translation += m_rotation * translation;
    m_rotation *= rotation;
}

bool Frame::pointIsVisible(const TMath::TVectord & point) const
{
    TMath_assert(point.size() == 3);
    TMath::TVectord wp = m_rotation * point + m_translation;
    if (wp(2) < std::numeric_limits<float>::epsilon())
        return false;
    return imagePointInFrame(m_camera->project(Point2d(wp(0) / wp(2), wp(1) / wp(2))));
}

bool Frame::imagePointInFrame(const Point2d & imagePoint) const
{
    if ((imagePoint.x < 0.0) || (imagePoint.y < 0.0))
        return false;
    if ((imagePoint.x >= m_camera->imageWidth()) || (imagePoint.y > m_camera->imageHeight()))
        return false;
    return true;
}

void Frame::copy(const Frame & frame)
{
    m_camera = frame.m_camera;
    m_rotation = frame.m_rotation;
    m_translation = frame.m_translation;
    if ((m_imagePyramid.size() == frame.m_imagePyramid.size()) &&
        (m_imagePyramid[0].size() == frame.m_imagePyramid[0].size())) {
        for (std::size_t i = 0; i < m_imagePyramid.size(); ++i)
            Image<uchar>::copyData(m_imagePyramid[i], frame.m_imagePyramid[i]);
    } else {
        m_imagePyramid.resize(frame.m_imagePyramid.size());
        for (std::size_t i = 0; i < m_imagePyramid.size(); ++i)
            m_imagePyramid[i] = frame.m_imagePyramid[i].copy();
    }
}


std::vector<Image<uchar>> Frame::getCopyOfImagePyramid() const
{
    std::vector<Image<uchar>> imagePyramid;
    imagePyramid.resize(m_imagePyramid.size());
    for (std::size_t i = 0; i < m_imagePyramid.size(); ++i)
        imagePyramid[i] = m_imagePyramid[i].copy();
    return imagePyramid;
}

bool Frame::equals(const Frame & frame) const
{
    return m_imagePyramid[0].equals(frame.m_imagePyramid[0]);
}

}
