#ifndef HOMOGRAPHYINITIALIZER_H
#define HOMOGRAPHYINITIALIZER_H

#include <utility>
#include <vector>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include "AR/Image.h"
#include "AR/TMath.h"
#include "AR/Pose3DEstimator.h"
#include "AR/Map.h"

namespace AR {

class HomographyInitializer
{
public:
    HomographyInitializer():_bestHomography(3, 3)
    {
        setMaxError(0.03f, 0.025f);
        _countRetries = 500;
        _scaleMap = 10.0f;
    }
    float maxErrorSquaredA() const { return _maxErrorSquaredA; }
    void setMaxError(float maxErrorA, float maxErrorB)
    {
        _maxErrorSquaredA = maxErrorA * maxErrorA;
        _maxErrorSquaredB = maxErrorB * maxErrorB;
    }
    void setCountRetries(int count) { _countRetries = count; }
    int countRetries() const { return _countRetries; }
    float scaleMap() const { return _scaleMap; }
    void setScaleMap(float scale) { _scaleMap = scale; }

    bool compute(Pose3DEstimator& pose3DEstimator, const std::vector<IPointF>& firstPoints, const std::vector<IPointF>& secondPoints);
    bool initializeMap(Pose3DEstimator& pose3DEstimator, Map& map,
                       const OF_System& opticalFlow, const IPoint& sizeImageOfPoint);

    QMatrix4x4 resultMotion() const { return _motionMatrix; }

private:
    typedef std::pair<IPointF, IPointF> Match;
    typedef struct HomographyDecomposition
    {
        TMath::TMatrix<float> rotation;
        TMath::TVector<float> translate;
        float score;
        float D;
        TMath::TVector<float> n;
    } HomographyDecomposition;

    float _maxErrorSquaredA;
    float _maxErrorSquaredB;
    int _countRetries;
    float _scaleMap;
    std::vector<Match> _matches;
    std::vector<int> _indicesInliners;
    std::vector<int> _indicesOutliners;
    std::vector<HomographyDecomposition> _decompositions;
    TMath::TMatrix<float> _bestHomography;
    TMath::TSVD<float> _svd;
    QMatrix4x4 _motionMatrix;

    inline float _getErrorSquared(const TMath::TMatrix<float>& homography, const Match& match) const;
    inline void _computeMatches(const Pose3DEstimator& pose3DEstimator,
                                const std::vector<IPointF>& first, const std::vector<IPointF>& second);
    inline TMath::TMatrix<float> _findHomography();
    inline bool _findBestHomography();
    inline float _sampsonusError(const TMath::TVector<float>& vSecond, const TMath::TMatrix<float>& mEssential,
                                 const TMath::TVector<float>& vFirst) const;
    void _computeTranslate(HomographyDecomposition& decomposition);
    bool _computeDecompositionsHomography();
    void _chooseBestDecomposition();
    void _getResultMotionMatrix();\
};

}

#endif // HOMOGRAPHYINITIALIZER_H
