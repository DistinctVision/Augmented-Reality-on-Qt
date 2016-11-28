#include "ARSceneSpace.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <QTime>
#include <random>
#include <cfloat>
#include <cmath>
#include "QScrollEngine/QEntity.h"

SunParticleSystem::SunParticleSystem()
{
    for (int i=0; i<countOriginalsParticles(); ++i)
        m_original[i] = nullptr;
    m_parentEntity = nullptr;
    m_timeParticle = 10;
    m_listParticles = nullptr;
    m_countParticles = 20;
    m_scaleParticle = 0.3f;
    m_deltaScaleParticle = 0.2f;
    m_radiusArea = 1.0f;
    m_speedLife = 0.1f;
    m_velocityParticle = 0.005f;
    m_deltaVelocityParticle = 0.0025f;
    m_dumpVelocity = 0.998f;
}

void SunParticleSystem::update()
{
    using namespace QScrollEngine;
    qsrand(QTime::currentTime().msec());
    QSh_ColoredPart* shader;
    int countParticles = 0;
    SunParticleList* prev = nullptr;
    SunParticleList* cur = m_listParticles;
    while (cur) {
        if (cur->p.time <= 0) {
            if (cur->p.lifeLimit > 0.0f) {
                cur->p.lifeLimit = std::max(cur->p.lifeLimit - m_speedLife, 0.0f);
            } else {
                if (prev) {
                    prev->next = cur->next;
                    delete cur->p.sprite;
                    delete cur;
                    cur = prev->next;
                } else {
                    m_listParticles = cur->next;
                    delete cur;
                    cur = m_listParticles;
                }
                continue;
            }
        } else {
            if (cur->p.time == m_timeParticle) {
                if (cur->p.lifeLimit >= 1.0f)
                    --cur->p.time;
                else
                    cur->p.lifeLimit = std::min(cur->p.lifeLimit + m_speedLife, 1.0f);
            } else {
                --cur->p.time;
            }
        }
        shader = static_cast<QSh_ColoredPart*>(cur->p.sprite->shader().data());
        shader->setLimit(1.0f - cur->p.lifeLimit * 0.5f);
        cur->p.velocity = cur->p.velocity * m_dumpVelocity;
        cur->p.sprite->setPosition(cur->p.sprite->position() + cur->p.velocity);
        cur->p.sprite->setAngle(cur->p.sprite->angle() + cur->p.angularVelocity);
        prev = cur;
        cur = cur->next;
        ++countParticles;
    }
    if (m_parentEntity == nullptr)
        return;
    if (m_original == nullptr)
        return;
    countParticles = m_countParticles - countParticles;
    if (countParticles <= 0)
        return;
    if (countParticles > 1)
        countParticles = 1;
    for (int i=0; i<countParticles; ++i) {
        cur = new SunParticleList;
        cur->next = m_listParticles;
        m_listParticles = cur;
        cur->p.sprite = m_original[qrand() % countOriginalsParticles()]->copy();
        cur->p.sprite->setParentEntity(m_parentEntity);
        float scale = m_scaleParticle + ((qrand() % 100) * 0.01f * m_deltaScaleParticle);
        cur->p.sprite->setScale(scale);
        QVector3D dir = (QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, (float)(qrand() % 360)) *
                QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, (float)(qrand() % 360))).rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
        dir.normalize();
        cur->p.sprite->setPosition(dir * (m_radiusArea + scale * 0.5f));
        cur->p.velocity = dir * (m_velocityParticle + (qrand() % 100) * 0.01f * m_deltaVelocityParticle);
        cur->p.angularVelocity = (float)(((qrand() % 100) * 0.01f * 360.0f - 180.0f) * 0.1f);
        cur->p.lifeLimit = 0.0f;
        cur->p.time = m_timeParticle;
    }
}

