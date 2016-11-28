#include "FrameReceiver.h"

FrameReceiver::FrameReceiver()
{
    m_enabled = true;
}

bool FrameReceiver::enabled() const
{
    return m_enabled;
}

void FrameReceiver::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}
