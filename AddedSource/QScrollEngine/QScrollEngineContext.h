#ifndef QSCROLLENGINECONTEXT_H
#define QSCROLLENGINECONTEXT_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QQuaternion>
#include <list>
#include <map>
#include <vector>
#include <clocale>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMessageBox>
#include <QString>
#include <QImage>
#include <QMutex>
#include <QOpenGLFramebufferObject>
#include <QSharedPointer>

#include "QScrollEngine/QFileSaveLoad3DS.h"
#include "QScrollEngine/Shaders/QSh_All.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QMesh.h"

namespace QScrollEngine {

class QScene;
class QMesh;
class QLight;
class QSpotLight;
class QSprite;
class QSceneObject3D;
class QCamera3D;

class QScrollEngineContext:
        public QOpenGLFunctions
{
public:
    explicit QScrollEngineContext(QOpenGLContext* context = nullptr);
    virtual ~QScrollEngineContext();

    enum StateTimeEvent
    {
        Play,
        Play_back,
        Play_next,
        Pause
    };

public:
    QColor backgroundColor;
    QCamera3D* camera;

    QOpenGLContext* openGLContext() const { return m_openGLContext; }
    void setOpenGLContext(QOpenGLContext* openGLContext);
    void setOpenGLContext(QOpenGLContext* openGLContext, int width, int height);
    void clearContext();

    void drawSprite(const QSprite* sprite);
    void drawMesh(const QMesh* mesh, const QDrawObject3D* drawObject, QSh* shader);
    void drawPartEntity(const QEntity::Part* part);
    void drawEntity(QEntity* entity);
    void drawLines(const QVector2D* lineVertices, size_t countVertices, const QColor& color,
                   const QMatrix4x4& matrixWorldViewProj = QMatrix4x4());

    void setStateTimeEvent(StateTimeEvent state);

    void setTimeStep(float timeStep);

    void error(const QString& message)
    {
        qCritical(message.toLatin1());
    }

    std::size_t countScenes() const { return m_scenes.size(); }
    QScene* scene(int i) const { return m_scenes[i]; }

    QOpenGLTexture* loadTexture(const QString& name, const QString& path);
    QOpenGLTexture* loadTexture(const QString& name, const QImage& image);
    QOpenGLTexture* emptyTexture() const { return m_emptyTexture; }
    void addTexture(const QString& name, QOpenGLTexture* texture);
    QEntity* loadEntity(const QString& path, const QString& textureDir = "!", const QString& prefixTextureName = "");
    bool saveEntity(QEntity* entity, const QString& path, const QString& textureDir = "!", const QString& prefixTextureName = "");
    QOpenGLTexture* texture(const QString& name);
    bool textureName(QString& result, const QOpenGLTexture* texture) const;
    void deleteTexture(QOpenGLTexture* texture);
    void deleteTexture(const QString& name);
    void deleteAllTextures();

    QOpenGLShaderProgram* shaderProgram(int indexType, int subIndexType = 0);
    QOpenGLShaderProgram* shaderProgram(QSh::Type type, int subIndexType = 0);
    const std::vector<QSh::VertexAttributes>& vertexAttributesOfShader(int indexType) const;
    const std::vector<QSh::VertexAttributes>& vertexAttributesOfShader(QSh::Type indexType) const;

    bool loadShader(QOpenGLShaderProgram* program, QString nameVertex, QString nameFragment);
    bool checkBindShader(QOpenGLShaderProgram* program, QString name);
    bool registerShader(QSh* shaderSample);
    void clearShaders();
    void registerDefaultShaders();
    StateTimeEvent stateTimeEvent() const { return m_stateTimeEvent; }

    bool enableClearing() const { return m_enableClearing; }
    void setEnableClearing(bool enable) { m_enableClearing = enable; }
    bool postEffectUsed() const { return m_postEffectUsed; }
    void setPostEffectUsed(bool enable)
    {
        deleteObjectsOfPostProcess();
        m_postEffectUsed = enable;
        initObjectsOfPostProcess();
    }

    const QMesh* screenQuad() const { return m_quad; }
    QMatrix4x4 screenQuadMatrix() const { return m_quadFinalMatrix; }

    QSize contextSize() const { return m_normalSize; }

    GLuint screenTexture() const { return (m_FBOs[0] == nullptr) ? 0 : m_FBOs[0]->texture(); }

    void initializeContext();
    void resizeContext(int width, int height);
    void paintContext();
    void beginPaint();
    void drawScenes();
    void endPaint(int defaultFBOId);

    float animationSpeed() const { return m_animationSpeed; }
    void setAnimationSpeed(float speed) { m_animationSpeed = speed; }

protected:
    void resolveScreenQuad();
    void initObjectsOfPostProcess();
    void deleteObjectsOfPostProcess();

private:
    friend class QScene;
    friend class QSceneObject3D;
    friend class QLight;
    friend class QSpotLight;
    friend class QSprite;
    friend class QEntity;
    friend class QMesh;

    typedef struct _CurrentDrawingObjects {
        std::vector<QSprite*> sprites;
        std::vector<QEntity::Part*> partEntities;
    } _CurrentDrawingObjects;

    typedef struct _Drawing {
        std::vector<_CurrentDrawingObjects> currentObjects;
        std::vector<QSharedPointer<QOpenGLShaderProgram>> programs;
        std::vector<QSh::VertexAttributes> attributes;
    } _Drawing;

    typedef struct TempAlphaObject
    {
        QDrawObject3D* drawObject;
        float zDistance;
    } TempAlphaObject;

    QOpenGLContext* m_openGLContext;

    StateTimeEvent m_stateTimeEvent;

    bool m_enableClearing;

    QOpenGLFramebufferObject* m_FBOs[3];
    float m_nDecreaseTexture;
    bool m_postEffectUsed;

    QMesh* m_quad;
    QMatrix4x4 m_quadFinalMatrix;
    QSize m_normalSize;
    QSize m_decreaseSize;
    QSh__BloomMap m_shader_bloomMap;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> m_shaderProgram_bloomMap;
    QSh__Blur m_shader_blur;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> m_shaderProgram_blur;
    QSh__Bloom m_shader_bloom;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> m_shaderProgram_bloom;

    std::map<int, _Drawing> m_drawings;
    std::list<TempAlphaObject> m_tempAlphaObjects;

    float m_animationSpeed;
    std::vector<QScene*> m_scenes;
    std::vector<int> m_sceneOrderStep;
    std::map<QString, QOpenGLTexture*> m_textures;
    QOpenGLTexture* m_emptyTexture;
    QFileSaveLoad3DS m_fileSaveLoad3DS;

    void _enableVertexAttributes(QOpenGLShaderProgram* program,
                                 const std::vector<QSh::VertexAttributes>& attributes);
    void _disableVertexAttributes(QOpenGLShaderProgram* program,
                                  const std::vector<QSh::VertexAttributes>& attributes);

    void _addScene(QScene* scene);
    void _deleteScene(QScene* scene);

    void _addTempAlphaObject(const TempAlphaObject& object);
    void _sortingTempAlphaObjects();

    void _drawCurrent();
};

}

#endif
