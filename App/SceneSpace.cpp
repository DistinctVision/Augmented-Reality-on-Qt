#include "App/SceneSpace.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <random>
#include <cfloat>
#include <cmath>
#include <QDebug>
#include "QScrollEngine/QEntity.h"

int OrbitShader::locationMatrixWVP;
int OrbitShader::locationTime;
int OrbitShader::locationColorA;
int OrbitShader::locationColorB;

bool OrbitShader::use(QScrollEngine::QScrollEngineContext*, QOpenGLShaderProgram *program)
{
    QSH_ASSERT(_sceneObject != nullptr);
    program->setUniformValue(locationMatrixWVP, _sceneObject->matrixWorldViewProj());
    program->setUniformValue(locationTime, _time / _maxTime);
    program->setUniformValue(locationColorA, _colorA);
    program->setUniformValue(locationColorB, _colorB);
    return true;
}

void OrbitShader::load(QScrollEngine::QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders)
{
    shaders.resize(1);
    shaders[0] = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram);
    const char* vertexShaderText =
            "#ifdef GL_ES\n"
            "precision mediump int;\n"
            "precision mediump float;\n"
            "#endif\n"

            "uniform mat4 matrix_wvp;\n"
            "uniform float time;\n"

            "attribute vec4 vertex_position;\n"
            "attribute vec2 vertex_texcoord;\n"
            "varying float v_time;\n"
            "void main()\n"
            "{\n"
            "    float t = vertex_texcoord.x + time;\n"
            "    v_time = t - ceil(t);\n"
            "    gl_Position = matrix_wvp * vertex_position;\n"
            "}";
    QOpenGLShader* vertexShader = new QOpenGLShader(QOpenGLShader::Vertex, shaders[0].data());
    vertexShader->compileSourceCode(vertexShaderText);
    shaders[0]->addShader(vertexShader);
    const char* fragmenShaderText =
            "#ifdef GL_ES\n"
            "precision mediump int;\n"
            "precision mediump float;\n"
            "#endif\n"

            "uniform vec4 colorA;\n"
            "uniform vec4 colorB;\n"

            "varying float v_time;\n"

            "void main()\n"
            "{\n"
            "    gl_FragColor = colorA * (1.0 - v_time) + colorB * v_time;\n"
            "}";
    QOpenGLShader* fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, shaders[0].data());
    fragmentShader->compileSourceCode(fragmenShaderText);
    shaders[0]->addShader(fragmentShader);
    if (!shaders[0]->link()) {
        qDebug() << "Error of link orbit shader.";
        return;
    }
    shaders[0]->bindAttributeLocation("vertex_position", 0);
    shaders[0]->bindAttributeLocation("vertex_texcoord", 1);
    if (!context->checkBindShader(shaders[0].data(), "Orbit Shader"))
        return;
    OrbitShader::locationMatrixWVP = shaders[0]->uniformLocation("matrix_wvp");
    OrbitShader::locationTime = shaders[0]->uniformLocation("time");
    OrbitShader::locationColorA = shaders[0]->uniformLocation("colorA");
    OrbitShader::locationColorB = shaders[0]->uniformLocation("colorB");
}

SunParticleSystem::SunParticleSystem()
{
    for (int i=0; i<countOriginalsParticles(); ++i)
        _original[i] = nullptr;
    _parentEntity = nullptr;
    _timeParticle = 10;
    _listParticles = nullptr;
    _countParticles = 20;
    _scaleParticle = 0.3f;
    _deltaScaleParticle = 0.2f;
    _radiusArea = 1.0f;
    _speedLife = 0.1f;
    _velocityParticle = 0.005f;
    _deltaVelocityParticle = 0.0025f;
    _dumpVelocity = 0.998f;
}

