#include "MapInitializer.h"
#include <cmath>
#include <ctime>
#include <climits>
#include <cassert>
#include <memory>
#include <algorithm>
#include <chrono>
#include "TMath/TMath.h"
#include "TukeyRobustCost.h"
#include "MapWriter.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "ImageProcessing.h"

namespace AR {

MapInitializer::MapInitializer():
        m_bestHomography(3, 3),
        m_bestEssential(3, 3)
{
    setMaxPixelError(2.0);
    m_countTimes = 1000;
    m_mapScale = 10.0;
    m_minCountFeatures = 40;
    m_minCountMapPoints = 30;
    setMinDisparity(10.0f);
    m_featureDetector.setGridSize(Point2i(20, 20));
    m_epsDistanceForPlane = 0.1;
}

InitConfiguration MapInitializer::configuration() const
{
    InitConfiguration configuration;

    configuration.maxPixelError = std::sqrt(m_maxSquarePixelError);
    configuration.epsDistanceForPlane = m_epsDistanceForPlane;
    configuration.countTimes = m_countTimes;
    configuration.mapScale = m_mapScale;
    configuration.minCountFeatures = m_minCountFeatures;
    configuration.minCountMapPoints = m_minCountMapPoints;
    configuration.maxCountFeatures = m_featureDetector.maxCountFeatures();
    configuration.minDisparity = std::sqrt(m_minDisparitySquared);
    configuration.frameGridSize = m_featureDetector.gridSize();
    configuration.featureCornerBarier = m_featureDetector.barrier();
    configuration.featureDetectionThreshold = m_featureDetector.detectionThreshold();
    configuration.countImageLevels = m_featureDetector.countLevels();
    configuration.minImageLevelForFeature = m_featureDetector.minLevelForFeature();
    configuration.maxImageLevelForFeature = m_featureDetector.maxNumberIterations();
    configuration.featureCursorSize = m_featureDetector.cursorSize();
    configuration.pixelEps = m_featureDetector.pixelEps();
    configuration.maxNumberIterationsForOpticalFlow = m_featureDetector.maxNumberIterations();

    return configuration;
}

void MapInitializer::setConfiguration(const InitConfiguration & configuration)
{
    setMaxPixelError(configuration.maxPixelError);
    m_epsDistanceForPlane = configuration.epsDistanceForPlane;
    m_countTimes = configuration.countTimes;
    m_mapScale = configuration.mapScale;
    m_minCountFeatures = configuration.minCountFeatures;
    m_minCountMapPoints = configuration.minCountMapPoints;
    m_featureDetector.setMaxCountFeatures(configuration.maxCountFeatures);
    setMinDisparity(configuration.minDisparity);
    m_featureDetector.setGridSize(configuration.frameGridSize);
    m_featureDetector.setBarrier(configuration.featureCornerBarier);
    m_featureDetector.setDetectionThreshold(configuration.featureDetectionThreshold);
    m_featureDetector.setCountLevels(configuration.countImageLevels);
    m_featureDetector.setLevelForFeature(configuration.minImageLevelForFeature,
                                         configuration.maxImageLevelForFeature);
    m_featureDetector.setCursorSize(configuration.featureCursorSize);
    m_featureDetector.setPixelEps(configuration.pixelEps);
    m_featureDetector.setMaxNumberIterations(configuration.maxNumberIterationsForOpticalFlow);
}

double MapInitializer::maxSquarePixelError() const
{
    return m_maxSquarePixelError;
}

void MapInitializer::setMaxPixelError(double maxPixelError)
{
    m_maxSquarePixelError = maxPixelError * maxPixelError;
}

double MapInitializer::epsDistanceForPlane() const
{
    return m_epsDistanceForPlane;
}

void MapInitializer::setEpsDistanceForPlane(double epsDistanceForPlane)
{
    m_epsDistanceForPlane = epsDistanceForPlane;
}

void MapInitializer::setCountTimes(int count)
{
    m_countTimes = count;
}

int MapInitializer::countTimes() const
{
    return m_countTimes;
}

double MapInitializer::mapScale() const
{
    return m_mapScale;
}

void MapInitializer::setMapScale(double scale)
{
    m_mapScale = scale;
}

int MapInitializer::minCountFeatures() const
{
    return m_minCountFeatures;
}

void MapInitializer::setMinCountFeatures(int count)
{
    m_minCountFeatures = count;
}

int MapInitializer::maxCountFeatures() const
{
    return m_featureDetector.maxCountFeatures();
}

void MapInitializer::setMaxCountFeatures(int maxCountFeatures)
{
    m_featureDetector.setMaxCountFeatures(maxCountFeatures);
}

float MapInitializer::minDisparity() const
{
    return std::sqrt(m_minDisparitySquared);
}

void MapInitializer::setMinDisparity(float value)
{
    m_minDisparitySquared = value * value;
}

int MapInitializer::minCountMapPoints() const
{
    return m_minCountMapPoints;
}

void MapInitializer::setMinCountMapPoints(int minCountMapPoints)
{
    m_minCountMapPoints = minCountMapPoints;
}

FeatureDetector & MapInitializer::featureDetector()
{
    return m_featureDetector;
}

const FeatureDetector & MapInitializer::featureDetector() const
{
    return m_featureDetector;
}

void MapInitializer::reset()
{
    m_matches.clear();
    m_inlinerIndices.clear();
    m_outlinersIndices.clear();
    m_decompositions.clear();
    m_featureDetector.reset();
    m_firstFeatures.clear();
    m_secondFeatures.clear();
}

ConstImage<uchar> MapInitializer::firstFrame() const
{
    return m_featureDetector.firstImage();
}

void MapInitializer::setFirstFrame(const std::shared_ptr<const Camera> & camera, const ImageRef<uchar> & frame)
{
    m_featureDetector.setFirstImage(frame);
    m_firstCamera = camera;
    Point2d pixelFocalLength = m_firstCamera->pixelFocalLength();
    m_firstCameraErrorMultipler = std::max(pixelFocalLength.x, pixelFocalLength.y);
    m_firstCameraErrorMultipler *= m_firstCameraErrorMultipler;
    m_firstFeatures.resize(0);
    m_featureImageLevel.resize(0);
    if (m_featureDetector.firstImage().data() != nullptr) {
        std::vector<FeatureDetector::FeatureCorner> features;
        m_featureDetector.detectFeaturesOnFirstImage(features);
        for (std::vector<FeatureDetector::FeatureCorner>::iterator it = features.begin(); it != features.end(); ++it) {
            m_firstFeatures.push_back(Point2f((float)it->pos.x, (float)it->pos.y));
            m_featureImageLevel.push_back(it->level);
        }
    }
}

ConstImage<uchar> MapInitializer::secondFrame() const
{
    return m_featureDetector.secondImage();
}

void MapInitializer::setSecondFrame(const std::shared_ptr<const Camera> & camera, const ImageRef<uchar> & frame)
{
    m_featureDetector.setSecondImage(frame);
    m_secondCamera = camera;
    Point2d pixelFocalLength = m_secondCamera->pixelFocalLength();
    m_secondCameraErrorMultipler = std::max(pixelFocalLength.x, pixelFocalLength.y);
    m_secondCameraErrorMultipler *= m_secondCameraErrorMultipler;
}

MapInitializer::InitializationResult MapInitializer::compute(Map * map, MapResourcesManager * manager, bool force)
{
    /*using namespace TMath;
    m_firstFeatures.clear();
    m_secondFeatures.clear();
    m_minCountFeatures = 7;
    m_minCountMapPoints = 7;
    std::vector<QVector3D> vv;
    QMatrix4x4 mPlane;
    mPlane.translate(0.0f, 0.0f, 5.0f);
    mPlane.rotate(QQuaternion::fromAxisAndAngle(2.0f, 1.0f, 3.0f, 30.0f));
    QMatrix4x4 realMotion;
    realMotion.translate(-0.5f, 0.0f, 1.0f);
    realMotion.rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -10.0f));
    for (int i=0; i<7; ++i) {
        QVector3D v((std::rand() % 100 - 50) * 0.05f, (std::rand() % 100 - 50) * 0.05f, 0.0f);
        v = mPlane * v;
        vv.push_back(v);
        Point2d d = m_firstCamera->project(Point2f(v.x() / v.z(), v.y() / v.z()));
        d += Point2d((std::rand() % 100 - 50) * 0.02f, (std::rand() % 100 - 50) * 0.02f);
        m_firstFeatures.push_back(Point2f((float)d.x, (float)d.y));
        v = realMotion * v;
        d = m_secondCamera->project(Point2f(v.x() / v.z(), v.y() / v.z()));
        d += Point2d((std::rand() % 100 - 50) * 0.04f, (std::rand() % 100 - 50) * 0.04f);
        m_secondFeatures.push_back(Point2f((float)d.x, (float)d.y));
        m_featureImageLevel.push_back(0);
    }
    for (int i=0; i<7; ++i) {
        QVector3D v((std::rand() % 100 - 50) * 0.05f, (std::rand() % 100 - 50) * 0.05f, (std::rand() % 100 - 50) * 0.05f);
        v = mPlane * v;
        vv.push_back(v);
        Point2d d = m_firstCamera->project(Point2f(v.x() / v.z(), v.y() / v.z()));
        d += Point2d((std::rand() % 100 - 50) * 0.02f, (std::rand() % 100 - 50) * 0.02f);
        m_firstFeatures.push_back(Point2f((float)d.x, (float)d.y));
        v = realMotion * v;
        d = m_secondCamera->project(Point2f(v.x() / v.z(), v.y() / v.z()));
        d += Point2d((std::rand() % 100 - 50) * 0.04f, (std::rand() % 100 - 50) * 0.04f);
        m_secondFeatures.push_back(Point2f((float)d.x, (float)d.y));
        m_featureImageLevel.push_back(0);
    }
    _computeMatches();
    _computeTransformation();
    QMatrix3x3 calculatedRotation = TTools::convert<QMatrix3x3>(resultRotation());
    QVector3D calculatedTranslation = TTools::convert<QVector3D>(resultTranslation());
    TMatrixd planeMatrix = _computePlaneMatrix();
    TMatrixd trR = planeMatrix.slice(3, 3);
    TVectord trT = planeMatrix.getColumn(3, 3);
    TMatrixd invPlaneMatrix = TTools::invertedWorldMatrix(planeMatrix);
    TMatrixd invTrR = invPlaneMatrix.slice(3, 3);
    TVectord invTrT = invPlaneMatrix.getColumn(3, 3);
    int i;
    TMatrixd firstR(3, 3);
    firstR.setToIdentity();
    TVectord firstT(3);
    firstT.setZero();
    firstT += firstR * invTrT;
    firstR *= invTrR;
    TMatrixd secondR = resultRotation();
    TVectord secondT = resultTranslation();
    secondT += secondR * invTrT;
    secondR *= invTrR;*/
    /*for (i=0; i<m_matches.size(); ++i) {
        TVectord v = trR * m_matches[i].position3d + trT;
        TVectord vf = firstR * v + firstT;
        TVectord vs = secondR * v + secondT;
        qDebug() << (m_firstCamera->project(Point2d(vf(0) / vf(2), vf(1) / vf(2))).cast<float>() -
                     m_matches[i].pixelFirst).lengthSquared();
        qDebug() << (m_secondCamera->project(Point2d(vs(0) / vs(2), vs(1) / vs(2))).cast<float>() -
                     m_matches[i].pixelSecond).lengthSquared();
    }
    return InitializationResult::Reset;*/
    if ((m_featureDetector.firstImage().data() != nullptr) &&
            (m_featureDetector.secondImage().data() != nullptr)) {
        if (m_secondFeatures.size() != m_firstFeatures.size()) {
            if ((m_firstCamera == m_secondCamera) ||
                    ((m_firstCamera->imageSize() == m_secondCamera->imageSize()) &&
                     (m_firstCamera->cameraParameters() == m_secondCamera->cameraParameters()))) {
                m_secondFeatures = m_firstFeatures;
            } else {
                m_secondFeatures.resize(m_firstFeatures.size());
                for (std::size_t i = 0; i < m_secondFeatures.size(); ++i) {
                    m_secondFeatures[i] = m_secondCamera->project(m_firstCamera->unproject(m_firstFeatures[i])).cast<float>();
                }
            }
        }
        std::vector<TrackingResult> success;
        m_featureDetector.tracking2dLK(success, m_secondFeatures, m_firstFeatures);
        std::vector<float> disparities;
        disparities.reserve(m_firstFeatures.size());
        std::size_t j = 0;
        for (std::size_t i = 0; i < success.size(); ++i) {
            if (success[i] != TrackingResult::Fail) {
                disparities.push_back((m_secondFeatures[j] - m_firstFeatures[j]).lengthSquared());
                ++j;
            } else {
                m_firstFeatures.erase(m_firstFeatures.begin() + j);
                m_secondFeatures.erase(m_secondFeatures.begin() + j);
                m_featureImageLevel.erase(m_featureImageLevel.begin() + j);
            }
        }
        if ((int)m_secondFeatures.size() >= m_minCountFeatures) {
            int offset = (int) std::floor(disparities.size() * 0.5f);
            std::nth_element(disparities.begin(), disparities.begin() + offset, disparities.end());
            if (disparities[offset] > m_minDisparitySquared) {
                _computeMatches();
                if (_computeTransformation()) {
                    if ((int)_computePoints3d() >= m_minCountMapPoints) {
                        if (map != nullptr)
                            _initializeMap(map, manager);
                        return InitializationResult::Success;
                    }
                }
            }
            if (!force)
                return InitializationResult::Failure;
        }
    }
    reset();
    return InitializationResult::Reset;
}

const std::vector<Point2f> & MapInitializer::firstFeatures() const
{
    return m_firstFeatures;
}

const std::vector<Point2f> & MapInitializer::secondFeatures() const
{
    return m_secondFeatures;
}

void MapInitializer::_initializeMap(Map * map, MapResourcesManager * manager)
{
    TMath_assert(!m_decompositions.empty());
    TMath_assert((m_decompositions[0].rotation.rows() == 3) && (m_decompositions[0].rotation.cols() == 3));
    TMath_assert(m_decompositions[0].translation.size() == 3);

    using namespace TMath;

    map->resetMap(manager);

    std::vector<Image<uchar>> imagePyramid;
    imagePyramid.resize(map->countImageLevels());
    int size = std::min((int)imagePyramid.size(), m_featureDetector.countLevels());
    int k;
    for (k = 0; k < size; ++k)
        imagePyramid[k] = m_featureDetector.firstImageAtLevel(k).copy();
    size = (int)imagePyramid.size();
    for (; k < size; ++k) {
        imagePyramid[k] = Image<uchar>(imagePyramid[k - 1].size() / 2);
        ImageProcessing::halfSample(imagePyramid[k], imagePyramid[k - 1]);
    }
    std::shared_ptr<KeyFrame> firstKeyFrame = map->createKeyFrame(m_firstCamera, imagePyramid);
    manager->lock(firstKeyFrame.get());
    TMatrixd rotation(3, 3);
    rotation.setToIdentity();
    TVectord translation(3);
    translation.setZero();
    firstKeyFrame->setRotation(rotation);
    firstKeyFrame->setTranslation(translation);

    size = std::min((int)imagePyramid.size(), m_featureDetector.countLevels());
    for (k = 0; k < size; ++k)
        imagePyramid[k] = m_featureDetector.secondImageAtLevel(k).copy();
    size = (int)imagePyramid.size();
    for (; k < size; ++k) {
        imagePyramid[k] = Image<uchar>(imagePyramid[k - 1].size() / 2);
        ImageProcessing::halfSample(imagePyramid[k], imagePyramid[k - 1]);
    }
    std::shared_ptr<KeyFrame> secondKeyFrame = map->createKeyFrame(m_secondCamera, imagePyramid);
    manager->lock(secondKeyFrame.get());
    secondKeyFrame->setRotation(m_decompositions[0].rotation);
    secondKeyFrame->setTranslation(m_decompositions[0].translation);

    std::vector<double> vectorDepths;
    vectorDepths.reserve(m_matches.size());

    for (std::size_t i = 0; i < m_matches.size(); ++i) {
        const Match & match = m_matches[i];
        if (match.position3d.size() == 3) {
            std::shared_ptr<MapPoint> mapPoint = map->createMapPoint(match.position3d);
            manager->lock(mapPoint.get());
            map->createFeature(firstKeyFrame, match.pixelFirst, match.imageLevel, mapPoint);
            map->createFeature(secondKeyFrame, match.pixelSecond, match.imageLevel, mapPoint);
            mapPoint->statistic().incSuccess();
            vectorDepths.push_back(match.position3d(2));
            manager->unlock(mapPoint.get());
        }
    }
    manager->unlock(firstKeyFrame.get());
    manager->unlock(secondKeyFrame.get());
    std::size_t medianOffset = (std::size_t)std::floor(vectorDepths.size() * 0.5);
    std::nth_element(vectorDepths.begin(),
                     vectorDepths.begin() + medianOffset,
                     vectorDepths.end());
    double scale = m_mapScale / vectorDepths[medianOffset];

    if (_findPlane()) {
        TMatrixd planeMatrix = _computePlaneMatrix();
        if (planeMatrix.data() != nullptr) {
            TMatrixd invertedPlaneMatrix = planeMatrix;
            if (invertedPlaneMatrix.invert())
                map->transform(manager, invertedPlaneMatrix.slice(3, 3), invertedPlaneMatrix.getColumn(3, 3));
        }
    }
    map->scale(manager, scale);
}

std::size_t MapInitializer::_computePoints3d()
{
    TMath_assert(!m_decompositions.empty());
    TMath_assert((m_decompositions[0].rotation.rows() == 3) && (m_decompositions[0].rotation.cols() == 3));
    TMath_assert(m_decompositions[0].translation.size() == 3);

    using namespace TMath;

    std::size_t countSuccess = 0;

    TMatrixd rotation = m_decompositions[0].rotation;
    TVectord translation = m_decompositions[0].translation;

    TMatrixd R = rotation;
    if (!TTools::matrix3x3Invert(R))
        return 0;
    TVectord t = - R * translation;

    TVectord trPoint(3);
    for (std::vector<std::size_t>::iterator it = m_inlinerIndices.begin();
         it != m_inlinerIndices.end(); ) {
        Match & match = m_matches[*it];
        match.position3d = TTools::triangulatePoint(R, t, match.camFirst, match.camSecond);
        if (match.position3d.size() != 3) {
            m_outlinersIndices.push_back(*it);
            it = m_inlinerIndices.erase(it);
            continue;
        }
        if (match.position3d(2) < 0.0) {
            m_outlinersIndices.push_back(*it);
            it = m_inlinerIndices.erase(it);
            match.position3d = TVectord();
            continue;
        }
        if ((m_firstCamera->project(Point2d(match.position3d(0) / match.position3d(2),
                                            match.position3d(1) / match.position3d(2))).cast<float>() -
                        match.pixelFirst).lengthSquared() > m_maxSquarePixelError) {
            m_outlinersIndices.push_back(*it);
            it = m_inlinerIndices.erase(it);
            continue;
        }
        trPoint = rotation * match.position3d + translation;
        if ((m_secondCamera->project(Point2d(trPoint(0) / trPoint(2),
                                             trPoint(1) / trPoint(2))).cast<float>() -
                        match.pixelSecond).lengthSquared() > m_maxSquarePixelError) {
            m_outlinersIndices.push_back(*it);
            it = m_inlinerIndices.erase(it);
            continue;
        }
        ++countSuccess;
        ++it;
    }
    return countSuccess;
}

bool MapInitializer::_findPlane()
{
    TMath_assert(m_inlinerIndices.size() >= 3);
    using namespace TMath;

    double bestScore = std::numeric_limits<double>::max();
    m_outlinersIndices.clear();
    std::size_t baseIndices[3];
    std::size_t i, j;
    TVectord va(3), vb(3), vc(3), d(3), nX(3), nY(3), nZ(3);
    TVectord bestP, bestN;
    double error;
    TMath::Random_mt19937 rnd;
    rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()).count());
    for (int iteration = 0; iteration < m_countTimes; ++iteration) {
        if ((iteration % 200) == 0)
            rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count() + iteration * 7);
        for (i = 0; i < 3; ++i) {
            bool isUnique;
            for(;;) {
                isUnique = true;
                baseIndices[i] = m_inlinerIndices[rnd() % m_inlinerIndices.size()];
                for (j = 0; j < i; ++j) {
                    if (baseIndices[j] == baseIndices[i]) {
                        isUnique = false;
                        break;
                    }
                }
                if (isUnique)
                    break;
            }
        }
        va = m_matches[baseIndices[0]].position3d;
        vb = m_matches[baseIndices[1]].position3d;
        vc = m_matches[baseIndices[2]].position3d;
        nX = (vb - va).normalized();
        nZ = cross3(vb - va, vc - va).normalized();
        //nY = cross3(nZ, nX);
        /*int count = 0;
        Point2d min(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        Point2d max(std::numeric_limits<double>::min(), std::numeric_limits<double>::min());*/
        Point2d p;
        double score = 0.0;
        for (i = 0; i < m_inlinerIndices.size(); ++i) {
            d = m_matches[m_inlinerIndices[i]].position3d - va;
            score += std::min(std::fabs(dot(d, nZ)), m_epsDistanceForPlane);
        }
        if (score < bestScore) {
            bestP = (va + vb + vc) / 3.0;
            bestN = nZ;
        }
    }
    if ((bestP.size() != 3) || (bestN.size() != 3))
        return false;
    std::vector<std::size_t> newInliners;
    for (i = 0; i < m_inlinerIndices.size(); ++i) {
        error = std::fabs(dot(m_matches[m_inlinerIndices[i]].position3d - bestP, bestN));
        if (error < m_epsDistanceForPlane)
            newInliners.push_back(m_inlinerIndices[i]);
    }
    m_inlinerIndices = std::move(newInliners);
    if (m_inlinerIndices.size() < 3)
        return false;
    return true;
}

