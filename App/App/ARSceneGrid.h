#ifndef ARSCENEGRID_H
#define ARSCENEGRID_H

#include "ARScene.h"
#include "QScrollEngine/QScrollEngine.h"
#include "TextureRenderer.h"

class ARSceneGrid:
        public ARScene
{
    Q_OBJECT
protected:
    ARSceneGrid();

    void initScene() override;
    void _endUpdate() override;

//private slots:
//    void drawGrid();

private:
    TextureRenderer* m_textureRenderer;
    QOpenGLTexture* m_tempFrameTexture;

    QScrollEngine::QEntity* _createHexGrid(float cellSize, int countX, int countY);
    QScrollEngine::QMesh* _createArrow(float length, float lengthArrow);
};

#endif