void SunParticleSystem::update()
{
    using namespace QScrollEngine;
    qsrand(QTime::currentTime().msec());
    QSh_ColoredPart* shader;
    int countParticles = 0;
    SunParticleList* prev = nullptr;
    SunParticleList* cur = _listParticles;
    while (cur) {
        if (cur->p.time <= 0) {
            if (cur->p.lifeLimit > 0.0f) {
                cur->p.lifeLimit = std::max(cur->p.lifeLimit - _speedLife, 0.0f);
            } else {
                if (prev) {
                    prev->next = cur->next;
                    delete cur->p.sprite;
                    delete cur;
                    cur = prev->next;
                } else {
                    _listParticles = cur->next;
                    delete cur;
                    cur = _listParticles;
                }
                continue;
            }
        } else {
            if (cur->p.time == _timeParticle) {
                if (cur->p.lifeLimit >= 1.0f)
                    --cur->p.time;
                else
                    cur->p.lifeLimit = std::min(cur->p.lifeLimit + _speedLife, 1.0f);
            } else {
                --cur->p.time;
            }
        }
        shader = static_cast<QSh_ColoredPart*>(cur->p.sprite->shader());
        shader->setLimit(1.0f - cur->p.lifeLimit * 0.5f);
        cur->p.velocity = cur->p.velocity * _dumpVelocity;
        cur->p.sprite->setPosition(cur->p.sprite->position() + cur->p.velocity);
        cur->p.sprite->setAngle(cur->p.sprite->angle() + cur->p.angularVelocity);
        prev = cur;
        cur = cur->next;
        ++countParticles;
    }
    if (_parentEntity == nullptr)
        return;
    if (_original == nullptr)
        return;
    countParticles = _countParticles - countParticles;
    if (countParticles <= 0)
        return;
    if (countParticles > 1)
        countParticles = 1;
    for (int i=0; i<countParticles; ++i) {
        cur = new SunParticleList;
        cur->next = _listParticles;
        _listParticles = cur;
        cur->p.sprite = _original[qrand() % countOriginalsParticles()]->copy();
        cur->p.sprite->setParentEntity(_parentEntity);
        float scale = _scaleParticle + ((qrand() % 100) * 0.01f * _deltaScaleParticle);
        cur->p.sprite->setScale(scale);
        QVector3D dir = (QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, static_cast<float>(qrand() % 360)) *
                QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, static_cast<float>(qrand() % 360))).rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
        dir.normalize();
        cur->p.sprite->setPosition(dir * (_radiusArea + scale * 0.5f));
        cur->p.velocity = dir * (_velocityParticle + (qrand() % 100) * 0.01f * _deltaVelocityParticle);
        cur->p.angularVelocity = static_cast<float>(((qrand() % 100) * 0.01f * 360.0f - 180.0f) * 0.1f);
        cur->p.lifeLimit = 0.0f;
        cur->p.time = _timeParticle;
    }
}