TMath::TMatrixd MapInitializer::_computePlaneMatrix()
{
    using namespace TMath;
    TMath::TMatrixd matrix(3, 3);
    matrix.setZero();
    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
    double sumWeight = 0.0;
    for (std::size_t i=0; i<m_inlinerIndices.size(); ++i) {
        const Match& match = m_matches[m_inlinerIndices[i]];
        if (match.position3d.size() == 3) {
            sum_x += match.position3d(0);
            sum_y += match.position3d(1);
            sum_z += match.position3d(2);
            matrix(0, 0) += match.position3d(0) * match.position3d(0);
            matrix(0, 1) += match.position3d(0) * match.position3d(1);
            matrix(0, 2) += match.position3d(0) * match.position3d(2);
            matrix(1, 0) += match.position3d(1) * match.position3d(0);
            matrix(1, 1) += match.position3d(1) * match.position3d(1);
            matrix(1, 2) += match.position3d(1) * match.position3d(2);
            matrix(2, 0) += match.position3d(2) * match.position3d(0);
            matrix(2, 1) += match.position3d(2) * match.position3d(1);
            matrix(2, 2) += match.position3d(2) * match.position3d(2);
            sumWeight += 1.0;
        }
    }
    double sum_x_x = (sum_x * sum_x) / sumWeight;
    double sum_x_y = (sum_x * sum_y) / sumWeight;
    double sum_x_z = (sum_x * sum_z) / sumWeight;
    double sum_y_y = (sum_y * sum_y) / sumWeight;
    double sum_y_z = (sum_y * sum_z) / sumWeight;
    double sum_z_z = (sum_z * sum_z) / sumWeight;
    matrix(0, 0) -= sum_x_x; matrix(0, 1) -= sum_x_y; matrix(0, 2) -= sum_x_z;
    matrix(1, 0) -= sum_x_y; matrix(1, 1) -= sum_y_y; matrix(1, 2) -= sum_y_z;
    matrix(2, 0) -= sum_x_z; matrix(2, 1) -= sum_y_z; matrix(2, 2) -= sum_z_z;
    TVectord p(3);
    p(0) = sum_x / sumWeight;
    p(1) = sum_y / sumWeight;
    p(2) = sum_z / sumWeight;
    m_svd.compute(matrix, false);
    TVectord planeLocalZ(3, m_svd.V_transposed().getDataRow(2));
    //planeLocalZ.normalize();
    TVectord planePos = TVectord::create(sum_x / sumWeight, sum_y / sumWeight, sum_z / sumWeight);
    TVectord cameraSecondPosition = - (resultRotation().refTransposed() * resultTranslation());
    if (dot(cameraSecondPosition - planePos, planeLocalZ) < 0.0) {
        planeLocalZ = - planeLocalZ;
    }
    TVectord planeLocalX, planeLocalY;
    planeLocalX = cross3(TVectord::create(0.0, 1.0, 0.0), planeLocalZ);
    if (planeLocalX.lengthSquared() < 5e-4) {
        planeLocalY = cross3(planeLocalZ, TVectord::create(1.0, 0.0, 0.0));
        planeLocalY.normalize();
        planeLocalX = cross3(planeLocalY, planeLocalZ);
    } else {
        planeLocalX.normalize();
        planeLocalY = cross3(planeLocalZ, planeLocalX);
    }
    TMatrixd planeMatrix(4, 4);
    planeMatrix.setColumn(0, planeLocalX);
    planeMatrix.setColumn(1, planeLocalY);
    planeMatrix.setColumn(2, planeLocalZ);
    planeMatrix.setColumn(3, planePos);
    planeMatrix.setRow(3, TVectord::create(0.0, 0.0, 0.0, 1.0));
    return planeMatrix;
}

