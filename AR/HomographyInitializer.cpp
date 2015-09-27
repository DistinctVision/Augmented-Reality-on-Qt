#include "HomographyInitializer.h"
#include "AR/OtherEstimators.h"
#include "AR/TWLS.h"
#include <float.h>
#include <QTime>
#include "QScrollEngine/QOtherMathFunctions.h"
#include <cassert>
#include <algorithm>
#include "AR/OF_ImageLevel.h"
#include "AR/OF_System.h"

namespace AR {

bool HomographyInitializer::compute(Pose3DEstimator& pose3DEstimator,
                                    const std::vector<IPointF>& firstPoints, const std::vector<IPointF>& secondPoints)
{
    _indicesInliners.clear();
    _indicesOutliners.clear();
    if (firstPoints.size() < 7)
        return false;
    _computeMatches(pose3DEstimator, firstPoints, secondPoints);
    if (!_findBestHomography())
        return false;
    if (_indicesInliners.size() < 4)
        return false;
    if (!_computeDecompositionsHomography())
        return false;
    _chooseBestDecomposition();
    _getResultMotionMatrix();
    return true;
}

void HomographyInitializer::_getResultMotionMatrix()
{
    assert(_decompositions.size() == 1);
    HomographyDecomposition& decomposition = _decompositions[0];
    int i, j;
    for (i=0; i<3; ++i) {
        for (j=0; j<3; ++j)
            _motionMatrix(i, j) = decomposition.rotation(i, j);
        _motionMatrix(i, 3) = decomposition.translate(i);
    }
    _motionMatrix(3, 0) = 0.0f;
    _motionMatrix(3, 1) = 0.0f;
    _motionMatrix(3, 2) = 0.0f;
    _motionMatrix(3, 3) = 1.0f;
}

void HomographyInitializer::_computeMatches(const Pose3DEstimator& pose3DEstimator,
                                            const std::vector<IPointF>& first, const std::vector<IPointF>& second)
{
    _matches.resize(std::min(first.size(), second.size()));
    for (unsigned int i=0; i<_matches.size(); ++i) {
        _matches[i].first = pose3DEstimator.unprojectPoint(first[i]);
        _matches[i].second = pose3DEstimator.unprojectPoint(second[i]);
    }
}

float HomographyInitializer::_getErrorSquared(const TMath::TMatrix<float>& homography, const Match& match) const
{
    const float scaled = 100.0f;
    TMath::TVector<float> v = homography * TMath::TVector<float>(match.first.x, match.first.y, -1.0f);
    IPointF estimatedSecond(v(0) / v(2), v(1) / v(2));
    return (estimatedSecond + match.second).lengthSquared() * scaled;
}

TMath::TMatrix<float> HomographyInitializer::_findHomography()
{
    assert(_indicesInliners.size() >= 4);
    TMath::TMatrix<float> M(std::max(_indicesInliners.size() * 2, static_cast<unsigned int>(9)), 9);
    float* dataRow = M.firstDataRow();
    for (unsigned int i=0; i<_indicesInliners.size(); ++i) {
        const Match& match = _matches[_indicesInliners[i]];

        dataRow[0] = match.first.x;
        dataRow[1] = match.first.y;
        dataRow[2] = - 1.0f;
        dataRow[3] = 0.0f;
        dataRow[4] = 0.0f;
        dataRow[5] = 0.0f;
        dataRow[6] = match.first.x * match.second.x;
        dataRow[7] = match.first.y * match.second.x;
        dataRow[8] = - match.second.x;
        dataRow = &dataRow[9];

        dataRow[0] = 0.0f;
        dataRow[1] = 0.0f;
        dataRow[2] = 0.0f;
        dataRow[3] = match.first.x;
        dataRow[4] = match.first.y;
        dataRow[5] = - 1.0f;
        dataRow[6] = match.first.x * match.second.y;
        dataRow[7] = match.first.y * match.second.y;
        dataRow[8] = - match.second.y;
        dataRow = &dataRow[9];
    }

    if (_indicesInliners.size() == 4) {
        for (int i=0; i<9; ++i)
            dataRow[i] = 0.0f;
    }

    _svd.compute(M, false);
    const float* constDataRow = _svd.V_transposed().getDataRow(8);
    TMath::TMatrix<float> homography(3, 3);
    homography.copyDataFrom(constDataRow);
    return homography;
}

bool HomographyInitializer::_findBestHomography()
{
    float bestScore = FLT_MAX;
    _indicesInliners.resize(4);
    unsigned int i, j;
    for (int iter=0; iter<_countRetries; ++iter) {
        if ((iter % 7) == 0)
            std::srand(QTime::currentTime().msec());
        for (i=0; i<4; ++i) {
            bool isUnique;
            for(;;) {
                isUnique = true;
                _indicesInliners[i] = std::rand() % _matches.size();
                for (j=0; j<i; ++j)
                    if (_indicesInliners[j] == _indicesInliners[i]) {
                        isUnique = false;
                        break;
                    }
                if (isUnique)
                    break;
            }
        }
        TMath::TMatrix<float> homography = _findHomography();
        float score = 0.0f;
        for (i=0; i<_matches.size(); ++i)
            score += std::min(_getErrorSquared(homography, _matches[i]), _maxErrorSquaredA);
        if (score < bestScore) {
            bestScore = score;
            _bestHomography = homography;
        }
    }
    _indicesInliners.clear();
    if (_bestHomography.rows() == 0)
        return false;
    for (i=0; i<_matches.size(); ++i) {
        if (_getErrorSquared(_bestHomography, _matches[i]) <= _maxErrorSquaredB)
            _indicesInliners.push_back(i);
        else
            _indicesOutliners.push_back(i);
    }
    if (_indicesInliners.size() < 4)
        return false;
    _bestHomography = _findHomography();
    return true;
}

bool HomographyInitializer::_computeDecompositionsHomography()
{
    _svd.compute(_bestHomography);
    TMath::TVector<float> W = _svd.W();
    float w1 = std::fabs(W(0)); // The paper suggests the square of these (e.g. the evalues of AAT)
    float w2 = std::fabs(W(1)); // should be used, but this is wrong. c.f. Faugeras' book.
    float w3 = std::fabs(W(2));

    TMath::TMatrix<float> U = _svd.U();
    TMath::TMatrix<float> V = _svd.V();

    float s = TMath::Tools<float>::matrix3x3Determinant(U) * TMath::Tools<float>::matrix3x3Determinant(V);

    float prime_PM = w2;

    if((w1 == w2) || (w2 == w3))
        return false;//This motion case is not implemented or is degenerate.

    float x1_PM;
    float x2;
    float x3_PM;

    // All below deals with (w1 != w2 &&  w2 != w3)
    {
        x1_PM = std::sqrt((w1*w1 - w2*w2) / (w1*w1 - w3*w3));
        x2    = 0.0f;
        x3_PM = std::sqrt((w2*w2 - w3*w3) / (w1*w1 - w3*w3));
    };

    const float e1[4] = {1.0f, -1.0f,  1.0f, -1.0f};
    const float e2[4] = {1.0f,  1.0f, -1.0f, -1.0f};

    _decompositions.resize(8);
    TMath::TVector<float> np(3), tp(3);

    // Case 1, d' > 0:
    float D = s * prime_PM;
    for(int signs=0; signs<4; ++signs) {
        HomographyDecomposition& decomposition = _decompositions[signs];

        decomposition.D = D;
        float sinTheta = (w1 - w3) * x1_PM * x3_PM * e1[signs] * e2[signs] / w2;
        float cosTheta = (w1 * x3_PM * x3_PM + w3 * x1_PM * x1_PM) / w2;
        decomposition.rotation.recreate(3, 3);
        decomposition.rotation.setToIdentity();
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
        decomposition.translate = U * tp;
    }
    // Case 1, d' < 0:
    D = - s * prime_PM;
    for(int signs=0; signs<4; signs++) {
        HomographyDecomposition& decomposition = _decompositions[signs + 4];

        decomposition.D = D;
        float sinPhi = (w1 + w3) * x1_PM * x3_PM * e1[signs] * e2[signs] / w2;
        float cosPhi = (w3 * x1_PM * x1_PM - w1 * x3_PM * x3_PM) / w2;
        decomposition.rotation.recreate(3, 3);
        decomposition.rotation.setToIdentity();
        decomposition.rotation *= -1.0f;
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
        decomposition.translate = U * tp;
    }
    return true;
}

float HomographyInitializer::_sampsonusError(const TMath::TVector<float>& vSecond, const TMath::TMatrix<float>& mEssential,
                                             const TMath::TVector<float>& vFirst) const
{
    float dError = TMath::dot(vSecond, mEssential * vFirst);

    TMath::TVector<float> fv3 = mEssential * vFirst;
    TMath::TVector<float> fTv3Dash = mEssential.refTransposed() * vSecond;

    return ((dError * dError) / (fv3(0) * fv3(0) + fv3(1) * fv3(1) + fTv3Dash(0) * fTv3Dash(0) + fTv3Dash(1) * fTv3Dash(1)));
}

void HomographyInitializer::_chooseBestDecomposition()
{
    assert(_decompositions.size() == 8);
    auto compareDecompositions = [] (const HomographyDecomposition& lhd, const HomographyDecomposition& rhd) -> bool
                            { return (lhd.score < rhd.score); };
    /*unsigned int i, k;
    for(i=0; i<_decompositions.size(); ++i) {
        HomographyDecomposition& decomposition = _decompositions[i];
        int nPositive = 0;
        for(k=0; k<_indicesInliners.size(); k++) {
          IPointF& first = _matches[_indicesInliners[k]].first;
          float visibilityTest = (_bestHomography(2, 0) * first.x + _bestHomography(2, 1) * first.y + _bestHomography(2, 2))
                  / decomposition.D;
          if(visibilityTest > 0.0f)
              nPositive++;
        }
        decomposition.score = - nPositive;
    }


    std::sort(_decompositions.begin(), _decompositions.end(), compareDecompositions);
    _decompositions.resize(4);

    for(i=0; i<_decompositions.size(); ++i) {
        HomographyDecomposition& decomposition = _decompositions[i];
        int nPositive = 0;
        for(k=0; k<_indicesInliners.size(); ++k) {
            IPointF& first = _matches[_indicesInliners[k]].first;
            TMath::TVector<float> v(first.x, first.y, 1.0f);
            float visibilityTest = - TMath::dot(v,  decomposition.n) / decomposition.D;
            if(visibilityTest > 0.0f)
                nPositive++;
        }
        decomposition.score = - nPositive;
    }

    sort(_decompositions.begin(), _decompositions.end(), compareDecompositions);
    _decompositions.resize(2);

    // According to Faugeras and Lustman, ambiguity exists if the two scores are equal
    // but in practive, better to look at the ratio!
    float dRatio = _decompositions[1].score / static_cast<float>(_decompositions[0].score);

    if(dRatio < 0.9f) { // no ambiguity!
        _decompositions.erase(_decompositions.begin() + 1);
    } else {             // two-way ambiguity. Resolve by sampsonus score of all points.
        float errorSquaredLimit  = _maxErrorSquared * 4.0f;
          float adSampsonusScores[2];
          for(i=0; i<2; i++) {
              HomographyDecomposition& decomposition = _decompositions[i];
              TMath::TMatrix<float> mEssential(3, 3);
              for(int j=0; j<3; j++)
                  mEssential.setColumn(j, TMath::cross(decomposition.translate, decomposition.rotation.getColumn(j)));

              float sumError = 0.0f;
              for(k=0; k < _matches.size(); ++k) {
                  Match& match = _matches[k];
                  float sError = _sampsonusError(TMath::TVector<float>(match.second.x, match.second.y, 1.0f), mEssential,
                                            TMath::TVector<float>(match.first.x, match.first.y, 1.0f));
                  if(sError > errorSquaredLimit)
                      sError = errorSquaredLimit;
                  sumError += sError;
              }
              adSampsonusScores[i] = sumError;
          }

          if(adSampsonusScores[0] <= adSampsonusScores[1])
              _decompositions.erase(_decompositions.begin() + 1);
          else
              _decompositions.erase(_decompositions.begin());
      }*/

    unsigned int i, k;
    for(i=0; i<_decompositions.size(); ++i) {
        HomographyDecomposition& decomposition = _decompositions[i];
        decomposition.score = 0;
        for (k=0; k<_matches.size(); ++k) {
            Match& match = _matches[k];
            TMath::TVector<float> dirA(- match.first.x, - match.first.y, - 1.0f);//planeLocalX
            TMath::TVector<float> dirB = decomposition.rotation * TMath::TVector<float>(- match.second.x, - match.second.y, - 1.0f);
            TMath::TVector<float> planeLocalZ = TMath::cross3(dirA, dirB);
            TMath::TVector<float> planeLocalY = TMath::cross3(planeLocalZ, dirA);
            float tA, tB;
            QVector2D planeDirB(TMath::dot(dirB, dirA), TMath::dot(dirB, planeLocalY));
            QVector2D planePointB(TMath::dot(decomposition.translate, dirA), TMath::dot(decomposition.translate, planeLocalY));
            QVector2D r;
            if (QScrollEngine::QOtherMathFunctions::collisionLines(r, tA, tB, QVector2D(0.0f, 0.0f), QVector2D(1.0f, 0.0f),
                                                                                        planePointB, planeDirB)) {
                if (tA > 0.0f)
                    decomposition.score -= 1.0f;
                if (tB > 0.0f)
                    decomposition.score -= 1.0f;
            }
        }
    }

    std::sort(_decompositions.begin(), _decompositions.end(), compareDecompositions);
    _decompositions.resize(2);

    // According to Faugeras and Lustman, ambiguity exists if the two scores are equal
    // but in practive, better to look at the ratio!
    float dRatio = _decompositions[1].score / static_cast<float>(_decompositions[0].score);
    if(dRatio < 0.9f) { // no ambiguity!
        _decompositions.erase(_decompositions.begin() + 1);
    } else {             // two-way ambiguity. Resolve by sampsonus score of all points.
        float sumError[2];
        for (i=0; i<2; ++i) {
            sumError[i] = 0.0f;
            HomographyDecomposition& decomposition = _decompositions[i];
            for (k=0; k<_matches.size(); ++k) {
                Match& match = _matches[k];
                TMath::TVector<float> dirA(- match.first.x, - match.first.y, - 1.0f);
                TMath::TVector<float> dirB = decomposition.rotation * TMath::TVector<float>(- match.second.x, - match.second.y, - 1.0f);
                sumError[i] += std::fabs(TMath::dot(decomposition.translate, TMath::cross3(dirA, dirB).normalized()));
            }
        }
        if(sumError[0] <= sumError[1])
            _decompositions.erase(_decompositions.begin() + 1);
        else
            _decompositions.erase(_decompositions.begin());
    }

    _decompositions[0].translate *= _scaleMap / _decompositions[0].translate.length();
}

bool HomographyInitializer::initializeMap(Pose3DEstimator& pose3DEstimator, Map& map,
                                          const OF_System& opticalFlow, const IPoint& sizeImageOfPoint)
{
    assert(map.countKeyFrames() >= 2);
    assert(map.keyFrame(0).projectedPoints.size() == map.keyFrame(1).projectedPoints.size());
    compute(pose3DEstimator, map.keyFrame(0).projectedPoints, map.keyFrame(1).projectedPoints);
    if (_indicesInliners.size() < 4)
        return false;
    QMatrix4x4 firstMatrix;
    firstMatrix.setToIdentity();
    QMatrix4x4 secondMatrix = _motionMatrix * firstMatrix;
    map.keyFrame(0).matrixWorld = firstMatrix;
    map.keyFrame(1).matrixWorld = secondMatrix;
    QVector3D cameraSecondPosition;
    cameraSecondPosition.setX(- (secondMatrix(0, 0) * secondMatrix(0, 3) +
                              secondMatrix(1, 0) * secondMatrix(1, 3) +
                              secondMatrix(2, 0) * secondMatrix(2, 3)));
    cameraSecondPosition.setY(- (secondMatrix(0, 1) * secondMatrix(0, 3) +
                              secondMatrix(1, 1) * secondMatrix(1, 3) +
                              secondMatrix(2, 1) * secondMatrix(2, 3)));
    cameraSecondPosition.setZ(- (secondMatrix(0, 2) * secondMatrix(0, 3) +
                              secondMatrix(1, 2) * secondMatrix(1, 3) +
                              secondMatrix(2, 2) * secondMatrix(2, 3)));
    std::vector<IPointF>& firstPoints = map.keyFrame(0).projectedPoints;
    std::vector<int>& indicesFirstPoints = map.keyFrame(0).indicesMapPoints;
    indicesFirstPoints.resize(firstPoints.size());
    std::vector<IPointF>& secondPoints = map.keyFrame(1).projectedPoints;
    std::vector<int>& indicesSecondPoints = map.keyFrame(1).indicesMapPoints;
    indicesSecondPoints.resize(secondPoints.size());
    std::vector<int> posibleIndices;
    TMath::TMatrix<float> matrix(3, 3);
    matrix.setZero();
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    QVector3D result;
    std::vector<Pose3DEstimator::ProjectionPoint> projections;
    projections.resize(2);
    projections[0].matrixWorld = firstMatrix;
    projections[1].matrixWorld = secondMatrix;
    unsigned int i;
    float sumWeight = 0.0f;
    for (i=0; i<_indicesInliners.size(); ++i) {
        int index = _indicesInliners[i];
        projections[0].projection = firstPoints[index];
        projections[1].projection = secondPoints[index];
        if (pose3DEstimator.getPointInWorld(result, projections)) {
            Image<OF_ImageLevel::OpticalFlowInfo2D> imageOfPoint(sizeImageOfPoint);
            if (opticalFlow.getSubOFImage(imageOfPoint, secondPoints[index])) {
                int indexMapPoint = map.addMapPoint(result, imageOfPoint,
                                                    pose3DEstimator.getSumErrorSquared(result, projections), 2,
                                                    pose3DEstimator.getDirFromScreen(secondMatrix, projections[1].projection));
                indicesFirstPoints[index] = indicesSecondPoints[index] = indexMapPoint;
                map._mapPoints[indexMapPoint].indexKeyFrame.push_back(0);
                map._mapPoints[indexMapPoint].indexKeyFrame.push_back(1);
                sum_x += result.x();
                sum_y += result.y();
                sum_z += result.z();
                matrix(0, 0) += result.x() * result.x();
                matrix(0, 1) += result.x() * result.y();
                matrix(0, 2) += result.x() * result.z();
                matrix(1, 0) += result.y() * result.x();
                matrix(1, 1) += result.y() * result.y();
                matrix(1, 2) += result.y() * result.z();
                matrix(2, 0) += result.z() * result.x();
                matrix(2, 1) += result.z() * result.y();
                matrix(2, 2) += result.z() * result.z();
                sumWeight += 1.0f;
                posibleIndices.push_back(index);
                continue;
            }
        }
        _indicesInliners.erase(_indicesInliners.begin() + i);
        --i;
    }
    const float sum_x_x = (sum_x * sum_x) / sumWeight;
    const float sum_x_y = (sum_x * sum_y) / sumWeight;
    const float sum_x_z = (sum_x * sum_z) / sumWeight;
    const float sum_y_y = (sum_y * sum_y) / sumWeight;
    const float sum_y_z = (sum_y * sum_z) / sumWeight;
    const float sum_z_z = (sum_z * sum_z) / sumWeight;
    matrix(0, 0) -= sum_x_x; matrix(0, 1) -= sum_x_y; matrix(0, 2) -= sum_x_z;
    matrix(1, 0) -= sum_x_y; matrix(1, 1) -= sum_y_y; matrix(1, 2) -= sum_y_z;
    matrix(2, 0) -= sum_x_z; matrix(2, 1) -= sum_y_z; matrix(2, 2) -= sum_z_z;
    int indexRow[3];
    indexRow[0] = 0; indexRow[1] = 1; indexRow[2] = 2;
    for (i=1; i<3; ++i)
        if (std::fabs(matrix(indexRow[0], 0)) < std::fabs(matrix(indexRow[i], 0))) {
            std::swap(indexRow[0], indexRow[i]);
        }
    if (std::fabs(matrix(indexRow[0], 0)) < FLT_EPSILON) {
        return false;
    }
    float k = matrix(indexRow[1], 0) / matrix(indexRow[0], 0);
    matrix(indexRow[1], 1) -= k * matrix(indexRow[0], 1);
    matrix(indexRow[1], 2) -= k * matrix(indexRow[0], 2);
    k = matrix(indexRow[2], 0) / matrix(indexRow[0], 0);
    matrix(indexRow[2], 1) -= k * matrix(indexRow[0], 1);
    matrix(indexRow[2], 2) -= k * matrix(indexRow[0], 2);
    if (std::fabs(matrix(indexRow[1], 1)) < std::fabs(matrix(indexRow[2], 1)))
        std::swap(indexRow[1], indexRow[2]);
    if (std::fabs(matrix(indexRow[1], 1)) < FLT_EPSILON)
        return false;
    k = matrix(indexRow[2], 1) / matrix(indexRow[1], 1);
    matrix(indexRow[2], 2) -= k * matrix(indexRow[1], 2);
    if (std::fabs(matrix(indexRow[2], 2)) > 0.4f)
        return false;
    QVector3D planePos, planeLocalX, planeLocalY, planeLocalZ;
    planePos.setX(sum_x / sumWeight);
    planePos.setY(sum_y / sumWeight);
    planePos.setZ(sum_z / sumWeight);
    planeLocalZ[indexRow[2]] = 1.0f;
    planeLocalZ[indexRow[1]] = - matrix(indexRow[1], 2) / matrix(indexRow[1], 1);
    planeLocalZ[indexRow[0]] = - (matrix(indexRow[0], 2) + matrix(indexRow[0], 1) * planeLocalZ[indexRow[1]]) / matrix(indexRow[0], 0);
    planeLocalZ.normalize();
    if (QVector3D::dotProduct(cameraSecondPosition - planePos, planeLocalZ) < 0.0f) {
        planeLocalZ = - planeLocalZ;
    }
    planeLocalX = QVector3D::crossProduct(QVector3D(0.0f, 1.0f, 0.0f), planeLocalZ);
    if (planeLocalX.lengthSquared() < 0.0005f) {
        planeLocalY = QVector3D::crossProduct(planeLocalZ, QVector3D(1.0f, 0.0f, 0.0f));
        planeLocalY.normalize();
        planeLocalX = QVector3D::crossProduct(planeLocalY, planeLocalZ);
    } else {
        planeLocalX.normalize();
        planeLocalY = QVector3D::crossProduct(planeLocalZ, planeLocalX);
    }
    QMatrix4x4 planeMatrix;
    planeMatrix(0, 0) = planeLocalX.x(); planeMatrix(0, 1) = planeLocalY.x(); planeMatrix(0, 2) = planeLocalZ.x();
    planeMatrix(0, 3) = planePos.x();
    planeMatrix(1, 0) = planeLocalX.y(); planeMatrix(1, 1) = planeLocalY.y(); planeMatrix(1, 2) = planeLocalZ.y();
    planeMatrix(1, 3) = planePos.y();
    planeMatrix(2, 0) = planeLocalX.z(); planeMatrix(2, 1) = planeLocalY.z(); planeMatrix(2, 2) = planeLocalZ.z();
    planeMatrix(2, 3) = planePos.z();
    planeMatrix(3, 0) = 0.0f; planeMatrix(3, 1) = 0.0f; planeMatrix(3, 2) = 0.0f; planeMatrix(3, 3) = 1.0f;
    QMatrix4x4 invPlaneMatrix = QScrollEngine::QOtherMathFunctions::invertedWorldMatrix(planeMatrix);
    map.keyFrame(0).matrixWorld *= planeMatrix;
    map.keyFrame(1).matrixWorld *= planeMatrix;
    for (i=0; i<_indicesInliners.size(); ++i) {
        int index = _indicesInliners[i];
        map.transformMapPoint(indicesFirstPoints[index], invPlaneMatrix);
    }
    float invScaled = _scaleMap / (cameraSecondPosition - planePos).length();
    map.scaleMap(invScaled);
    projections[0].matrixWorld = map.keyFrame(0).matrixWorld;
    projections[1].matrixWorld = map.keyFrame(1).matrixWorld;
    float sumErrorSquared;
    for (i=0; i<_indicesOutliners.size(); ++i) {
        break;//Don't create other points
        int index = _indicesOutliners[i];
        projections[0].projection = firstPoints[index];
        projections[1].projection = secondPoints[index];
        if (pose3DEstimator.getPointInWorld(result, projections)) {
            sumErrorSquared = pose3DEstimator.getSumErrorSquared(result, projections);
            if ((sumErrorSquared * 0.5f) < _maxErrorSquaredB) {
                Image<OF_ImageLevel::OpticalFlowInfo2D> imageOfPoint(sizeImageOfPoint);
                if (opticalFlow.getSubOFImage(imageOfPoint, secondPoints[index])) {
                    int indexMapPoint = map.addMapPoint(result, imageOfPoint,
                                                        pose3DEstimator.getSumErrorSquared(result, projections), 2,
                                                        pose3DEstimator.getDirFromScreen(secondMatrix, projections[1].projection));
                    indicesFirstPoints[index] = indicesSecondPoints[index] = indexMapPoint;
                    map._mapPoints[indexMapPoint].indexKeyFrame.push_back(0);
                    map._mapPoints[indexMapPoint].indexKeyFrame.push_back(1);
                    posibleIndices.push_back(index);
                }
            }
        }
    }
    std::sort(posibleIndices.begin(), posibleIndices.end());
    Map::KeyFrame& firstKeyFrame = map.keyFrame(0);
    Map::KeyFrame& secondKeyFrame = map.keyFrame(1);
    int ci = 0;
    for (i=0; i<posibleIndices.size(); ++i) {
        int index = posibleIndices[i];
        firstKeyFrame.projectedPoints[ci] = firstKeyFrame.projectedPoints[index];
        firstKeyFrame.indicesMapPoints[ci] = firstKeyFrame.indicesMapPoints[index];
        secondKeyFrame.projectedPoints[ci] = secondKeyFrame.projectedPoints[index];
        secondKeyFrame.indicesMapPoints[ci] = secondKeyFrame.indicesMapPoints[index];
        ++ci;
    }
    firstKeyFrame.projectedPoints.resize(ci);
    firstKeyFrame.indicesMapPoints.resize(ci);
    secondKeyFrame.projectedPoints.resize(ci);
    secondKeyFrame.indicesMapPoints.resize(ci);
    return true;
}

}