SceneSpace::SceneSpace(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    setAmbientColor(70, 70, 70);
    context->glLineWidth(1.5f);
    scale = QVector3D(0.6667f, 0.6667f, 0.6667f);
    OrbitShader orbitShaderSample;
    context->registerShader(&orbitShaderSample);
    context->setPostEffectUsed(false);
    _planarShadows.setScene(this);
    _planarShadows.setColorShadows(QColor(0, 0, 0, 55));
    _planarShadows.setAlpha(true);
    _planarShadows.setPlanePos(QVector3D(0.0f, 0.0f, 0.0f));
    _planarShadows.setPlaneDir(QVector3D(0.0f, 0.0f, 1.0f));

    _solarSystem = new QScrollEngine::QEntity(this);
    _planarShadows.addEntity(_solarSystem);

    _sunLight = new QLight(_solarSystem);
    _sunLight->setLightSoft(0.25f);
    _sunLight->setPosition(0.0f, 0.0f, 0.0f);
    _sunLight->setRadius(100.0f);
    _sceneLight = new QLight();
    _sceneLight->setPosition(0.0f, 0.0f, 100.0f);
    _planarShadows.setLight(_sceneLight);

    float earthAngularVelocity = static_cast<float>(2.0f * M_PI * 0.003f);

    QSprite* layer;
    TurnedTextureSphereShader sphere_texture;
    TurnedLightSphereShader sphere_light;

    QEntity::QPartEntity* partEntity;

    QMesh* meshSphere = new QMesh(context);
    QGLPrimitiv::createSphere(meshSphere, 0.5f, 16, 16);

    //Sun
    SpaceObject spaceObject;
    QEntity* sunEntity = new QEntity(_solarSystem);
    sunEntity->setName("Sun");
    sunEntity->setPosition(0.0f, 0.0f, 2.0f);
    _sunLight->setParentEntity(sunEntity);
    spaceObject.entity = new QEntity(sunEntity);
    spaceObject.entity->addPart(meshSphere, nullptr);
    spaceObject.entity->setScale(2.0f);
    spaceObject.orbitAngle = 0.0f;
    spaceObject.orbitAngularVelocity = 0.0f;
    layer = new QSprite(spaceObject.entity);
    layer->setShader(new QSh_Texture1(context->loadTexture("Glow", ":/GameData/Glow.png"), QColor(255, 255, 0, 255)));
    layer->setScale(4.2f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    layer = new QSprite(spaceObject.entity);
    sphere_texture.shader = new QSh_Sphere_Texture1(1.0f, QQuaternion(), context->loadTexture("SunSurface", ":/GameData/Sun.jpg"));
    sphere_texture.axis = QVector3D(0.0f, 0.0f, 1.0f);
    sphere_texture.angle = 0.0f;
    sphere_texture.angularVelocity = static_cast<float>(2.0f * M_PI * 0.01f);
    _turnedTexturedSphereShaders.push_back(sphere_texture);
    layer->setShader(sphere_texture.shader);
    layer->setScale(2.0f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.05f);
    _spaceObjects.push_back(spaceObject);
    //Sun Particles
    _sunParticleSystem.setParentEntity(sunEntity);
    _sunParticleSystem.setRadiusArea(1.0f);
    _sunParticleSystem.setOriginalParticle(0, new QSprite());
    _sunParticleSystem.originalParticle(0)->setShader(new QSh_ColoredPart(context->loadTexture("SmoothParticle_0",
                                                                                               ":/GameData/SmokeParticle_0.jpg"),
                                                                          QColor(255, 100, 0, 0), QColor(255, 255, 100, 150), 1.0f, 0.5f));
    _sunParticleSystem.originalParticle(0)->setAlpha(true);
    _sunParticleSystem.setOriginalParticle(1, new QSprite());
    _sunParticleSystem.originalParticle(1)->setShader(new QSh_ColoredPart(context->loadTexture("SmoothParticle_1",
                                                                                               ":/GameData/SmokeParticle_1.jpg"),
                                                                          QColor(255, 100, 0, 0), QColor(255, 255, 100, 150), 1.0f, 0.5f));
    _sunParticleSystem.originalParticle(1)->setAlpha(true);
    _sunParticleSystem.setOriginalParticle(2, new QSprite());
    _sunParticleSystem.originalParticle(2)->setShader(new QSh_ColoredPart(context->loadTexture("SmoothParticle_2",
                                                                                               ":/GameData/SmokeParticle_2.jpg"),
                                                                          QColor(255, 100, 0, 0), QColor(255, 255, 100, 150), 1.0f, 0.5f));
    _sunParticleSystem.originalParticle(2)->setAlpha(true);
    _sunParticleSystem.setOriginalParticle(3, new QSprite());
    _sunParticleSystem.originalParticle(3)->setShader(new QSh_ColoredPart(context->loadTexture("SmoothParticle_3",
                                                                                               ":/GameData/SmokeParticle_3.jpg"),
                                                                          QColor(255, 100, 0, 0), QColor(255, 255, 100, 150), 1.0f, 0.5f));
    _sunParticleSystem.originalParticle(3)->setAlpha(true);

    //Mercury
    spaceObject.orbitAngle = 0.0f;
    spaceObject.orbitAngularVelocity = earthAngularVelocity * 4.152056f;
    QEntity* mercuryEntity = new QEntity(sunEntity);
    mercuryEntity->setName("Mercury");
    mercuryEntity->setScale(1.0f);
    spaceObject.entity = new QEntity(mercuryEntity);
    spaceObject.entity->setScale(0.8f);
    spaceObject.entity->addPart(meshSphere, nullptr);
    partEntity = mercuryEntity->addPart(nullptr, nullptr);
    _createOrbitPart(partEntity, 0.4f, 2.0f, 36,
                     QColor(155, 100, 55, 255), QColor(255, 255, 255, 255));
    spaceObject.entity->setPosition(2.0f, 0.0f, 0.0);
    layer = new QSprite(spaceObject.entity);
    layer->setShader(new QSh_Light(context->texture("Glow"), QColor(155, 100, 55, 255)));
    layer->setScale(1.5f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.4f, QQuaternion(), context->loadTexture("MercurySurface", ":/GameData/Mercury.jpg"),
                                               QColor(255, 255, 255, 255), 1.0f, 0.5f, 0.5f);
    sphere_light.axis = QVector3D(0.0f, 0.0f, 1.0f);
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.014f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(0.9f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.025f);
    _spaceObjects.push_back(spaceObject);

    //Venus
    spaceObject.orbitAngle = 0.0f;
    spaceObject.orbitAngularVelocity = earthAngularVelocity * 1.625518f;
    QEntity* venusEntity = new QEntity(sunEntity);
    venusEntity->setName("Venus");
    venusEntity->setScale(1.0f);
    spaceObject.entity = new QEntity(venusEntity);
    spaceObject.entity->setScale(1.0f);
    spaceObject.entity->addPart(meshSphere, nullptr);
    partEntity = venusEntity->addPart(nullptr, nullptr);
    _createOrbitPart(partEntity, 0.5f, 3.0f, 36,
                     QColor(255, 120, 100, 255), QColor(255, 255, 255, 255));
    spaceObject.entity->setPosition(3.0f, 0.0f, 0.0);
    layer = new QSprite(spaceObject.entity);
    layer->setShader(new QSh_Light(context->texture("Glow"), QColor(255, 200, 155, 255)));
    layer->setScale(1.8f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.5f, QQuaternion(), context->loadTexture("VenusSurface", ":/GameData/Venus.jpg"),
                                               QColor(255, 255, 255, 255), 1.0f, 0.5f, 1.0f);
    sphere_light.axis = QVector3D(0.0f, 0.0f, 1.0f);
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.04f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(1.1f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.025f);
    _spaceObjects.push_back(spaceObject);

    //Earth
    spaceObject.orbitAngle = 0.0f;
    spaceObject.orbitAngularVelocity = earthAngularVelocity;
    QEntity* earthEntity = new QEntity(sunEntity);
    earthEntity->setName("Earth");
    earthEntity->setScale(1.0f);
    spaceObject.entity = new QEntity(earthEntity);
    spaceObject.entity->setScale(1.0f);
    spaceObject.entity->addPart(meshSphere, nullptr);
    partEntity = earthEntity->addPart(nullptr, nullptr);
    _createOrbitPart(partEntity, 0.5f, 4.5f, 42,
                     QColor(50, 120, 255, 255), QColor(255, 255, 255, 255));
    spaceObject.entity->setPosition(4.5f, 0.0f, 0.0);
    layer = new QSprite(spaceObject.entity);
    layer->setShader(new QSh_Light(context->texture("Glow"), QColor(100, 180, 255, 255)));
    layer->setScale(2.0f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.5f, QQuaternion(), nullptr,
                                               QColor(55, 145, 255, 255), 1.0f, 1.5f, 10.0f);
    sphere_light.axis = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 7.155f).rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.01f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(1.1f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.025f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.5f, QQuaternion(), context->loadTexture("EarthSurface", ":/GameData/Earth.png"),
                                               QColor(255, 255, 255, 255), 1.0f, 0.0f, 1.0f);
    //sphere_light.axis = QVector3D(0.0f, 0.0f, 1.0f);
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.01f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(1.1f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.05f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.55f, QQuaternion(), context->loadTexture("EarthCloud", ":/GameData/EarthCloud.png"),
                                               QColor(255, 255, 255, 255), 1.0f, 0.0f, 1.0f);
    //sphere_light.axis = QVector3D(0.0f, 0.0f, 1.0f);
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.015f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(1.2f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.075f);
    _spaceObjects.push_back(spaceObject);

    //Mars
    spaceObject.orbitAngle = 0.0f;
    spaceObject.orbitAngularVelocity = earthAngularVelocity * 0.53191489361f;
    QEntity* marsEntity = new QEntity(sunEntity);
    marsEntity->setName("Mars");
    marsEntity->setScale(1.0f);
    spaceObject.entity = new QEntity(marsEntity);
    spaceObject.entity->setScale(1.0f);
    spaceObject.entity->addPart(meshSphere, nullptr);
    partEntity = marsEntity->addPart(nullptr, nullptr);
    _createOrbitPart(partEntity, 0.4f, 5.5f, 48,
                     QColor(255, 100, 0, 255), QColor(255, 255, 255, 255));
    spaceObject.entity->setPosition(5.5f, 0.0f, 0.0);
    layer = new QSprite(spaceObject.entity);
    layer->setShader(new QSh_Light(context->texture("Glow"), QColor(255, 100, 55, 255)));
    layer->setScale(1.8f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.0f);
    layer = new QSprite(spaceObject.entity);
    sphere_light.shader = new QSh_Sphere_Light(0.4f, QQuaternion(), context->loadTexture("MarsSurface", ":/GameData/Mars.jpg"),
                                               QColor(255, 255, 255, 255), 1.0f, 0.5f, 1.0f);
    sphere_light.axis = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 5.65f).rotatedVector(QVector3D(0.0f, 0.0f, 1.0f));
    sphere_light.angle = 0.0f;
    sphere_light.angularVelocity = static_cast<float>(2.0f * M_PI * 0.012f);
    _turnedLightSphereShaders.push_back(sphere_light);
    layer->setShader(sphere_light.shader);
    layer->setScale(0.9f);
    layer->setAlpha(true);
    layer->setPosition(0.0f, 0.0f, 0.025f);
    _spaceObjects.push_back(spaceObject);

    _needUpdateScene = false;
    _thread = new QThread(this);
    connect(_thread, &QThread::started, this, &SceneSpace::updateSceneThread);
    //_thread->start();
}