TMath::TMatrixd MapInitializer::resultRotation() const
{
    if (m_decompositions.empty()) {
        TMath::TMatrixd R(3, 3);
        R.setToIdentity();
        return R;
    }
    return m_decompositions[0].rotation * (1.0 / TMath::TTools::matrix3x3Determinant(m_decompositions[0].rotation));
}

TMath::TVectord MapInitializer::resultTranslation() const
{
    if (m_decompositions.empty()) {
        TMath::TVectord t(3);
        t.setZero();
        return t;
    }
    return m_decompositions[0].translation;
}

TMath::TMatrixd MapInitializer::resultMotion() const
{
    TMath::TMatrixd motion(4, 4);
    if (m_decompositions.empty()) {
        motion.setToIdentity();
    } else {
        const HomographyDecomposition& decomposition = m_decompositions[0];
        motion.fill(0, 0, decomposition.rotation);
        motion.setColumn(3, decomposition.translation);
        motion.setRow(3, TMath::TVectord::create(0.0f, 0.0f, 0.0f, 1.0f));
    }
    return motion;
}

void MapInitializer::_computeMatches()
{
    m_matches.resize(std::min(m_firstFeatures.size(), m_secondFeatures.size()));
    Point2d d;
    Camera::ProjectionInfo projectionInfo;
    for (std::size_t i = 0; i < m_matches.size(); ++i) {
        Match & match = m_matches[i];
        match.imageLevel = m_featureImageLevel[i];
        match.pixelFirst = m_firstFeatures[i];
        d = m_firstCamera->unproject(match.pixelFirst);
        match.camFirst = TMath::TVectord::create(d.x, d.y, 1.0);
        match.pixelSecond = m_secondFeatures[i];
        d = m_secondCamera->unproject(match.pixelSecond, projectionInfo);
        match.camSecond = TMath::TVectord::create(d.x, d.y, 1.0);

        match.secondPixelProjDerivs = m_secondCamera->getProjectionDerivatives(projectionInfo);
    }
}

