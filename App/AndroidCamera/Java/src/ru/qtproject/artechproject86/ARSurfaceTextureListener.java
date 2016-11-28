package ru.qtproject.artechproject86;

import android.graphics.SurfaceTexture;

public class ARSurfaceTextureListener implements SurfaceTexture.OnFrameAvailableListener
{
    private final int texID;

    public ARSurfaceTextureListener (int texID)
    {
        this.texID = texID;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture)
    {
        notifyFrameAvailable(texID);
    }

    private static native void notifyFrameAvailable(int id);
}
