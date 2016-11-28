#include "OpticalFlowCalculator.h"
#include "TMath/TMath.h"
#include <cmath>
#include <limits>
#include <climits>
#include <cassert>

namespace AR {

OpticalFlowCalculator::OpticalFlowCalculator()
{
    m_H3 = TMath::TMatrixf(3, 3);
    m_invMaxErrorSquared = 0.0f;
    m_gaussian = nullptr;
    //m_derivatives1d = nullptr;
    m_derivatives2d = nullptr;
    m_cursorSize.set(4, 4);
    setCursorSize(m_cursorSize);
    m_pixelEps = 1e-2f;
    m_numberIterations = 8;
}

OpticalFlowCalculator::~OpticalFlowCalculator()
{
    if (m_gaussian != nullptr)
        delete[] m_gaussian;
    //if (m_derivatives1d != nullptr)
    //    delete[] m_derivatives1d;
    if (m_derivatives2d != nullptr)
        delete[] m_derivatives2d;
}

Point2i OpticalFlowCalculator::cursorSize() const
{
    return m_cursorSize;
}

int OpticalFlowCalculator::cursorWidth() const
{
    return m_cursorSize.x;
}

int OpticalFlowCalculator::cursorHeight() const
{
    return m_cursorSize.y;
}

void OpticalFlowCalculator::setCursorSize(const Point2i& cursorSize)
{
    assert((cursorSize.x > 0) && (cursorSize.y > 0));
    m_cursorSize = cursorSize;
    m_pathSize = m_cursorSize * 2 + Point2i(1, 1);
    m_sigmaGaussian = std::max(m_cursorSize.x, m_cursorSize.y) / 2.5f;
    int memSize = m_pathSize.y * m_pathSize.x;
    m_gaussian = static_cast<float*>(realloc(m_gaussian, sizeof(float) * memSize));
    //m_derivatives1d = static_cast<float*>(realloc(m_derivatives1d, sizeof(float) * memSize));
    m_derivatives2d = static_cast<Point2f*>(realloc(m_derivatives2d, sizeof(Point2f) * memSize));
    _solveGaussian();
    if (m_secondImage.size() != Point2i(0, 0)) {
        m_begin_first = m_cursorSize + Point2i(2, 2);
        m_end_first = m_firstImage.size() - (m_cursorSize + Point2i(3, 3));
        m_begin_second = m_cursorSize + Point2i(2, 2);
        m_end_second = m_secondImage.size() - (m_cursorSize + Point2i(3, 3));
    }
    m_path = Image<uchar>(m_pathSize + Point2i(2, 2));
}

Image<uchar> OpticalFlowCalculator::patch()
{
    return m_path;
}

ConstImage<uchar> OpticalFlowCalculator::patch() const
{
    return m_path;
}

void OpticalFlowCalculator::_setSigmaGaussian(float sigma)
{
    m_sigmaGaussian = sigma;
    _solveGaussian();
}

void OpticalFlowCalculator::_solveGaussian()
{
    double maxErrorSquared = 0.0f;
    float sigmaSquare = m_sigmaGaussian * m_sigmaGaussian;
    float k1 = (float)(1.0 / (2.0 * M_PI * sigmaSquare));
    float invK2 = 1.0f / (2.0f * sigmaSquare);
    Point2i p;
    int k = 0;
    for (p.y = - m_cursorSize.y; p.y <= m_cursorSize.y; ++p.y) {
        for (p.x = - m_cursorSize.x; p.x <= m_cursorSize.x; ++p.x, ++k) {
            m_gaussian[k] = k1 * std::exp(- p.lengthSquared() * invK2);
            maxErrorSquared += (m_gaussian[k] * m_gaussian[k]) * (255.0 * 255.0);
        }
    }
    if (maxErrorSquared > std::numeric_limits<float>::epsilon())
        m_invMaxErrorSquared = (float)(1.0 / maxErrorSquared);
    else
        m_invMaxErrorSquared = 0.0f;
}

int OpticalFlowCalculator::numberIterations() const
{
    return m_numberIterations;
}

void OpticalFlowCalculator::setNumberIterations(int numberIterations)
{
    m_numberIterations = numberIterations;
}

float OpticalFlowCalculator::pixelEps() const
{
    return m_pixelEps;
}

void OpticalFlowCalculator::setPixelEps(float eps)
{
    m_pixelEps = std::fabs(eps);
}

void OpticalFlowCalculator::setFirstImage(const ImageRef<uchar>& image)
{
    m_firstImage = image;
    m_begin_first = m_cursorSize + Point2i(2, 2);
    m_end_first = m_firstImage.size() - (m_cursorSize + Point2i(3, 3));
}

const ConstImage<uchar>& OpticalFlowCalculator::firstImage() const
{
    return m_firstImage;
}

void OpticalFlowCalculator::setSecondImage(const ImageRef<uchar>& image)
{
    m_secondImage = image;
    m_begin_second = m_cursorSize + Point2i(2, 2);
    m_end_second = m_secondImage.size() - (m_cursorSize + Point2i(3, 3));
}

const ConstImage<uchar>& OpticalFlowCalculator::secondImage() const
{
    return m_secondImage;
}

void OpticalFlowCalculator::getSubPixelImageF(Image<float>& outImage,
                                              const ImageRef<uchar>& image,
                                              const Point2f& beginPoint)
{
    Point2i beginPoint_i((int)std::floor(beginPoint.x), (int)std::floor(beginPoint.y));
    TMath_assert((beginPoint_i.x >= 0) && (beginPoint_i.y >= 0));
    TMath_assert(((beginPoint_i.x + outImage.width() + 1) <= image.width()) &&
                 ((beginPoint_i.y + outImage.height() + 1) <= image.height()));
    const uchar* imageStr = image.pointer(beginPoint_i);
    const uchar* imageStrNext = &imageStr[image.width()];

    float* outStr = outImage.data();

    Point2f sub_pix(beginPoint.x - beginPoint_i.x, beginPoint.y - beginPoint_i.y);
    // interpolation weights
    float wTL, wTR, wBL, wBR;
    wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
    wTR = sub_pix.x * (1.0f - sub_pix.y);
    wBL = (1.0f - sub_pix.x) * sub_pix.y;
    wBR = sub_pix.x * sub_pix.y;

    Point2i p;
    for (p.y = 0; p.y < outImage.height(); ++p.y) {
        for (p.x = 0; p.x < outImage.width(); ++p.x) {
            outStr[p.x] = wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                          wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1];
        }
        imageStr = imageStrNext;
        imageStrNext = &imageStrNext[image.width()];
        outStr = &outStr[outImage.width()];
    }
}