double MapInitializer::_getHomographyError(const TMath::TMatrixd& homography, const Match& match) const
{
    TMath::TVectord v = homography * match.camFirst;
    v(0) = v(0) / v(2) - match.camSecond(0);
    v(1) = v(1) / v(2) - match.camSecond(1);
    return (v(0) * v(0) + v(1) * v(1)) * m_secondCameraErrorMultipler;
}

TMath::TMatrixd MapInitializer::_findHomography()
{
    TMath_assert(m_inlinerIndices.size() >= 4);
    TMath::TMatrixd M(std::max((int)(m_inlinerIndices.size() * 2), 9), 9);
    double* dataRow = M.firstDataRow();
    for (std::size_t i = 0; i < m_inlinerIndices.size(); ++i) {
        const Match & match = m_matches[m_inlinerIndices[i]];

        dataRow[0] = match.camFirst(0);
        dataRow[1] = match.camFirst(1);
        dataRow[2] = 1.0;
        dataRow[3] = 0.0f;
        dataRow[4] = 0.0f;
        dataRow[5] = 0.0f;
        dataRow[6] = - match.camFirst(0) * match.camSecond(0);
        dataRow[7] = - match.camFirst(1) * match.camSecond(0);
        dataRow[8] = - match.camSecond(0);
        dataRow = &dataRow[9];

        dataRow[0] = 0.0f;
        dataRow[1] = 0.0f;
        dataRow[2] = 0.0f;
        dataRow[3] = match.camFirst(0);
        dataRow[4] = match.camFirst(1);
        dataRow[5] = 1.0;
        dataRow[6] = - match.camFirst(0) * match.camSecond(1);
        dataRow[7] = - match.camFirst(1) * match.camSecond(1);
        dataRow[8] = - match.camSecond(1);

        dataRow = &dataRow[9];
    }

    if (m_inlinerIndices.size() == 4) {
        for (int i=0; i<9; ++i)
            dataRow[i] = 0.0;
    }

    m_svd.compute(M, false);
    const double * constDataRow = m_svd.V_transposed().getDataRow(8);
    TMath::TMatrixd homography(3, 3, constDataRow);
    return homography;
}

