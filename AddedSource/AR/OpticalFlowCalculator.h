#ifndef AR_OPTICALFLOWCALCULATOR
#define AR_OPTICALFLOWCALCULATOR

#include "Image.h"
#include "TMath/TMatrix.h"

namespace AR {

enum class TrackingResult {
    Fail,
    Completed,
    Uncompleted
};

class OpticalFlowCalculator
{
public:
    OpticalFlowCalculator();
    ~OpticalFlowCalculator();

    Point2i cursorSize() const;
    int cursorWidth() const;
    int cursorHeight() const;
    void setCursorSize(const Point2i& cursorSize);

    Image<uchar> patch();
    ConstImage<uchar> patch() const;

    void setFirstImage(const ImageRef<uchar>& image);
    const ConstImage<uchar>& firstImage() const;

    void setSecondImage(const ImageRef<uchar>& image);
    const ConstImage<uchar>& secondImage() const;

    int numberIterations() const;
    void setNumberIterations(int numberIterations);

    float pixelEps() const;
    void setPixelEps(float pixelEps);

    static void getSubPixelImageF(Image<float>& outImage, const ImageRef<uchar>& image,
                                 const Point2f& beginPoint);
    static Image<float> getSubPixelImageF(const ImageRef<uchar>& image, const Point2f& beginPoint,
                                         const Point2i& size);
    static void getSubPixelImage(Image<uchar>& outImage, const ImageRef<uchar>& image,
                                 const Point2f& beginPoint);
    static Image<uchar> getSubPixelImage(const ImageRef<uchar>& image, const Point2f& beginPoint,
                                         const Point2i& size);

    TrackingResult tracking2d(Point2f& position, const Point2f& imagePosition);
    TrackingResult tracking2dLK(Point2f& position, const Point2f& imagePosition);

    TrackingResult tracking2d_patch(Point2f& position) const;
    TrackingResult tracking2dLK_patch(Point2f& position) const;

    TrackingResult tracking2d_line(Point2f& position, const Point2f& imagePosition, const Point2f& line_n);
    TrackingResult tracking2dLK_line(Point2f& position, const Point2f& imagePosition, const Point2f& line_n);

    TrackingResult tracking2d_patch_line(Point2f& position, const Point2f& line_n) const;
    TrackingResult tracking2dLK_patch_line(Point2f& position, const Point2f& line_n) const;

private:
    ConstImage<uchar> m_firstImage;
    ConstImage<uchar> m_secondImage;

    Point2i m_cursorSize;
    Point2i m_pathSize;
    float m_invMaxErrorSquared;

    Point2i m_begin_second;
    Point2i m_end_second;

    Point2i m_begin_first;
    Point2i m_end_first;

    float* m_gaussian;
    float m_sigmaGaussian;

    Image<uchar> m_path;
    //float* m_derivatives1d;
    Point2f* m_derivatives2d;

    mutable TMath::TMatrixf m_H3;

    int m_numberIterations;
    float m_pixelEps;

    void _solveGaussian();
    void _setSigmaGaussian(float sigma);

    TrackingResult _tracking2dOnSecondImage(Point2f& position, const uchar* imageRef, int imageStride) const;
    TrackingResult _tracking2dOnSecondImageLK(Point2f& position, const uchar* imageRef, int imageStride) const;
    TrackingResult _tracking2dOnSecondImage_line(Point2f& position, const Point2f& line_n,
                                                 const uchar* imageRef, int imageStride) const;
    TrackingResult _tracking2dOnSecondImageLK_line(Point2f& position, const Point2f& line_n,
                                                   const uchar* imageRef, int imageStride) const;
};

}

#endif // AR_OPTICALFLOWCALCULATOR
