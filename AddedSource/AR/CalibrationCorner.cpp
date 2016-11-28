#include "CalibrationCorner.h"
#include "ImageProcessing.h"
#include "Painter.h"
#include "TMath/TCholesky.h"
#include "TMath/TWLS.h"

namespace AR {

Image<float> CalibrationCorner::m_sharedSourceTemplateImage;

CalibrationCorner::CalibrationCorner()
{
    m_lastError = 0.0;
    if (m_sharedSourceTemplateImage.width() == 0)
        _makeSharedTemplate(50);
}

CalibrationCorner::CalibrationCorner(int sideSize, float blurSigma)
{
    TMath_assert((sideSize > 0) && (blurSigma > 0.0f));
    m_lastError = 0.0;
    if (m_sharedSourceTemplateImage.width() == 0)
        _makeSharedTemplate(100);
    m_templateImage = _makeTemplate(Point2i(sideSize, sideSize), blurSigma);
}

double CalibrationCorner::lastError() const
{
    return m_lastError;
}

void CalibrationCorner::draw(const CalibrationCorner::Properties& properties,
                             const Image<uchar>& image) const
{
    using namespace TMath;

    Point2d halfTemplateSize((m_templateImage.width() - 1) * 0.5,
                             (m_templateImage.height() - 1) * 0.5);
    TMatrixd warp = properties.warp();
    Point2d posInTemplate, posInImage_d;
    Point2i posInImage_i = posInImage_d.cast<int>();
    Point2i p;
    for (p.y=0; p.y<m_templateImage.height(); ++p.y) {
        for (p.x=0; p.x<m_templateImage.width(); ++p.x) {
            posInTemplate.set(p.x - halfTemplateSize.x, p.y - halfTemplateSize.y);
            posInImage_d.set(properties.pos.x + warp(0, 0) * posInTemplate.x + warp(0, 1) * posInTemplate.y,
                           properties.pos.y + warp(1, 0) * posInTemplate.x + warp(1, 1) * posInTemplate.y);
            if ((posInImage_i.x < 0) || (posInImage_i.y < 0) ||
                    (posInImage_i.x >= image.width()) || (posInImage_i.y > image.height()))
                continue;
            image(posInImage_i) = m_templateImage(p);
        }
    }
}

void CalibrationCorner::draw(const CalibrationCorner::Properties& properties,
                             const Image<Rgba>& image) const
{
    using namespace TMath;

    Point2d halfTemplateSize((m_templateImage.width() - 1) * 0.5,
                             (m_templateImage.height() - 1) * 0.5);
    TMatrixd warp = properties.warp();
    Point2d posInTemplate, posInImage_d;
    Point2i posInImage_i = posInImage_d.cast<int>();
    Point2i p;
    for (p.y=0; p.y<m_templateImage.height(); ++p.y) {
        for (p.x=0; p.x<m_templateImage.width(); ++p.x) {
            posInTemplate.set(p.x - halfTemplateSize.x, p.y - halfTemplateSize.y);
            posInImage_d.set(properties.pos.x + warp(0, 0) * posInTemplate.x + warp(0, 1) * posInTemplate.y,
                           properties.pos.y + warp(1, 0) * posInTemplate.x + warp(1, 1) * posInTemplate.y);
            posInImage_i = posInImage_d.cast<int>();
            if ((posInImage_i.x < 0) || (posInImage_i.y < 0) ||
                    (posInImage_i.x >= image.width()) || (posInImage_i.y > image.height()))
                continue;
            uchar c = m_templateImage(p) * properties.gain + properties.mean;
            image(posInImage_i).set(c, c, c, 255);
        }
    }
}

Image<float> CalibrationCorner::_makeTemplate(const Point2i& size, float blurSigma) const
{
    int halfSizeBlur = std::min((int)std::ceil(blurSigma * 3.0f),
                                std::min(size.x, size.y) / 2 - 1);
    Image<float> tempImage(size);
    Image<float> templateImage(size);
    Painter::drawImage(templateImage, m_sharedSourceTemplateImage, Point2f(0.0f, 0.0f), Point2f(size.x, size.y));
    ImageProcessing::gaussianBlurX<float, float>(tempImage, templateImage, halfSizeBlur, blurSigma);
    ImageProcessing::gaussianBlurY<float, float>(templateImage, tempImage, halfSizeBlur, blurSigma);
    return templateImage;
}

double CalibrationCorner::iterate(CalibrationCorner::Properties& properties, const ImageRef<uchar>& image, double scale)
{
    using namespace TMath;

    Point2d halfTemplateSize((m_templateImage.width() - 1) * 0.5, (m_templateImage.height() - 1) * 0.5);
    Point2d cornerBegin = (properties.pos - halfTemplateSize) * scale;
    if ((cornerBegin.x < 0.0) || (cornerBegin.y < 0.0))
        return -1.0;
    Point2d cornerEnd = (properties.pos + halfTemplateSize) * scale;
    if ((cornerEnd.x >= (image.width() - 1.0)) || (cornerEnd.y >= (image.height() - 1.0)))
        return -1.0;

    TMatrixd warp = properties.warp();
    TMatrixd swarp = warp * scale;
    TWLS<double> wls(6);

    TVectord jac(6);

    double sum = 0.0, diff;

    const float* templateStr = m_templateImage.data();

    Point2d topLeftImage(1.0, 1.0);
    Point2d bottomRightImage = Point2d(image.width(), image.height()) - Point2d(2.0, 2.0);

    const uchar* imageData = image.data();
    const uchar* imageStr_d;
    const uchar* imageStr_cur;
    const uchar* imageStr_next;

    Point2d posInTemplate;
    Point2d posInImage;
    Point2i posInImage_i;
    Point2f subPixel;
    Point2f invSubPixel;
    float subPixel_delta;
    float invSubPixel_delta;
    Point2d diffImage;
    Point2i p;
    Point2d scaledPos = properties.pos * scale;
    for (p.y=0; p.y<m_templateImage.height(); ++p.y) {
        for (p.x=0; p.x<m_templateImage.width(); ++p.x) {
            posInTemplate.set(p.x - halfTemplateSize.x, p.y - halfTemplateSize.y);
            posInImage.set(scaledPos.x + swarp(0, 0) * posInTemplate.x + swarp(0, 1) * posInTemplate.y,
                           scaledPos.y + swarp(1, 0) * posInTemplate.x + swarp(1, 1) * posInTemplate.y);
            if ((posInImage.x < topLeftImage.x) || (posInImage.y < topLeftImage.y) ||
                    (posInImage.x > bottomRightImage.x) || (posInImage.y > bottomRightImage.y))
                continue;

            posInImage_i.set((int)std::floor(posInImage.x), (int)std::floor(posInImage.y));
            subPixel.set(posInImage.x - posInImage_i.x, posInImage.y - posInImage_i.y);
            invSubPixel.set(1.0f - subPixel.x, 1.0f - subPixel.y);
            imageStr_cur = &imageData[posInImage_i.y * image.width() + posInImage_i.x];
            imageStr_next = &imageStr_cur[image.width()];

            diff = (imageStr_cur[0] * (invSubPixel.x * invSubPixel.y) +
                    imageStr_cur[1] * (subPixel.x * invSubPixel.y) +
                    imageStr_next[0] * (invSubPixel.x * subPixel.y) +
                    imageStr_next[1] * (subPixel.x * subPixel.y)) -
                    (templateStr[p.x] * properties.gain + properties.mean);

            if (subPixel.x < 0.5f) {
                subPixel_delta = subPixel.x + 0.5f;
                invSubPixel_delta = 1.0f - subPixel_delta;
                diffImage.x = ((imageStr_cur[0] - imageStr_cur[- 1]) * (invSubPixel_delta * invSubPixel.y) +
                               (imageStr_cur[1] - imageStr_cur[0]) * (subPixel_delta * invSubPixel.y) +
                               (imageStr_next[0] - imageStr_next[- 1]) * (invSubPixel_delta * subPixel.y) +
                               (imageStr_next[1] - imageStr_next[0]) * (subPixel_delta * subPixel.y));
            } else {
                subPixel_delta = subPixel.x - 0.5f;
                invSubPixel_delta = 1.0f - subPixel_delta;
                diffImage.x = ((imageStr_cur[1] - imageStr_cur[0]) * (invSubPixel_delta * invSubPixel.y) +
                               (imageStr_cur[2] - imageStr_cur[1]) * (subPixel_delta * invSubPixel.y) +
                               (imageStr_next[1] - imageStr_next[0]) * (invSubPixel_delta * subPixel.y) +
                               (imageStr_next[2] - imageStr_next[1]) * (subPixel_delta * subPixel.y));
            }
            if (subPixel.y < 0.5f) {
                subPixel_delta = subPixel.y + 0.5f;
                invSubPixel_delta = 1.0f - subPixel_delta;
                imageStr_d = &imageStr_cur[-image.width()];
                diffImage.y = ((imageStr_cur[0] - imageStr_d[0]) * (invSubPixel.x * invSubPixel_delta) +
                               (imageStr_cur[1] - imageStr_d[1]) * (subPixel.x * invSubPixel_delta) +
                               (imageStr_next[0] - imageStr_cur[0]) * (invSubPixel.x * subPixel_delta) +
                               (imageStr_next[1] - imageStr_cur[1]) * (subPixel.x * subPixel_delta));
            } else {
                subPixel_delta = subPixel.y - 0.5f;
                invSubPixel_delta = 1.0f - subPixel_delta;
                imageStr_d = &imageStr_next[image.width()];
                diffImage.y = ((imageStr_next[0] - imageStr_cur[0]) * (invSubPixel.x * invSubPixel_delta) +
                               (imageStr_next[1] - imageStr_cur[1]) * (subPixel.x * invSubPixel_delta) +
                               (imageStr_d[0] - imageStr_next[0]) * (invSubPixel.x * subPixel_delta) +
                               (imageStr_d[1] - imageStr_next[1]) * (subPixel.x * subPixel_delta));
            }
            sum += std::fabs(diff);

            jac(0) = - diffImage.x;
            jac(1) = - diffImage.y;

            jac(2) = - ((diffImage.y * warp(0, 0) - diffImage.x * warp(1, 0)) * posInTemplate.x);
            jac(3) = - ((diffImage.y * warp(0, 1) - diffImage.x * warp(1, 1)) * posInTemplate.y);

            jac(4) = 1.0;

            jac(5) = templateStr[p.x];

            wls.addMeasurement(diff, jac);
        }
        templateStr = &templateStr[m_templateImage.width()];
    }

    wls.compute();
    TVectord update = wls.X();
    for (int i=0; i<5; ++i) {
        if (std::isnan(update(i)))
            return -1.0;
    }
    properties.pos.x += update(0);
    properties.pos.y += update(1);
    properties.angles.x += update(2);
    properties.angles.y += update(3);
    properties.mean += update(4);
    properties.gain += update(5);
    m_lastError = sum / (double)m_templateImage.area();
    return update.slice(2).length();
}

void CalibrationCorner::_makeSharedTemplate(int halfSideSize)
{
    m_sharedSourceTemplateImage = Image<float>(Point2i(halfSideSize * 2, halfSideSize * 2));

    float* str = m_sharedSourceTemplateImage.data();
    Point2i p;
    float fX, fY;
    for (p.y=0; p.y<m_sharedSourceTemplateImage.height(); ++p.y) {
        for (p.x=0; p.x<m_sharedSourceTemplateImage.width(); ++p.x) {
            fX = (p.x < halfSideSize) ? 1.0f : -1.0f;
            fY = (p.y < halfSideSize) ? 1.0f : -1.0f;
            str[p.x] = fX * fY;
        }
        str = &str[m_sharedSourceTemplateImage.width()];
    }
}

CalibrationCorner::Properties::Properties()
{
    angles.x = 0.0;
    angles.y = M_PI * 0.5;
    mean = 0.0;
    gain = 1.0;
}

TMath::TMatrixd CalibrationCorner::Properties::warp() const
{
    TMath::TMatrixd warp(2, 2);
    warp(0, 0) = std::cos(angles.x);
    warp(0, 1) = std::cos(angles.y);
    warp(1, 0) = std::sin(angles.x);
    warp(1, 1) = std::sin(angles.y);
    return warp;
}

} // namespace AR
