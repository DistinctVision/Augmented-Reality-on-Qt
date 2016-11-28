#include "OpticalFlow.h"
#include "ImageProcessing.h"
#include "Frame.h"
#include <cassert>

namespace AR {

OpticalFlow::OpticalFlow()
{
    setCountLevels(3);
    setCursorSize(Point2i(15, 15));
}

Point2i OpticalFlow::cursorSize() const
{
    return m_opticalFlowCalculator.cursorSize();
}

void OpticalFlow::setCursorSize(const Point2i& size)
{
    m_opticalFlowCalculator.setCursorSize(size);
    m_maxVelocitySquared = std::min(size.x, size.y);
    m_maxVelocitySquared *= m_maxVelocitySquared;
}

float OpticalFlow::pixelEps() const
{
    return m_opticalFlowCalculator.pixelEps();
}

void OpticalFlow::setPixelEps(float pixelEps)
{
    m_opticalFlowCalculator.setPixelEps(pixelEps);
}

int OpticalFlow::maxNumberIterations() const
{
    return m_opticalFlowCalculator.numberIterations();
}

void OpticalFlow::setMaxNumberIterations(int count)
{
    m_opticalFlowCalculator.setNumberIterations(count);
}

int OpticalFlow::countLevels() const
{
    return (int)(m_levels_first.size() + 1);
}

void OpticalFlow::setCountLevels(int count)
{
    assert(count > 0);
    m_levels_first.resize(std::max(count - 1, 0));
    m_levels_second.resize(std::max(count - 1, 0));
    m_shared_first = true;
    m_shared_second = true;
}

void OpticalFlow::setFirstImage(const ImageRef<uchar>& firstImage)
{
    assert(m_levels_first.size() > 0);
    if ((m_level0_first.size() == firstImage.size()) && !m_shared_first) {
        m_level0_first = firstImage;
        //Point2i firstSize = m_level0_first.size();
        ConstImage<uchar> prevFirstImage = m_level0_first;
        for (std::size_t i = 0; i < m_levels_first.size(); ++i) {
            //firstSize /= 2;
            ImageProcessing::halfSample(m_levels_first[i], prevFirstImage);
            prevFirstImage = m_levels_first[i];
        }
    } else {
        m_level0_first = firstImage;
        Point2i firstSize = m_level0_first.size();
        ConstImage<uchar> prevFirstImage = m_level0_first;
        for (std::size_t i = 0; i < m_levels_first.size(); ++i) {
            firstSize /= 2;
            m_levels_first[i] = Image<uchar>(firstSize);
            ImageProcessing::halfSample(m_levels_first[i], prevFirstImage);
            prevFirstImage = m_levels_first[i];
        }
    }
    m_shared_first = false;
}

void OpticalFlow::setSecondImage(const ImageRef<uchar>& secondImage)
{
    assert(m_levels_second.size() > 0);
    if ((m_level0_second.size() == secondImage.size()) && (!m_shared_second)) {
        m_level0_second = secondImage;
        Point2i secondSize = m_level0_second.size();
        ConstImage<uchar> prevSecondImage = m_level0_second;
        for (std::size_t i = 0; i < m_levels_second.size(); ++i) {
            secondSize /= 2;
            ImageProcessing::halfSample(m_levels_second[i], prevSecondImage);
            prevSecondImage = m_levels_second[i];
        }
    } else {
        m_level0_second = secondImage;
        Point2i secondSize = m_level0_second.size();
        ConstImage<uchar> prevSecondImage = m_level0_second;
        for (std::size_t i = 0; i < m_levels_second.size(); ++i) {
            secondSize /= 2;
            m_levels_second[i] = Image<uchar>(secondSize);
            ImageProcessing::halfSample(m_levels_second[i], prevSecondImage);
            prevSecondImage = m_levels_second[i];
        }
    }
    m_shared_second = false;
}

void OpticalFlow::setFirstImage(const Frame& frame)
{
    assert(frame.countImageLevels() > 0);
    m_level0_first = frame.m_imagePyramid[0];
    Point2i firstSize = m_level0_first.size();
    ConstImage<uchar> prevFirstImage = m_level0_first;
    int i, count = std::min((int)(frame.m_imagePyramid.size() - 1), (int)(m_levels_first.size()));
    for (i = 0; i < count; ++i) {
        firstSize /= 2;
        m_levels_first[i] = frame.m_imagePyramid[i + 1];
        prevFirstImage = m_levels_first[i];
    }
    for (; i < (int)m_levels_first.size(); ++i) {
        firstSize /= 2;
        if (m_levels_first[i].size() != firstSize)
            m_levels_first[i] = Image<uchar>(firstSize);
        ImageProcessing::halfSample(m_levels_first[i], prevFirstImage);
        prevFirstImage = m_levels_first[i];
    }
    m_shared_first = true;
}

void OpticalFlow::setSecondImage(const Frame& frame)
{
    assert(frame.countImageLevels() > 0);
    m_level0_second = frame.m_imagePyramid[0];
    Point2i secondSize = m_level0_second.size();
    ConstImage<uchar> prevSecondImage = m_level0_second;
    int i, count = std::min((int)(frame.m_imagePyramid.size() - 1), (int)(m_levels_second.size()));
    for (i = 0; i < count; ++i) {
        secondSize /= 2;
        m_levels_second[i] = frame.m_imagePyramid[i + 1];
        prevSecondImage = m_levels_second[i];
    }
    for (; i < (int)m_levels_second.size(); ++i) {
        secondSize /= 2;
        if (m_levels_second[i].size() != secondSize)
            m_levels_second[i] = Image<uchar>(secondSize);
        ImageProcessing::halfSample(m_levels_second[i], prevSecondImage);
        prevSecondImage = m_levels_second[i];
    }
    m_shared_second = true;
}

ConstImage<uchar> OpticalFlow::firstImageAtLevel(int level) const
{
    return (level > 0) ? m_levels_first.at(level - 1) : m_level0_first;
}

ConstImage<uchar> OpticalFlow::secondImageAtLevel(int level) const
{
    return (level > 0) ? m_levels_second.at(level - 1) : m_level0_second;
}

ConstImage<uchar> OpticalFlow::firstImage() const
{
    return m_level0_first;
}

ConstImage<uchar> OpticalFlow::secondImage() const
{
    return m_level0_second;
}

void OpticalFlow::reset()
{
    m_level0_first = ConstImage<uchar>();
    m_level0_second = ConstImage<uchar>();
    for (std::size_t level = 0; level < m_levels_first.size(); ++level) {
        m_levels_first[level] = Image<uchar>();
        m_levels_second[level] = Image<uchar>();
    }
}

TrackingResult OpticalFlow::tracking2d(Point2f& secondPosition, const Point2f& firstPosition)
{
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        p = secondPosition / scale;
        prev = p;
        if (m_opticalFlowCalculator.tracking2d(p, firstPosition / scale) != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                secondPosition = p * scale;
            } else {
                return TrackingResult::Fail;
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    p = secondPosition;
    prev = p;
    TrackingResult result = m_opticalFlowCalculator.tracking2d(p, firstPosition);
    if (result != TrackingResult::Fail) {
        if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
            secondPosition = p;
        } else {
            return TrackingResult::Fail;
        }
    }
    return result;
}

void OpticalFlow::tracking2d(std::vector<TrackingResult>& success,
                             std::vector<Point2f>& secondPoints,
                             const std::vector<Point2f>& firstPoints)
{
    assert(secondPoints.size() == firstPoints.size());
    if (success.size() != secondPoints.size()) {
        success.resize(secondPoints.size(), TrackingResult::Completed);
    } else {
        std::fill(success.begin(), success.end(), TrackingResult::Completed);
    }
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        std::vector<Point2f>::iterator itSecond = secondPoints.begin();
        std::vector<Point2f>::const_iterator itFirst = firstPoints.cbegin();
        std::vector<TrackingResult>::iterator itSuccess = success.begin();
        for ( ; itSecond != secondPoints.end(); ++itSecond, ++itFirst, ++itSuccess) {
            if (*itSuccess == TrackingResult::Fail)
                continue;
            p = *itSecond / scale;
            prev = p;
            if (m_opticalFlowCalculator.tracking2d(p, *itFirst / scale) != TrackingResult::Fail) {
                if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                    *itSecond = p * scale;
                } else {
                    *itSuccess = TrackingResult::Fail;
                }
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    std::vector<Point2f>::iterator itSecond = secondPoints.begin();
    std::vector<Point2f>::const_iterator itFirst = firstPoints.cbegin();
    std::vector<TrackingResult>::iterator itSuccess = success.begin();
    for ( ; itSecond != secondPoints.end(); ++itSecond, ++itFirst, ++itSuccess) {
        if (*itSuccess == TrackingResult::Fail)
            continue;
        p = *itSecond;
        prev = p;
        *itSuccess = m_opticalFlowCalculator.tracking2d(p, *itFirst);
        if (*itSuccess != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                *itSecond = p;
            } else {
                *itSuccess = TrackingResult::Fail;
            }
        }
    }
}

TrackingResult OpticalFlow::tracking2dLK(Point2f& secondPosition, const Point2f& firstPosition)
{
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        p = secondPosition / scale;
        prev = p;
        if (m_opticalFlowCalculator.tracking2dLK(p, firstPosition / scale) != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                secondPosition = p * scale;
            } else {
                return TrackingResult::Fail;
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    p = secondPosition;
    prev = p;
    TrackingResult result = m_opticalFlowCalculator.tracking2dLK(p, firstPosition);
    if (result != TrackingResult::Fail) {
        if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
            secondPosition = p;
        } else {
            return TrackingResult::Fail;
        }
    }
    return result;
}

void OpticalFlow::tracking2dLK(std::vector<TrackingResult>& success,
                               std::vector<Point2f>& secondPoints,
                               const std::vector<Point2f>& firstPoints)
{
    assert(secondPoints.size() == firstPoints.size());
    if (success.size() != secondPoints.size()) {
        success.resize(secondPoints.size(), TrackingResult::Completed);
    } else {
        std::fill(success.begin(), success.end(), TrackingResult::Completed);
    }
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        std::vector<Point2f>::iterator itSecond = secondPoints.begin();
        std::vector<Point2f>::const_iterator itFirst = firstPoints.cbegin();
        std::vector<TrackingResult>::iterator itSuccess = success.begin();
        for ( ; itSecond != secondPoints.end(); ++itSecond, ++itFirst, ++itSuccess) {
            if (*itSuccess == TrackingResult::Fail)
                continue;
            p = *itSecond / scale;
            prev = p;
            if (m_opticalFlowCalculator.tracking2dLK(p, *itFirst / scale) != TrackingResult::Fail) {
                if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                    *itSecond = p * scale;
                } else {
                    *itSuccess = TrackingResult::Fail;
                }
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    std::vector<Point2f>::iterator itSecond = secondPoints.begin();
    std::vector<Point2f>::const_iterator itFirst = firstPoints.cbegin();
    std::vector<TrackingResult>::iterator itSuccess = success.begin();
    for ( ; itSecond != secondPoints.end(); ++itSecond, ++itFirst, ++itSuccess) {
        if (*itSuccess == TrackingResult::Fail)
            continue;
        p = *itSecond;
        prev = p;
        *itSuccess = m_opticalFlowCalculator.tracking2dLK(p, *itFirst);
        if (*itSuccess != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                *itSecond = p;
            } else {
                *itSuccess = TrackingResult::Fail;
            }
        }
    }
}

TrackingResult OpticalFlow::tracking2d_line(Point2f& secondPosition, const Point2f& firstPosition, const Point2f& line_n)
{
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        p = secondPosition / scale;
        prev = p;
        if (m_opticalFlowCalculator.tracking2d_line(p, firstPosition / scale, line_n) != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                secondPosition = p * scale;
            } else {
                return TrackingResult::Fail;
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    p = secondPosition;
    prev = p;
    TrackingResult result = m_opticalFlowCalculator.tracking2d_line(p, firstPosition, line_n);
    if (result != TrackingResult::Fail) {
        if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
            secondPosition = p;
        } else {
            return TrackingResult::Fail;
        }
    }
    return result;
}

TrackingResult OpticalFlow::tracking2dLK_line(Point2f& secondPosition, const Point2f& firstPosition, const Point2f& line_n)
{
    Point2f p, prev;
    for (int level = m_levels_second.size() - 1; level >= 0; --level) {
        TMath_assert(m_levels_second[level].data() != nullptr);
        m_opticalFlowCalculator.setFirstImage(m_levels_first[level]);
        m_opticalFlowCalculator.setSecondImage(m_levels_second[level]);
        float scale = (1 << level);
        p = secondPosition / scale;
        prev = p;
        if (m_opticalFlowCalculator.tracking2dLK_line(p, firstPosition / scale, line_n) != TrackingResult::Fail) {
            if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
                secondPosition = p * scale;
            } else {
                return TrackingResult::Fail;
            }
        }
    }
    TMath_assert(m_level0_second.data() != nullptr);
    m_opticalFlowCalculator.setFirstImage(m_level0_first);
    m_opticalFlowCalculator.setSecondImage(m_level0_second);
    p = secondPosition;
    prev = p;
    TrackingResult result = m_opticalFlowCalculator.tracking2dLK_line(p, firstPosition, line_n);
    if (result != TrackingResult::Fail) {
        if ((p - prev).lengthSquared() < m_maxVelocitySquared) {
            secondPosition = p;
        } else {
            return TrackingResult::Fail;
        }
    }
    return result;
}

void OpticalFlow::swapFirstSecondImages()
{
    ConstImage<uchar>::swap(m_level0_first, m_level0_second);
    for (std::size_t i = 0; i < m_levels_second.size(); ++i) {
        ImageRef<uchar>::swap(m_levels_first[i], m_levels_second[i]);
    }
}

std::vector<Image<uchar>> OpticalFlow::getCopyOfImagePyramid_first() const
{
    std::vector<Image<uchar>> imagePyramid;
    imagePyramid.resize(m_levels_first.size() + 1);
    imagePyramid[0] = m_level0_first.copy();
    for (std::size_t i = 0; i < m_levels_first.size(); ++i) {
        imagePyramid[i + 1] = m_levels_first[i].copy();
    }
    return imagePyramid;
}

std::vector<Image<uchar>> OpticalFlow::getCopyOfImagePyramid_second() const
{
    std::vector<Image<uchar>> imagePyramid;
    imagePyramid.resize(m_levels_second.size() + 1);
    imagePyramid[0] = m_level0_second.copy();
    for (std::size_t i = 0; i < m_levels_second.size(); ++i) {
        imagePyramid[i + 1] = m_levels_second[i].copy();
    }
    return imagePyramid;
}

}
