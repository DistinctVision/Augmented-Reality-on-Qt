#include "AR/Pose3DEstimator.h"
#include <QVector2D>
#include <QMatrix2x2>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QDebug>

namespace AR {

Pose3DEstimator::Pose3DEstimator():
    _wls(6), _transform(4, 4)

{
    _stop_epsilon = 0.00005f;
    _distortion_P[0] = 0.0f;
    _distortion_P[1] = 0.0f;
    _distortion_K[0] = 0.0f;
    _distortion_K[1] = 0.0f;
    QMatrix4x4 proj;
    proj.perspective(30.0f, 640.0f / 480.0f, 1.0f, 100.0f);
    IPointF focalLength, opticalCenter;
    getCameraParameters(focalLength, opticalCenter, proj);
    setCameraParameters(IPoint(640, 480), focalLength, opticalCenter, false, false);
    resetBeginTransform();
}

Pose3DEstimator::~Pose3DEstimator()
{
    _pointsUV.clear();
    _points.clear();
}

bool Pose3DEstimator::getPointInWorld(QVector3D& result, const std::vector<ProjectionPoint>& projections)
{
    TMath_assert(projections.size() >= 2);
    unsigned int i;
    TMath::TMatrix<float> M(projections.size() * 2, 4);
    float* dataRow = M.firstDataRow();
    for (i=0; i<projections.size(); ++i) {
        const ProjectionPoint& p = projections[i];
        IPointF point = unprojectPoint(p.projection);
        dataRow[0] = p.matrixWorld(0, 0) + point.x * p.matrixWorld(2, 0);
        dataRow[1] = p.matrixWorld(0, 1) + point.x * p.matrixWorld(2, 1);
        dataRow[2] = p.matrixWorld(0, 2) + point.x * p.matrixWorld(2, 2);
        dataRow[3] = p.matrixWorld(0, 3) + point.x * p.matrixWorld(2, 3);
        dataRow = &dataRow[4];

        dataRow[0] = p.matrixWorld(1, 0) + point.y * p.matrixWorld(2, 0);
        dataRow[1] = p.matrixWorld(1, 1) + point.y * p.matrixWorld(2, 1);
        dataRow[2] = p.matrixWorld(1, 2) + point.y * p.matrixWorld(2, 2);
        dataRow[3] = p.matrixWorld(1, 3) + point.y * p.matrixWorld(2, 3);
        dataRow = &dataRow[4];
    }
    _svd.compute(M, false);
    if (qAbs(_svd.W(3)) > 1.0f)
        return false;
    const float* resultData = _svd.V_transposed().getDataRow(3);
    if (qAbs(resultData[3]) < 0.0001f)
        return false;
    result.setX(resultData[0] / resultData[3]);
    result.setY(resultData[1] / resultData[3]);
    result.setZ(resultData[2] / resultData[3]);
    return true;
}

float Pose3DEstimator::getErrorSquared(const QMatrix4x4& matrixWorld, const QVector3D& worldPoint, const IPointF& projection) const
{
    const float scaled = 100.0f;
    float x = worldPoint.x() * matrixWorld(0, 0) + worldPoint.y() * matrixWorld(0, 1) +
               worldPoint.z() * matrixWorld(0, 2) + matrixWorld(0, 3);
    float y = worldPoint.x() * matrixWorld(1, 0) + worldPoint.y() * matrixWorld(1, 1) +
               worldPoint.z() * matrixWorld(1, 2) + matrixWorld(1, 3);
    float z = worldPoint.x() * matrixWorld(2, 0) + worldPoint.y() * matrixWorld(2, 1) +
               worldPoint.z() * matrixWorld(2, 2) + matrixWorld(2, 3);
    IPointF errorVector = unprojectPoint(projection);
    errorVector.x += x / z;
    errorVector.y += y / z;
    return errorVector.lengthSquared() * scaled;
}

float Pose3DEstimator::getSumErrorSquared(const QVector3D& worldPoint, const std::vector<ProjectionPoint>& projections) const
{
    float sumErrorSquared = 0.0f;
    for (unsigned int i=0; i<projections.size(); ++i) {
        sumErrorSquared += getErrorSquared(projections[i].matrixWorld, worldPoint, projections[i].projection);
    }
    return sumErrorSquared;
}

QVector3D Pose3DEstimator::getDirFromScreen(const QMatrix4x4& matrixWorld, const IPointF& point) const
{
    IPointF p = unprojectPoint(point);
    QVector3D local(- p.x, - p.y, - 1.0f);
    return QVector3D(local.x() * matrixWorld(0, 0) + local.y() * matrixWorld(1, 0) + local.z() * matrixWorld(2, 0),
                     local.x() * matrixWorld(0, 1) + local.y() * matrixWorld(1, 1) + local.z() * matrixWorld(2, 1),
                     local.x() * matrixWorld(0, 2) + local.y() * matrixWorld(1, 2) + local.z() * matrixWorld(2, 2));
}

void Pose3DEstimator::getResultMatrixWorld(QMatrix4x4& matrixWorld) const
{
    int i, j;
    for (i=0; i<4; ++i)
        for (j=0; j<4; ++j)
            matrixWorld(i, j) = _transform(i, j);
}

void Pose3DEstimator::setBeginTransform(const QMatrix4x4& matrixWorld)
{
    int i, j;
    for (i=0; i<4; ++i)
        for (j=0; j<4; ++j)
            _transform(i, j) = matrixWorld(i, j);
}

float Pose3DEstimator::getCurrentErrorSquared(int indexPoint) const
{
    const float scaled = 100.0f;
    return (_pointsUV[indexPoint] + _projectToUV(_transformPoint(_points[indexPoint]))).lengthSquared() * scaled;
}

void Pose3DEstimator::resetBeginTransform()
{
    _transform.setToIdentity();
    _transform(2, 3) = - 10.0f;
}

void Pose3DEstimator::beginEstimatePose()
{
    _pointsUV.resize(0);
    _points.resize(0);
}

void Pose3DEstimator::estimatePose()
{
    /*_getPlaneCoords();
    if (_isCoplanar)
        _computeFirstPoseForCoplanarPoints();
    else
        _computeFirstPoseForNoncoplanarPoints();*/
    _refinePose();
}

void Pose3DEstimator::addPoint(const QVector3D& worldPoint, const IPointF& imagePoint, float weight)
{
    _points.push_back(QVector4D(worldPoint, weight));
    _pointsUV.push_back(unprojectPoint(imagePoint));
}

void Pose3DEstimator::_refinePose()
{
    using namespace TMath;
    TMath_assert(_points.size() >= 6);
    unsigned int i;
    TVector<float> JX(6), JY(6);
    for (int iter = 0; iter < 30; ++iter) {
        _wls.clear();
        for (i=0; i<_points.size(); ++i) {
            QVector3D tPoint = _transformPoint(_points[i]);
            float z_inv = 1.0f / tPoint.z();
            float z_inv_square = z_inv * z_inv;

            JX(0) = - z_inv;                        // -1/z
            JX(1) = 0.0f;                           // 0
            JX(2) = tPoint.x() * z_inv_square;      // x/z^2
            JX(3) = tPoint.y() * JX(2);             // x*y/z^2
            JX(4) = - (1.0f + tPoint.x() * JX(2));  // -(1.0 + x^2/z^2)
            JX(5) = tPoint.y() * z_inv;             // y/z

            JY(0) = 0.0f;                           // 0
            JY(1) = - z_inv;                        // -1/z
            JY(2) = tPoint.y() * z_inv_square;      // y/z^2
            JY(3) = 1.0f + tPoint.y() * JY(2);      // 1.0 + y^2/z^2
            JY(4) = - JX(3);                        // -x*y/z^2
            JY(5) = - tPoint.x() * z_inv;           // -x/z
            IPointF errorVector = _pointsUV[i] + _projectToUV(tPoint);
            _wls.add_mJ(errorVector.x, JX, _points[i].w());
            _wls.add_mJ(errorVector.y, JY, _points[i].w());
        }
        _wls.compute();
        TMatrix<float> dT = Tools<float>::exp_transformMatrix(_wls.mu());
        _transform = dT * _transform;
        bool success = true;
        for (i=0; i<3; ++i) {
            if (dT(i, 3) > _stop_epsilon) {
                success = false;
                break;
            }
        }
        if (success)
            break;
    }
}

void Pose3DEstimator::_getPlaneCoords()
{
    float sumWeight = 0.0f;
    TMath::TMatrix<float> matrix(3, 3);
    matrix.setZero();
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    unsigned int i;
    for (i=0; i<_points.size(); ++i) {
        QVector4D& point = _points[i];
        QVector3D weightPoint(point.x() * point.w(), point.y() * point.w(), point.z() * point.w());
        sum_x += weightPoint.x();
        sum_y += weightPoint.y();
        sum_z += weightPoint.z();
        matrix(0, 0) += weightPoint.x() * point.x();
        matrix(0, 1) += weightPoint.x() * point.y();
        matrix(0, 2) += weightPoint.x() * point.z();
        matrix(1, 0) += weightPoint.y() * point.x();
        matrix(1, 1) += weightPoint.y() * point.y();
        matrix(1, 2) += weightPoint.y() * point.z();
        matrix(2, 0) += weightPoint.z() * point.x();
        matrix(2, 1) += weightPoint.z() * point.y();
        matrix(2, 2) += weightPoint.z() * point.z();
        sumWeight += point.w();
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
    unsigned int indexRow[3];
    indexRow[0] = 0; indexRow[1] = 1; indexRow[2] = 2;
    for (i=1; i<3; ++i)
        if (std::fabs(matrix(indexRow[0], 0)) < std::fabs(matrix(indexRow[i], 0))) {
            std::swap(indexRow[0], indexRow[i]);
        }
    if (std::fabs(matrix(indexRow[0], 0)) < FLT_EPSILON) {
        _isCoplanar = false;
        return;
    }
    float k = matrix(indexRow[1], 0) / matrix(indexRow[0], 0);
    matrix(indexRow[1], 1) -= k * matrix(indexRow[0], 1);
    matrix(indexRow[1], 2) -= k * matrix(indexRow[0], 2);
    k = matrix(indexRow[2], 0) / matrix(indexRow[0], 0);
    matrix(indexRow[2], 1) -= k * matrix(indexRow[0], 1);
    matrix(indexRow[2], 2) -= k * matrix(indexRow[0], 2);
    if (std::fabs(matrix(indexRow[1], 1)) < std::fabs(matrix(indexRow[2], 1)))
        std::swap(indexRow[1], indexRow[2]);
    if (std::fabs(matrix(indexRow[1], 1)) < FLT_EPSILON) {
        _isCoplanar = false;
        return;
    }
    k = matrix(indexRow[2], 1) / matrix(indexRow[1], 1);
    matrix(indexRow[2], 2) -= k * matrix(indexRow[1], 2);
    _isCoplanar = (std::fabs(matrix(indexRow[2], 2)) < 0.2f);
    if (!_isCoplanar)
        return;
    _planePos.setX(sum_x / sumWeight);
    _planePos.setY(sum_y / sumWeight);
    _planePos.setZ(sum_z / sumWeight);
    _planeLocalZ[indexRow[2]] = 1.0f;
    _planeLocalZ[indexRow[1]] = - matrix(indexRow[1], 2) / matrix(indexRow[1], 1);
    _planeLocalZ[indexRow[0]] = - (matrix(indexRow[0], 2) + matrix(indexRow[0], 1) * _planeLocalZ[indexRow[1]]) / matrix(indexRow[0], 0);
    _planeLocalZ.normalize();
    TMath::TVector<float> prevWorldPos = _transform.getColumn(3);
    QVector3D prevCamPos(TMath::dot(prevWorldPos, _transform.getColumn(0)),
                         TMath::dot(prevWorldPos, _transform.getColumn(1)),
                         TMath::dot(prevWorldPos, _transform.getColumn(2)));
    if (QVector3D::dotProduct(prevCamPos - _planePos, _planeLocalZ) < 0.0f)
        _planeLocalZ = - _planeLocalZ;
    _planeLocalX = QVector3D::crossProduct(QVector3D(0.0f, 1.0f, 0.0f), _planeLocalZ);
    if (_planeLocalX.lengthSquared() < 0.0005f) {
        _planeLocalY = QVector3D::crossProduct(_planeLocalZ, QVector3D(1.0f, 0.0f, 0.0f));
        _planeLocalY.normalize();
        _planeLocalX = QVector3D::crossProduct(_planeLocalY, _planeLocalZ);
    } else {
        _planeLocalX.normalize();
        _planeLocalY = QVector3D::crossProduct(_planeLocalZ, _planeLocalX);
    }
}

void Pose3DEstimator::_getCoordSystemFromPlaneCoords(QVector3D& nX, QVector3D& nY, QVector3D& nZ, QVector3D& pos) const
{
    pos = QVector3D(- QVector3D::dotProduct(nX, pos),
                    - QVector3D::dotProduct(nY, pos),
                    - QVector3D::dotProduct(nZ, pos));
    pos = _planeLocalX * pos.x() + _planeLocalY * pos.y() + _planeLocalZ * pos.z() + _planePos;
    QVector3D v;
    v.setX(_planeLocalX.x() * nX.x() + _planeLocalY.x() * nY.x() + _planeLocalZ.x() * nZ.x());
    v.setY(_planeLocalX.y() * nX.x() + _planeLocalY.y() * nY.x() + _planeLocalZ.y() * nZ.x());
    v.setZ(_planeLocalX.z() * nX.x() + _planeLocalY.z() * nY.x() + _planeLocalZ.z() * nZ.x());
    nX.setX(v.x()); nY.setX(v.y()); nZ.setX(v.z());
    v.setX(_planeLocalX.x() * nX.y() + _planeLocalY.x() * nY.y() + _planeLocalZ.x() * nZ.y());
    v.setY(_planeLocalX.y() * nX.y() + _planeLocalY.y() * nY.y() + _planeLocalZ.y() * nZ.y());
    v.setZ(_planeLocalX.z() * nX.y() + _planeLocalY.z() * nY.y() + _planeLocalZ.z() * nZ.y());
    nX.setY(v.x()); nY.setY(v.y()); nZ.setY(v.z());
    v.setX(_planeLocalX.x() * nX.z() + _planeLocalY.x() * nY.z() + _planeLocalZ.x() * nZ.z());
    v.setY(_planeLocalX.y() * nX.z() + _planeLocalY.y() * nY.z() + _planeLocalZ.y() * nZ.z());
    v.setZ(_planeLocalX.z() * nX.z() + _planeLocalY.z() * nY.z() + _planeLocalZ.z() * nZ.z());
    nX.setZ(v.x()); nY.setZ(v.y()); nZ.setZ(v.z());
    pos = QVector3D(- (nX.x() * pos.x() + nY.x() * pos.y() + nZ.x() * pos.z()),
                    - (nX.y() * pos.x() + nY.y() * pos.y() + nZ.y() * pos.z()),
                    - (nX.z() * pos.x() + nY.z() * pos.y() + nZ.z() * pos.z()));
}

void Pose3DEstimator::_computeFirstPoseForCoplanarPoints()
{
    TMath_assert(_isCoplanar);
    TMath_assert(_points.size() >= 4);
    TMath::TMatrix<float> M(_points.size() * 2, 9);
    unsigned int i;
    float* dataRow = M.firstDataRow();
    for (i=0; i<_points.size(); ++i) {
        const QVector4D& point = _points[i];
        QVector3D delta(point.x() - _planePos.x(),
                        point.y() - _planePos.y(),
                        point.z() - _planePos.z());
        QVector2D weightPointInPlane(QVector3D::dotProduct(_planeLocalX, delta) * point.w(),
                                     QVector3D::dotProduct(_planeLocalY, delta) * point.w());
        const IPointF& uv = _pointsUV[i];

        dataRow[0] = weightPointInPlane.x();
        dataRow[1] = weightPointInPlane.y();
        dataRow[2] = point.w();
        dataRow[3] = 0.0f;
        dataRow[4] = 0.0f;
        dataRow[5] = 0.0f;
        dataRow[6] = weightPointInPlane.x() * uv.x;
        dataRow[7] = weightPointInPlane.y() * uv.x;
        dataRow[8] = point.w() * uv.x;
        dataRow = &dataRow[9];

        dataRow[0] = 0.0f;
        dataRow[1] = 0.0f;
        dataRow[2] = 0.0f;
        dataRow[3] = weightPointInPlane.x();
        dataRow[4] = weightPointInPlane.y();
        dataRow[5] = point.w();
        dataRow[6] = weightPointInPlane.x() * uv.y;
        dataRow[7] = weightPointInPlane.y() * uv.y;
        dataRow[8] = point.w() * uv.y;
        dataRow = &dataRow[9];
    }
    if (_points.size() == 4) {
        dataRow[0] = 0.0f;
        dataRow[1] = 0.0f;
        dataRow[2] = 0.0f;
        dataRow[3] = 0.0f;
        dataRow[4] = 0.0f;
        dataRow[5] = 0.0f;
        dataRow[6] = 0.0f;
        dataRow[7] = 0.0f;
        dataRow[8] = 0.0f;
    }
    _svd.compute(M, false);
    QVector3D nX, nY, nZ, pos;
    const float* vtdata = _svd.V_transposed().getDataRow(8);
    nX.setX(vtdata[0]); nX.setY(vtdata[3]); nX.setZ(vtdata[6]);
    float lengthX = nX.length();
    if (lengthX > FLT_EPSILON) {
        nX /= lengthX;
    } else {
        nX.setX(0.0f);
        nX.setX(0.0f);
        nX.setX(0.0f);
    }
    nY.setX(vtdata[1]); nY.setY(vtdata[4]); nY.setZ(vtdata[7]);
    float lengthY = nY.length();
    if (lengthY > FLT_EPSILON) {
        nY /= lengthY;
    } else {
        nY.setX(0.0f);
        nY.setY(0.0f);
        nY.setZ(0.0f);
    }
    nZ = QVector3D::crossProduct(nX, nY);
    float invScale = 2.0f / (lengthX + lengthY);
    pos.setX(vtdata[2] * invScale); pos.setY(vtdata[5] * invScale); pos.setZ(vtdata[8] * invScale);
    float camPosZ = QVector3D::dotProduct(pos, nZ);
    if (camPosZ < 0.0f) {
        nX = - nX;
        nY = - nY;
        pos = - pos;
    }
    _getCoordSystemFromPlaneCoords(nX, nY, nZ, pos);
    _transform(0, 0) = nX.x(); _transform(0, 1) = nY.x(); _transform(0, 2) = nZ.x(); _transform(0, 3) = pos.x();
    _transform(1, 0) = nX.y(); _transform(1, 1) = nY.y(); _transform(1, 2) = nZ.y(); _transform(1, 3) = pos.y();
    _transform(2, 0) = nX.z(); _transform(2, 1) = nY.z(); _transform(2, 2) = nZ.z(); _transform(2, 3) = pos.z();
}

void Pose3DEstimator::_computeFirstPoseForNoncoplanarPoints()
{
    TMath_assert(!_isCoplanar);
    TMath_assert(_points.size() >= 6);
    TMath::TMatrix<float> M(_points.size() * 2, 12);
    unsigned int i;
    float* dataRow = M.firstDataRow();
    for (i=0; i<_points.size(); ++i) {
        const QVector4D& point = _points[i];
        const QVector3D weightPoint(point.x() * point.w(), point.y() * point.w(), point.z() * point.w());
        const IPointF& uv = _pointsUV[i];

        dataRow[0] = weightPoint.x();
        dataRow[1] = weightPoint.y();
        dataRow[2] = weightPoint.z();
        dataRow[3] = point.w();
        dataRow[4] = 0.0f;
        dataRow[5] = 0.0f;
        dataRow[6] = 0.0f;
        dataRow[7] = 0.0f;
        dataRow[8] = weightPoint.x() * uv.x;
        dataRow[9] = weightPoint.y() * uv.x;
        dataRow[10] = weightPoint.z() * uv.x;
        dataRow[11] = point.w() * uv.x;
        dataRow = &dataRow[12];

        dataRow[0] = 0.0f;
        dataRow[1] = 0.0f;
        dataRow[2] = 0.0f;
        dataRow[3] = 0.0f;
        dataRow[4] = weightPoint.x();
        dataRow[5] = weightPoint.y();
        dataRow[6] = weightPoint.z();
        dataRow[7] = point.w();
        dataRow[8] = weightPoint.x() * uv.y;
        dataRow[9] = weightPoint.y() * uv.y;
        dataRow[10] = weightPoint.z() * uv.y;
        dataRow[11] = point.w() * uv.y;
        dataRow = &dataRow[12];
    }
    _svd.compute(M, false);
    QVector3D nX, nY, nZ, pos;
    const float* vtdata = _svd.V_transposed().getDataRow(11);
    nX.setX(vtdata[0]); nY.setX(vtdata[1]); nZ.setX(vtdata[2]);  pos.setX(vtdata[3]);
    nX.setY(vtdata[4]); nY.setY(vtdata[5]); nZ.setY(vtdata[6]);  pos.setY(vtdata[7]);
    nX.setZ(vtdata[8]); nY.setZ(vtdata[9]); nZ.setZ(vtdata[10]); pos.setZ(vtdata[11]);
    TMath::TMatrix<float> mTransform(3, 4);
    mTransform.copyDataFrom(vtdata);
    if (TMath::Tools<float>::matrix3x3Determinant(mTransform.slice(3, 3)) < 0.0f) {
        nX *= -1.0f;
        nZ *= -1.0f;
        pos *= -1.0f;
    }
    float lengthX = nX.length();
    float lengthY = nY.length();
    float lengthZ = nZ.length();
    pos *= 3.0f / (lengthX + lengthY + lengthZ);
    if (lengthZ > FLT_EPSILON) {
        nZ /= lengthZ;
    } else {
        nZ.setX(0.0f);
        nZ.setY(0.0f);
        nZ.setZ(0.0f);
    }
    nY = QVector3D::crossProduct(nZ, nX).normalized();
    nX = QVector3D::crossProduct(nY, nZ);
    _transform(0, 0) = nX.x(); _transform(0, 1) = nY.x(); _transform(0, 2) = nZ.x(); _transform(0, 3) = pos.x();
    _transform(1, 0) = nX.y(); _transform(1, 1) = nY.y(); _transform(1, 2) = nZ.y(); _transform(1, 3) = pos.y();
    _transform(2, 0) = nX.z(); _transform(2, 1) = nY.z(); _transform(2, 2) = nZ.z(); _transform(2, 3) = pos.z();
}

}
