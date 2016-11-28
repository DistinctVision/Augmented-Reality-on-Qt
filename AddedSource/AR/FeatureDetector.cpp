#include "FeatureDetector.h"
#include <cassert>

namespace AR {

FeatureDetector::FeatureDetector():
        OpticalFlow()
{
    m_barrier = 3;
    m_detectionThreshold = 5.0f;
    m_minLevelForFeature = m_maxLevelForFeature = -1;
    setGridSize(Point2i(20, 20));
    m_maxCountFeatures = 100;
}

void FeatureDetector::setFirstImage(const ImageRef<uchar> & image)
{
    OpticalFlow::setFirstImage(image);
    m_cellSize.set(m_level0_first.width() / (float)(m_gridSize.x),
                   m_level0_first.height() / (float)(m_gridSize.y));
}

void FeatureDetector::setFirstImage(const Frame & frame)
{
    OpticalFlow::setFirstImage(frame);
    m_cellSize.set(m_level0_first.width() / (float)(m_gridSize.x),
                   m_level0_first.height() / (float)(m_gridSize.y));
}

Point2i FeatureDetector::gridSize() const
{
    return m_gridSize;
}

void FeatureDetector::setGridSize(const Point2i & gridSize)
{
    assert((gridSize.x > 0) && (gridSize.y > 0));
    m_gridSize = gridSize;
    Cell cell;
    cell.level = 0;
    cell.lock = false;
    cell.pos.setZero();
    cell.score = 0.0f;
    m_cells.resize(m_gridSize.y * m_gridSize.x, cell);
    m_cellSize.set(m_level0_first.width() / (float)(m_gridSize.x),
                   m_level0_first.height() / (float)(m_gridSize.y));
}

void FeatureDetector::freeGrid()
{
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it)
        it->lock = false;
}

void FeatureDetector::setCellLock(const Point2i & point)
{
    int k = (int)(std::floor(point.y / m_cellSize.y) * m_gridSize.y + std::floor(point.x / m_cellSize.x));
    m_cells[k].lock = true;
}

void FeatureDetector::setCellLock(const Point2f & point)
{
    int k = (int)(std::floor(point.y / m_cellSize.y) * m_gridSize.y + std::floor(point.x / m_cellSize.x));
    m_cells[k].lock = true;
}

int FeatureDetector::indexOfCell(const Point2i & point) const
{
    return (int)(std::floor(point.y / m_cellSize.y) * m_gridSize.y + std::floor(point.x / m_cellSize.x));
}

int FeatureDetector::indexOfCell(const Point2f & point) const
{
    return (int)(std::floor(point.y / m_cellSize.y) * m_gridSize.y + std::floor(point.x / m_cellSize.x));
}

int FeatureDetector::maxCountFeatures() const
{
    return m_maxCountFeatures;
}

void FeatureDetector::setMaxCountFeatures(int maxCountFeatures)
{
    m_maxCountFeatures = maxCountFeatures;
}

int FeatureDetector::barrier() const
{
    return m_barrier;
}

void FeatureDetector::setBarrier(int barrier)
{
    m_barrier = barrier;
}

float FeatureDetector::detectionThreshold() const
{
    return m_detectionThreshold;
}

void FeatureDetector::setDetectionThreshold(float threshold)
{
    assert(threshold > 0.0f);
    m_detectionThreshold = threshold;
}

int FeatureDetector::minLevelForFeature() const
{
    return m_minLevelForFeature;
}

int FeatureDetector::maxLevelForFeature() const
{
    return m_maxLevelForFeature;
}

void FeatureDetector::setLevelForFeature(int minLevel, int maxLevel)
{
    m_minLevelForFeature = minLevel;
    m_maxLevelForFeature = maxLevel;
}

void FeatureDetector::reset()
{
    m_candidates.clear();
    m_finalCandidates.clear();
    OpticalFlow::reset();
}

