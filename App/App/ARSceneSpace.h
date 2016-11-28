#ifndef ARSCENESPACE_H
#define ARSCENESPACE_H

#include <vector>
#include <QVector3D>
#include <QColor>
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QSprite.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "QScrollEngine/QMesh.h"
#include <vector>
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Tools/QPlanarShadows.h"
#include "ARScene.h"

class SunParticleSystem
{
public:
    struct SunParticle
    {
        QScrollEngine::QSprite* sprite;
        QVector3D velocity;
        float angularVelocity;
        float lifeLimit;
        int time;
    };

    struct SunParticleList
    {
        SunParticle p;
        SunParticleList* next;
    };

    SunParticleSystem();

    QScrollEngine::QSprite* originalParticle(int index) const { return m_original[index]; }
    void setOriginalParticle(int index, QScrollEngine::QSprite* sprite) { m_original[index] = sprite; }
    inline int countOriginalsParticles() const { return 4; }
    QScrollEngine::QEntity* parentEntity() const { return m_parentEntity; }
    void setParentEntity(QScrollEngine::QEntity* entity) { m_parentEntity = entity; }
    int cloudParticles() const { return m_countParticles; }
    void setCloudParticles(int count) { m_countParticles = count; }
    float scalePartice() const { return m_scaleParticle; }
    void setScaleParticle(float scale) { m_scaleParticle = scale; }
    float deltaScaleParticle() const { return m_deltaScaleParticle; }
    void setDeltaScaleParticle(float deltaScale) { m_deltaScaleParticle = deltaScale; }
    float radiusArea() const { return m_radiusArea; }
    void setRadiusArea(float radius) { m_radiusArea = radius; }
    int timeParticle() const { return m_timeParticle; }
    void setTimeParticle(int time) { m_timeParticle = time; }
    float velocityParticle() const { return m_velocityParticle; }
    void setVelocityParticle(float velocity) { m_velocityParticle = velocity; }
    float deltaVelocityParticle() const { return m_deltaVelocityParticle; }
    void setDeltaVelocityParticle(float deltaVelocity) { m_deltaVelocityParticle = deltaVelocity; }

    void update();

private:
    SunParticleList* m_listParticles;
    QScrollEngine::QSprite* m_original[4];
    QScrollEngine::QEntity* m_parentEntity;
    int m_countParticles;
    int m_timeParticle;
    float m_scaleParticle;
    float m_deltaScaleParticle;
    float m_radiusArea;
    float m_speedLife;
    float m_velocityParticle;
    float m_deltaVelocityParticle;
    float m_dumpVelocity;
};

class ARSceneSpace:
        public ARScene
{
    Q_OBJECT
protected:
    void initScene() override;

    void _beginUpdate() override;
    void _endUpdate() override;

    inline static float mod(float a, float b)
    {
        while (a > b)
            a -= b;
        while (a < 0.0f)
            a += b;
        return a;
    }

private:
    struct TurnedObject
    {
        QScrollEngine::QEntity* entity;
        QQuaternion turn;
    };

    std::vector<TurnedObject> m_turnedObjects;
    QScrollEngine::QPlanarShadows* m_planarShadows;
    QScrollEngine::QLight* m_sunLight;
    QScrollEngine::QLight* m_sceneLight;
    QScrollEngine::QEntity* m_solarSystem;
    SunParticleSystem m_sunParticleSystem;

    void _createOrbitPart(QScrollEngine::QEntity::Part* part, float planetRadius, float orbitRadius,
                          size_t countVertices, const QColor& color);
    void _updateTurnedObjects();
};

#endif // ARSCENESPACE_H
