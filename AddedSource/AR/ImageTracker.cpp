#include "ImageTracker.h"
#include "LocationOptimizer.h"
#include "Camera.h"
#include "TMath/TMath.h"
#include "TukeyRobustCost.h"
#include <cmath>

namespace AR {

ImageTracker::ImageTracker():
    m_firstRotation(3, 3), m_firstTranslation(3),
    m_secondRotation(3, 3), m_secondTranslation(3)
{
    m_last_X = TMath::TVectord(6);
    m_last_X.setZero();
    m_eps = 3e-5;
    m_numberIterations = 20;
    m_minLevel = 0;
    m_maxLevel = 2;
}

ImageTracker::~ImageTracker()
{
}

void ImageTracker::setFirstFrame(const Frame& frame,
                                 const std::vector<ConstImage<float>>& depthImagePyramid,
                                 const std::vector<ConstImage<float>>& weightImagePyramid)
{
    using namespace TMath;

    int countLevels = (int)m_firstFrameImagePyramid.size();

    m_firstCamera = frame.camera();
    m_firstFrameImagePyramid.resize(countLevels);
    m_firstDepthImagePyramid.resize(countLevels);
    m_firstWeightImagePyramid.resize(countLevels);
    m_tempCashePyramid.resize(countLevels);
    for (int i = 0; i < countLevels; ++i) {
        m_firstFrameImagePyramid[i] = frame.imageLevel(i);
        m_firstDepthImagePyramid[i] = depthImagePyramid[i];
        m_firstWeightImagePyramid[i] = weightImagePyramid[i];
        assert(m_firstFrameImagePyramid[i].size() == m_firstDepthImagePyramid[i].size());
        if (m_firstFrameImagePyramid[i].size() != m_tempCashePyramid[i].size()) {
            m_tempCashePyramid[i] = Image<CashePixelInfo>(m_firstFrameImagePyramid[i].size());
        }
    }
    m_firstRotation = frame.rotation();
    m_firstTranslation = frame.translation();

    int minLevel = (m_minLevel < 0) ? 0 : m_minLevel;
    int maxLevel = ((m_maxLevel < 0) || (m_maxLevel >= countLevels)) ? (countLevels - 1) : m_maxLevel;

    float J_x[6], J_y[6];
    Camera::ProjectionInfo projectionInfo;
    TMatrixd projectionDerivatives;
    Point2i p;
    for (int i = minLevel; i < maxLevel; ++i) {
        ConstImage<float> depthImage = m_firstDepthImagePyramid[i];
        const float* depthImageStr = depthImage.data();
        CashePixelInfo* tempPixelInfoStr = m_tempCashePyramid[i].data();
        for (p.y = 0; p.y < depthImage.height(); ++p.y) {
            for (p.x = 0; p.x < depthImage.width(); ++p.x) {
                Point2d u_p = m_firstCamera->unproject(p, projectionInfo);
                tempPixelInfoStr[p.x].localPos[0] = (float)u_p.x;
                tempPixelInfoStr[p.x].localPos[1] = (float)u_p.y;
                tempPixelInfoStr[p.x].localPos[2] = 1.0f;
                LocationOptimizer::jacobian_xyz2uv(J_x, J_y, tempPixelInfoStr[p.x].localPos);
                projectionDerivatives = m_firstCamera->getProjectionDerivatives(projectionInfo);
                for (int j = 0; j < 6; ++j) {
                    tempPixelInfoStr[p.x].J_x[j] = projectionDerivatives(0, 0) * J_x[j] +
                            projectionDerivatives(0, 1) * J_y[j];
                    tempPixelInfoStr[p.x].J_y[j] = projectionDerivatives(1, 0) * J_x[j] +
                            projectionDerivatives(1, 1) * J_y[j];
                }
            }
            depthImageStr = &depthImageStr[depthImage.width()];
        }
    }
}

void ImageTracker::setSecondFrame(const Frame& frame)
{
    m_secondCamera = frame.camera();
    m_secondFrameImagePyramid.resize(frame.countImageLevels());
    for (int i = 0; i < frame.countImageLevels(); ++i)
        m_secondFrameImagePyramid[i] = frame.imageLevel(i);
    m_secondRotation = frame.rotation();
    m_secondTranslation = frame.translation();
}

double ImageTracker::eps() const
{
    return m_eps;
}

void ImageTracker::setEps(double eps)
{
    m_eps = eps;
}

int ImageTracker::numberIterations() const
{
    return m_numberIterations;
}

void ImageTracker::setNumberIterations(int numberIterations)
{
    m_numberIterations = numberIterations;
}

int ImageTracker::minLevel() const
{
    return m_minLevel;
}

int ImageTracker::maxLevel() const
{
    return m_maxLevel;
}

void ImageTracker::setMinMaxLevel(int minLevel, int maxLevel)
{
    m_minLevel = minLevel;
    m_maxLevel = maxLevel;
}

TMath::TMatrixd ImageTracker::secondRotation() const
{
    return m_secondRotation;
}

TMath::TVectord ImageTracker::secondTranslation() const
{
    return m_secondTranslation;
}

void ImageTracker::tracking()
{
    using namespace TMath;
    TMath_assert((m_firstFrameImagePyramid.size() > 0) && (m_secondFrameImagePyramid.size() > 0));

    int countLevels = std::min((int)m_firstFrameImagePyramid.size(), (int)m_secondFrameImagePyramid.size());

    int minLevel = (m_minLevel < 0) ? 0 : m_minLevel;
    int maxLevel = ((m_maxLevel < 0) || (m_maxLevel >= countLevels)) ? (countLevels - 1) : m_maxLevel;

    TMatrixd firstInvRotation = m_firstRotation;
    if (!TTools::matrix3x3Invert(firstInvRotation))
        return;
    TMatrixd deltaRotation = m_secondRotation * firstInvRotation;
    TVectord deltaTranslation = m_secondRotation * (- firstInvRotation * m_firstTranslation) + m_secondTranslation;
    TMatrixd prevDeltaRotation, currentDeltaRotation;
    TVectord prevDeltaTranslation, currentDeltaTranslation;
    for (int level = maxLevel; level >= minLevel; --level) {
        _computeLevelInfo(deltaRotation, deltaTranslation, level);
        prevDeltaRotation = deltaRotation;
        prevDeltaTranslation = deltaTranslation;
        double prevError = _optimize(deltaRotation, deltaTranslation, level), error = 0.0;
        for (int i = 1; i < m_numberIterations; ++i) {
            currentDeltaRotation = deltaRotation;
            currentDeltaTranslation = deltaTranslation;
            error = _optimize(deltaRotation, deltaTranslation, level);
            if (_needToStop())
                break;
            if (prevError * 1.5 < error) {
                error = prevError;
                deltaRotation = prevDeltaRotation;
                deltaTranslation = prevDeltaTranslation;
                break;
            } else if (error < prevError) {
                prevError = error;
                prevDeltaRotation = currentDeltaRotation;
                prevDeltaTranslation = currentDeltaTranslation;
            }
        }
        if (prevError < error) {
            deltaRotation = prevDeltaRotation;
            deltaTranslation = prevDeltaTranslation;
        }
    }
    m_secondRotation = deltaRotation * m_firstRotation;
    m_secondTranslation = deltaRotation * m_firstTranslation + deltaTranslation;
}

void ImageTracker::_computeLevelInfo(const TMath::TMatrixd& deltaRotation, const TMath::TVectord& deltaTranslation, int level)
{
    using namespace TMath;

    ConstImage<float> firstWeights = m_firstWeightImagePyramid[level];
    ConstImage<float> firstDepth = m_firstDepthImagePyramid[level];
    ConstImage<uchar> firstImage = m_firstFrameImagePyramid[level];
    ConstImage<uchar> secondImage = m_secondFrameImagePyramid[level];
    ConstImage<CashePixelInfo> casheImage = m_tempCashePyramid[level];

    float scale = 1.0f / (float)(1 << level);

    Point2i p;
    float dT;
    Point2f d;

    double commonMultiplier = 1.0 / (double)(1 << level);

    const float* weightsStr = firstWeights.pointer(1, 1);
    const float* depthStr = firstDepth.pointer(1, 1);
    const uchar* firstImageStr = firstImage.pointer(1, 1);
    const uchar* firstImageStr_prev = &firstImageStr[-firstImage.width()];
    const uchar* firstImageStr_next = &firstImageStr[firstImage.width()];
    const uchar* secondImageStr = secondImage.pointer(1, 1);
    CashePixelInfo* casheStr = casheImage.pointer(1, 1);

    double sumError = 0.0;

    for (p.y = 1; p.y < firstImage.height() - 1; ++p.y) {
        for (p.x = 1; p.x < firstImage.width() - 1; ++p.x) {
            dT = secondImageStr[p.x] - firstImageStr[p.x];
            sumError += dT;
            d.x = firstImageStr[p.x + 1] - firstImageStr[p.x - 1];
            d.y = firstImageStr_next[p.x] - firstImageStr_prev[p.x];
            casheStr->J[0] = (casheStr[p.x].J_x[0] * d.x + casheStr[p.x].J_y[0] * d.y) * commonMultiplier;
            casheStr->J[1] = (casheStr[p.x].J_x[1] * d.x + casheStr[p.x].J_y[1] * d.y) * commonMultiplier;
            casheStr->J[2] = (casheStr[p.x].J_x[2] * d.x + casheStr[p.x].J_y[2] * d.y) * commonMultiplier;
            casheStr->J[3] = (casheStr[p.x].J_x[3] * d.x + casheStr[p.x].J_y[3] * d.y) * commonMultiplier;
            casheStr->J[4] = (casheStr[p.x].J_x[4] * d.x + casheStr[p.x].J_y[4] * d.y) * commonMultiplier;
            casheStr->J[5] = (casheStr[p.x].J_x[5] * d.x + casheStr[p.x].J_y[5] * d.y) * commonMultiplier;
        }
        weightsStr = &weightsStr[firstImage.width()];
        depthStr = &depthStr[firstDepth.width()];
        firstImageStr_prev = firstImageStr;
        firstImageStr = firstImageStr_next;
        firstImageStr_next = &firstImageStr_next[firstImageStr];
        secondImageStr = &secondImageStr[secondImage.width()];
        casheStr = &casheStr[casheImage.width()];
    }
    double sigma = sumError / (double)((firstImage.width() - 2) * (firstImage.height() - 2));
    m_sigmaSquared = (float)(sigma * sigma);
}

double ImageTracker::_optimize(TMath::TMatrixd& deltaRotation, TMath::TVectord& deltaTranslation, int level)
{
    using namespace TMath;

    ConstImage<uchar> secondImage = m_secondFrameImagePyramid[level];

    float scale = 1.0f / (float)(1 << level);

    double A_raw[21];

    TVectord B(6);
    B.setZero();

    int i, j, k, t;

    for (i = 0; i < 21; ++i)
        A_raw[i] = 0.0;

    Point2i p;
    Point2i pos_i;
    Point2f pos;
    Point2f subpix;
    float w_tl, w_tr, w_bl, w_br;
    float dt, wdt, weight;

    TVectord v;

    double error = 0.0;

    for (std::size_t f = 0; f < m_featuresInfo.size(); ++f) {
        FeatureInfo& featureInfo = m_featuresInfo[f];

        if (!featureInfo.visible)
            continue;

        v = deltaRotation * featureInfo.localPosition + deltaTranslation;

        if (v(2) <= std::numeric_limits<float>::epsilon()) {
            featureInfo.visible = false;
            continue;
        }

        pos = m_secondCamera->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>() * scale;
        pos_i.set((int)std::floor(pos.x), (int)std::floor(pos.y));

        if ((pos_i.x < begin.x) || (pos_i.y < begin.y) ||
                (pos_i.x > end.x) || (pos_i.y > end.y)) {
            featureInfo.visible = false;
            continue;
        }

        subpix.set((float)(pos.x - pos_i.x), (float)(pos.y - pos_i.y));

        w_tl = (1.0f - subpix.x) * (1.0f - subpix.y);
        w_tr = subpix.x * (1.0f - subpix.y);
        w_bl = (1.0f - subpix.x) * subpix.y;
        w_br = subpix.x * subpix.y;

        const uchar* imageStr = secondImage.pointer(pos_i.x, pos_i.y - m_cursorSize.y);
        const uchar* imageStrNext = &imageStr[secondImage.width()];
        k = 0;
        const float* pixels_cache = featureInfo.pixels_cache;
        const double* jacobian_cache = featureInfo.jacobian_cache;
        for (p.y = - m_cursorSize.y; p.y <= m_cursorSize.y; ++p.y) {
            for (p.x = - m_cursorSize.x; p.x <= m_cursorSize.x; ++p.x, ++k) {
                dt = (imageStr[p.x] * w_tl +
                      imageStr[p.x + 1] * w_tr +
                      imageStrNext[p.x] * w_bl +
                      imageStrNext[p.x + 1] * w_br) - *pixels_cache;
                weight = m_gaussian[k] * TukeyRobustCost::weight(dt * dt, m_sigmaSquared);
                wdt = dt * weight;

                error += wdt * dt;

                t = 0;
                for (i = 0; i < 6; ++i) {
                    for (j = 0; j <= i; ++j) {
                        A_raw[t] += jacobian_cache[i] * jacobian_cache[j] * weight;//J * J.transposed() * w
                        ++t;
                    }
                    B(i) += jacobian_cache[i] * wdt; // J * w
                }

                ++pixels_cache;
                jacobian_cache = &jacobian_cache[6];
            }
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[secondImage.width()];
        }
        ++m_countTrackedFeatures;
    }

    if (m_countTrackedFeatures == 0)
        return std::numeric_limits<double>::max();

    TMatrixd A(6, 6);
    t = 0;
    for (i = 0; i < 6; ++i) {
        for (j = 0; j < i; ++j) {
            A(i, j) = A(j, i) = A_raw[t];
            ++t;
        }
        A(i, i) = A_raw[t];
        ++t;
    }
    TCholesky<double> cholesky(A);
    m_last_X = cholesky.backsub(B);
    for (i = 0; i < 6; ++i) {
        if (std::isnan(m_last_X[i]))
            return std::numeric_limits<double>::max();
    }

    TMatrixd dRotation(3, 3);
    TVectord dTranslation(3);
    TTools::exp_transform(dRotation, dTranslation, m_last_X);

    deltaRotation = dRotation * deltaRotation;
    deltaTranslation = dRotation * deltaTranslation + dTranslation;

    return (error * m_invMaxErrorSquared) / (double)m_countTrackedFeatures;
}

bool ImageTracker::_needToStop() const
{
    double max = 0.0;
    for (int i = 0; i < m_last_X.size(); ++i) {
        double fv = std::fabs(m_last_X(i));
        if (fv > max) {
            max = fv;
        }
    }
    return (max < m_eps);
}

}
