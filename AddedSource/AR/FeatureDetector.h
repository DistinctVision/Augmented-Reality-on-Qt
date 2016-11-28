#ifndef AR_FEATUREDETECTOR_H
#define AR_FEATUREDETECTOR_H

#include <vector>
#include "Image.h"
#include "OpticalFlow.h"
#include "FastCorner.h"

namespace AR {

class FeatureDetector: public OpticalFlow
{
public:
    struct FeatureCorner {
        Point2i pos;
        int level;

        FeatureCorner() {}
        FeatureCorner(const Point2i& pos, int level)
        {
            this->pos = pos;
            this->level = level;
        }
    };

    FeatureDetector();

    Point2i gridSize() const;
    void setGridSize(const Point2i& gridSize);

    int barrier() const;
    void setBarrier(int barrier);

    float detectionThreshold() const;
    void setDetectionThreshold(float threshold);

    int minLevelForFeature() const;
    int maxLevelForFeature() const;
    void setLevelForFeature(int minLevel, int maxLevel);

    void setFirstImage(const ImageRef<uchar>& image) override;
    void setFirstImage(const Frame& frame) override;

    void detectFeaturesOnFirstImage(std::vector<FeatureCorner>& features);
    void fillFeaturesOnFirstImage(std::vector<FeatureCorner>& features);

    void reset() override;

    void freeGrid();
    void setCellLock(const Point2i& point);
    void setCellLock(const Point2f& point);

    int indexOfCell(const Point2i& point) const;
    int indexOfCell(const Point2f& point) const;

    int maxCountFeatures() const;
    void setMaxCountFeatures(int maxCountFeatures);

protected:
    struct Cell {
        Point2i pos;
        float score;
        int level;
        bool lock;
    };

    int m_barrier;
    float m_detectionThreshold;
    int m_minLevelForFeature;
    int m_maxLevelForFeature;
    int m_maxCountFeatures;

    std::vector<FastCorner::Corner> m_candidates;
    std::vector<FastCorner::Corner> m_finalCandidates;
    Point2i m_gridSize;
    Point2f m_cellSize;
    std::vector<Cell> m_cells;
};

}

#endif // AR_FEATUREDETECTOR_H