Image<float> OpticalFlowCalculator::getSubPixelImageF(const ImageRef<uchar>& image,
                                                      const Point2f& beginPoint,
                                                      const Point2i& size)
{
    Image<float> outImage(size);
    getSubPixelImageF(outImage, image, beginPoint);
    return outImage;
}

void OpticalFlowCalculator::getSubPixelImage(Image<uchar>& outImage,
                                             const ImageRef<uchar>& image,
                                             const Point2f& beginPoint)
{
    Point2i beginPoint_i((int)std::floor(beginPoint.x), (int)std::floor(beginPoint.y));
    TMath_assert((beginPoint_i.x >= 0) && (beginPoint_i.y >= 0));
    TMath_assert(((beginPoint_i.x + outImage.width() + 1) <= image.width()) &&
                 ((beginPoint_i.y + outImage.height() + 1) <= image.height()));
    const uchar* imageStr = image.pointer(beginPoint_i);
    const uchar* imageStrNext = &imageStr[image.width()];

    uchar* outStr = outImage.data();

    Point2f sub_pix(beginPoint.x - beginPoint_i.x, beginPoint.y - beginPoint_i.y);
    // interpolation weights
    float wTL, wTR, wBL, wBR;
    wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
    wTR = sub_pix.x * (1.0f - sub_pix.y);
    wBL = (1.0f - sub_pix.x) * sub_pix.y;
    wBR = sub_pix.x * sub_pix.y;

    Point2i p;
    for (p.y = 0; p.y < outImage.height(); ++p.y) {
        for (p.x = 0; p.x < outImage.width(); ++p.x) {
            outStr[p.x] = (uchar)(wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                                  wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1]);
        }
        imageStr = imageStrNext;
        imageStrNext = &imageStrNext[image.width()];
        outStr = &outStr[outImage.width()];
    }
}

Image<uchar> OpticalFlowCalculator::getSubPixelImage(const ImageRef<uchar>& image,
                                                     const Point2f& beginPoint,
                                                     const Point2i& size)
{
    Image<uchar> outImage(size);
    getSubPixelImage(outImage, image, beginPoint);
    return outImage;
}

