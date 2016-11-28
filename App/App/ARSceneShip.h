#ifndef SCENESHIP_H
#define SCENESHIP_H

#include <cmath>
#include <vector>
#include <QSize>
#include <QVector2D>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QTime>
#include <QMutexLocker>
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QMesh.h"
#include "ARScene.h"


class Sh_WarpRiple:
        public QScrollEngine::QSh
{
public:
    Sh_WarpRiple();

    int typeIndex() const override { return 1411; }
    void preprocess(const QScrollEngine::QDrawObject3D* ) override;
    bool use(QScrollEngine::QScrollEngineContext* context, QOpenGLShaderProgram* program,
             const QScrollEngine::QDrawObject3D* drawObject) override;
    void load(QScrollEngine::QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& programs);

    QScrollEngine::QShPtr copy() const override
    {
        Sh_WarpRiple* sh = new Sh_WarpRiple();
        sh->setFrameTexture(m_frameTextureId, m_egl, m_textureMatrix);
        sh->setBorderSize(m_borderSize);
        sh->setRadius(m_radius);
        sh->setSourceRipple(m_sourceRipple);
        sh->setHeightRipple(m_heightRipple);
        sh->setShadowPower(m_shadowPower);
        return QScrollEngine::QShPtr(sh);
    }

    void setFrameTexture(GLuint textureId, bool egl, const QMatrix3x3& textureMatrix)
    {
        m_frameTextureId = textureId;
        m_egl = egl;
        m_textureMatrix = textureMatrix;
    }
    GLuint frameTextureId() const { return m_frameTextureId; }
    bool egl() const { return m_egl; }

    void setBorderSize(const QVector2D& borderSize) { m_borderSize = borderSize; }
    QVector2D borderSize() const { return m_borderSize; }

    void setRadius(float radius) { m_radius = radius; }
    float radius() const { return m_radius; }

    void setSourceRipple(const QVector2D& sourceRipple) { m_sourceRipple = sourceRipple; }
    QVector2D sourceRipple() const { return m_sourceRipple; }

    void setHeightRipple(float heightRipple) { m_heightRipple = heightRipple; }
    float heightRipple() const { return m_heightRipple; }

    void setOffset(float offset) { m_offset = offset; }
    float offset() const { return m_offset; }

    void setShadowPower(float shadowPower) { m_shadowPower = qMin(qMax(shadowPower, 0.0f), 1.0f); }
    float shadowPower() const { return m_shadowPower; }

    QVector3D transform(const QVector3D& point) const;

    std::vector<VertexAttributes> attributes() const override { return std::vector<VertexAttributes>(); }

private:
    GLuint m_frameTextureId;
    bool m_egl;
    QMatrix3x3 m_textureMatrix;
    QVector2D m_borderSize;
    float m_radius;
    QVector2D m_sourceRipple;
    float m_heightRipple;
    float m_offset;
    float m_shadowPower;

private:
    static int locationMatrixWVP[2];
    static int locationFrameTexture[2];
    static int locationBorderSize[2];
    static int locationRadius[2];
    static int locationSourceRipple[2];
    static int locationHeightRipple[2];
    static int locationOffset[2];
    static int locationTextureMatrix[2];
    static int locationShadowPower[2];
};

class ARSceneShip:
        public ARScene
{
    Q_OBJECT
public:
    ~ARSceneShip();

    void setFrameTexture(GLuint frameTexture, const QMatrix3x3& textureMatrix, bool egl) override;

    static QScrollEngine::QEntity* createGrid(QScrollEngine::QScene* scene, const QVector2D& size, const QSize& countSteps);

protected:
    void initScene() override;
    void _beginUpdate() override;
    void _endUpdate() override;

private:
    QScrollEngine::QEntity* m_grid;
    QScrollEngine::QEntity* m_water;
    QScrollEngine::QEntity* m_ship;
    float m_state;
    float m_speedOfChangeState;
    float m_shipAngle;
    float m_shipAngularDamping;
    float m_shipStartDeep;
    float m_shipCurrentDeep;
    float m_shipDeepVelocity;
    QVector2D m_shipSize;

    QScrollEngine::QEntity* _createWater(const QVector2D& size, const QSize& countSteps);
    void _updateShip();
};

#endif // SCENESHIP_H
