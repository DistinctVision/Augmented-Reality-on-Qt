#include "CameraCalibrator.h"
#include "TMath/TMath.h"

namespace AR {

CameraCalibrator::CameraCalibrator()
{
    m_camera = std::shared_ptr<Camera>(new Camera());
    reset();
}

void CameraCalibrator::reset()
{
    m_meanPixelError = 0.0f;
    m_camera->setCameraParameters(Camera::defaultCameraParameters);
    m_calibrationFrames.clear();
}

CalibrationConfiguration CameraCalibrator::configuration() const
{
    return m_configuration;
}

void CameraCalibrator::setConfiguration(const CalibrationConfiguration& configuration)
{
    m_configuration = configuration;
}

Point2d CameraCalibrator::imageSize() const
{
    return m_camera->imageSize();
}

std::shared_ptr<Camera> CameraCalibrator::camera()
{
    return m_camera;
}

std::shared_ptr<const Camera> CameraCalibrator::camera() const
{
    return m_camera;
}

double CameraCalibrator::meanPixelError() const
{
    return m_meanPixelError;
}

void CameraCalibrator::addCalibrationFrame(std::shared_ptr<CalibrationFrame>& calibrationFrame)
{
    assert(calibrationFrame->isCreated());
    assert(calibrationFrame->camera() == m_camera);
    if (m_calibrationFrames.empty()) {
        m_camera->setImageSize(calibrationFrame->imageSize());
    } else {
        if (m_calibrationFrames[0]->imageSize() != calibrationFrame->imageSize()) {
            m_camera->setImageSize(calibrationFrame->imageSize());
            m_calibrationFrames.clear();
        }
    }
    if ((int)m_calibrationFrames.size() >= m_configuration.counOfUsedCalibrationFrames) {
        int offset_i = m_calibrationFrames.size() - m_configuration.counOfUsedCalibrationFrames;
        for (int i = 0; i < m_configuration.counOfUsedCalibrationFrames; ++i) {
            m_calibrationFrames[i] = m_calibrationFrames[offset_i + i];
        }
        m_calibrationFrames.resize(m_configuration.counOfUsedCalibrationFrames);
    } else {
        m_calibrationFrames.push_back(calibrationFrame);
    }
}

void CameraCalibrator::clearCalibrationFrames()
{
    m_calibrationFrames.clear();
}

/*void CameraCalibrator::OptimizeOneStep()
{
  int nViews = mvCalibImgs.size();
  int nDim = 6 * nViews + NUMTRACKERCAMPARAMETERS;
  int nCamParamBase = nDim - NUMTRACKERCAMPARAMETERS;

  Matrix<> mJTJ(nDim, nDim);
  Vector<> vJTe(nDim);
  mJTJ = Identity; // Weak stabilizing prior
  vJTe = Zeros;

  if(*mgvnDisableDistortion) mCamera.DisableRadialDistortion();


  double dSumSquaredError = 0.0;
  int nTotalMeas = 0;

  for(int n=0; n<nViews; n++)
    {
      int nMotionBase = n*6;
      vector<CalibImage::ErrorAndJacobians> vEAJ = mvCalibImgs[n].Project(mCamera);
      for(unsigned int i=0; i<vEAJ.size(); i++)
    {
      CalibImage::ErrorAndJacobians &EAJ = vEAJ[i];
      // All the below should be +=, but the MSVC compiler doesn't seem to understand that. :(
      mJTJ.slice(nMotionBase, nMotionBase, 6, 6) =
      mJTJ.slice(nMotionBase, nMotionBase, 6, 6) + EAJ.m26PoseJac.T() * EAJ.m26PoseJac;
      mJTJ.slice(nCamParamBase, nCamParamBase, NUMTRACKERCAMPARAMETERS, NUMTRACKERCAMPARAMETERS) =
      mJTJ.slice(nCamParamBase, nCamParamBase, NUMTRACKERCAMPARAMETERS, NUMTRACKERCAMPARAMETERS) +
EAJ.m2NCameraJac.T() * EAJ.m2NCameraJac;
      mJTJ.slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) =
      mJTJ.slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) + EAJ.m26PoseJac.T() * EAJ.m2NCameraJac;
      mJTJ.T().slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) =
      mJTJ.T().slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) + EAJ.m26PoseJac.T() * EAJ.m2NCameraJac;
      // Above does twice the work it needs to, but who cares..

      vJTe.slice(nMotionBase,6) =
      vJTe.slice(nMotionBase,6) + EAJ.m26PoseJac.T() * EAJ.v2Error;
      vJTe.slice(nCamParamBase,NUMTRACKERCAMPARAMETERS) =
      vJTe.slice(nCamParamBase,NUMTRACKERCAMPARAMETERS) + EAJ.m2NCameraJac.T() * EAJ.v2Error;

      dSumSquaredError += EAJ.v2Error * EAJ.v2Error;
      ++nTotalMeas;
    }
    };

  mdMeanPixelError = sqrt(dSumSquaredError / nTotalMeas);

  SVD<> svd(mJTJ);
  Vector<> vUpdate(nDim);
  vUpdate= svd.backsub(vJTe);
  vUpdate *= 0.1; // Slow down because highly nonlinear...
  for(int n=0; n<nViews; n++)
    mvCalibImgs[n].mse3CamFromWorld = SE3<>::exp(vUpdate.slice(n * 6, 6)) * mvCalibImgs[n].mse3CamFromWorld;
  mCamera.UpdateParams(vUpdate.slice(nCamParamBase, NUMTRACKERCAMPARAMETERS));
};*/

void CameraCalibrator::optimizeStep(const TMath::TVector<bool>& fixedCameraParamters)
{
    TMath_assert(fixedCameraParamters.size() == 5);
    using namespace TMath;

    int countViews = m_calibrationFrames.size();
    int countDims = 6 * countViews + 5;
    int nCamParamBase = countDims - 5;

    TMatrixd JTJ(countDims, countDims);
    TVectord JTe(countDims);
    JTJ.setToIdentity(); // Weak stabilizing prior
    JTe.setZero();

    double sumSquaredError = 0.0;
    int countTotalMeas = 0;

    for (int n = 0; n < countViews; ++n) {
        int nMotionBase = n * 6;

        std::vector<CalibrationFrame::ErrorAndJacobians> EAJs = m_calibrationFrames[n]->project(fixedCameraParamters);
        for(std::size_t i=0; i<EAJs.size(); ++i) {
            const CalibrationFrame::ErrorAndJacobians& EAJ = EAJs[i];
            JTJ.fill(nMotionBase, nMotionBase,
                     JTJ.slice(nMotionBase, nMotionBase, 6, 6) + EAJ.poseJac.refTransposed() * EAJ.poseJac);
            JTJ.fill(nCamParamBase, nCamParamBase, JTJ.slice(nCamParamBase, nCamParamBase, 5, 5) +
                     EAJ.cameraJac.refTransposed() * EAJ.cameraJac);
            JTJ.fill(nMotionBase, nCamParamBase, JTJ.slice(nMotionBase, nCamParamBase, 6, 5) +
                     EAJ.poseJac.refTransposed() * EAJ.cameraJac);
            JTJ.fill(nCamParamBase, nMotionBase,
                     JTJ.slice(nCamParamBase, nMotionBase, 5, 6) +
                     EAJ.cameraJac.refTransposed() * EAJ.poseJac);
            // Above does twice the work it needs to, but who cares..

            TVectord vectorOfError = TVectord::create(EAJ.error.x, EAJ.error.y);
            JTe.fill(nMotionBase, JTe.slice(nMotionBase, 6) +
                     EAJ.poseJac.refTransposed() * vectorOfError);
            JTe.fill(nCamParamBase, JTe.slice(nCamParamBase, 5) +
                     EAJ.cameraJac.refTransposed() * vectorOfError);

            sumSquaredError += EAJ.error.lengthSquared();
            ++countTotalMeas;
        }
    }

    m_meanPixelError = std::sqrt(sumSquaredError / countTotalMeas);

    TSVD<double> svd;
    svd.compute(JTJ);
    TVectord update(countDims);
    update = svd.backsub(JTe);

    update *= 0.1; // Slow down because highly nonlinear...
    TMatrixd r(3, 3);
    TVectord t(3);
    for (int n = 0; n < countViews; ++n) {
        TTools::exp_transform(r, t, update.slice(n * 6, 6));
        m_calibrationFrames[n]->setTranslation(r * m_calibrationFrames[n]->translation() + t);
        m_calibrationFrames[n]->setRotation(r * m_calibrationFrames[n]->rotation());
    }
    m_camera->setCameraParameters(m_camera->cameraParameters() + update.slice(nCamParamBase, 5));
}

void CameraCalibrator::forceOptimize(const TMath::TVector<bool>& fixedCameraParamters)
{
    optimizeStep(fixedCameraParamters);
    double prevPixelError = m_meanPixelError;
    TMath::TVectord prevCameraParameters = m_camera->cameraParameters(), currentCameraParameters;
    for (int iteration = 0; iteration < 100; ++iteration) {
        currentCameraParameters = m_camera->cameraParameters();
        optimizeStep(fixedCameraParamters);
        if (prevPixelError * 1.25 < m_meanPixelError) {
            m_camera->setCameraParameters(prevCameraParameters);
            m_meanPixelError = prevPixelError;
            break;
        } else if (prevPixelError > m_meanPixelError) {
            prevPixelError = m_meanPixelError;
            prevCameraParameters = currentCameraParameters;
        }
    }
    if (prevPixelError < m_meanPixelError) {
        m_camera->setCameraParameters(prevCameraParameters);
    }
}

} // namespace AR