void ARSceneSpace::initScene()
{
    using namespace QScrollEngine;

    scene()->setAmbientColor(70, 70, 70);

    QScrollEngineContext* context = scene()->parentContext();

    m_planarShadows = new QPlanarShadows(scene());

    context->glLineWidth(1.5f);
    scene()->scale = QVector3D(0.6667f, 0.6667f, 0.6667f);
    m_planarShadows->setScene(scene());
    m_planarShadows->setColorShadows(QColor(0, 0, 0, 55));
    m_planarShadows->setAlpha(true);
    m_planarShadows->setPlanePos(QVector3D(0.0f, 0.0f, 0.0f));
    m_planarShadows->setPlaneDir(QVector3D(0.0f, 0.0f, 1.0f));

    m_solarSystem = new QEntity(scene());
    m_planarShadows->addEntity(m_solarSystem);

    m_sunLight = new QLight(m_solarSystem);
    m_sunLight->setLightSoft(0.25f);
    m_sunLight->setPosition(0.0f, 0.0f, 0.0f);
    m_sunLight->setRadius(100.0f);
    m_sceneLight = new QLight();
    m_sceneLight->setPosition(0.0f, 0.0f, 100.0f);
    m_planarShadows->setLight(m_sceneLight);

    float earthAngularVelocity = (float)(2.0f * M_PI * 0.06f);

    QSprite* layer;

    QEntity::Part* partEntity;

    QMesh* meshSphere = new QMesh(context);
    QGLPrimitiv::createSphere(meshSphere, 0.5f, 24, 24);
    QMatrix4x4 m;
    m.rotate(90.0f, -1.0f, 0.0f, 0.0f);
    meshSphere->transformVertices(m);
    meshSphere->transformNormals(m.normalMatrix());
    meshSphere->applyChanges();

    //Sun
    QEntity* sunMain = new QEntity(m_solarSystem);
    sunMain->setPosition(0.0f, 0.0f, 2.0f);
    QEntity* sunEntity = new QEntity(sunMain);
    sunEntity->setName("Sun");
    m_sunLight->setParentEntity(sunEntity);
    QOpenGLTexture* sunTexture = context->texture("SunSurface");
    if (sunTexture == nullptr)
        sunTexture = context->loadTexture("SunSurface", ":/Data/Sun.jpg");
    sunEntity->addPart(meshSphere, QShPtr(new QSh_Texture(sunTexture)));
    sunEntity->setScale(2.0f);
    layer = new QSprite(sunEntity);

    QOpenGLTexture* textureGlow = context->texture("Glow");
    if (textureGlow == nullptr)
        textureGlow = context->loadTexture("Glow", ":/Data/Glow.png");
    layer->setShader(QShPtr(new QSh_Texture(textureGlow, QColor(255, 255, 0, 255))));
    layer->setScale(4.2f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    //Sun Particles
    m_sunParticleSystem.setParentEntity(sunEntity);
    m_sunParticleSystem.setRadiusArea(0.6f);
    for (int i = 0; i < 4; ++i) {
        m_sunParticleSystem.setOriginalParticle(i, new QSprite());
        QString textureName = "SmokeParticle_" + QString::number(i);
        QOpenGLTexture* smokeParticleTexture = context->texture(textureName);
        if (smokeParticleTexture == nullptr)
            smokeParticleTexture = context->loadTexture(textureName, ":/Data/" + textureName + ".jpg");
        m_sunParticleSystem.originalParticle(i)->setShader(QShPtr(new QSh_ColoredPart(smokeParticleTexture,
                                                           QColor(255, 100, 0, 0), QColor(255, 255, 100, 150), 1.0f, 0.5f)));
        m_sunParticleSystem.originalParticle(i)->setAlpha(true);
    }
    m_turnedObjects.push_back({ sunEntity, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 1.0f) });

    //Mercury
    QEntity* mercuryMain = new QEntity(sunMain);
    QEntity* mercuryEntity = new QEntity(mercuryMain);
    mercuryEntity->setName("Mercury");
    mercuryEntity->setScale(0.8f);
    QOpenGLTexture* textureMercury = context->texture("MercurySurface");
    if (textureMercury == nullptr)
        textureMercury = context->loadTexture("MercurySurface", ":/Data/Mercury.jpg");
    mercuryEntity->addPart(meshSphere, QShPtr(new QSh_Light(textureMercury)));
    partEntity = mercuryMain->addPart();
    _createOrbitPart(partEntity, 0.4f, 2.0f, 36, QColor(155, 100, 55, 155));
    mercuryEntity->setPosition(2.0f, 0.0f, 0.0);
    layer = new QSprite(mercuryEntity);
    layer->setShader(QShPtr(new QSh_Texture(textureGlow, QColor(155, 100, 55, 255))));
    layer->setScale(1.6f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    m_turnedObjects.push_back({ mercuryMain, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, earthAngularVelocity * 4.152056f) });
    m_turnedObjects.push_back({ mercuryEntity, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 14.0f) });

    //Venus
    QEntity* venusMain = new QEntity(sunMain);
    QEntity* venusEntity = new QEntity(venusMain);
    venusEntity->setName("Venus");
    venusEntity->setScale(1.0f);
    QOpenGLTexture* textureVenus = context->texture("VenusSurface");
    if (textureVenus == nullptr)
        textureVenus = context->loadTexture("VenusSurface", ":/Data/Venus.jpg");
    venusEntity->addPart(meshSphere, QShPtr(new QSh_Light(textureVenus)));
    partEntity = venusMain->addPart();
    _createOrbitPart(partEntity, 0.5f, 3.0f, 36, QColor(255, 120, 100, 255));
    venusEntity->setPosition(3.0f, 0.0f, 0.0);
    layer = new QSprite(venusEntity);
    layer->setShader(QShPtr(new QSh_Texture(textureGlow, QColor(255, 200, 155, 255))));
    layer->setScale(1.8f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    m_turnedObjects.push_back({ venusMain, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, earthAngularVelocity * 1.625518f) });
    m_turnedObjects.push_back({ venusEntity, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 3.0f) });

    //Earth
    QEntity* earthMain = new QEntity(sunMain);
    QEntity* earthEntity = new QEntity(earthMain);
    earthEntity->setName("Earth");
    earthEntity->setScale(1.0f);
    earthEntity->setScale(1.0f);
    earthEntity->setPosition(4.5f, 0.0f, 0.0);
    partEntity = earthMain->addPart();
    _createOrbitPart(partEntity, 0.5f, 4.5f, 42, QColor(50, 120, 255, 255));
    layer = new QSprite(earthEntity);
    layer->setShader(QShPtr(new QSh_Texture(textureGlow, QColor(100, 180, 255, 255))));
    layer->setScale(2.0f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    earthEntity->addPart(meshSphere, QShPtr(new QSh_Light(nullptr, QColor(55, 145, 255, 255), 1.5f, 10.0f)));
    QOpenGLTexture* textureEarth = context->texture("EarthSurface");
    if (textureEarth == nullptr)
        textureEarth = context->loadTexture("EarthSurface", ":/Data/Earth.png");
    QEntity* earthGround = new QEntity(earthEntity);
    earthGround->setScale(1.01f);
    earthGround->addPart(meshSphere, QShPtr(new QSh_Light(textureEarth)))->setAlpha(true);
    QEntity* earthCloud = new QEntity(earthEntity);
    earthCloud->setScale(1.15f);
    QOpenGLTexture* textureEarthCloud = context->texture("EarthCloud");
    if (textureEarthCloud == nullptr)
        textureEarthCloud = context->loadTexture("EarthCloud", ":/Data/EarthCloud.png");
    earthCloud->addPart(meshSphere, QShPtr(new QSh_Texture(textureEarthCloud)), false)->setAlpha(true);
    earthCloud->part(0)->mesh()->setLocalBoundingBox(earthCloud->part(0)->mesh()->localBoundingBox().expanded(3.0f));
    m_turnedObjects.push_back({ earthMain, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, earthAngularVelocity) });
    m_turnedObjects.push_back({ earthEntity, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 2.3f) });
    m_turnedObjects.push_back({ earthCloud, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 3.0f) });

    //Mars
    QEntity* marsMain = new QEntity(sunMain);
    QEntity* marsEntity = new QEntity(marsMain);
    marsEntity->setName("Mars");
    marsEntity->setScale(0.9f);
    partEntity = marsMain->addPart();
    _createOrbitPart(partEntity, 0.45f, 5.5f, 48, QColor(255, 100, 0, 255));
    marsEntity->setPosition(5.5f, 0.0f, 0.0);
    layer = new QSprite(marsEntity);
    layer->setShader(QShPtr(new QSh_Texture(textureGlow, QColor(255, 100, 55, 255))));
    layer->setScale(1.6f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    QOpenGLTexture* textureMars = context->texture("MarsSurface");
    if (textureMars == nullptr)
        textureMars = context->loadTexture("MarsSurface", ":/Data/Mars.jpg");
    marsEntity->addPart(meshSphere, QShPtr(new QSh_Light(textureMars)));
    m_turnedObjects.push_back({ marsMain, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, earthAngularVelocity * 0.53191489361f) });
    m_turnedObjects.push_back({ marsEntity, QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 1.5f) });
}

