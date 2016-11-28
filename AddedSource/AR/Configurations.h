#ifndef AR_CONFIGURATION_H
#define AR_CONFIGURATION_H

#include "Point2.h"

namespace AR {

struct InitConfiguration
{
    double maxPixelError;
    double epsDistanceForPlane;
    int countTimes;
    double mapScale;
    int minCountFeatures;
    int maxCountFeatures;
    float minDisparity;
    int minCountMapPoints;
    Point2i frameGridSize;
    int featureCornerBarier;
    float featureDetectionThreshold;
    int countImageLevels;
    int minImageLevelForFeature;
    int maxImageLevelForFeature;
    Point2i featureCursorSize;
    float pixelEps;
    int maxNumberIterationsForOpticalFlow;

    InitConfiguration()
    {
        maxPixelError = 3.0;
        epsDistanceForPlane = 0.05;
        countTimes = 1000;
        mapScale = 10.0;
        minCountFeatures = 40;
        maxCountFeatures = 100;
        minCountMapPoints = 30;
        minDisparity = 7.0f;
        frameGridSize = Point2i(20, 20);
        featureCornerBarier = 3;
        featureDetectionThreshold = 5.0f;
        countImageLevels = 3;
        minImageLevelForFeature = 0;
        maxImageLevelForFeature = 2;
        featureCursorSize = Point2i(10, 10);
        pixelEps = 1e-3f;
        maxNumberIterationsForOpticalFlow = 20;
    }
};

struct TrackingConfiguration
{
    int countImageLevels;
    int mapPoint_goodSuccessLimit;
    int mapPoint_failedLimit;
    int candidate_failedLimit;
    int maxNumberOfUsedKeyFrames;
    int frameBorder;
    int maxNumberOfFeaturesOnFrame;
    Point2i frameGridSize;
    Point2i featureCursorSize;
    float pixelEps;
    double locationEps;
    double locationMaxPixelError;
    int locationNumberIterations;
    int numberPointsForSructureOptimization;
    int numberIterationsForStructureOptimization;
    double toleranceOfCreatingFrames;
    int minNumberTrackingPoints;
    int preferredNumberTrackingPoints;
    int sizeOfSmallImage;
    int maxCountKeyFrames;
    int featureMaxNumberIterations;
    double tracker_eps;
    int tracker_numberIterations;
    int tracker_minImageLevel;
    int tracker_maxImageLevel;
    Point2i tracker_cursorSize;

    TrackingConfiguration()
    {
        countImageLevels = 3;
        mapPoint_goodSuccessLimit = 10;
        mapPoint_failedLimit = 15;
        candidate_failedLimit = 30;
        maxNumberOfUsedKeyFrames = 4;
        frameBorder = 2;
        maxNumberOfFeaturesOnFrame = 40;
        frameGridSize = Point2i(7, 7);
        featureCursorSize = Point2i(2, 2);
        featureMaxNumberIterations = 4;
        pixelEps = 1e-2f;
        locationEps = 3e-5;
        locationMaxPixelError = 4.0;
        locationNumberIterations = 30;
        numberPointsForSructureOptimization = 10;
        numberIterationsForStructureOptimization = 10;
        toleranceOfCreatingFrames = 0.2;
        preferredNumberTrackingPoints = 30;
        minNumberTrackingPoints = 20;
        sizeOfSmallImage = 32;
        maxCountKeyFrames = 10;
        tracker_eps = 1e-3;
        tracker_numberIterations = 15;
        tracker_minImageLevel = 1;
        tracker_maxImageLevel = -1;
        tracker_cursorSize = Point2i(2, 2);
    }
};

struct MapPointsDetectorConfiguration
{
    int maxNumberOfUsedFrames;
    int maxNumberOfSearchSteps;
    float seedConvergenceSquaredSigmaThresh;
    int sizeOfCommitMapPoints;
    Point2i frameGridSize;
    int featureCornerBarier;
    float featureDetectionThreshold;
    int minImageLevelForFeature;
    int maxImageLevelForFeature;
    Point2i featureCursorSize;
    float pixelEps;
    int maxNumberIterationsForOpticalFlow;
    int maxCountCandidatePoints;

    MapPointsDetectorConfiguration()
    {
        maxNumberOfUsedFrames = 4;
        maxNumberOfSearchSteps = 100;
        seedConvergenceSquaredSigmaThresh = 12.0f;
        sizeOfCommitMapPoints = 5;
        frameGridSize = Point2i(7, 7);
        featureCornerBarier = 3;
        featureDetectionThreshold = 5.0f;
        minImageLevelForFeature = -1;
        maxImageLevelForFeature = -1;
        featureCursorSize = Point2i(10, 10);
        pixelEps = 1e-2f;
        maxNumberIterationsForOpticalFlow = 20;
        maxCountCandidatePoints = 40;
    }
};

struct CalibrationConfiguration
{
    int countImageLevels;
    int minImageLevelForCorner;
    int maxImageLevelForCorner;
    double minDistanceForCorner;
    int cornerPatchPixelSize;
    double cornerPathEps;
    //float cornerDetectRadius;
    int maxCountIterations;
    double maxError;
    int minCornersForGrabbedFrame;
    double expandByStepMaxDistFrac;
    int meanGate;
    double minGain;
    int counOfUsedCalibrationFrames;

    CalibrationConfiguration()
    {
        countImageLevels = 3;
        minImageLevelForCorner = 1;
        minImageLevelForCorner = 2;
        minDistanceForCorner = 20.0;
        cornerPatchPixelSize = 20;
        cornerPathEps = 1e-3;
        //cornerDetectRadius = 8.0f;
        maxCountIterations = 80;
        maxError = 100.0;
        minCornersForGrabbedFrame = 20;
        expandByStepMaxDistFrac = 0.15;
        meanGate = 40;
        minGain = 5.0;
        counOfUsedCalibrationFrames = 5;
    }
};

}

#endif // AR_CONFIGURATION_H