bool MapInitializer::_findBestHomography()
{
    double bestScore = std::numeric_limits<double>::max();
    m_outlinersIndices.clear();
    m_inlinerIndices.resize(4);
    std::size_t i, j;
    TMath::Random_mt19937 rnd;
    rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()).count());
    for (int iteration = 0; iteration < m_countTimes; ++iteration) {
        if ((iteration % 200) == 0)
            rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count() + iteration * 7);
        for (i = 0; i < 4; ++i) {
            bool isUnique;
            for(;;) {
                isUnique = true;
                m_inlinerIndices[i] = rnd() % m_matches.size();
                for (j = 0; j < i; ++j) {
                    if (m_inlinerIndices[j] == m_inlinerIndices[i]) {
                        isUnique = false;
                        break;
                    }
                }
                if (isUnique)
                    break;
            }
        }
        TMath::TMatrixd homography = _findHomography();
        double score = 0.0, error;
        for (i = 0; i < m_matches.size(); ++i) {
            error = _getHomographyError(homography, m_matches[i]);
            score += std::min(error, m_maxSquarePixelError);
        }
        if (score < bestScore) {
            bestScore = score;
            m_bestHomography = homography;
        }
    }
    m_inlinerIndices.clear();
    if (m_bestHomography.rows() == 0)
        return false;
    for (i = 0; i < m_matches.size(); ++i) {
        if (_getHomographyError(m_bestHomography, m_matches[i]) <= m_maxSquarePixelError)
            m_inlinerIndices.push_back(i);
        else
            m_outlinersIndices.push_back(i);
    }
    if (m_inlinerIndices.size() < 4)
        return false;
    return true;
}

void MapInitializer::_refineHomography(TMath::TMatrixd & homography)
{
    using namespace TMath;

    TWLS<double> wls(9);
    wls.addPrior(1.0);

    std::vector<TMatrixd> jacobians;
    std::vector<Point2d> errors;
    std::vector<double> errorSquared;

    for(std::size_t i = 0; i < m_inlinerIndices.size(); ++i) {
        Match & match = m_matches[m_inlinerIndices[i]];

        // First, find error.
        TMath::TVectord second = homography * match.camFirst;
        Point2d error(second(0) / second(2) - match.camSecond(0), second(1) / second(2) - match.camSecond(1));
        error.set(match.secondPixelProjDerivs(0, 0) * error.x +
                               match.secondPixelProjDerivs(0, 1) * error.y,
                  match.secondPixelProjDerivs(1, 0) * error.x +
                               match.secondPixelProjDerivs(1, 1) * error.y);

        errorSquared.push_back(error.lengthSquared());
        errors.push_back(error);

        TMatrixd jacobian(2, 9);
        double denominator = second(2);

        // Jacobians wrt to the elements of the homography:
        // For x:
        jacobian(0, 0) = match.camFirst(0) / denominator;
        jacobian(0, 1) = match.camFirst(1) / denominator;
        jacobian(0, 2) = 1.0 / denominator;
        jacobian(0, 3) = 0.0;
        jacobian(0, 4) = 0.0;
        jacobian(0, 5) = 0.0;
        double a = - second(0) / (denominator * denominator);
        jacobian(0, 6) = match.camFirst(0) * a;
        jacobian(0, 7) = match.camFirst(1) * a;
        jacobian(0, 8) = a;

        // For y:
        jacobian(1, 0) = 0.0;
        jacobian(1, 1) = 0.0;
        jacobian(1, 2) = 0.0;
        jacobian(1, 3) = match.camFirst(0) / denominator;
        jacobian(1, 4) = match.camFirst(1) / denominator;
        jacobian(1, 5) = 1.0 / denominator;
        a = - second(1) / (denominator * denominator);
        jacobian(1, 6) = match.camFirst(0) * a;
        jacobian(1, 7) = match.camFirst(1) * a;
        jacobian(1, 8) = a;

        jacobians.push_back(match.secondPixelProjDerivs * jacobian);
    }

    // Calculate robust sigma:
    std::vector<double> es = errorSquared;
    double sigmaSquared = TukeyRobustCost::findSquareSigma(es);

    // Add re-weighted measurements to WLS:
    for(std::size_t i = 0; i < m_inlinerIndices.size(); ++i) {
        double weight = TukeyRobustCost::weight(errorSquared[i], sigmaSquared);
        Point2d & error = errors[i];
        TMatrixd & jacobian = jacobians[i];
        wls.addMeasurement(error.x, jacobian.getRow(0), weight);
        wls.addMeasurement(error.y, jacobian.getRow(1), weight);
    }
    wls.compute();
    TVectord updateVector = wls.X();
    TMatrixd updateMatrix(3, 3, updateVector.data());//          _[ ]_
    homography -= updateMatrix; //In PTAM error here?         \_&(0_o)&_]
}//                                                                "