void ARSceneSpace::_createOrbitPart(QScrollEngine::QEntity::Part* part, float planetRadius, float orbitRadius,
                                    std::size_t countVertices, const QColor& color)
{
    using namespace QScrollEngine;
    QVector2D intersect;
    intersect.setX((2.0f * orbitRadius * orbitRadius - planetRadius * planetRadius) / (2.0f * orbitRadius));
    if (intersect.x() >= orbitRadius)
        return;
    intersect.setY(qSqrt(orbitRadius * orbitRadius - intersect.x() * intersect.x()));
    float halfDeltaAngle = qAtan2(intersect.y(), intersect.x()) * 0.999f;
    float sector = static_cast<float>(2.0f * (M_PI - halfDeltaAngle));
    QMesh* mesh = new QMesh(scene()->parentContext());
    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->setCountVertices(countVertices + 1);
    float step = 1.0f / (float)(countVertices);
    mesh->setSizeOfELement(1);
    mesh->setCountPoints(countVertices + 1);
    for (std::size_t i=0; i<=countVertices; ++i) {
        float k = i * step;
        float angle = halfDeltaAngle + k * sector;
        mesh->setVertexPosition(i, QVector3D(qCos(angle) * orbitRadius, qSin(angle) * orbitRadius, 0.0f));
        mesh->setVertexTextureCoord(i, QVector2D(k, 0.0f));
        mesh->setPoint(i, i);
    }
    mesh->setVertexTextureCoord(countVertices, mesh->vertexTextureCoord(countVertices) * 0.99f);
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    part->setMesh(mesh);
    part->setShader(QShPtr(new QSh_Color(color)));
    part->setDrawMode(GL_LINE_STRIP);
}

void ARSceneSpace::_updateTurnedObjects()
{
    using namespace QScrollEngine;
    for (std::vector<TurnedObject>::iterator it = m_turnedObjects.begin(); it != m_turnedObjects.end(); ++it) {
        it->entity->setOrientation(it->turn * it->entity->orientation());
    }
}

void ARSceneSpace::_beginUpdate()
{
    scene()->parentContext()->glLineWidth(1.5f);
}

void ARSceneSpace::_endUpdate()
{
    ARTracker* arTracker = this->arTracker();
    if (arTracker->trackingState() == ARTracker::Tracking) {
        scene()->setEnabled(true);
        _updateTurnedObjects();
        m_sunParticleSystem.update();
        return;
    }
    scene()->setEnabled(false);
}
