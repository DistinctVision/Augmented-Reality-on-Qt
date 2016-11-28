#ifndef AR_OPTICALFLOW_H
#define AR_OPTICALFLOW_H

#include <vector>
#include <utility>
#include "Image.h"
#include "OpticalFlowCalculator.h"

namespace AR {

class Frame;

class OpticalFlow
{
public:
    OpticalFlow();

    Point2i cursorSize() const;
    void setCursorSize(const Point2i& size);

    float pixelEps() const;
    void setPixelEps(float pixelEps);

    int maxNumberIterations() const;
    void setMaxNumberIterations(int count);

    int countLevels() const;
    void setCountLevels(int count);

    ConstImage<uchar> firstImageAtLevel(int level) const;
    ConstImage<uchar> secondImageAtLevel(int level) const;

    ConstImage<uchar> firstImage() const;
    ConstImage<uchar> secondImage() const;

    virtual void setFirstImage(const ImageRef<uchar>& firstImage);
    virtual void setSecondImage(const ImageRef<uchar>& secondImage);

    virtual void setFirstImage(const Frame& frame);
    virtual void setSecondImage(const Frame& frame);

    void swapFirstSecondImages();

    TrackingResult tracking2d(Point2f& secondPosition, const Point2f& firstPosition);
    void tracking2d(std::vector<TrackingResult>& success,
                    std::vector<Point2f>& secondPoints,
                    const std::vector<Point2f>& firstPoints);
    TrackingResult tracking2dLK(Point2f& secondPosition, const Point2f& firstPosition);
    void tracking2dLK(std::vector<TrackingResult>& success,
                      std::vector<Point2f>& secondPoints,
                      const std::vector<Point2f>& firstPoints);
    TrackingResult tracking2d_line(Point2f& secondPosition, const Point2f& firstPosition, const Point2f& line_n);
    TrackingResult tracking2dLK_line(Point2f& secondPosition, const Point2f& firstPosition, const Point2f& line_n);

    virtual void reset();

    std::vector<Image<uchar>> getCopyOfImagePyramid_first() const;
    std::vector<Image<uchar>> getCopyOfImagePyramid_second() const;

protected:
    ConstImage<uchar> m_level0_first;
    ConstImage<uchar> m_level0_second;
    bool m_shared_first;
    bool m_shared_second;
    std::vector<Image<uchar>> m_levels_first;
    std::vector<Image<uchar>> m_levels_second;
    OpticalFlowCalculator m_opticalFlowCalculator;

    float m_maxVelocitySquared;
};

}

#endif // AR_OPTICALFLOW_H