SceneSpace::~SceneSpace()
{
    delete _sceneLight;
    delete _thread;
}

void SceneSpace::_createOrbitPart(QScrollEngine::QEntity::QPartEntity* part, float planetRadius, float orbitRadius, unsigned int countVertices,
                                  const QColor& colorA, const QColor& colorB)
{
    using namespace QScrollEngine;
    QVector2D intersect;
    intersect.setX((2.0f * orbitRadius * orbitRadius - planetRadius * planetRadius) / (2.0f * orbitRadius));
    if (intersect.x() >= orbitRadius)
        return;
    intersect.setY(qSqrt(orbitRadius * orbitRadius - intersect.x() * intersect.x()));
    float halfDeltaAngle = qAtan2(intersect.y(), intersect.x()) * 0.999f;
    float sector = static_cast<float>(2.0f * (M_PI - halfDeltaAngle));
    QMesh* mesh = new QMesh(parentContext());
    mesh->setCountVertices(countVertices + 1);
    float step = 1.0f / static_cast<float>(countVertices);
    mesh->setSizeOfELement(1);
    mesh->setCountPoints(countVertices + 1);
    for (unsigned int i=0; i<=countVertices; ++i) {
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
    part->setShader(new OrbitShader(0.0f, static_cast<float>(2.0f * M_PI), colorA, colorB));
    part->setDrawMode(GL_LINE_STRIP);
}

void SceneSpace::_updatePlanets()
{
    using namespace QScrollEngine;
    updateCameraInfo(parentContext()->camera);
    QQuaternion globalOrientation;
    QVector3D globalScale;
    QQuaternion orientation = cameraOrientation();
    //OrbitShader* shader;
    for (std::vector<SpaceObject>::iterator it = _spaceObjects.begin(); it != _spaceObjects.end(); ++it) {
        QEntity* entity = it->entity;
        QEntity* parentEntity = entity->parentEntity();
        it->orbitAngle = SceneShip::mod(it->orbitAngle + it->orbitAngularVelocity, static_cast<float>(2.0f * M_PI));
        parentEntity->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, static_cast<float>(it->orbitAngle * (180.0f / M_PI))));
        parentEntity->updateTransform();
        parentEntity->getGlobalParameters(globalScale, globalOrientation);
        entity->setOrientation(globalOrientation.conjugate() * orientation);
        /*for (int i=0; i<entity->countParts(); ++i) {
            shader = dynamic_cast<OrbitShader*>(entity->part(i));
            if (shader)
                shader->setTime(it->orbitAngle);
        }*/
    }
    for (std::vector<TurnedTextureSphereShader>::iterator it = _turnedTexturedSphereShaders.begin();
         it != _turnedTexturedSphereShaders.end();
         ++it) {
        it->angle = SceneShip::mod(it->angle + it->angularVelocity, static_cast<float>(2.0f * M_PI));
        it->shader->setSphereOrientation(QQuaternion::fromAxisAndAngle(it->axis, it->angle * static_cast<float>((180.0f / M_PI))));
    }
    for (std::vector<TurnedLightSphereShader>::iterator it = _turnedLightSphereShaders.begin();
         it != _turnedLightSphereShaders.end();
         ++it) {
        it->angle = SceneShip::mod(it->angle + it->angularVelocity, static_cast<float>(2.0f * M_PI));
        it->shader->setSphereOrientation(QQuaternion::fromAxisAndAngle(it->axis, it->angle * static_cast<float>((180.0f / M_PI))));
    }
}

void SceneSpace::beginUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    _needUpdateScene = true;
}

void SceneSpace::endUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    parentContext()->lock();
    _updatePlanets();
    _sunParticleSystem.update();
    parentContext()->unlock();
}

void SceneSpace::updateSceneThread()
{
    _mutex.lock();
    if (_needUpdateScene) {
        _needUpdateScene = false;
    }
    _mutex.unlock();
    QTimer::singleShot(2, this, &SceneSpace::updateSceneThread);
}