void FeatureDetector::detectFeaturesOnFirstImage(std::vector<FeatureCorner> & features)
{
    assert((m_gridSize.x > 0) && (m_gridSize.y > 0));

    Point2i imageSize = m_level0_first.size();

    assert((imageSize.x != 0) && (imageSize.y != 0));

    Point2f cellSize(imageSize.x / (float)(m_gridSize.x),
                     imageSize.y / (float)(m_gridSize.y));

    m_finalCandidates.resize(0);
    std::size_t prevSize = 0;

    int minLevel = (m_minLevelForFeature < 0) ? 0 : m_minLevelForFeature;
    int maxLevel = ((m_maxLevelForFeature < 0) || (m_maxLevelForFeature >= countLevels())) ?
                        (countLevels() - 1) :
                        m_maxLevelForFeature;

    for (int level = minLevel; level <= maxLevel; ++level) {
        ConstImage<uchar> image = firstImageAtLevel(level);

        m_candidates.resize(0);
        FastCorner::fast_corner_detect_10(image,
                                          m_opticalFlowCalculator.cursorSize(),
                                          image.size() - (m_opticalFlowCalculator.cursorSize() + Point2i(1, 1)),
                                          m_candidates,
                                          m_barrier);
        FastCorner::fastNonmaxSuppression(m_candidates, m_finalCandidates);
        if (level > 0) {
            int scale = (1 << level);
            for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin() + prevSize;
                 it != m_finalCandidates.end();
                 ++it) {
                it->pos *= scale;
                if ((it->pos.x < 0) || (it->pos.y < 0) || (it->pos.x >= imageSize.x) || (it->pos.y >= imageSize.y)) {
                    assert(false);
                }
                it->level = level;
            }
            break;
        } else {
            for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin() + prevSize;
                 it != m_finalCandidates.end();
                 ++it) {
                it->level = 0;
            }
        }
        prevSize = m_finalCandidates.size();
    }

    for (std::vector<Cell>::iterator it = m_cells.begin(); it != m_cells.end(); ++it) {
        it->score = 0.0f;
    }

    ConstImage<uchar> firstImage = this->firstImage();
    int k;
    float score;
    for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin();
            it != m_finalCandidates.end();
            ++it) {
        k = (int)(std::floor(it->pos.y / cellSize.y) * m_gridSize.y + std::floor(it->pos.x / cellSize.x));
        if (m_cells[k].lock)
            continue;
        score = FastCorner::shiTomasiScore_10(firstImage, it->pos);
        if (score > m_cells[k].score) {
            m_cells[k].score = score;
            m_cells[k].pos = it->pos;
            m_cells[k].level = it->level;
        }
    }

    std::vector<int> cellOrders;
    cellOrders.resize(m_cells.size());

    for (int i = 0; i < (int)cellOrders.size(); ++i)
        cellOrders[i] = i;
    std::random_shuffle(cellOrders.begin(), cellOrders.end());

    for (std::vector<int>::iterator it = cellOrders.begin(); it != cellOrders.end(); ++it) {
        const Cell& cell = m_cells[*it];
        if (cell.score > m_detectionThreshold) {
            features.push_back(FeatureCorner(cell.pos, cell.level));
            if ((int)features.size() > m_maxCountFeatures)
                return;
        }
    }
}

void FeatureDetector::fillFeaturesOnFirstImage(std::vector<FeatureCorner> & features)
{
    assert((m_gridSize.x > 0) && (m_gridSize.y > 0));

    Point2i imageSize = m_level0_first.size();

    assert((imageSize.x != 0) && (imageSize.y != 0));

    Point2f cellSize(imageSize.x / (float)(m_gridSize.x),
                     imageSize.y / (float)(m_gridSize.y));

    m_finalCandidates.resize(0);
    std::size_t prevSize = 0;

    int minLevel = (m_minLevelForFeature < 0) ? 0 : m_minLevelForFeature;
    int maxLevel = ((m_maxLevelForFeature < 0) || (m_maxLevelForFeature >= countLevels())) ?
                        (countLevels() - 1) :
                        m_maxLevelForFeature;

    for (int level = minLevel; level <= maxLevel; ++level) {
        ConstImage<uchar> image = firstImageAtLevel(level);

        m_candidates.resize(0);
        FastCorner::fast_corner_detect_10(image,
                                          m_opticalFlowCalculator.cursorSize(),
                                          image.size() - (m_opticalFlowCalculator.cursorSize() + Point2i(1, 1)),
                                          m_candidates,
                                          m_barrier);
        FastCorner::fastNonmaxSuppression(m_candidates, m_finalCandidates);
        if (level > 0) {
            int scale = (1 << level);
            for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin() + prevSize;
                 it != m_finalCandidates.end();
                 ++it) {
                it->pos *= scale;
                if ((it->pos.x < 0) || (it->pos.y < 0) || (it->pos.x >= imageSize.x) || (it->pos.y >= imageSize.y)) {
                    assert(false);
                }
                it->level = level;
            }
            break;
        } else {
            for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin() + prevSize;
                 it != m_finalCandidates.end();
                 ++it) {
                it->level = 0;
            }
        }
        prevSize = m_finalCandidates.size();
    }

    int k = 0;
    for (std::vector<Cell>::iterator it = m_cells.begin(); it != m_cells.end(); ++it) {
        it->score = 0.0f;
        it->level = 0;
        it->pos.set((int)(k % m_gridSize.x + 0.5f) * m_cellSize.x,
                    (int)(k / m_gridSize.x + 0.5f) * m_cellSize.y);
    }

    ConstImage<uchar> firstImage = this->firstImage();
    float score;
    for (std::vector<FastCorner::Corner>::iterator it = m_finalCandidates.begin();
            it != m_finalCandidates.end();
            ++it) {
        k = (int)(std::floor(it->pos.y / cellSize.y) * m_gridSize.y + std::floor(it->pos.x / cellSize.x));
        if (m_cells[k].lock)
            continue;
        score = FastCorner::shiTomasiScore_10(firstImage, it->pos);
        if (score > m_cells[k].score) {
            m_cells[k].score = score;
            m_cells[k].pos = it->pos;
            m_cells[k].level = it->level;
        }
    }

    std::vector<int> cellOrders;
    cellOrders.resize(m_cells.size());

    for (int i = 0; i < (int)cellOrders.size(); ++i)
        cellOrders[i] = i;
    std::random_shuffle(cellOrders.begin(), cellOrders.end());

    for (std::vector<int>::iterator it = cellOrders.begin(); it != cellOrders.end(); ++it) {
        const Cell& cell = m_cells[*it];
        //if (cell.score > m_detectionThreshold) {
            features.push_back(FeatureCorner(cell.pos, cell.level));
            if ((int)features.size() > m_maxCountFeatures)
                return;
        //}
    }
}


}

