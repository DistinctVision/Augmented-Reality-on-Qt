#include "PreviewFrame.h"
#include "MapPoint.h"
#include "TMath/TMath.h"
#include "KeyFrame.h"
#include "Feature.h"
#include "MapPoint.h"
#include "MapResourceLocker.h"
#include "MapResourcesManager.h"
#include <algorithm>

namespace AR {

PreviewFrame::PreviewFrame(const std::shared_ptr<const Camera> & camera,
                           const std::vector<Image<uchar>> & imagePyramid,
                           const TMath::TMatrixd & rotation,
                           const TMath::TVectord & translation):
              Frame(camera, imagePyramid, rotation, translation)
{
}

std::size_t PreviewFrame::countPreviewFeatures() const
{
    return m_previewFeatures.size();
}

PreviewFrame::PreviewFeature & PreviewFrame::previewFeature(std::size_t index)
{
    return m_previewFeatures[index];
}

const PreviewFrame::PreviewFeature & PreviewFrame::previewFeature(std::size_t index) const
{
    return m_previewFeatures[index];
}

std::vector<PreviewFrame::PreviewFeature> & PreviewFrame::previewFeatures()
{
    return m_previewFeatures;
}
const std::vector<PreviewFrame::PreviewFeature> & PreviewFrame::previewFeatures() const
{
    return m_previewFeatures;
}

void PreviewFrame::addPreviewFeature(const PreviewFeature & previewFeature)
{
    m_previewFeatures.push_back(previewFeature);
}

void PreviewFrame::addPreviewFeature(const std::shared_ptr<MapPoint> & mapPoint,
                                     const Point2f & positionOnFrame,
                                     int imageLevel)
{
    m_previewFeatures.push_back({ mapPoint, positionOnFrame, imageLevel });
}

void PreviewFrame::deletePreviewFeature(std::size_t index)
{
    m_previewFeatures.erase(m_previewFeatures.begin() + index);
}

void PreviewFrame::copy(const std::shared_ptr<KeyFrame> & keyFrame)
{
    m_camera = keyFrame->camera();
    m_rotation = keyFrame->rotation();
    m_translation = keyFrame->translation();
    if (((int)m_imagePyramid.size() == keyFrame->countImageLevels()) &&
        (m_imagePyramid[0].size() == keyFrame->imageSize())) {
        for (std::size_t i = 0; i < m_imagePyramid.size(); ++i)
            Image<uchar>::copyData(m_imagePyramid[i], keyFrame->imageLevel(i));
    } else {
        m_imagePyramid.resize(keyFrame->countImageLevels());
        for (std::size_t i = 0; i < m_imagePyramid.size(); ++i)
            m_imagePyramid[i] = keyFrame->imageLevel(i).copy();
    }
    m_previewFeatures.clear();
    for (int i = 0; i < keyFrame->countFeatures(); ++i) {
        const std::shared_ptr<Feature> feature = keyFrame->feature(i);
        m_previewFeatures.push_back({ feature->mapPoint(), feature->positionOnFrame(), feature->imageLevel() });
    }
}

void PreviewFrame::copy(const PreviewFrame & frame)
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
    m_previewFeatures = frame.m_previewFeatures;
}

void PreviewFrame::getDepth(MapResourcesManager * manager, double & depthMean, double & depthMin, PreviewFrame & frame)
{
    using namespace TMath;

    std::vector<PreviewFeature> & features = frame.previewFeatures();

    if (features.empty()) {
        depthMean = depthMin = 0.0;
        return;
    }

    TMatrixd rotation = frame.rotation();
    TVectord translation = frame.translation();

    std::vector<double> vectorDepth;
    vectorDepth.reserve(features.size());
    depthMin = std::numeric_limits<double>::max();
    for (auto it = features.begin(); it != features.end(); ++it) {
        MapResourceLocker lockerR(manager, it->mapPoint.get()); (void)lockerR;
        TVectord localPos = rotation * it->mapPoint->position() + translation;
        if (depthMin > localPos(2))
            depthMin = localPos(2);
        vectorDepth.push_back(localPos(2));
    }
    if (vectorDepth.empty())
        return;
    std::size_t offset = vectorDepth.size() / 2;
    std::nth_element(vectorDepth.begin(), vectorDepth.begin() + offset, vectorDepth.end());
    depthMean = vectorDepth[offset];
}

}
