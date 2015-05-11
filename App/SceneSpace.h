#ifndef SCENESPACE_H
#define SCENESPACE_H

#include <vector>
#include <QVector3D>
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QSprite.h"
#include "App/ARCameraQml.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "QScrollEngine/QMesh.h"
#include <vector>
#include "App/SceneShip.h"
#include "QScrollEngine/Shaders/QSh.h"

class OrbitShader:
        public QScrollEngine::QSh
{
public:
    static int locationMatrixWVP;
    static int locationTime;
    static int locationColorA;
    static int locationColorB;

public:
    OrbitShader()
    {
        _currentIndexType = indexType();
        _subIndexType = 0;
        _time = 0.0f;
        _maxTime = static_cast<float>(2.0f * M_PI);
        _colorA = QColor(255, 255, 255, 255);
        _colorB = QColor(0, 0, 0, 255);
    }
    OrbitShader(float time, float maxTime = static_cast<float>(2.0f * M_PI),
                const QColor& colorA = QColor(255, 255, 255, 255), const QColor& colorB = QColor(0, 0, 0, 255))
    {
        _currentIndexType = indexType();
        _subIndexType = 0;
        _time = time;
        _maxTime = maxTime;
        _colorA = colorA;
        _colorB = colorB;
    }
    float time() const { return _time; }
    void setTime(float time) { _time = time; }
    float maxTime() const { return _maxTime; }
    void setMaxTime(float maxTime) { _maxTime = maxTime; }
    QColor colorA() const { return _colorA; }
    void setColorA(const QColor& color) { _colorA = color; }
    QColor colorB() const { return _colorB; }
    void setColorB(const QColor& color) { _colorB = color; }
    OrbitShader(const OrbitShader* s)
    {
        _currentIndexType = indexType();
        _subIndexType = 0;
        _time = s->time();
        _maxTime = s->maxTime();
        _colorA = s->colorA();
        _colorB = s->colorB();
    }
    int indexType() const override { return 3434; }
    QSh* copy() const override { return new OrbitShader(this); }
    bool use(QScrollEngine::QScrollEngineContext* , QOpenGLShaderProgram* program) override;
    void load(QScrollEngine::QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;

private:
    float _time;
    float _maxTime;
    QColor _colorA;
    QColor _colorB;
};

class SunParticleSystem
{
public:
    typedef struct SunParticle
    {
        QScrollEngine::QSprite* sprite;
        QVector3D velocity;
        float angularVelocity;
        float lifeLimit;
        int time;
    } SunParticle;

    typedef struct SunParticleList
    {
        SunParticle p;
        SunParticleList* next;
    } SunParticleList;

    SunParticleSystem();

    QScrollEngine::QSprite* originalParticle(int index) const { return _original[index]; }
    void setOriginalParticle(int index, QScrollEngine::QSprite* sprite) { _original[index] = sprite; }
    inline int countOriginalsParticles() const { return 4; }
    QScrollEngine::QEntity* parentEntity() const { return _parentEntity; }
    void setParentEntity(QScrollEngine::QEntity* entity) { _parentEntity = entity; }
    int cloudParticles() const { return _countParticles; }
    void setCloudParticles(int count) { _countParticles = count; }
    float scalePartice() const { return _scaleParticle; }
    void setScaleParticle(float scale) { _scaleParticle = scale; }
    float deltaScaleParticle() const { return _deltaScaleParticle; }
    void setDeltaScaleParticle(float deltaScale) { _deltaScaleParticle = deltaScale; }
    float radiusArea() const { return _radiusArea; }
    void setRadiusArea(float radius) { _radiusArea = radius; }
    int timeParticle() const { return _timeParticle; }
    void setTimeParticle(int time) { _timeParticle = time; }
    float velocityParticle() const { return _velocityParticle; }
    void setVelocityParticle(float velocity) { _velocityParticle = velocity; }
    float deltaVelocityParticle() const { return _deltaVelocityParticle; }
    void setDeltaVelocityParticle(float deltaVelocity) { _deltaVelocityParticle = deltaVelocity; }

    void update();

private:
    SunParticleList* _listParticles;
    QScrollEngine::QSprite* _original[4];
    QScrollEngine::QEntity* _parentEntity;
    int _countParticles;
    int _timeParticle;
    float _scaleParticle;
    float _deltaScaleParticle;
    float _radiusArea;
    float _speedLife;
    float _velocityParticle;
    float _deltaVelocityParticle;
    float _dumpVelocity;
};


class SceneSpace:
        public SceneInterface
{
    Q_OBJECT
public:
    SceneSpace(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~SceneSpace();

    void beginUpdate() override;
    void endUpdate() override;

public slots:
    void updateSceneThread();

private:
    typedef struct SpaceObject
    {
        QScrollEngine::QEntity* entity;
        float orbitAngle;
        float orbitAngularVelocity;
    } SpaceObject;

    typedef struct TurnedTextureSphereShader
    {
        QScrollEngine::QSh_Sphere_Texture1* shader;
        QVector3D axis;
        float angle;
        float angularVelocity;
    } TurnedTextureSphereShader;

    typedef struct TurnedLightSphereShader
    {
        QScrollEngine::QSh_Sphere_Light* shader;
        QVector3D axis;
        float angle;
        float angularVelocity;
    } TurnedLightSphereShader;

    QThread* _thread;
    QMutex _mutex;
    bool _needUpdateScene;
    std::vector<SpaceObject> _spaceObjects;
    std::vector<TurnedTextureSphereShader> _turnedTexturedSphereShaders;
    std::vector<TurnedLightSphereShader> _turnedLightSphereShaders;
    QScrollEngine::PlanarShadows _planarShadows;
    QScrollEngine::QLight* _sunLight;
    QScrollEngine::QLight* _sceneLight;
    QScrollEngine::QEntity* _solarSystem;
    SunParticleSystem _sunParticleSystem;

    void _createOrbitPart(QScrollEngine::QEntity::QPartEntity* part, float planetRadius, float orbitRadius, unsigned int countVertices,
                          const QColor& colorA, const QColor& colorB);
    void _updatePlanets();
};

#endif // SCENESPACE_H
