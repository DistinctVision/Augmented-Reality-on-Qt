#include "CalibrationFrame.h"
#include "ImageProcessing.h"
#include "CameraCalibrator.h"
#include <cmath>
#include <utility>
#include <climits>
#include "TMath/TMath.h"
#include "FastCorner.h"

namespace AR {

CalibrationFrame::CalibrationFrame(const std::shared_ptr<const Camera>& camera,
                                   const std::vector<Image<uchar>>& imagePyramid,
                                   const CalibrationConfiguration& configuration):
    Frame(camera, imagePyramid)
{
    m_isCreated = false;

    m_configuration = configuration;

    m_corners.clear();
    m_gridCorners.clear();

    int minLevel = (m_configuration.minImageLevelForCorner < 0) ? 0 : m_configuration.minImageLevelForCorner;
    int maxLevel = (m_configuration.maxImageLevelForCorner < 0) ? countImageLevels() - 1 : m_configuration.maxImageLevelForCorner;
    minLevel = std::min(std::max(minLevel, 0), countImageLevels() - 1);
    maxLevel = std::min(std::max(maxLevel, minLevel), countImageLevels() - 1);

    for (int level = minLevel; level <= maxLevel; ++level){
        ConstImage<uchar> image = m_imagePyramid[level];
        _make_fast_pixel_offset(image.width());
        //_makeRingOffsets(image.width(), m_configuration.cornerDetectRadius);
        float level_scale = (float)(1 << level);
        Point2i topLeft(3, 3);
        Point2i bottomRight = image.size() - (topLeft + Point2i(1, 1));
        Point2i p;
        const uchar* imageStr = image.pointer(0, topLeft.y);
        for (p.y = topLeft.y; p.y < bottomRight.y; ++p.y) {
            for (p.x = topLeft.x; p.x < bottomRight.x; ++p.x) {
                if (_isCorner(&imageStr[p.x], m_configuration.meanGate)) {
                    m_corners.push_back(Point2i((int)(p.x * level_scale), (int)(p.y * level_scale)));
                }
            }
            imageStr = &imageStr[image.width()];
        }
    }

    // If there's not enough corners, i.e. camera pointing somewhere random, abort.
    if ((int)(m_corners.size()) < m_configuration.minCornersForGrabbedFrame)
        return;

    // Pick a central corner point...
    Point2i centerOfImage = m_imagePyramid[0].size() / 2;
    Point2i bestCenterPos(0, 0);
    int bestDistanceSquared = std::numeric_limits<int>::max();
    for (std::size_t i = 0; i < m_corners.size(); ++i) {
        int dist = (m_corners[i] - centerOfImage).lengthSquared();
        if (dist < bestDistanceSquared) {
            bestDistanceSquared = dist;
            bestCenterPos = m_corners[i];
        }
    }

    if (!m_imagePyramid[0].pointInImageWithBorder(bestCenterPos, 11))
        return;

    // ... and try to fit a corner-patch to that.
    m_currentPath = CalibrationCorner(m_configuration.cornerPatchPixelSize / 2, m_configuration.cornerPatchPixelSize / 10.0f);
    CalibrationCorner::Properties cornerPathProperties;
    cornerPathProperties.pos = Point2d((double)bestCenterPos.x, (double)bestCenterPos.y);
    cornerPathProperties.angles = _guessInitialAngles(cornerPathProperties.pos);
    cornerPathProperties.gain = 80.0;
    cornerPathProperties.mean = 120.0;

    if (!_iterateOnImage(cornerPathProperties))
        return;

    // The first found corner patch becomes the origin of the detected grid.
    GridCorner firstCorner;
    firstCorner.properties = cornerPathProperties;
    m_gridCorners.push_back(firstCorner);

    // Next, go in two compass directions from the origin patch, and see if
    // neighbors can be found.
    if (!_expandByDir(0, 0))
        return;
    if (!_expandByDir(0, 1))
        return;

    m_gridCorners[1].inheritedSteps = m_gridCorners[2].inheritedSteps = _getGridSteps(m_gridCorners[0]);

    // The three initial grid elements are enough to find the rest of the grid.
    int next = 0;
    for (int iteration = 0; iteration < 300; ++iteration) {
        next = _nextToExpand();
        if (next < 0)
            break;
        _expandByStep(next);
    }
    if (next >= 0)
        return;

    if ((int)m_gridCorners.size() < m_configuration.minCornersForGrabbedFrame)
        return;

    m_isCreated = true;
    return;
}

void CalibrationFrame::_make_fast_pixel_offset(int row_stride)
{
    m_fast_pixel_ring[0] = 0 + row_stride * 3;
    m_fast_pixel_ring[1] = 1 + row_stride * 3;
    m_fast_pixel_ring[2] = 2 + row_stride * 2;
    m_fast_pixel_ring[3] = 3 + row_stride * 1;
    m_fast_pixel_ring[4] = 3 + row_stride * 0;
    m_fast_pixel_ring[5] = 3 + row_stride * -1;
    m_fast_pixel_ring[6] = 2 + row_stride * -2;
    m_fast_pixel_ring[7] = 1 + row_stride * -3;
    m_fast_pixel_ring[8] = 0 + row_stride * -3;
    m_fast_pixel_ring[9] = -1 + row_stride * -3;
    m_fast_pixel_ring[10] = -2 + row_stride * -2;
    m_fast_pixel_ring[11] = -3 + row_stride * -1;
    m_fast_pixel_ring[12] = -3 + row_stride * 0;
    m_fast_pixel_ring[13] = -3 + row_stride * 1;
    m_fast_pixel_ring[14] = -2 + row_stride * 2;
    m_fast_pixel_ring[15] = -1 + row_stride * 3;
}

void CalibrationFrame::_makeRingOffsets(int row_stride, float radius)
{
    std::vector<Point2i> offsets;
    for (float angle = 0.0f; angle < 2.0f * M_PI; angle += 0.001f) {
        Point2i c((int)std::floor(std::cos(angle) * radius), (int)std::floor(std::sin(angle) * radius));
        for (std::size_t i = 0; i < offsets.size(); ++i) {
            if (offsets[i] == c)
                goto cont;
        }
        offsets.push_back(c);
        cont:;
    }
    m_ringOffsets.clear();
    for (std::vector<Point2i>::iterator it = offsets.begin(); it != offsets.end(); ++it) {
        m_ringOffsets.push_back(it->x + row_stride * it->y);
    }
    m_ringPixels.resize(m_ringOffsets.size());
}

Point2i CalibrationFrame::imageSize() const
{
    return m_imagePyramid.empty() ? Point2i(0, 0) : m_imagePyramid[0].size();
}

CalibrationFrame::GridCorner::GridCorner():
    inheritedSteps(2, 2)
{
    gridPos.setZero();
    for (int i = 0; i < 4; ++i)
        neighborStates[i] = NOT_TRIED;
}

/*bool CalibrationFrame::_isCorner(const uchar* dataPoint, int nGate) const
{
    // Does a quick check to see if a point in an image could be a grid corner.
    // Does this by going around a 16-pixel ring, and checking that there's four
    // transitions (black - white- black - white - )
    // Also checks that the central pixel is blurred.

    // Find the mean intensity of the pixel ring...

    int nSum = 0;
    for (std::size_t i = 0; i < m_ringPixels.size(); ++i)  {
        m_ringPixels[i] = dataPoint[m_ringOffsets[i]];
        nSum += m_ringPixels[i];
    }
    int nMean = nSum / m_ringOffsets.size();
    int nHiThresh = nMean + nGate;
    int nLoThresh = nMean - nGate;

    // If the center pixel is roughly the same as the mean, this isn't a corner.
    int nCenter = *dataPoint;
    if ((nCenter <= nLoThresh) || (nCenter >= nHiThresh))
        return false;

    // Count transitions around the ring... there should be four!
    bool state = (m_ringPixels[m_ringPixels.size()-1] > nMean);
    int nSwaps = 0;
    for (std::size_t i = 0; i < m_ringPixels.size(); ++i) {
        uchar valNow = m_ringPixels[i];
        if (state) {
            if (valNow < nLoThresh) {
                state = false;
                ++nSwaps;
            }
        } else if (valNow > nHiThresh) {
              state = true;
              ++nSwaps;
        }
    }
    return (nSwaps == 4);
}*/

bool CalibrationFrame::_isCorner(const uchar* dataPoint, int nGate) const
{
    // Does a quick check to see if a point in an image could be a grid corner.
    // Does this by going around a 16-pixel ring, and checking that there's four
    // transitions (black - white- black - white - )
    // Also checks that the central pixel is blurred.

    // Find the mean intensity of the pixel ring...

    static uchar pixels[16];

    int nSum = 0;
    for (std::size_t i = 0; i < 16; ++i)  {
        pixels[i] = dataPoint[m_fast_pixel_ring[i]];
        nSum += pixels[i];
    }
    int nMean = nSum / 16;
    int nHiThresh = nMean + nGate;
    int nLoThresh = nMean - nGate;

    // If the center pixel is roughly the same as the mean, this isn't a corner.
    int nCenter = *dataPoint;
    if ((nCenter <= nLoThresh) || (nCenter >= nHiThresh))
        return false;

    // Count transitions around the ring... there should be four!
    bool state = (pixels[15] > nMean);
    int nSwaps = 0;
    for (std::size_t i = 0; i < 16; ++i) {
        uchar valNow = pixels[i];
        if (state) {
            if (valNow < nLoThresh) {
                state = false;
                ++nSwaps;
            }
        } else if (valNow > nHiThresh) {
              state = true;
              ++nSwaps;
        }
    }
    return (nSwaps == 4);
}

Point2d CalibrationFrame::_guessInitialAngles(const Point2d& center) const
{
    // The iterative patch-finder works better if the initial guess
    // is roughly aligned! Find one of the line-axes by searching round
    // the circle for the strongest gradient, and use that and +90deg as the
    // initial guesses for patch angle.
    //
    // Yes, this is a very poor estimate, but it's generally (hopefully?)
    // enough for the iterative finder to converge.
    using namespace TMath;

    double bestAngle = 0;
    double bestGradMag = 0;
    double gradAtBest = 0;
    for (double angle = 0.0; angle < M_PI; angle += 0.05) {
        Point2d dirn(std::cos(angle), std::sin(angle));
        Point2d perp(-dirn.y, dirn.x);

        double G = ImageProcessing::interpolate<double>(m_imagePyramid[0], center + dirn * 10.0 + perp * 4.0) -
                   ImageProcessing::interpolate<double>(m_imagePyramid[0], center + dirn * 10.0 - perp * 4.0) +
                   ImageProcessing::interpolate<double>(m_imagePyramid[0], center - dirn * 10.0 - perp * 4.0) -
                   ImageProcessing::interpolate<double>(m_imagePyramid[0], center - dirn * 10.0 + perp * 4.0);
        double fG = std::fabs(G);
        if (fG > bestGradMag) {
            bestGradMag = fG;
            gradAtBest = G;
            bestAngle = angle;
        }
    }

    Point2d result = (gradAtBest > 0.0) ? Point2d(bestAngle, bestAngle + M_PI * 0.5) :
                                          Point2d(bestAngle - M_PI * 0.5, bestAngle);
    return result;
}

bool CalibrationFrame::isCreated() const
{
    return m_isCreated;
}

bool CalibrationFrame::_iterateOnImage(CalibrationCorner::Properties& properties)
{
    CalibrationCorner::Properties p = properties;

    double lastDeltaStep = 0.0;
    int i, j;
    for (i = countImageLevels() - 1; i >= 0; --i) {
        for (j = 0; j < m_configuration.maxCountIterations; ++j) {
            double scale = (1 << i);
            lastDeltaStep = m_currentPath.iterate(p, m_imagePyramid[i], 1.0 / scale);
            if (lastDeltaStep < 0.0)
                return false;
            if (lastDeltaStep < m_configuration.cornerPathEps * scale)
                break;
        }
    }

    /*TMath::TMatrixd w = p.warp();
    Point2d halfTemplateSize((m_templateImage.width() - 1) * 0.5, (m_templateImage.height() - 1) * 0.5);
    Point2i t;
    for (t.y = 0; t.y < m_templateImage.height(); ++t.y) {
        for (t.x = 0; t.x < m_templateImage.width(); ++t.x) {
            Point2d tp(t.x - halfTemplateSize.x, t.y - halfTemplateSize.y);
            uchar r = (uchar)(std::min(std::max(m_templateImage(t) * p.gain + p.mean, 0.0), 255.0));
            Point2i nt((int)(tp.x * w(0, 0) + tp.y * w(0, 1) + p.pos.x),
                       (int)(tp.x * w(1, 0) + tp.y * w(1, 1) + p.pos.y));
            if (m_image.pointInImageWithBorder(nt, 1)) {
                m_image(nt) = r;
            }
        }
    }

    return false;*/
    if (lastDeltaStep > m_configuration.cornerPathEps)
        return false;
    if (std::fabs(std::sin(p.angles.x - p.angles.y)) < std::sin(M_PI / 6.0))
        return false;
    if (std::fabs(p.gain) < m_configuration.minGain)
        return false;
    if (m_currentPath.lastError() > m_configuration.maxError)
        return false;

    properties = p;
    return true;
}

bool CalibrationFrame::_expandByDir(int indexScrGridCorner, int dirn)
{
    float minDisSquared = m_configuration.minDistanceForCorner;
    minDisSquared *= minDisSquared;
    dirn = dirn % 2;
    const double cutOff = std::cos(M_PI / 18.0);
    GridCorner& gridCorner = m_gridCorners[indexScrGridCorner];
    Point2i bestCornerA(0, 0), bestCornerB(0, 0);
    double distance;
    Point2d dir;
    double bestDistanceSquaredA = std::numeric_limits<double>::max();
    double bestDistanceSquaredB = std::numeric_limits<double>::max();
    double vDot;
    TMath::TMatrixd warp = gridCorner.properties.warp();
    Point2d targetDir = Point2d(warp(0, dirn), warp(1, dirn));
    for (std::size_t i=0; i < m_corners.size(); ++i) {
        Point2d diff = m_corners[i].cast<double>() - gridCorner.properties.pos;
        double distanceSquared = diff.lengthSquared();
        if (distanceSquared < minDisSquared)
            continue;
        if (distanceSquared < bestDistanceSquaredA) {
            distance = std::sqrt(distanceSquared);
            dir = diff / distance;
            vDot = Point2d::dot(dir, targetDir);
            if (vDot > cutOff) {
                bestDistanceSquaredA = distanceSquared;
                bestCornerA = m_corners[i];
            }
            if (distanceSquared < bestDistanceSquaredB) {
                if ((-vDot) > cutOff) {
                    bestDistanceSquaredB = distanceSquared;
                    bestCornerB = m_corners[i];
                }
            }
        } else if (distanceSquared < bestDistanceSquaredB) {
            distance = std::sqrt(distanceSquared);
            dir = diff / distance;
            vDot = Point2d::dot(dir, targetDir);
            if ((-vDot) > cutOff) {
                bestDistanceSquaredB = distanceSquared;
                bestCornerB = m_corners[i];
            }
        }
    }

    GridCorner targetGridCornerA, targetGridCornerB;
    targetGridCornerA.properties = targetGridCornerB.properties = gridCorner.properties;
    targetGridCornerA.properties.pos = bestCornerA.cast<double>();
    targetGridCornerB.properties.pos = bestCornerB.cast<double>();
    targetGridCornerA.properties.gain *= -1.0;
    targetGridCornerB.properties.gain *= -1.0;

    GridCorner finishTargetGridCorner;
    int offset = 0;
    if (bestDistanceSquaredA < bestDistanceSquaredB) {
        if (_iterateOnImage(targetGridCornerA.properties)) {
            finishTargetGridCorner = targetGridCornerA;
            finishTargetGridCorner.gridPos = gridCorner.gridPos;
            if (dirn == 0)
                finishTargetGridCorner.gridPos.x++;
            else
                finishTargetGridCorner.gridPos.y++;
            offset = 0;
        } else {
            if (_iterateOnImage(targetGridCornerB.properties)) {
                finishTargetGridCorner = targetGridCornerB;
                finishTargetGridCorner.gridPos = gridCorner.gridPos;
                if (dirn == 0)
                    finishTargetGridCorner.gridPos.x--;
                else
                    finishTargetGridCorner.gridPos.y--;
                offset = 2;
            } else {
                return false;
            }
        }
    } else {
        if (_iterateOnImage(targetGridCornerB.properties)) {
            finishTargetGridCorner = targetGridCornerB;
            finishTargetGridCorner.gridPos = gridCorner.gridPos;
            if (dirn == 0)
                finishTargetGridCorner.gridPos.x--;
            else
                finishTargetGridCorner.gridPos.y--;
            offset = 2;
        } else {
            if (_iterateOnImage(targetGridCornerA.properties)) {
                finishTargetGridCorner = targetGridCornerA;
                finishTargetGridCorner.gridPos = gridCorner.gridPos;
                if (dirn == 0)
                    finishTargetGridCorner.gridPos.x++;
                else
                    finishTargetGridCorner.gridPos.y++;
                offset = 0;
            } else {
                return false;
            }
        }
    }
    finishTargetGridCorner.neighborStates[dirn + (2 - offset)] = indexScrGridCorner;

    m_gridCorners.push_back(finishTargetGridCorner);
    m_gridCorners[indexScrGridCorner].neighborStates[dirn + offset] = m_gridCorners.size() - 1;
    return true;
}

double CalibrationFrame::_expansionPotential(const CalibrationFrame::GridCorner& gridCorner) const
{
    // Scoring function. How good would this grid corner be at finding a neighbor?
    // The best case is if it's already surrounded by three neighbors and only needs
    // to find the last one (because it'll have the most accurate guess for where
    // the last one should be) and so on.
    int missing = 0;
    for (int i=0; i<4; ++i)
        if (gridCorner.neighborStates[i] == NOT_TRIED)
            missing++;

    switch (missing) {
    case 0:
        return 0.0;
    case 1:
        return 100.0;
    case 3:
        return 1.0;
    case 2: {
        int first = 0;
        while (gridCorner.neighborStates[first] != NOT_TRIED)
            first++;
        if (gridCorner.neighborStates[(first + 2) % 4] == NOT_TRIED)
            return 10.0;
        return 20.0;
    }
    default:
        break;
    }

    assert(false); // should never get here
    return 0.0;
}


TMath::TMatrixd CalibrationFrame::_getGridSteps(const CalibrationFrame::GridCorner& gridCorner) const
{
    TMath::TMatrixd steps(2, 2);
    for (int dirn = 0; dirn < 2; ++dirn) {
        Point2d dir;
        dir.setZero();
        int found = 0;
        if (gridCorner.neighborStates[dirn] >= 0) {
            dir += m_gridCorners[gridCorner.neighborStates[dirn]].properties.pos - gridCorner.properties.pos;
            ++found;
        }
        if (gridCorner.neighborStates[dirn + 2] >= 0) {
            dir -= m_gridCorners[gridCorner.neighborStates[dirn + 2]].properties.pos - gridCorner.properties.pos;
            ++found;
        }
        if (found == 0) {
            steps(dirn, 0) = gridCorner.inheritedSteps(dirn, 0);
            steps(dirn, 1) = gridCorner.inheritedSteps(dirn, 1);
        } else {
            steps(dirn, 0) = dir.x / (double)found;
            steps(dirn, 1) = dir.y / (double)found;
        }
    }
    return steps;
}

int CalibrationFrame::_nextToExpand()
{
    int indexBest = -1;
    double best = 0.0;

    for (std::size_t i = 0; i < m_gridCorners.size(); ++i) {
        double d = _expansionPotential(m_gridCorners[i]);
        if (d > best) {
            indexBest = i;
            best = d;
        }
    }
    return indexBest;
}

void CalibrationFrame::_expandByStep(int n)
{
    using namespace TMath;

    GridCorner& gridCorner = m_gridCorners[n];

    // First, choose which direction to expand in...
    // Ideally, choose a dirn for which the Step calc is good!
    int dirn = -10;
    for (int i = 0; (dirn == -10) && (i < 4); ++i) {
        if ((gridCorner.neighborStates[i] == NOT_TRIED) &&
            (gridCorner.neighborStates[(i + 2) % 4] >= 0)) {
          dirn = i;
        }
    }
    if (dirn == -10) {
        for (int i = 0; (dirn == -10) && (i < 4); ++i) {
            if (gridCorner.neighborStates[i] == NOT_TRIED)
                dirn = i;
        }
    }
    assert(dirn != -10);

    Point2i gridStep = _getDirection(dirn);

    TVectord step = _getGridSteps(gridCorner).refTransposed() * TVectord::create((double)gridStep.x, (double)gridStep.y);

    Point2d searchPos = gridCorner.properties.pos;
    searchPos.x += step(0);
    searchPos.y += step(1);

    // Before the search: pre-fill the failure result for easy returns.
    gridCorner.neighborStates[dirn] = FAILED;

    Point2i bestCorner(0, 0);
    double bestDistance = std::numeric_limits<double>::max();
    for (std::size_t i = 0; i < m_corners.size(); ++i) {
        Point2d diff = Point2d((double)m_corners[i].x, (double)m_corners[i].y) - searchPos;
        double lengthSquared = diff.lengthSquared();
        if (lengthSquared > bestDistance * bestDistance)
            continue;
        bestDistance = std::sqrt(lengthSquared);
        bestCorner = m_corners[i];
    }

    double stepDistance = step.length();
    if (bestDistance > m_configuration.expandByStepMaxDistFrac * stepDistance)
        return;

    GridCorner targetGridCorner;
    targetGridCorner.properties = gridCorner.properties;
    targetGridCorner.properties.pos.set((double)bestCorner.x, (double)bestCorner.y);
    targetGridCorner.properties.gain *= -1.0;
    targetGridCorner.gridPos = gridCorner.gridPos + gridStep;
    targetGridCorner.inheritedSteps = _getGridSteps(gridCorner);
    if (!_iterateOnImage(targetGridCorner.properties))
        return;

    // Update connection states:
    std::size_t targetIndex = m_gridCorners.size();
    for (int dirn = 0; dirn < 4; ++dirn) {
        Point2i search = targetGridCorner.gridPos + _getDirection(dirn);
        for (std::size_t i = 0; i < m_gridCorners.size(); ++i) {
            if (m_gridCorners[i].gridPos == search) {
                targetGridCorner.neighborStates[dirn] = i;
                m_gridCorners[i].neighborStates[(dirn + 2) % 4] = targetIndex;
            }
        }
    }
    m_gridCorners.push_back(targetGridCorner);
}

Point2i CalibrationFrame::_getDirection(int nDirn) const
{
    if (nDirn % 2 == 0)
        return (nDirn == 0) ? Point2i(1, 0) : Point2i(-1, 0);
    return (nDirn == 1) ? Point2i(0, 1) : Point2i(0, -1);
}

void CalibrationFrame::guessInitialPose()
{
    // First, find a homography which maps the grid to the unprojected image coords
    // Use the standard null-space-of-SVD-thing to find 9 homography parms
    // (c.f. appendix of thesis)
    using namespace TMath;

    int countPoints = (int)m_gridCorners.size();
    if (countPoints < 9)
        return;
    TMatrixd M(2 * countPoints, 9);
    double* dataRow = M.firstDataRow();
    for (int n = 0; n < countPoints; ++n) {
        // First, un-project the points to the image plane
        Point2d unproj = m_camera->unproject(m_gridCorners[n].properties.pos);
        // Then fill in the matrix..
        double x = (double)m_gridCorners[n].gridPos.x;
        double y = (double)m_gridCorners[n].gridPos.y;

        dataRow[0] = x;
        dataRow[1] = y;
        dataRow[2] = 1.0;
        dataRow[3] = 0.0;
        dataRow[4] = 0.0;
        dataRow[5] = 0.0;
        dataRow[6] = - x * unproj.x;
        dataRow[7] = - y * unproj.x;
        dataRow[8] = - unproj.x;
        dataRow = M.nextRow(dataRow);

        dataRow[0] = 0.0;
        dataRow[1] = 0.0;
        dataRow[2] = 0.0;
        dataRow[3] = x;
        dataRow[4] = y;
        dataRow[5] = 1.0;
        dataRow[6] = - x * unproj.y;
        dataRow[7] = - y * unproj.y;
        dataRow[8] = - unproj.y;
        dataRow = M.nextRow(dataRow);
    }

    // The right null-space (should only be one) of the matrix gives the homography...
    TSVD<double> svd;
    svd.compute(M, true);
    TMatrixd homography(3, 3, svd.V_transposed().getDataRow(8));

    // Fix up possibly poorly conditioned bits of the homography
    /*{
        svd.compute(homography.slice(2, 2));
        TVectord diagonal = svd.diagonalW();
        homography *= 1.0 / diagonal(0);
        diagonal /= diagonal(0);
        double lambda2 = diagonal(1);

        TVectord b(2);   // This is one hypothesis for b ; the other is the negative.
        b(0) = 0.0;
        b(1) = std::sqrt(1.0 - (lambda2 * lambda2));

        TVectord aprime = b * svd.V_transposed();

        TVectord a = homography.getRow(2, 2);

        if (dot(a, aprime) > 0.0)
            homography.setRow(2, aprime);
        else
            homography.setRow(2, - aprime);
    }*/

    // OK, now turn homography into something 3D ...simple gram-schmidt ortho-norm
    // Take 3x3 matrix H with column: abt
    // And add a new 3rd column: abct
    homography *= 1.0 / homography.getColumn(0).length();

    m_rotation.setColumn(0, homography.getColumn(0));
    m_rotation.setColumn(1, (homography.getColumn(1) - homography.getColumn(0) *
                        dot(homography.getColumn(0), homography.getColumn(1))).normalized());
    m_rotation.setColumn(2, cross3(m_rotation.getColumn(0, 3), m_rotation.getColumn(1, 3)));
    homography.getColumn(m_translation, 2, 3);
}

std::vector<CalibrationFrame::ErrorAndJacobians> CalibrationFrame::project(const TMath::TVector<bool>& fixedCameraParamters) const
{
    using namespace TMath;
    std::vector<ErrorAndJacobians> result;
    for (std::size_t i = 0; i < m_gridCorners.size(); ++i) {
        ErrorAndJacobians EAJ;
        EAJ.poseJac = TMatrixd(2, 6);

        const GridCorner& gridCorner = m_gridCorners[i];

        // First, project into image...
        TVectord worldPoint(3);
        worldPoint(0) = (double)gridCorner.gridPos.x;
        worldPoint(1) = (double)gridCorner.gridPos.y;
        worldPoint(2) = 0.0;

        TVectord camWorld = m_rotation * worldPoint + m_translation;
        if (camWorld(2) <= 0.001)
            continue;

        double oneOverCameraZ = 1.0 / camWorld(2);
        Camera::ProjectionInfo projectionInfo;
        Point2d imagePoint = m_camera->project(Point2d(camWorld(0) * oneOverCameraZ, camWorld(1) * oneOverCameraZ), projectionInfo);
        if (projectionInfo.invalid)
            continue;

        EAJ.error = gridCorner.properties.pos - imagePoint;

        // Now find motion jacobian...
        TMatrixd camDerivatives = m_camera->getProjectionDerivatives(projectionInfo);

        TVectord camWorld_4 = TVectord::create(camWorld(0), camWorld(1), camWorld(2), 1.0);

        for (int dof = 0; dof < 6; ++dof) {
            const TVectord motion = TTools::generator_field(dof, camWorld_4);
            TVectord camFrameMotion(2);
            camFrameMotion(0) = (motion(0) - camWorld(0) * motion(2) * oneOverCameraZ) * oneOverCameraZ;
            camFrameMotion(1) = (motion(1) - camWorld(1) * motion(2) * oneOverCameraZ) * oneOverCameraZ;
            EAJ.poseJac.setColumn(dof, camDerivatives * camFrameMotion);
        }

        // Finally, the camera provids its own jacobian
        EAJ.cameraJac = m_camera->getCameraParametersDerivatives(projectionInfo.camPoint, fixedCameraParamters);
        result.push_back(EAJ);
    }
    return result;
}


std::vector<std::pair<Point2f, Point2f>> CalibrationFrame::debugLinesOfImageGrid() const
{
    std::vector<std::pair<Point2f, Point2f>> result;
    for (std::size_t i = 0; i < m_gridCorners.size(); ++i) {
        const GridCorner& gridCorner = m_gridCorners[i];
        for (int dirn = 0; dirn < 4; ++dirn) {
            if (gridCorner.neighborStates[dirn] > (int)(i)) {
                result.push_back(std::make_pair(gridCorner.properties.pos.cast<float>(),
                      m_gridCorners[gridCorner.neighborStates[dirn]].properties.pos.cast<float>()));
            }
        }
    }
    /*for (std::size_t i=0; i < m_gridCorners.size(); ++i) {
        const GridCorner& gridCorner = m_gridCorners[i];
        result.push_back(std::make_pair(gridCorner.properties.pos.cast<float>() + Point2f(6.0f, 6.0f),
                                                     gridCorner.properties.pos.cast<float>() + Point2f(- 6.0f, - 6.0f)));
        result.push_back(std::pair<Point2d, Point2d>(gridCorner.properties.pos.cast<float>() + Point2f(6.0f, - 6.0f),
                                                     gridCorner.properties.pos.cast<float>() + Point2f(- 6.0f, 6.0f)));
    }*/
    /*for (std::size_t i=0; i < m_corners.size(); ++i) {
        result.push_back(std::pair<Point2f, Point2f>(Point2f(m_corners[i].x + 1.0f, m_corners[i].y + 1.0f),
                                                     Point2f(m_corners[i].x - 1.0f, m_corners[i].y - 1.0f)));
        result.push_back(std::pair<Point2f, Point2f>(Point2f(m_corners[i].x + 1.0f, m_corners[i].y - 1.0f),
                                                     Point2f(m_corners[i].x - 1.0f, m_corners[i].y + 1.0f)));
    }*/
    return result;
}

std::vector<std::pair<Point2f, Point2f>> CalibrationFrame::debugLinesOf3DGrid() const
{
    using namespace TMath;
    std::vector<std::pair<Point2f, Point2f>> result;
    return result;
    for (std::size_t i = 0; i < m_gridCorners.size(); ++i) {
        const GridCorner& gridCornerA = m_gridCorners[i];
        TVectord vA = m_rotation *
                TVectord::create((double)gridCornerA.gridPos.x, (double)gridCornerA.gridPos.y, 0.0) +
                    m_translation;
        Point2f pA = m_camera->project(Point2d(vA(0) / vA(2), vA(1) / vA(2))).cast<float>();
        for (int dirn = 0; dirn < 4; ++dirn) {
            if (gridCornerA.neighborStates[dirn] > static_cast<int>(i)) {
                const GridCorner& gridCornerB = m_gridCorners[gridCornerA.neighborStates[dirn]];
                TVectord vB = m_rotation *
                        TVectord::create((double)gridCornerB.gridPos.x, (double)gridCornerB.gridPos.y, 0.0) +
                            m_translation;
                Point2f pB = m_camera->project(Point2d(vB(0) / vB(2), vB(1) / vB(2))).cast<float>();
                result.push_back(std::make_pair(pA, pB));
            }
        }
    }
    return result;
}

std::vector<std::pair<Point2f, Point2f>> CalibrationFrame::debugLineErrorsOfCalibration(float gain) const
{
    using namespace TMath;
    std::vector<std::pair<Point2f, Point2f>> result;
    return result;
    for(std::size_t i = 0; i < m_gridCorners.size(); ++i) {
        const GridCorner& gridCorner = m_gridCorners[i];
        TVectord v = m_rotation *
                TVectord::create((double)gridCorner.gridPos.x, (double)gridCorner.gridPos.y, 0.0) +
                    m_translation;
        Point2f projected = m_camera->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>();
        result.push_back(std::make_pair(projected,
                      projected + (gridCorner.properties.pos.cast<float>() - projected) * gain));
    }
    return result;
}

void CalibrationFrame::drawGridCorners(const Image<Rgba>& image) const
{
    for (auto it = m_gridCorners.cbegin(); it != m_gridCorners.end(); ++it) {
        m_currentPath.draw(it->properties, image);
    }
}

} // namespace AR
