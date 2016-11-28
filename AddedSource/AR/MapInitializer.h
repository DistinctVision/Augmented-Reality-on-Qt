#ifndef AR_MAPINITIALIZER_H
#define AR_MAPINITIALIZER_H

#include <utility>
#include <vector>
#include <memory>
#include "Image.h"
#include "Camera.h"
#include "FeatureDetector.h"
#include "TMath/TSVD.h"
#include "Map.h"
#include "MapResourcesManager.h"
#include "Configurations.h"

namespace AR {

class MapInitializer
{
public:
    enum class InitializationResult: int {
        Success,
        Failure,
        Reset
    };

    MapInitializer();

    InitConfiguration configuration() const;
    void setConfiguration(const InitConfiguration & configuration);

    double maxSquarePixelError() const;
    void setMaxPixelError(double maxPixelError);

    int countTimes() const;
    void setCountTimes(int count);

    double mapScale() const;
    void setMapScale(double scale);

    int minCountFeatures() const;
    void setMinCountFeatures(int count);

    float minDisparity() const;
    void setMinDisparity(float value);

    int minCountMapPoints() const;
    void setMinCountMapPoints(int minCountMapPoints);

    int maxCountFeatures() const;
    void setMaxCountFeatures(int maxCountFeatures);

    ConstImage<uchar> firstFrame() const;
    void setFirstFrame(const std::shared_ptr<const Camera> & camera, const ImageRef<uchar> & frame);
    ConstImage<uchar> secondFrame() const;
    void setSecondFrame(const std::shared_ptr<const Camera> & camera, const ImageRef<uchar> & frame);

    FeatureDetector & featureDetector();
    const FeatureDetector & featureDetector() const;

    void reset();

    TMath::TMatrixd resultRotation() const;
    TMath::TVectord resultTranslation() const;
    TMath::TMatrixd resultMotion() const;

    InitializationResult compute(Map * map, MapResourcesManager * manager, bool force = false);

    const std::vector<Point2f> & firstFeatures() const;
    const std::vector<Point2f> & secondFeatures() const;

    double epsDistanceForPlane() const;
    void setEpsDistanceForPlane(double epsDistanceForPlane);

private:
    struct Match
    {
        int imageLevel;
        Point2f pixelFirst;
        TMath::TVectord camFirst;
        Point2f pixelSecond;
        TMath::TVectord camSecond;

        TMath::TMatrixd secondPixelProjDerivs;

        TMath::TVectord position3d;
    };

    struct HomographyDecomposition
    {
        TMath::TMatrixd rotation;
        TMath::TVectord translation;
        double score;
        double D;
        TMath::TVectord n;
    };

    double m_maxSquarePixelError;
    int m_countTimes;
    double m_mapScale;
    int m_minCountFeatures;
    int m_minCountMapPoints;
    float m_minDisparitySquared;
    double m_epsDistanceForPlane;

    std::vector<Match> m_matches;
    std::vector<std::size_t> m_inlinerIndices;
    std::vector<std::size_t> m_outlinersIndices;
    std::vector<HomographyDecomposition> m_decompositions;
    TMath::TMatrixd m_bestHomography;
    TMath::TMatrixd m_bestEssential;
    TMath::TSVD<double> m_svd;

    FeatureDetector m_featureDetector;
    std::shared_ptr<const Camera> m_firstCamera;
    double m_firstCameraErrorMultipler;
    std::shared_ptr<const Camera> m_secondCamera;
    double m_secondCameraErrorMultipler;
    std::vector<int> m_featureImageLevel;
    std::vector<Point2f> m_firstFeatures;
    std::vector<Point2f> m_secondFeatures;

    void _computeMatches();
    double _getHomographyError(const TMath::TMatrixd & homography, const Match & match) const;
    double _getEssentialError(const TMath::TMatrixd & essential, const Match & match) const;
    TMath::TMatrixd _findHomography();
    bool _findBestHomography();
    void _refineHomography(TMath::TMatrixd & homography);
    bool _computeDecompositionsOfHomography();
    TMath::TMatrixd _fixEssentialMatrix(const TMath::TMatrixd & essential);
    int _computeEssential_from7points(TMath::TMatrixd results[3]);
    int _computeEssentials(TMath::TMatrixd results[3], const TMath::TMatrixd & F1, const TMath::TMatrixd & F2) const;
    bool _findBestEssential();
    int _getRealRoots(double * roots, double a, double b, double c, double d) const;
    void _essentialToDecompositions(const TMath::TMatrixd & essential);
    TMath::TMatrixd _sqew3d(const TMath::TVectord & v) const;
    void _initializeMap(Map * map, MapResourcesManager * manager);
    size_t _computePoints3d();
    bool _findPlane();
    TMath::TMatrixd _computePlaneMatrix();
    void _chooseBestDecomposition();
    bool _computeTransformation();
};

}

#endif // AR_MAPINITIALIZER_H