bool MapInitializer::_computeDecompositionsOfHomography()
{
    using namespace TMath;

    m_svd.compute(m_bestHomography);

    TVectord W = m_svd.diagonalW();
    double w1 = std::fabs(W(0)); // The paper suggests the square of these (e.g. the evalues of AAT)
    double w2 = std::fabs(W(1)); // should be used, but this is wrong. c.f. Faugeras' book.
    double w3 = std::fabs(W(2));

    TMatrixd U = m_svd.U();
    TMatrixd V = m_svd.V();

    double s = TTools::matrix3x3Determinant(U) * TTools::matrix3x3Determinant(V);

    double prime_PM = w2;

    if ((w1 == w2) || (w2 == w3))
        return false;//This motion case is not implemented or is degenerate.

    double x1_PM;
    double x2;
    double x3_PM;

    // All below deals with (w1 != w2 &&  w2 != w3)
    {
        x1_PM = std::sqrt((w1 * w1 - w2 * w2) / (w1 * w1 - w3 * w3));
        x2    = 0.0;
        x3_PM = std::sqrt((w2 * w2 - w3 * w3) / (w1 * w1 - w3 * w3));
    }

    const double e1[4] = { 1.0, -1.0,  1.0, -1.0 };
    const double e2[4] = { 1.0,  1.0, -1.0, -1.0 };

    m_decompositions.resize(8);
    TVectord np(3), tp(3);

    //QMatrix4x4 m;
    //QVector3D t;

    // Case 1, d' > 0:
    double D = s * prime_PM;
    for (int signs = 0; signs < 4; ++signs) {
        HomographyDecomposition & decomposition = m_decompositions[signs];

        decomposition.D = D;
        double sinTheta = (w1 - w3) * x1_PM * x3_PM * e1[signs] * e2[signs] / w2;
        double cosTheta = (w1 * x3_PM * x3_PM + w3 * x1_PM * x1_PM) / w2;
        decomposition.rotation = TMatrixd::Identity(3);
        decomposition.rotation(0, 0) = cosTheta;
        decomposition.rotation(0, 2) = - sinTheta;
        decomposition.rotation(2, 0) = sinTheta;
        decomposition.rotation(2, 2) = cosTheta;

        tp(0) = (w1 - w3) * x1_PM * e1[signs];
        tp(1) = 0.0f;
        tp(2) = - (w1 - w3) * x3_PM * e2[signs];

        np(0) = x1_PM * e1[signs];
        np(1) = x2;
        np(2) = x3_PM * e2[signs];
        decomposition.n = V * np;

        decomposition.rotation = U * decomposition.rotation * V.refTransposed() * s;
        decomposition.translation = U * tp;

        /*TMath::TTools::convert(m, decomposition.rotation);
        t.setX(decomposition.translate(0));
        t.setY(decomposition.translate(1));
        t.setZ(decomposition.translate(2));
        t.x();*/
    }
    // Case 1, d' < 0:
    D = - s * prime_PM;
    for (int signs = 0; signs < 4; ++signs) {
        HomographyDecomposition & decomposition = m_decompositions[signs + 4];

        decomposition.D = D;
        double sinPhi = (w1 + w3) * x1_PM * x3_PM * e1[signs] * e2[signs] / w2;
        double cosPhi = (w3 * x1_PM * x1_PM - w1 * x3_PM * x3_PM) / w2;
        decomposition.rotation = - TMatrixd::Identity(3);
        decomposition.rotation(0, 0) = cosPhi;
        decomposition.rotation(0, 2) = sinPhi;
        decomposition.rotation(2, 0) = sinPhi;
        decomposition.rotation(2, 2) = - cosPhi;

        tp(0) = (w1 + w3) * x1_PM * e1[signs];
        tp(1) = 0.0f;
        tp(2) = (w1 + w3) * x3_PM * e2[signs];

        np(0) = x1_PM * e1[signs];
        np(1) = x2;
        np(2) = x3_PM * e2[signs];
        decomposition.n = V * np;

        decomposition.rotation = U * decomposition.rotation * V.refTransposed() * s;
        decomposition.translation = U * tp;

        /*TMath::TTools::convert(m, decomposition.rotation);
        t.setX(decomposition.translation(0));
        t.setY(decomposition.translation(1));
        t.setZ(decomposition.translation(2));
        t.x();*/
    }
    return true;
}

double MapInitializer::_getEssentialError(const TMath::TMatrixd& essential, const Match& match) const
{
    TMath::TVectord v = essential * match.camFirst;
    double s2 = m_secondCameraErrorMultipler / (v(0) * v(0) + v(1) * v(1));
    double d2 = match.camSecond(0) * v(0) + match.camSecond(1) * v(1) + v(2);

    v = essential.refTransposed() * match.camSecond;
    double s1 = m_firstCameraErrorMultipler / (v(0) * v(0) + v(1) * v(1));
    double d1 = match.camFirst(0) * v(0) + match.camFirst(1) * v(1) + v(2);

    return std::max(d1 * d1 * s1, d2 * d2 * s2);
}

TMath::TMatrixd MapInitializer::_fixEssentialMatrix(const TMath::TMatrixd& essential)
{
    m_svd.compute(essential);

    TMath::TMatrixd E(3, 3);
    E.setZero();
    E(0, 0) = m_svd.diagonalW()(0);
    E(1, 1) = m_svd.diagonalW()(1);
    E(2, 2) = 0.0;

    return m_svd.U() * E * m_svd.V_transposed();
}

int MapInitializer::_computeEssential_from7points(TMath::TMatrixd results[3])
{
    TMath_assert(m_inlinerIndices.size() >= 7);
    TMath::TMatrixd M(7, 9);
    double * dataRow = M.firstDataRow();
    for (std::size_t i = 0; i < m_inlinerIndices.size(); ++i) {
        const Match & match = m_matches[m_inlinerIndices[i]];

        dataRow[0] = match.camFirst(0) * match.camSecond(0);
        dataRow[1] = match.camFirst(1) * match.camSecond(0);
        dataRow[2] = match.camSecond(0);
        dataRow[3] = match.camFirst(0) * match.camSecond(1);
        dataRow[4] = match.camFirst(1) * match.camSecond(1);
        dataRow[5] = match.camSecond(1);
        dataRow[6] = match.camFirst(0);
        dataRow[7] = match.camFirst(1);
        dataRow[8] = 1.0;

        dataRow = &dataRow[9];
    }

    m_svd.compute(M, true);
    TMath::TMatrixd F1(3, 3, m_svd.V_transposed().getDataRow(7));
    TMath::TMatrixd F2(3, 3, m_svd.V_transposed().getDataRow(8));
    return _computeEssentials(results, F1, F2);
}

int MapInitializer::_computeEssentials(TMath::TMatrixd results[3],
                                       const TMath::TMatrixd & F1,
                                       const TMath::TMatrixd & F2) const
{
    using namespace TMath;

    const double * f1 = F1.data();
    const double * f2 = F2.data();
    double k[4];
    k[0] =  (f1[0] * f1[4] * f1[8])
          - (f1[0] * f1[5] * f1[7])
          - (f1[1] * f1[3] * f1[8])
          + (f1[1] * f1[5] * f1[6])
          + (f1[2] * f1[3] * f1[7])
          - (f1[2] * f1[4] * f1[6]);
    k[1] =  (f1[0] * f1[4] * f2[8] + (f2[0] * f1[4] + f1[0] * f2[4]) * f1[8])
          - (f1[0] * f1[5] * f2[7] + (f2[0] * f1[5] + f1[0] * f2[5]) * f1[7])
          - (f1[1] * f1[3] * f2[8] + (f2[1] * f1[3] + f1[1] * f2[3]) * f1[8])
          + (f1[1] * f1[5] * f2[6] + (f2[1] * f1[5] + f1[1] * f2[5]) * f1[6])
          + (f1[2] * f1[3] * f2[7] + (f2[2] * f1[3] + f1[2] * f2[3]) * f1[7])
          - (f1[2] * f1[4] * f2[6] + (f2[2] * f1[4] + f1[2] * f2[4]) * f1[6]);
    k[2] =  ((f2[0] * f1[4] + f1[0] * f2[4]) * f2[8] + f2[0] * f2[4] * f1[8])
          - ((f2[0] * f1[5] + f1[0] * f2[5]) * f2[7] + f2[0] * f2[5] * f1[7])
          - ((f2[1] * f1[3] + f1[1] * f2[3]) * f2[8] + f2[1] * f2[3] * f1[8])
          + ((f2[1] * f1[5] + f1[1] * f2[5]) * f2[6] + f2[1] * f2[5] * f1[6])
          + ((f2[2] * f1[3] + f1[2] * f2[3]) * f2[7] + f2[2] * f2[3] * f1[7])
          - ((f2[2] * f1[4] + f1[2] * f2[4]) * f2[6] + f2[2] * f2[4] * f1[6]);
    k[3] =  (f2[0] * f2[4] * f2[8])
          - (f2[0] * f2[5] * f2[7])
          - (f2[1] * f2[3] * f2[8])
          + (f2[1] * f2[5] * f2[6])
          + (f2[2] * f2[3] * f2[7])
          - (f2[2] * f2[4] * f2[6]);
    double roots[3];
    int countRoots = _getRealRoots(roots, k[0], k[1], k[2], k[3]);
    for (int i = 0; i < countRoots; ++i) {
        results[i] = F1 * roots[i] + F2;
        if (std::fabs(results[i](2, 2)) > 1e-6)
            results[i] *= (1.0 / results[i](2, 2));
    }
    return countRoots;
}

