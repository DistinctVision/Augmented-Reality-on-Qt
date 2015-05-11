#ifndef SCENESHIP_H
#define SCENESHIP_H

#include <cmath>
#include <vector>
#include <QVector2D>
#include <QVector3D>
#include <QThread>
#include <QMutex>
#include <QTime>
#include <QMutexLocker>
#include "QScrollEngine/QScene.h"
#include "App/ARCameraQml.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/Tools/PlanarShadows.h"
#include "QScrollEngine/Tools/QIsoSurface.h"

template<typename Type>
class RandomValue
{
public:
    RandomValue()
    {
        _value = _oldValue = _dstValue = Type();
        _minPeriod = 120;
        _maxPeriod = 240;
        setRandomDst();
    }
    Type value() const { return _value; }
    Type dstValue() const { return _dstValue; }
    void clearRandom() { _random.clear(); }
    void extandRandom(const Type& element) { _random.push_back(element); }
    void setRandomDst()
    {
        _oldValue = _value;
        _dstTime = std::max(_minPeriod, qrand() % _maxPeriod);
        _time = 0;
        if (_random.empty())
            return;
        int weight = qrand() % 1000;
        _dstValue = _random[0] * weight;
        for (unsigned int i=1; i<_random.size(); ++i) {
            int w = qrand() % 1000;
            _dstValue += _random[i] * w;
            weight += w;
        }
        if (weight == 0) {
            _dstValue = _random[qrand() % _random.size()];
        } else {
            _dstValue *= 1.0f / static_cast<float>(weight);
        }
    }
    void setRandom()
    {
        setRandomDst();
        _value = _oldValue = _dstValue;
        setRandomDst();
    }
    int time() const { return _time; }
    int maxPeriod() const { return _maxPeriod; }
    void setMaxPeriod(int period) { _maxPeriod = period; }
    int minPeriod() const { return _minPeriod; }
    void setMinPeriod(int period) { _minPeriod = period; }
    void update()
    {
        ++_time;
        if (_time > _dstTime)
            setRandomDst();
        _value = _oldValue + (_dstValue - _oldValue) * (_time / static_cast<float>(_dstTime));
    }
    RandomValue(const Type& value):RandomValue() { _oldValue = _value = value; setRandomDst(); }
    RandomValue& operator = (const Type& value) { _oldValue = _value = value; setRandomDst(); return (*this); }
    operator Type() const { return _value; }

private:
    Type _oldValue;
    Type _value;
    Type _dstValue;
    int _time;
    int _dstTime;
    int _maxPeriod;
    int _minPeriod;
    std::vector<Type> _random;
};

class WaterField:
        public QScrollEngine::QIsoSurface::ScalarField
{
public:
    RandomValue<float> scaleWaves;
    RandomValue<float> speedWaves;
    RandomValue<float> heightWaves;
    RandomValue<float> scaleWaves_small;
    RandomValue<float> localSpeedWaves_small;
    RandomValue<float> heightWaves_small;
    RandomValue<float> angleSource;
    RandomValue<float> distanceSource;

    WaterField();

    float value(const QVector3D& point) override;
    inline float getSubWaveHeight(float x, float y) const;
    inline float getWaveHeight(float x, float y) const;

    float radius() const { return _radius; }
    void setRadius(float radius) { _radius = radius; }
    QVector2D sourceWavesPosition() const { return _sourceWavesPosition; }
    void update();
private:
    QVector2D _sourceWavesPosition;
    float _radius;
    float _time;
    float _time_small;

};

class CloudParticleSystem
{
public:
    typedef struct CloudParticle
    {
        QScrollEngine::QSprite* sprite;
        QVector2D velocity;
        float angularVelocity;
        float radiusOfArea;
        float lifeLimit;
        float scale;
        int time;
    } CloudParticle;

    typedef struct CloudParticleList
    {
        CloudParticle p;
        CloudParticleList* next;
    } CloudParticleList;

    CloudParticleSystem();

    QScrollEngine::QSprite* original() const { return _original; }
    void setOriginalParticle(QScrollEngine::QSprite* sprite) { _original = sprite; }
    QScrollEngine::QEntity* parentEntity() const { return _parentEntity; }
    void setParentEntity(QScrollEngine::QEntity* entity) { _parentEntity = entity; }
    int countParticles() const { return _countParticles; }
    void setCountParticles(int count) { _countParticles = count; }
    float scalePartice() const { return _scaleParticle; }
    void setScaleParticle(float scale) { _scaleParticle = scale; }
    float deltaScaleParticle() const { return _deltaScaleParticle; }
    void setDeltaScaleParticle(float deltaScale) { _deltaScaleParticle = deltaScale; }
    float radiusArea() const { return _radiusArea; }
    void setRadiusArea(float radius) { _radiusArea = radius; }
    float heightArea() const { return _heightArea; }
    void setHeightArea(float height) { _heightArea = height; }
    float detlaHeightArea() const { return _deltaHeightArea; }
    void setDeltaHeightArea(float delta) { _deltaHeightArea = delta; }
    int timeParticle() const { return _timeParticle; }
    void setTimeParticle(int time) { _timeParticle = time; }

    void update(const QVector2D& acceleration);

private:
    CloudParticleList* _listParticles;
    QScrollEngine::QSprite* _original;
    QScrollEngine::QEntity* _parentEntity;
    int _countParticles;
    int _timeParticle;
    float _scaleParticle;
    float _deltaScaleParticle;
    float _radiusArea;
    float _heightArea;
    float _deltaHeightArea;
    float _speedLife;
    float _dumpVelocity;
};

class SceneShip:
        public SceneInterface
{
    Q_OBJECT
public:
    SceneShip(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~SceneShip();

    void beginUpdate() override;
    void endUpdate() override;

    inline static float mod(float a, float b)
    {
        while (a > b)
            a -= b;
        while (a < 0.0f)
            a += b;
        return a;
    }

public slots:
    void updateSceneThread();

private:
    QScrollEngine::QEntity* _mainEntity;
    QScrollEngine::QEntity* _ship;
    CloudParticleSystem _particleSystem;
    float _shipAngle;
    float _shipAngularVelocity;
    QVector2D _shipSize;
    QScrollEngine::QEntity* _water;
    std::vector<QVector3D> _vertices;
    std::vector<QVector2D> _texcoords;
    std::vector<QVector3D> _normals;
    std::vector<GLuint> _triangles;
    QScrollEngine::QIsoSurface _isoSurface;
    WaterField _waterField;
    QThread* _thread;
    QMutex _mutex;
    bool _needUpdateScene;

    void _updateShip();
};

#endif // SCENESHIP_H