TrackingResult OpticalFlowCalculator::tracking2d(Point2f& position, const Point2f& imagePosition)
{
    if ((imagePosition.x < m_begin_first.x) || (imagePosition.y < m_begin_first.y) ||
        (imagePosition.x >= m_end_first.x) || (imagePosition.y >= m_end_first.y))
        return TrackingResult::Fail;
    getSubPixelImage(m_path, m_firstImage, Point2f(imagePosition.x - (m_cursorSize.x + 1.5f),
                                                   imagePosition.y - (m_cursorSize.y + 1.5f)));
    return _tracking2dOnSecondImage(position, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2dLK(Point2f& position, const Point2f& imagePosition)
{
    if ((imagePosition.x < m_begin_first.x) || (imagePosition.y < m_begin_first.y) ||
        (imagePosition.x >= m_end_first.x) || (imagePosition.y >= m_end_first.y))
        return TrackingResult::Fail;
    getSubPixelImage(m_path, m_firstImage, Point2f(imagePosition.x - (m_cursorSize.x + 1.5f),
                                                   imagePosition.y - (m_cursorSize.y + 1.5f)));
    return _tracking2dOnSecondImageLK(position, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2d_patch(Point2f& position) const
{
    return _tracking2dOnSecondImage(position, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2dLK_patch(Point2f& position) const
{
    return _tracking2dOnSecondImageLK(position, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2d_line(Point2f& position, const Point2f& imagePosition, const Point2f& line_n)
{
    if ((imagePosition.x < m_begin_first.x) || (imagePosition.y < m_begin_first.y) ||
        (imagePosition.x >= m_end_first.x) || (imagePosition.y >= m_end_first.y))
        return TrackingResult::Fail;
    getSubPixelImage(m_path, m_firstImage, Point2f(imagePosition.x - (m_cursorSize.x + 1.5f),
                                                   imagePosition.y - (m_cursorSize.y + 1.5f)));
    return _tracking2dOnSecondImage_line(position, line_n, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2dLK_line(Point2f& position, const Point2f& imagePosition, const Point2f& line_n)
{
    if ((imagePosition.x < m_begin_first.x) || (imagePosition.y < m_begin_first.y) ||
        (imagePosition.x >= m_end_first.x) || (imagePosition.y >= m_end_first.y))
        return TrackingResult::Fail;
    getSubPixelImage(m_path, m_firstImage, Point2f(imagePosition.x - (m_cursorSize.x + 1.5f),
                                                   imagePosition.y - (m_cursorSize.y + 1.5f)));
    return _tracking2dOnSecondImageLK_line(position, line_n, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2d_patch_line(Point2f& position, const Point2f& line_n) const
{
    return _tracking2dOnSecondImage_line(position, line_n, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::tracking2dLK_patch_line(Point2f& position, const Point2f& line_n) const
{
    return _tracking2dOnSecondImageLK_line(position, line_n, m_path.data(), m_path.width());
}

TrackingResult OpticalFlowCalculator::_tracking2dOnSecondImage(Point2f& position, const uchar* imageRef, int imageStride) const
{
    m_H3.setZero();
    const uchar* basePathRef = &imageRef[imageStride + 1];
    const uchar* pathStr = basePathRef;
    const uchar* pathStrPrev = &pathStr[-imageStride];
    const uchar* pathStrNext = &pathStr[imageStride];
    Point2f wd;
    Point2i p;
    int k = 0;
    for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
        for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
            Point2f& d = m_derivatives2d[k];
            d.x = (pathStr[p.x + 1] - pathStr[p.x - 1]) * 0.5f;
            d.y = (pathStrNext[p.x] - pathStrPrev[p.x]) * 0.5f;
            wd = d * m_gaussian[k];

            m_H3(0, 0) += d.x * wd.x;
            m_H3(0, 1) += d.x * wd.y;
            m_H3(0, 2) += wd.x;
            m_H3(1, 1) += d.y * wd.y;
            m_H3(1, 2) += wd.y;
            m_H3(2, 2) += m_gaussian[k];

            ++k;
        }
        pathStrPrev = pathStr;
        pathStr = pathStrNext;
        pathStrNext = &pathStrNext[imageStride];
    }
    m_H3(1, 0) = m_H3(0, 1);
    m_H3(2, 0) = m_H3(0, 2);
    m_H3(2, 1) = m_H3(1, 2);

    if (!TMath::TTools::matrix3x3Invert(m_H3))
        return TrackingResult::Fail;

    Point2i position_i;
    Point2f sub_pix;

    // compute interpolation weights
    float wTL, wTR, wBL, wBR;

    float wdt, pixelValue;
    float mean_diff = 0.0f;

    TMath::TVectorf Jres(3), delta(3);
    Point2f prevDelta;

    for (int iteration = 0; iteration < m_numberIterations; ++iteration) {
        position_i.x = (int)std::floor(position.x);
        position_i.y = (int)std::floor(position.y);

        if ((position_i.x < m_begin_second.x) || (position_i.y < m_begin_second.y) ||
            (position_i.x >= m_end_second.x) || (position_i.y >= m_end_second.y))
            return TrackingResult::Fail;

        // compute interpolation weights
        sub_pix.x = position.x - position_i.x;
        sub_pix.y = position.y - position_i.y;

        wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
        wTR = sub_pix.x * (1.0f - sub_pix.y);
        wBL = (1.0f - sub_pix.x) * sub_pix.y;
        wBR = sub_pix.x * sub_pix.y;

        pathStr = basePathRef;

        const uchar* imageStr = m_secondImage.pointer(position_i - m_cursorSize);
        const uchar* imageStrNext = &imageStr[m_secondImage.width()];

        Jres.setZero();

        k = 0;
        for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
            for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
                pixelValue = wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                             wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1]
                        + mean_diff;
                wdt = (pixelValue - pathStr[p.x]) * m_gaussian[k];

                Point2f& d = m_derivatives2d[k];

                Jres(0) += wdt * d.x;
                Jres(1) += wdt * d.y;
                Jres(2) += wdt;

                ++k;
            }
            pathStr = &pathStr[imageStride];
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[m_secondImage.width()];
        }

        delta = m_H3 * Jres;
        position.x -= delta(0);
        position.y -= delta(1);
        mean_diff += delta(2);

        if (iteration > 0) {
            if ((std::fabs(delta(0) + prevDelta.x) < m_pixelEps) && (std::fabs(delta(1) + prevDelta.y) < m_pixelEps)) {
                return TrackingResult::Completed;
            }
        }
        prevDelta.set(delta(0), delta(1));
    }
    return TrackingResult::Uncompleted;
}

TrackingResult OpticalFlowCalculator::_tracking2dOnSecondImageLK(Point2f& position, const uchar *imageRef, int imageStride) const
{
    const uchar* basePathRef = &imageRef[imageStride + 1];

    const uchar* pathStr = basePathRef;
    const uchar* pathStrPrev = &pathStr[-imageStride];
    const uchar* pathStrNext = &pathStr[imageStride];
    Point2f wd;
    Point2i p;
    float Dxx = 0.0f, Dxy = 0.0f, Dyy = 0.0f;
    int k = 0;
    for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
        for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
            Point2f& d = m_derivatives2d[k];
            d.x = (pathStr[p.x + 1] - pathStr[p.x - 1]) * 0.5f;
            d.y = (pathStrNext[p.x] - pathStrPrev[p.x]) * 0.5f;
            wd = d * m_gaussian[k];

            Dxx += d.x * wd.x;
            Dxy += d.x * wd.y;
            Dyy += d.y * wd.y;

            ++k;
        }
        pathStrPrev = pathStr;
        pathStr = pathStrNext;
        pathStrNext = &pathStrNext[imageStride];
    }
    float det = Dxx * Dyy - Dxy * Dxy;
    if (std::fabs(det) < 1e-5)
        return TrackingResult::Fail;

    wd.x = Dxx;
    Dxx = Dyy / det;
    Dyy = wd.x / det;
    Dxy = - Dxy / det;

    Point2i position_i;
    Point2f sub_pix;

    // interpolation weights
    float wTL, wTR, wBL, wBR;

    float wdt, pixelValue;

    Point2f b, delta;
    Point2f prevDelta;

    for (int iteration = 0; iteration < m_numberIterations; ++iteration) {
        position_i.x = (int)std::floor(position.x);
        position_i.y = (int)std::floor(position.y);

        if ((position_i.x < m_begin_second.x) || (position_i.y < m_begin_second.y) ||
            (position_i.x >= m_end_second.x) || (position_i.y >= m_end_second.y))
            return TrackingResult::Fail;

        // compute interpolation weights
        sub_pix.x = position.x - position_i.x;
        sub_pix.y = position.y - position_i.y;

        wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
        wTR = sub_pix.x * (1.0f - sub_pix.y);
        wBL = (1.0f - sub_pix.x) * sub_pix.y;
        wBR = sub_pix.x * sub_pix.y;

        pathStr = basePathRef;

        const uchar* imageStr = m_secondImage.pointer(position_i - m_cursorSize);
        const uchar* imageStrNext = &imageStr[m_secondImage.width()];

        b.set(0.0f, 0.0f);

        k = 0;
        for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
            for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
                pixelValue = wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                             wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1];
                wdt = (pixelValue - pathStr[p.x]) * m_gaussian[k];

                Point2f& d = m_derivatives2d[k];

                b += d * wdt;

                ++k;
            }
            pathStr = &pathStr[imageStride];
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[m_secondImage.width()];
        }

        delta.set(Dxx * b.x + Dxy * b.y, Dxy * b.x + Dyy * b.y);
        position -= delta;

        if (iteration > 0) {
            if ((std::fabs(delta.x + prevDelta.x) < m_pixelEps) && (std::fabs(delta.y + prevDelta.y) < m_pixelEps)) {
                return TrackingResult::Completed;
            }
        }
        prevDelta = delta;
    }

    return TrackingResult::Uncompleted;
}


TrackingResult OpticalFlowCalculator::_tracking2dOnSecondImage_line(Point2f& position,
                                                                    const Point2f& line_n,
                                                                    const uchar* imageRef,
                                                                    int imageStride) const
{
    m_H3.setZero();
    const uchar* basePathRef = &imageRef[imageStride + 1];
    const uchar* pathStr = basePathRef;
    const uchar* pathStrPrev = &pathStr[-imageStride];
    const uchar* pathStrNext = &pathStr[imageStride];
    Point2f wd;
    Point2i p;
    int k = 0;
    for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
        for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
            Point2f& d = m_derivatives2d[k];
            d.x = (pathStr[p.x + 1] - pathStr[p.x - 1]) * 0.5f;
            d.y = (pathStrNext[p.x] - pathStrPrev[p.x]) * 0.5f;
            wd = d * m_gaussian[k];

            m_H3(0, 0) += d.x * wd.x;
            m_H3(0, 1) += d.x * wd.y;
            m_H3(0, 2) += wd.x;
            m_H3(1, 1) += d.y * wd.y;
            m_H3(1, 2) += wd.y;
            m_H3(2, 2) += m_gaussian[k];

            ++k;
        }
        pathStrPrev = pathStr;
        pathStr = pathStrNext;
        pathStrNext = &pathStrNext[imageStride];
    }
    m_H3(1, 0) = m_H3(0, 1);
    m_H3(2, 0) = m_H3(0, 2);
    m_H3(2, 1) = m_H3(1, 2);

    if (!TMath::TTools::matrix3x3Invert(m_H3))
        return TrackingResult::Fail;

    Point2i position_i;
    Point2f sub_pix;

    // compute interpolation weights
    float wTL, wTR, wBL, wBR;

    float wdt, pixelValue;
    float mean_diff = 0.0f, d;

    TMath::TVectorf Jres(3), delta(3);
    Point2f prevDelta;

    for (int iteration = 0; iteration < m_numberIterations; ++iteration) {
        position_i.x = (int)std::floor(position.x);
        position_i.y = (int)std::floor(position.y);

        if ((position_i.x < m_begin_second.x) || (position_i.y < m_begin_second.y) ||
            (position_i.x >= m_end_second.x) || (position_i.y >= m_end_second.y))
            return TrackingResult::Fail;

        // compute interpolation weights
        sub_pix.x = position.x - position_i.x;
        sub_pix.y = position.y - position_i.y;

        wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
        wTR = sub_pix.x * (1.0f - sub_pix.y);
        wBL = (1.0f - sub_pix.x) * sub_pix.y;
        wBR = sub_pix.x * sub_pix.y;

        pathStr = basePathRef;

        const uchar* imageStr = m_secondImage.pointer(position_i - m_cursorSize);
        const uchar* imageStrNext = &imageStr[m_secondImage.width()];

        Jres.setZero();

        k = 0;
        for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
            for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
                pixelValue = wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                             wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1]
                        + mean_diff;
                wdt = (pixelValue - pathStr[p.x]) * m_gaussian[k];

                Point2f& d = m_derivatives2d[k];

                Jres(0) += wdt * d.x;
                Jres(1) += wdt * d.y;
                Jres(2) += wdt;

                ++k;
            }
            pathStr = &pathStr[imageStride];
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[m_secondImage.width()];
        }

        delta = m_H3 * Jres;
        d = delta(0) * line_n.x + delta(1) * line_n.y;
        position.x += line_n.x * d - delta(0);
        position.y += line_n.y * d - delta(1);

        mean_diff += delta(2);

        if (iteration > 0) {
            if ((std::fabs(delta(0) + prevDelta.x) < m_pixelEps) && (std::fabs(delta(1) + prevDelta.y) < m_pixelEps)) {
                return TrackingResult::Completed;
            }
        }
        prevDelta.set(delta(0), delta(1));
    }
    return TrackingResult::Uncompleted;
}