bool MapInitializer::_findBestEssential()
{
    using namespace TMath;

    double bestScore = std::numeric_limits<double>::max();
    m_outlinersIndices.clear();
    m_inlinerIndices.resize(7);
    std::size_t i, j;
    Random_mt19937 rnd;
    rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()).count());

    TMatrixd essentials[3];
    int countEssentials;

    for (int iteration = 0; iteration < m_countTimes; ++iteration) {
        if ((iteration % 200) == 0)
            rnd.seed((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()).count() + iteration * 7);
        for (i = 0; i < 7; ++i) {
            bool isUnique;
            for(;;) {
                isUnique = true;
                m_inlinerIndices[i] = rnd() % m_matches.size();
                for (j = 0; j < i; ++j) {
                    if (m_inlinerIndices[j] == m_inlinerIndices[i]) {
                        isUnique = false;
                        break;
                    }
                }
                if (isUnique)
                    break;
            }
        }
        countEssentials = _computeEssential_from7points(essentials);
        for (int k = 0; k < countEssentials; ++k) {
            const TMatrixd& essential = essentials[k];
            double score = 0.0, error;
            for (i = 0; i < m_matches.size(); ++i) {
                error = _getEssentialError(essential, m_matches[i]);
                score += std::min(error, m_maxSquarePixelError);
            }
            if (score < bestScore) {
                bestScore = score;
                m_bestEssential = essential;
            }
        }
    }
    m_inlinerIndices.clear();
    if (m_bestEssential.rows() == 0)
        return false;
    for (i = 0; i < m_matches.size(); ++i) {
        if (_getEssentialError(m_bestEssential, m_matches[i]) <= m_maxSquarePixelError)
            m_inlinerIndices.push_back(i);
        else
            m_outlinersIndices.push_back(i);
    }
    if (m_inlinerIndices.size() < 7)
        return false;
    return true;
}

TMath::TMatrixd MapInitializer::_sqew3d(const TMath::TVectord& v) const
{
    TMath_assert(v.size() == 3);
    TMath::TMatrixd m(3, 3);
    m(0, 0) = 0.0;
    m(0, 1) = - v(2);
    m(0, 2) = v(1);
    m(1, 0) = v(2);
    m(1, 1) = 0.0;
    m(1, 2) = - v(0);
    m(2, 0) = - v(1);
    m(2, 1) = v(0);
    m(2, 2) = 0.0;
    return m;
}

int MapInitializer::_getRealRoots(double * roots, double a, double b, double c, double d) const
{
    if (std::fabs(a) < 1e-6) {
        if (std::fabs(b) < 1e-6) {
            if (std::fabs(c) < 1e-6) {
                return 0;
            }
            roots[0] = - d / c;
            return 1;
        } else {
            double D = c * c - 4.0 * b * d;
            if (D < 0.0) {
                return 0;
            } else if (D < 1e-6) {
                roots[0] = - c / (2.0 * b);
                return 1;
            } else {
                D = std::sqrt(D);
                double b2 = 2.0 * b;
                roots[0] = (- c + D) / (b2);
                roots[1] = (- (c + D)) / (b2);
                return 2;
            }
        }
    }
    double a1 = b / a, b1 = c / a, c1 = d / a;
    double Q = (a1 * a1 - 3.0 * b1) / 9.0;
    double R = (2.0 * a1 * a1 * a1 - 9.0 * a1 * b1 + 27.0 * c1) / 54.0;
    double R_2 = R * R;
    double Q_3 = Q * Q * Q;
    if (R_2 < Q_3) {
        double t = std::acos(R / std::sqrt(Q_3)) / 3.0;
        double m = - 2.0 * std::sqrt(Q);
        roots[0] = m * std::cos(t) - a1 / 3.0;
        roots[1] = m * std::cos(t + ((2.0 / 3.0) * M_PI)) - a1 / 3.0;
        roots[2] = m * std::cos(t - ((2.0 / 3.0) * M_PI)) - a1 / 3.0;
        return 3;
    }
    double A = - std::pow(std::fabs(R) + std::sqrt(R_2 - Q_3), 1.0 / 3.0);
    if (R < 0.0)
        A = - A;
    double B = (std::fabs(A) > 1e-6) ? Q / A : 0.0;
    roots[0] = (A + B) - a1 / 3.0;
    if (std::fabs(A - B) < 1e-6) {
        roots[1] = - A - a1 / 3.0;
        return 2;
    }
    return 1;
}

void MapInitializer::_essentialToDecompositions(const TMath::TMatrixd & essential)
{
    using namespace TMath;

    TMatrixd W(3, 3);
    W.setZero();
    W(0, 1) = 1.0;
    W(1, 0) = - 1.0;
    W(2, 2) = 1.0;

    m_svd.compute(essential);

    TMatrixd U = m_svd.U();
    if (TTools::matrix3x3Determinant(U) < 0.0)
        U *= -1.0;
    TMatrixd V = m_svd.V();
    if (TTools::matrix3x3Determinant(V) < 0.0)
        V *= -1.0;

    TMatrixd R1 = U * W * V.refTransposed();
    TMatrixd R2 = U * W.refTransposed() * V.refTransposed();

    TVectord c1 = U.getColumn(2);
    TVectord c2 = - c1;

    m_decompositions.push_back({ R1, - R1 * c1, 0.0, 0.0, TVectord::create(0.0, 0.0, 0.0) });
    m_decompositions.push_back({ R1, - R1 * c2, 0.0, 0.0, TVectord::create(0.0, 0.0, 0.0) });
    m_decompositions.push_back({ R2, - R2 * c1, 0.0, 0.0, TVectord::create(0.0, 0.0, 0.0) });
    m_decompositions.push_back({ R2, - R2 * c2, 0.0, 0.0, TVectord::create(0.0, 0.0, 0.0) });
}

