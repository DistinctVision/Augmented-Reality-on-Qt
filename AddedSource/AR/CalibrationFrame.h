#ifndef AR_CALIBRATIONFRAME_H
#define AR_CALIBRATIONFRAME_H

#include <vector>
#include <memory>
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "Frame.h"
#include "CalibrationCorner.h"
#include "Camera.h"
#include "Configurations.h"

namespace AR {

class CalibrationFrame: public Frame
{
public:
    struct Parameters {
    };

    enum NeighborStates: int {
        NOT_TRIED = - 1,
        FAILED = -2
    };

    struct ErrorAndJacobians {
        Point2d error;
        TMath::TMatrixd poseJac;
        TMath::TMatrixd cameraJac;
    };

    struct GridCorner
    {
        CalibrationCorner::Properties properties;
        Point2i gridPos;
        int neighborStates[4];

        TMath::TMatrixd inheritedSteps;
        GridCorner();
    };

public:
    CalibrationFrame(const std::shared_ptr<const Camera>& camera,
                     const std::vector<Image<uchar>>& imagePyramid,
                     const CalibrationConfiguration& configuration);
    bool isCreated() const;

    void guessInitialPose();

    std::vector<ErrorAndJacobians> project(const TMath::TVector<bool>& fixedCameraParamters) const;

    Point2i imageSize() const;

    std::vector<std::pair<Point2f, Point2f>> debugLinesOfImageGrid() const;
    std::vector<std::pair<Point2f, Point2f>> debugLinesOf3DGrid() const;
    std::vector<std::pair<Point2f, Point2f>> debugLineErrorsOfCalibration(float gain) const;

    void drawGridCorners(const Image<Rgba>& image) const;

protected:
    bool m_isCreated;
    std::vector<Point2i> m_corners;
    std::vector<GridCorner> m_gridCorners;
    CalibrationCorner m_currentPath;
    CalibrationConfiguration m_configuration;

    std::vector<int> m_ringOffsets;
    mutable std::vector<uchar> m_ringPixels;

    int m_fast_pixel_ring[16];

    void _makeRingOffsets(int row_stride, float radius);
    void _make_fast_pixel_offset(int row_stride);

    Point2i _getDirection(int nDirn) const;
    Point2d _guessInitialAngles(const Point2d& center) const;
    inline bool _isCorner(const uchar* dataPoint, int nGate) const;
    double _expansionPotential(const GridCorner& gridCorner) const;
    TMath::TMatrixd _getGridSteps(const GridCorner& gridCorner) const;
    bool _expandByDir(int indexScrGridCorner, int dirn);
    int _nextToExpand();
    void _expandByStep(int n);
    bool _iterateOnImage(CalibrationCorner::Properties& properties);
};

} // namespace AR

#endif // AR_CALIBRATIONFRAME_H
