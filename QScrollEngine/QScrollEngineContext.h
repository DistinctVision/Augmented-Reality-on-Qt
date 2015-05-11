#ifndef QSCROLLENGINECONTEXT_H
#define QSCROLLENGINECONTEXT_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QQuaternion>
#include <list>
#include <map>
#include <vector>
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
    friend class QScene;
    friend class QSceneObject3D;
    friend class QLight;
    friend class QSpotLight;
    friend class QSprite;
    friend class QEntity;
    friend class QMesh;

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

    QOpenGLContext* openGLContext() const { return _openGLContext; }
    void setOpenGLContext(QOpenGLContext* openGLContext);
    void setOpenGLContext(QOpenGLContext* openGLContext, int width, int height);
    void clearContext();

    void drawSprite(const QSprite* sprite);
    void drawPartEntity(const QEntity::QPartEntity* part);
    void drawMesh(GLenum drawMode, const QMesh* mesh, QOpenGLShaderProgram* program);
    void drawEntity(QEntity* entity);
    void drawLines(const QVector2D* lineVertices, unsigned int countVertices, const QColor& color,
                   const QMatrix4x4& matrixWorldViewProj = QMatrix4x4());

    void setStateTimeEvent(StateTimeEvent state);

    void setTimeStep(float timeStep);

    void error(const QString& message)
    {
        qWarning(message.toLatin1());
    }

    int countScenes() const { return _scenes.size(); }
    QScene* scene(int i) const { return _scenes[i]; }

    QOpenGLTexture* loadTexture(const QString& name, const QString& path);
    QOpenGLTexture* loadTexture(const QString& name, const QImage& image);
    QOpenGLTexture* loadBWTexture(const QString& name, const QString& path);
    QOpenGLTexture* emptyTexture() const { return _emptyTexture; }
    void addTexture(QOpenGLTexture* texture, const QString& textureName);
    QEntity* loadEntity(const QString& path, const QString& textureDir = "!", const QString& prefixTextureName = "");
    bool saveEntity(QEntity* entity, const QString& path, const QString& textureDir = "!", const QString& prefixTextureName = "");
    QOpenGLTexture* texture(const QString& name);
    bool textureName(QString& result, const QOpenGLTexture* texture) const;
    void deleteTexture(QOpenGLTexture* texture);
    void deleteTexture(const QString& name);
    void deleteAllTextures();

    QOpenGLShaderProgram* shaderProgram(int indexType, int subIndexType = 0);

    void draw();

    bool loadShader(QOpenGLShaderProgram* program, QString nameVertex, QString nameFragment);
    bool checkBindShader(QOpenGLShaderProgram* program, QString name);
    bool registerShader(QSh* shaderSample);
    void clearShaders();
    void registerDefaultShaders();
    StateTimeEvent stateTimeEvent() const { return _stateTimeEvent; }

    bool enableClearing() const { return _enableClearing; }
    void setEnableClearing(bool enable) { _enableClearing = enable; }
    bool postEffectUsed() const { return _postEffectUsed; }
    void setPostEffectUsed(bool enable)
    {
        deleteObjectsOfPostProcess();
        _postEffectUsed = enable;
        initObjectsOfPostProcess();
    }

    const QMesh* screenQuad() const { return _quad; }
    QMatrix4x4 screenQuadMatrix() const { return _quadFinalMatrix; }

    QSize contextSize() const { return _normalSize; }

    GLuint screenTexture() const { return (_FBOs[0] == nullptr) ? 0 : _FBOs[0]->texture(); }

    void lock() { _locker.lock(); }
    void unlock() { _locker.unlock(); }

    void initializeContext();
    void resizeContext(int width, int height);
    void paintContext();
    void beginPaint();
    void drawScenes();
    void endPaint(int defaultFBOId);

    float animationSpeed() const { return _animationSpeed; }
    void setAnimationSpeed(float speed) { _animationSpeed = speed; }

protected:
    void resolveScreenQuad();
    void initObjectsOfPostProcess();
    void deleteObjectsOfPostProcess();

private:
    typedef struct _CurrentDrawingObjects {
        std::vector<QSprite*> sprites;
        std::vector<QEntity::QPartEntity*> partEntities;
    } _CurrentDrawingObjects;

    typedef struct _Drawing {
       std::vector<_CurrentDrawingObjects> currentObjects;
       std::vector<QSharedPointer<QOpenGLShaderProgram>> programs;
    } _Drawing;
    typedef struct TempAlphaObject
    {
        QDrawObject3D* drawObject;
        float zDistance;
    } TempAlphaObject;

    QOpenGLContext* _openGLContext;

    StateTimeEvent _stateTimeEvent;

    bool _enableClearing;

    QOpenGLFramebufferObject* _FBOs[3];
    float _nDecreaseTexture;
    bool _postEffectUsed;

    QMesh* _quad;
    QMatrix4x4 _quadFinalMatrix;
    QSize _normalSize;
    QSize _decreaseSize;
    QSh__BloomMap _shader_bloomMap;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> _shaderProgram_bloomMap;
    QSh__Blur _shader_blur;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> _shaderProgram_blur;
    QSh__Bloom _shader_bloom;
    std::vector<QSharedPointer<QOpenGLShaderProgram>> _shaderProgram_bloom;

    std::map<int, _Drawing> _drawings;
    std::list<TempAlphaObject> _tempAlphaObjects;

    float _animationSpeed;
    std::vector<QScene*> _scenes;
    std::vector<int> _sceneOrderStep;
    std::map<QString, QOpenGLTexture*> _textures;
    QOpenGLTexture* _emptyTexture;
    QFileSaveLoad3DS _fileSaveLoad3DS;

    QMutex _locker;

    void _addScene(QScene* scene);
    void _deleteScene(QScene* scene);

    void _addTempAlphaObject(const TempAlphaObject& object);
    void _sortingTempAlphaObjects();

    void _drawCurrent();
};

}

#endif