TrackingResult OpticalFlowCalculator::_tracking2dOnSecondImageLK_line(Point2f& position,
                                                                      const Point2f& line_n,
                                                                      const uchar* imageRef,
                                                                      int imageStride) const
{
    const uchar* basePathRef = &imageRef[imageStride + 1];

    const uchar* pathStr = basePathRef;
    const uchar* pathStrPrev = &pathStr[-imageStride];
    const uchar* pathStrNext = &pathStr[imageStride];
    Point2f wd;
    Point2i p;
    float Dxx = 0.0f, Dxy = 0.0f, Dyy = 0.0f;
    int k = 0;
    for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
        for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
            Point2f& d = m_derivatives2d[k];
            d.x = (pathStr[p.x + 1] - pathStr[p.x - 1]) * 0.5f;
            d.y = (pathStrNext[p.x] - pathStrPrev[p.x]) * 0.5f;
            wd = d * m_gaussian[k];

            Dxx += d.x * wd.x;
            Dxy += d.x * wd.y;
            Dyy += d.y * wd.y;

            ++k;
        }
        pathStrPrev = pathStr;
        pathStr = pathStrNext;
        pathStrNext = &pathStrNext[imageStride];
    }
    float det = Dxx * Dyy - Dxy * Dxy;
    if (std::fabs(det) < 1e-5)
        return TrackingResult::Fail;

    wd.x = Dxx;
    Dxx = Dyy / det;
    Dyy = wd.x / det;
    Dxy = - Dxy / det;

    Point2i position_i;
    Point2f sub_pix;

    // interpolation weights
    float wTL, wTR, wBL, wBR;

    float wdt, pixelValue;

    Point2f b, delta;
    Point2f prevDelta;

    for (int iteration = 0; iteration < m_numberIterations; ++iteration) {
        position_i.x = (int)std::floor(position.x);
        position_i.y = (int)std::floor(position.y);

        if ((position_i.x < m_begin_second.x) || (position_i.y < m_begin_second.y) ||
            (position_i.x >= m_end_second.x) || (position_i.y >= m_end_second.y))
            return TrackingResult::Fail;

        // compute interpolation weights
        sub_pix.x = position.x - position_i.x;
        sub_pix.y = position.y - position_i.y;

        wTL = (1.0f - sub_pix.x) * (1.0f - sub_pix.y);
        wTR = sub_pix.x * (1.0f - sub_pix.y);
        wBL = (1.0f - sub_pix.x) * sub_pix.y;
        wBR = sub_pix.x * sub_pix.y;

        pathStr = basePathRef;

        const uchar* imageStr = m_secondImage.pointer(position_i - m_cursorSize);
        const uchar* imageStrNext = &imageStr[m_secondImage.width()];

        b.set(0.0f, 0.0f);

        k = 0;
        for (p.y = 0; p.y < m_pathSize.y; ++p.y) {
            for (p.x = 0; p.x < m_pathSize.x; ++p.x) {
                pixelValue = wTL * imageStr[p.x] + wTR * imageStr[p.x + 1] +
                             wBL * imageStrNext[p.x] + wBR * imageStrNext[p.x + 1];
                wdt = (pixelValue - pathStr[p.x]) * m_gaussian[k];

                Point2f& d = m_derivatives2d[k];

                b += d * wdt;

                ++k;
            }
            pathStr = &pathStr[imageStride];
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[m_secondImage.width()];
        }

        delta.set(Dxx * b.x + Dxy * b.y, Dxy * b.x + Dyy * b.y);
        position += line_n * Point2f::dot(delta, line_n) - delta;

        if (iteration > 0) {
            if ((std::fabs(delta.x + prevDelta.x) < m_pixelEps) && (std::fabs(delta.y + prevDelta.y) < m_pixelEps)) {
                return TrackingResult::Completed;
            }
        }
        prevDelta = delta;
    }

    return TrackingResult::Uncompleted;
}

}
