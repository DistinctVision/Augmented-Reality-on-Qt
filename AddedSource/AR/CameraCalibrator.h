#ifndef AR_CAMERACALIBRATOR_H
#define AR_CAMERACALIBRATOR_H

#include "Image.h"
#include "Camera.h"
#include "CalibrationFrame.h"
#include <vector>
#include <memory>

namespace AR {

class CameraCalibrator
{
public:
    CameraCalibrator();

    void reset();

    void addCalibrationFrame(std::shared_ptr<CalibrationFrame>& calibrationFrame);
    void clearCalibrationFrames();

    void forceOptimize(const TMath::TVector<bool>& fixedCameraParamters);
    void optimizeStep(const TMath::TVector<bool>& fixedCameraParamters);
    double meanPixelError() const;

    std::shared_ptr<Camera> camera();
    std::shared_ptr<const Camera> camera() const;

    CalibrationConfiguration configuration() const;
    void setConfiguration(const CalibrationConfiguration& configuration);

    Point2d imageSize() const;

protected:
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<CalibrationFrame>> m_calibrationFrames;
    double m_meanPixelError;
    CalibrationConfiguration m_configuration;
};

} // namespace AR

#endif
