#ifndef AR_CALIBRATIONCORNER_H
#define AR_CALIBRATIONCORNER_H

#include "TMath/TMatrix.h"
#include "Image.h"
#include <memory>

namespace AR {

class CalibrationCorner
{
public:
    struct Properties {
        Point2d pos;
        Point2d angles;
        double mean;
        double gain;

        Properties();
        TMath::TMatrixd warp() const;
    };

    CalibrationCorner();
    CalibrationCorner(int sideSize, float blurSigma);

    double iterate(CalibrationCorner::Properties& properties, const ImageRef<uchar>& image, double scale);
    double lastError() const;

    void draw(const Properties& properties, const Image<uchar>& image) const;
    void draw(const Properties& properties, const Image<Rgba>& image) const;

private:
    ConstImage<float> m_templateImage;
    double m_lastError;

    Image<float> _makeTemplate(const Point2i &size, float blurSigma) const;

    void _makeSharedTemplate(int halfSideSize);

    static Image<float> m_sharedSourceTemplateImage;
};

} // namespace AR

#endif // AR_CALIBRATIONCORNER_H
