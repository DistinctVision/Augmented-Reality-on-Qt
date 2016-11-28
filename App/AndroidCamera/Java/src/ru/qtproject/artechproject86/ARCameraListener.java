package ru.qtproject.artechproject86;

import android.hardware.Camera;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.util.Log;
import java.lang.Math;
import java.util.concurrent.locks.ReentrantLock;

public class ARCameraListener implements Camera.ShutterCallback,
                                         Camera.PictureCallback,
                                         Camera.AutoFocusCallback,
                                         Camera.PreviewCallback
{
    private int m_cameraId = -1;
    private byte[][] m_cameraPreviewBuffer = null;
    private volatile int m_actualPreviewBuffer = 0;
    private final ReentrantLock m_buffersLock = new ReentrantLock();
    private boolean m_fetchEachFrame = false;

    private static final String TAG = "AR Camera";

    private ARCameraListener(int id)
    {
        m_cameraId = id;
    }

    public void preparePreviewBuffer(Camera camera)
    {
        Camera.Size previewSize = camera.getParameters().getPreviewSize();
        double bytesPerPixel = ImageFormat.getBitsPerPixel(camera.getParameters().getPreviewFormat()) / 8.0;
        int bufferSizeNeeded = (int)Math.ceil(bytesPerPixel*previewSize.width*previewSize.height);
        m_buffersLock.lock();
        if (m_cameraPreviewBuffer == null || m_cameraPreviewBuffer[0].length < bufferSizeNeeded)
            m_cameraPreviewBuffer = new byte[2][bufferSizeNeeded];
        m_buffersLock.unlock();
    }

    public void fetchEachFrame(boolean fetch)
    {
        m_fetchEachFrame = fetch;
    }

    public byte[] lockAndFetchPreviewBuffer()
    {
        //This method should always be followed by unlockPreviewBuffer()
        //This method is not just a getter. It also marks last preview as already seen one.
        //We should reset actualBuffer flag here to make sure we will not use old preview with future captures
        byte[] result = null;
        m_buffersLock.lock();
        result = m_cameraPreviewBuffer[(m_actualPreviewBuffer == 1) ? 0 : 1];
        m_actualPreviewBuffer = 0;
        return result;
    }

    public void unlockPreviewBuffer()
    {
        if (m_buffersLock.isHeldByCurrentThread())
            m_buffersLock.unlock();
    }

    public byte[] callbackBuffer()
    {
        return m_cameraPreviewBuffer[(m_actualPreviewBuffer == 1) ? 1 : 0];
    }

    @Override
    public void onShutter()
    {
        notifyPictureExposed(m_cameraId);
    }

    @Override
    public void onPictureTaken(byte[] data, Camera camera)
    {
        notifyPictureCaptured(m_cameraId, data);
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera)
    {
        m_buffersLock.lock();

        if (data != null && m_fetchEachFrame)
            notifyFrameFetched(m_cameraId, data);

        if (data == m_cameraPreviewBuffer[0])
            m_actualPreviewBuffer = 1;
        else if (data == m_cameraPreviewBuffer[1])
            m_actualPreviewBuffer = 2;
        else
            m_actualPreviewBuffer = 0;
        camera.addCallbackBuffer(m_cameraPreviewBuffer[(m_actualPreviewBuffer == 1) ? 1 : 0]);
        m_buffersLock.unlock();
    }

    @Override
    public void onAutoFocus(boolean success, Camera camera)
    {
        notifyAutoFocusComplete(m_cameraId, success);
    }

    private static native void notifyAutoFocusComplete(int id, boolean success);
    private static native void notifyPictureExposed(int id);
    private static native void notifyPictureCaptured(int id, byte[] data);
    private static native void notifyFrameFetched(int id, byte[] data);
}
