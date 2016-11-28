#ifndef FRAMERECEIVER_H
#define FRAMERECEIVER_H

#include <QObject>
#include <QMatrix3x3>
#include <QVideoFrame>
#include "AR/Image.h"

class FrameReceiver: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    FrameReceiver();

    virtual void receiveFrame(const AR::ImageRef<AR::Rgba>& image, const QMatrix3x3& textureMatrix,
                              const QVideoFrame::PixelFormat& pixelFormat) {
        Q_UNUSED(image);
        Q_UNUSED(textureMatrix);
        Q_UNUSED(pixelFormat);
    }

    bool enabled() const;
    void setEnabled(bool enabled);

signals:
    void enabledChanged();

private:
    bool m_enabled;
};

#endif // IMAGERECEIVER_H
