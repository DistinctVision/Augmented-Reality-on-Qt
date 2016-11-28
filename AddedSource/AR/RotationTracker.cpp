#include "RotationTracker.h"
#include "Camera.h"
#include "TMath/TMath.h"
#include "TukeyRobustCost.h"
#include <cmath>
#include <algorithm>

namespace AR {

RotationTracker::RotationTracker():
    m_firstRotation(3, 3), m_firstTranslation(3),
    m_secondRotation(3, 3), m_secondTranslation(3)
{
    m_last_X = TMath::TVectord(3);
    m_last_X.setZero();
    m_countTrackedFeatures = 0;
    m_gaussian = nullptr;
    setCursorSize(Point2i(2, 2));
    m_eps = 3e-5;
    m_numberIterations = 12;
    m_minLevel = 0;
    m_maxLevel = 2;
}

RotationTracker::~RotationTracker()
{
    if (m_gaussian != nullptr)
        delete[] m_gaussian;
}

void RotationTracker::setFirstFrame(const Frame& frame, std::vector<Point2f>& features)
{
    using namespace TMath;

    m_firstCamera = frame.camera();
    m_firstFrameImagePyramid.resize(frame.countImageLevels());
    for (int i = 0; i < frame.countImageLevels(); ++i) {
        m_firstFrameImagePyramid[i] = frame.imageLevel(i);
    }
    m_firstRotation = frame.rotation();
    m_firstTranslation = frame.translation();
    m_featuresInfo.resize(0);
    m_featuresInfo.reserve(features.size());
    FeatureInfo featureInfo;
    featureInfo.J_x = TVectord(3);
    featureInfo.J_y = TVectord(3);
    TVectord J_x(3), J_y(3);
    Camera::ProjectionInfo projectionInfo;
    TMatrixd projectionDerivatives;
    for (auto it = features.begin(); it != features.end(); ++it) {
        Point2d p = m_firstCamera->unproject(*it, projectionInfo);
        featureInfo.imagePosition = *it;
        featureInfo.localPosition = TVectord::create(p.x, p.y, 1.0);
        jacobian_xyz2uv(J_x, J_y, featureInfo.localPosition);
        projectionDerivatives = m_firstCamera->getProjectionDerivatives(projectionInfo);
        for (int j = 0; j < 3; ++j) {
            featureInfo.J_x(j) = projectionDerivatives(0, 0) * J_x(j) + projectionDerivatives(0, 1) * J_y(j);
            featureInfo.J_y(j) = projectionDerivatives(1, 0) * J_x(j) + projectionDerivatives(1, 1) * J_y(j);
        }
        m_featuresInfo.push_back(featureInfo);
    }
}

void RotationTracker::setSecondFrame(const Frame& frame)
{
    m_secondCamera = frame.camera();
    m_secondFrameImagePyramid.resize(frame.countImageLevels());
    for (int i = 0; i < frame.countImageLevels(); ++i)
        m_secondFrameImagePyramid[i] = frame.imageLevel(i);
    m_secondRotation = frame.rotation();
    m_secondTranslation = frame.translation();
}

double RotationTracker::eps() const
{
    return m_eps;
}

void RotationTracker::setEps(double eps)
{
    m_eps = eps;
}

int RotationTracker::numberIterations() const
{
    return m_numberIterations;
}

void RotationTracker::setNumberIterations(int numberIterations)
{
    m_numberIterations = numberIterations;
}

int RotationTracker::minLevel() const
{
    return m_minLevel;
}

int RotationTracker::maxLevel() const
{
    return m_maxLevel;
}

void RotationTracker::setMinMaxLevel(int minLevel, int maxLevel)
{
    m_minLevel = minLevel;
    m_maxLevel = maxLevel;
}

Point2i RotationTracker::cursorSize() const
{
    return m_cursorSize;
}

void RotationTracker::setCursorSize(const Point2i& cursorSize)
{
    assert((cursorSize.x > 0) && (cursorSize.y > 0));
    m_cursorSize = cursorSize;
    m_sigmaGaussian = std::max(m_cursorSize.x, m_cursorSize.y) / 3.0f;
    int memSize = (m_cursorSize.y * 2 + 1) * (m_cursorSize.x * 2 + 1);
    m_gaussian = static_cast<float*>(realloc(m_gaussian, sizeof(float) * memSize));
    _solveGaussian();
}

int RotationTracker::countTrackedFeatures() const
{
    return m_countTrackedFeatures;
}
void RotationTracker::_setSigmaGaussian(float sigma)
{
    m_sigmaGaussian = sigma;
    _solveGaussian();
}

void RotationTracker::_solveGaussian()
{
    double maxErrorSquared = 0.0;
    float sigmaSquare = m_sigmaGaussian * m_sigmaGaussian;
    double k1 = 1.0 / (2.0 * M_PI * sigmaSquare);
    double invK2 = 1.0 / (2.0 * sigmaSquare);
    Point2i p;
    int k = 0;
    for (p.y = - m_cursorSize.y; p.y <= m_cursorSize.y; ++p.y) {
        for (p.x = - m_cursorSize.x; p.x <= m_cursorSize.x; ++p.x, ++k) {
            m_gaussian[k] = (float)(k1 * std::exp(- p.lengthSquared() * invK2));
            maxErrorSquared += (m_gaussian[k] * m_gaussian[k]) * (255.0 * 255.0);
        }
    }
    if (maxErrorSquared > std::numeric_limits<float>::epsilon())
        m_invMaxErrorSquared = (float)(1.0 / maxErrorSquared);
    else
        m_invMaxErrorSquared = 0.0f;
}

TMath::TMatrixd RotationTracker::secondRotation() const
{
    return m_secondRotation;
}

TMath::TVectord RotationTracker::secondTranslation() const
{
    return m_secondTranslation;
}

bool RotationTracker::tracking()
{
    using namespace TMath;
    TMath_assert((m_firstFrameImagePyramid.size() > 0) && (m_secondFrameImagePyramid.size() > 0));
    Point2i patchSize = m_cursorSize * 2 + Point2i(1, 1);
    m_jacobian_cashe = Image<double>(Point2i(patchSize.y * patchSize.x * 6, (int)m_featuresInfo.size()));
    m_pixels_cashe = Image<float>(Point2i(patchSize.y * patchSize.x, (int)m_featuresInfo.size()));
    int i = 0;
    for (std::vector<FeatureInfo>::iterator it = m_featuresInfo.begin();
            it != m_featuresInfo.end();
            ++it) {
        it->jacobian_cache = m_jacobian_cashe.pointer(0, i);
        it->pixels_cache = m_pixels_cashe.pointer(0, i);
        ++i;
    }

    int countLevels = std::min((int)m_firstFrameImagePyramid.size(), (int)m_secondFrameImagePyramid.size());

    int minLevel = (m_minLevel < 0) ? 0 : m_minLevel;
    int maxLevel = ((m_maxLevel < 0) || (m_maxLevel >= countLevels)) ? (countLevels - 1) : m_maxLevel;

    TMatrixd firstInvRotation = m_firstRotation;
    if (!TTools::matrix3x3Invert(firstInvRotation))
        return false;
    bool success = false;
    TMatrixd deltaRotation = m_secondRotation * firstInvRotation;
    TVectord deltaTranslation = m_secondRotation * (- firstInvRotation * m_firstTranslation) + m_secondTranslation;
    TMatrixd prevDeltaRotation, currentDeltaRotation;
    TVectord prevDeltaTranslation, currentDeltaTranslation;
    for (int level = maxLevel; level >= minLevel; --level) {
        _computeLevelInfo(deltaRotation, deltaTranslation, level);
        prevDeltaRotation = deltaRotation;
        prevDeltaTranslation = deltaTranslation;
        double prevError = _optimize(deltaRotation, deltaTranslation, level), error = 0.0;
        for (i = 1; i < m_numberIterations; ++i) {
            currentDeltaRotation = deltaRotation;
            currentDeltaTranslation = deltaTranslation;
            error = _optimize(deltaRotation, deltaTranslation, level);
            if (_needToStop()) {
                success = true;
                break;
            }
            if (prevError * 1.2 < error) {
                error = prevError;
                deltaRotation = prevDeltaRotation;
                deltaTranslation = prevDeltaTranslation;
                break;
            } else if (error < prevError) {
                success = true;
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
    return success;
}


float RotationTracker::computeDisparity() const
{
    using namespace TMath;

    if (m_featuresInfo.empty())
        return 0.0f;

    TMatrixd firstInvRotation = m_firstRotation;
    if (!TTools::matrix3x3Invert(firstInvRotation))
        return 0.0f;
    TMatrixd deltaRotation = m_secondRotation * firstInvRotation;
    TVectord deltaTranslation = m_secondRotation * (- firstInvRotation * m_firstTranslation) + m_secondTranslation;

    std::vector<float> disparities;
    disparities.resize(m_featuresInfo.size());
    TVectord v(3);
    for (std::size_t i = 0; i < m_featuresInfo.size(); ++i) {
        v = deltaRotation * m_featuresInfo[i].localPosition + deltaTranslation;
        Point2f p = m_secondCamera->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>();
        disparities[i] = (p - m_featuresInfo[i].imagePosition).lengthSquared();
    }
    int offset = (int) std::floor(disparities.size() * 0.5f);
    std::nth_element(disparities.begin(), disparities.begin() + offset, disparities.end());
    return std::sqrt(disparities[offset]);
}

void RotationTracker::_computeLevelInfo(const TMath::TMatrixd& deltaRotation, const TMath::TVectord& deltaTranslation, int level)
{
    using namespace TMath;

    m_square_errors.resize(0);

    ConstImage<uchar> firstImage = m_firstFrameImagePyramid[level];
    ConstImage<uchar> secondImage = m_secondFrameImagePyramid[level];

    Point2i begin = m_cursorSize + Point2i(1, 1);
    Point2i end = firstImage.size() - (m_cursorSize + Point2i(2, 2));

    float scale = 1.0f / (float)(1 << level);

    TVectord v;

    Point2i p;
    Point2i pos_i;
    Point2f pos;
    Point2f subpix;
    float w_tl, w_tr, w_bl, w_br;
    Point2i sec_pos_i;
    Point2f sec_pos;
    Point2f sec_subpix;
    float sec_w_tl, sec_w_tr, sec_w_bl, sec_w_br;
    Point2f d;

    double commonMultiplier = 1.0 / (double)(1 << level);

    float dt, featureError;

    int k;

    for (std::size_t i = 0; i < m_featuresInfo.size(); ++i) {
        FeatureInfo& featureInfo = m_featuresInfo[i];

        pos = featureInfo.imagePosition * scale;
        pos_i.set((int)std::floor(pos.x), (int)std::floor(pos.y));

        if ((pos_i.x < begin.x) || (pos_i.y < begin.y) ||
                (pos_i.x > end.x) || (pos_i.y > end.y)) {
            featureInfo.visible = false;
            continue;
        }

        featureInfo.visible = true;

        featureError = 0.0f;

        subpix.set(pos.x - pos_i.x, pos.y - pos_i.y);
        w_tl = (1.0f - subpix.x) * (1.0f - subpix.y);
        w_tr = subpix.x * (1.0f - subpix.y);
        w_bl = (1.0f - subpix.x) * subpix.y;
        w_br = subpix.x * subpix.y;

        const uchar* imageStr = firstImage.pointer(pos_i.x, pos_i.y - m_cursorSize.y);
        const uchar* imageStrPrev = &imageStr[-firstImage.width()];
        const uchar* imageStrNext = &imageStr[firstImage.width()];
        const uchar* imageStrNextNext = &imageStrNext[firstImage.width()];

        if (!featureInfo.visible)
            continue;

        v = deltaRotation * featureInfo.localPosition + deltaTranslation;

        if (v(2) <= std::numeric_limits<float>::epsilon()) {
            featureInfo.visible = false;
            continue;
        }

        sec_pos = m_secondCamera->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>() * scale;
        sec_pos_i.set((int)std::floor(sec_pos.x), (int)std::floor(sec_pos.y));

        if ((sec_pos_i.x < begin.x) || (sec_pos_i.y < begin.y) ||
                (sec_pos_i.x > end.x) || (sec_pos_i.y > end.y)) {
            featureInfo.visible = false;
            continue;
        }

        sec_subpix.set((float)(sec_pos.x - sec_pos_i.x), (float)(sec_pos.y - sec_pos_i.y));

        sec_w_tl = (1.0f - sec_subpix.x) * (1.0f - sec_subpix.y);
        sec_w_tr = sec_subpix.x * (1.0f - sec_subpix.y);
        sec_w_bl = (1.0f - sec_subpix.x) * sec_subpix.y;
        sec_w_br = sec_subpix.x * sec_subpix.y;

        const uchar* secondImageStr = secondImage.pointer(sec_pos_i.x, sec_pos_i.y - m_cursorSize.y);
        const uchar* secondImageStrNext = &secondImageStr[secondImage.width()];

        double* jacobian_cache = featureInfo.jacobian_cache;
        float* pixels_cache = featureInfo.pixels_cache;
        k = 0;
        for (p.y = - m_cursorSize.y; p.y <= m_cursorSize.y; ++p.y) {
            for (p.x = - m_cursorSize.x; p.x <= m_cursorSize.x; ++p.x, ++k) {
                *pixels_cache = imageStr[p.x] * w_tl +
                                imageStr[p.x + 1] * w_tr +
                                imageStrNext[p.x] * w_bl +
                                imageStrNext[p.x + 1] * w_br;
                dt = (secondImageStr[p.x] * sec_w_tl +
                      secondImageStr[p.x + 1] * sec_w_tr +
                      secondImageStrNext[p.x] * sec_w_bl +
                      secondImageStrNext[p.x + 1] * sec_w_br) - *pixels_cache;
                featureError += dt * m_gaussian[k];
                ++pixels_cache;

                d.x = ((imageStr[p.x + 1] - imageStr[p.x - 1]) * w_tl +
                       (imageStr[p.x + 2] - imageStr[p.x]) * w_tr +
                       (imageStrNext[p.x + 1] - imageStrNext[p.x - 1]) * w_bl +
                       (imageStrNext[p.x + 2] - imageStrNext[p.x]) * w_br) * 0.5f;
                d.y = ((imageStrNext[p.x] - imageStrPrev[p.x]) * w_tl +
                       (imageStrNext[p.x + 1] - imageStrPrev[p.x + 1]) * w_tr +
                       (imageStrNextNext[p.x] - imageStr[p.x]) * w_bl +
                       (imageStrNextNext[p.x + 1] - imageStr[p.x + 1]) * w_br) * 0.5f;

                //don't believe in optimizator
                jacobian_cache[0] = (featureInfo.J_x(0) * d.x + featureInfo.J_y(0) * d.y) * commonMultiplier;
                jacobian_cache[1] = (featureInfo.J_x(1) * d.x + featureInfo.J_y(1) * d.y) * commonMultiplier;
                jacobian_cache[2] = (featureInfo.J_x(2) * d.x + featureInfo.J_y(2) * d.y) * commonMultiplier;
                jacobian_cache = &jacobian_cache[3];
            }
            imageStrPrev = imageStr;
            imageStr = imageStrNext;
            imageStrNext = imageStrNextNext;
            imageStrNextNext = &imageStrNextNext[firstImage.width()];

            secondImageStr = secondImageStrNext;
            secondImageStrNext = &secondImageStrNext[secondImage.width()];
        }
        m_square_errors.push_back(featureError * featureError);
    }
    if (m_square_errors.empty())
        m_sigmaSquared = 1.0f;
    else
        m_sigmaSquared = TukeyRobustCost::findSquareSigma(m_square_errors);
}

double RotationTracker::_optimize(TMath::TMatrixd& deltaRotation, TMath::TVectord& deltaTranslation, int level)
{
    using namespace TMath;

    ConstImage<uchar> secondImage = m_secondFrameImagePyramid[level];

    Point2i begin = m_cursorSize + Point2i(1, 1);
    Point2i end = secondImage.size() - (m_cursorSize + Point2i(2, 2));

    float scale = 1.0f / (float)(1 << level);

    double A_raw[6];

    TVectord B(3);
    B.setZero();

    int i, j, k, t;

    for (i = 0; i < 6; ++i)
        A_raw[i] = 0.0;

    Point2i p;
    Point2i pos_i;
    Point2f pos;
    Point2f subpix;
    float w_tl, w_tr, w_bl, w_br;
    float dt, wdt, weight;

    TVectord v;

    double error = 0.0;
    m_countTrackedFeatures = 0;

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
                for (i = 0; i < 3; ++i) {
                    for (j = 0; j <= i; ++j) {
                        A_raw[t] += jacobian_cache[i] * jacobian_cache[j] * weight;//J * J.transposed() * w
                        ++t;
                    }
                    B(i) += jacobian_cache[i] * wdt; // J * w
                }

                ++pixels_cache;
                jacobian_cache = &jacobian_cache[3];
            }
            imageStr = imageStrNext;
            imageStrNext = &imageStrNext[secondImage.width()];
        }
        ++m_countTrackedFeatures;
    }

    if (m_countTrackedFeatures == 0)
        return std::numeric_limits<double>::max();

    TMatrixd A(3, 3);
    t = 0;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < i; ++j) {
            A(i, j) = A(j, i) = A_raw[t];
            ++t;
        }
        A(i, i) = A_raw[t];
        ++t;
    }
    TCholesky<double> cholesky(A);
    m_last_X = cholesky.backsub(B);
    for (i = 0; i < 3; ++i) {
        if (std::isnan(m_last_X(i)))
            return std::numeric_limits<double>::max();
    }

    TMatrixd dRotation(3, 3);
    TVectord dTranslation(3);
    dTranslation.setZero();
    TTools::exp_rotationMatrix(dRotation, m_last_X);

    deltaRotation = dRotation * deltaRotation;
    deltaTranslation = dRotation * deltaTranslation + dTranslation;

    return (error * m_invMaxErrorSquared) / (double)m_countTrackedFeatures;
}

bool RotationTracker::_needToStop() const
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