void MapInitializer::_chooseBestDecomposition()
{
    using namespace TMath;

    //assert(m_decompositions.size() == 8);
    auto compareDecompositions = [] (const HomographyDecomposition & lhd,
                                     const HomographyDecomposition & rhd) -> bool
                                     { return (lhd.score < rhd.score); };
    double errorSquaredLimit = m_maxSquarePixelError * 2.0;
    for (std::size_t i = 0; i < m_decompositions.size(); ++i) {
        HomographyDecomposition& decomposition = m_decompositions[i];
        TMatrixd R = TTools::matrix3x3Inverted(decomposition.rotation);
        TVectord t = - R * decomposition.translation;

        decomposition.score = 0.0;
        for(std::size_t k = 0; k < m_inlinerIndices.size(); ++k) {
            const Match& match = m_matches[m_inlinerIndices[k]];
            TVectord v = TTools::triangulatePoint(R, t, match.camFirst, match.camSecond);
            if (v.size() != 3) {
                decomposition.score += errorSquaredLimit;
                continue;
            }
            if (v(2) > 1e-6) {
                decomposition.score += std::min((m_firstCamera->project(Point2d(v(0) / v(2), v(1) / v(2))) -
                                                    match.pixelFirst.cast<double>()).lengthSquared(),
                                                 errorSquaredLimit);
            } else {
                decomposition.score += errorSquaredLimit;
            }
            v = decomposition.rotation * v + decomposition.translation;
            if (v(2) > 1e-6) {
                decomposition.score += std::min((m_secondCamera->project(Point2d(v(0) / v(2), v(1) / v(2))) -
                                                    match.pixelSecond.cast<double>()).lengthSquared(),
                                                 errorSquaredLimit);
            } else {
                decomposition.score += errorSquaredLimit;
            }
        }
    }
    std::sort(m_decompositions.begin(), m_decompositions.end(), compareDecompositions);
}

bool MapInitializer::_computeTransformation()
{
    /*using namespace TMath;
    static const double eps = 1e-2;
    static const double sqrt_2 = std::sqrt(2.0);

    int countNulls = 0;
    _findBestEssential();
    Point2d T_firstCenter(0.0, 0.0), T_secondCenter(0.0, 0.0);
    double T_firstScale = 0.0, T_secondScale = 0.0;
    if ((int)m_inlinerIndices.size() >= m_minCountMapPoints) {

        std::size_t k;
        for (k = 0; k < m_inlinerIndices.size(); ++k) {
            const Match& match = m_matches[m_inlinerIndices[k]];

            T_firstCenter.x += match.camFirst(0);
            T_firstCenter.y += match.camFirst(1);
            T_secondCenter.x += match.camSecond(0);
            T_secondCenter.y += match.camSecond(1);
        }
        T_firstCenter /= (double)(m_inlinerIndices.size());
        T_secondCenter /= (double)(m_inlinerIndices.size());

        for (k = 0; k < m_inlinerIndices.size(); ++k) {
            const Match& match = m_matches[m_inlinerIndices[k]];

            T_firstScale += Point2d(match.camFirst(0) - T_firstCenter.x,
                                    match.camFirst(1) - T_firstCenter.y).length();
            T_secondScale += Point2d(match.camSecond(0) - T_secondCenter.x,
                                     match.camSecond(1) - T_secondCenter.y).length();
        }
        T_firstScale /= (double)(m_inlinerIndices.size());
        T_secondScale /= (double)(m_inlinerIndices.size());

        T_firstScale = sqrt_2 / T_firstScale;
        T_secondScale = sqrt_2 / T_secondScale;

        TMatrixd A((int)m_inlinerIndices.size(), 9);
        double* A_dataRow = A.firstDataRow();
        for (k = 0; k < m_inlinerIndices.size(); ++k) {
            const Match& match = m_matches[m_inlinerIndices[k]];

            const Point2d uv_first((match.camFirst(0) - T_firstCenter.x) * T_firstScale,
                                   (match.camFirst(1) - T_firstCenter.y) * T_firstScale);
            const Point2d uv_second((match.camSecond(0) - T_secondCenter.x) * T_secondScale,
                                    (match.camSecond(1) - T_secondCenter.y) * T_secondScale);

            A_dataRow[0] = uv_first.x * uv_second.x;
            A_dataRow[1] = uv_first.y * uv_second.x;
            A_dataRow[2] = uv_second.x;
            A_dataRow[3] = uv_first.x * uv_second.y;
            A_dataRow[4] = uv_first.y * uv_second.y;
            A_dataRow[5] = uv_second.y;
            A_dataRow[6] = uv_first.x;
            A_dataRow[7] = uv_first.y;
            A_dataRow[8] = 1.0;

            A_dataRow = &A_dataRow[9];
        }

        m_svd.compute(A, (A.rows() < 9));

        TVectord W = m_svd.diagonalW();
        double eps2 = std::fabs(W(8)) + eps;
        for (int i = 8; i >= 0; --i) {
            if (std::fabs(W(i)) < eps2)
                ++countNulls;
        }
    } else {
        countNulls = 3;
    }
    if ((countNulls == 0) || (countNulls > 3))
        return false;
    if (countNulls == 3) {*/
        _findBestHomography();
        if ((int)m_inlinerIndices.size() < m_minCountMapPoints)
            return false;
        m_bestHomography = _findHomography();
        if (!_computeDecompositionsOfHomography())
            return false;
        _chooseBestDecomposition();
        return true;/*
    }
    TMatrixd T_first(3, 3);
    T_first(0, 0) = T_firstScale;
    T_first(0, 1) = 0.0;
    T_first(0, 2) = - T_firstCenter.x * T_firstScale;
    T_first(1, 0) = 0.0;
    T_first(1, 1) = T_firstScale;
    T_first(1, 2) = - T_firstCenter.y * T_firstScale;
    T_first(2, 0) = 0.0;
    T_first(2, 1) = 0.0;
    T_first(2, 2) = 1.0;
    TMatrixd T_second(3, 3);
    T_second(0, 0) = T_secondScale;
    T_second(0, 1) = 0.0;
    T_second(0, 2) = - T_secondCenter.x * T_secondScale;
    T_second(1, 0) = 0.0;
    T_second(1, 1) = T_secondScale;
    T_second(1, 2) = - T_secondCenter.y * T_secondScale;
    T_second(2, 0) = 0.0;
    T_second(2, 1) = 0.0;
    T_second(2, 2) = 1.0;
    if (countNulls == 2) {
        TMatrixd F1(3, 3, m_svd.V_transposed().getDataRow(7));
        TMatrixd F2(3, 3, m_svd.V_transposed().getDataRow(8));
        TMatrixd essentials[3];
        int count = _computeEssentials(essentials, F1, F2);
        if (count <= 0)
            return false;
        for (int i = 0; i < count; ++i) {
            essentials[i] = T_second.refTransposed() * essentials[i] * T_first;
            essentials[i] = _fixEssentialMatrix(essentials[i]);
            _essentialToDecompositions(essentials[i]);
        }
    } else {
        m_bestEssential = TMatrixd(3, 3, m_svd.V_transposed().getDataRow(8));
        m_bestEssential = _fixEssentialMatrix(m_bestEssential);
        m_bestEssential = T_second.refTransposed() * m_bestEssential * T_first;
        _essentialToDecompositions(m_bestEssential);
    }
    _chooseBestDecomposition();
    return true;*/
}

}
