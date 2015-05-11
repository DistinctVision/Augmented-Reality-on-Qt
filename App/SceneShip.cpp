#include "App/SceneShip.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include <QTimer>
#include <cmath>

WaterField::WaterField()
{
    _radius = 3.0f;
    _time = 0.0f;
    _time_small = 0.0f;
    scaleWaves.extandRandom(0.5f);
    scaleWaves.extandRandom(1.5f);
    scaleWaves.setRandom();
    speedWaves.extandRandom(- 0.12f);
    speedWaves.extandRandom(- 0.2f);
    speedWaves.setRandom();
    heightWaves.extandRandom(0.3f);
    heightWaves.extandRandom(0.5f);
    heightWaves.setRandom();
    scaleWaves_small.extandRandom(2.0f);
    scaleWaves_small.extandRandom(4.0f);
    scaleWaves_small.setRandom();
    localSpeedWaves_small.extandRandom(-0.6f);
    localSpeedWaves_small.extandRandom(0.6f);
    localSpeedWaves_small.setRandom();
    heightWaves_small.extandRandom(0.0f);
    heightWaves_small.extandRandom(0.2f);
    heightWaves_small.setRandom();
    angleSource.extandRandom(0.0f);
    angleSource.extandRandom(static_cast<float>(2.0f * M_PI));
    angleSource.setMinPeriod(80);
    angleSource.setMaxPeriod(160);
    angleSource.setRandom();
    distanceSource.extandRandom(4.0f);
    distanceSource.extandRandom(10.0f);
    distanceSource.setRandom();
}

float WaterField::getWaveHeight(float x, float y) const
{
    QVector2D dir(x - _sourceWavesPosition.x(), y - _sourceWavesPosition.y());
    float distance = dir.length();
    float t = SceneShip::mod(distance * scaleWaves.value() + _time, static_cast<float>(2.0f * M_PI));
    return qSin(t) * heightWaves.value();
}

float WaterField::getSubWaveHeight(float x, float y) const
{
    float t_sX = SceneShip::mod(x * scaleWaves_small.value() + _time_small, static_cast<float>(2.0f * M_PI));
    float t_sY = SceneShip::mod(y * scaleWaves_small.value() + _time_small, static_cast<float>(2.0f * M_PI));
    return qSin(t_sX) * qCos(t_sY) * heightWaves_small.value();
}

float WaterField::value(const QVector3D& point)
{
    float a = _radius - point.length();
    float b = getWaveHeight(point.x(), point.y()) + getSubWaveHeight(point.x(), point.y()) - point.z();
    return qMin(a, b);
}

void WaterField::update()
{
    speedWaves.update();
    heightWaves.update();
    scaleWaves_small.update();
    localSpeedWaves_small.update();
    heightWaves_small.update();
    angleSource.update();
    distanceSource.update();
    _sourceWavesPosition.setX(qCos(angleSource.value()));
    _sourceWavesPosition.setY(qSin(angleSource.value()));
    _sourceWavesPosition *= distanceSource.value();
    _time = SceneShip::mod(_time + speedWaves.value(), static_cast<float>(2.0f * M_PI));
    _time_small = SceneShip::mod(_time_small + localSpeedWaves_small.value(), static_cast<float>(2.0f * M_PI));
    scaleWaves.update();
}

CloudParticleSystem::CloudParticleSystem()
{
    _original = nullptr;
    _parentEntity = nullptr;
    _timeParticle = 50;
    _listParticles = nullptr;
    _countParticles = 10;
    _scaleParticle = 0.4f;
    _deltaScaleParticle = 0.4f;
    _radiusArea = 3.0f;
    _heightArea = 2.0f;
    _deltaHeightArea = 0.2f;
    _speedLife = 0.1f;
    _dumpVelocity = 0.9f;
}

void CloudParticleSystem::update(const QVector2D& acceleration)
{
    using namespace QScrollEngine;
    qsrand(QTime::currentTime().msec());
    float createAngle = - std::atan2(acceleration.y(), acceleration.x());
    if (createAngle != createAngle)
        createAngle = static_cast<float>(((qrand() % 100) * 0.01f * 2.0f * M_PI));
    createAngle -= static_cast<float>(M_PI * 0.5f);
    QSh_ColoredPart* shader;
    int countParticles = 0;
    CloudParticleList* prev = nullptr;
    CloudParticleList* cur = _listParticles;
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
        cur->p.velocity = (cur->p.velocity + acceleration) * _dumpVelocity;
        cur->p.sprite->setPosition(cur->p.sprite->position() + QVector3D(cur->p.velocity.x(), cur->p.velocity.y(), 0.0f));
        cur->p.sprite->setAngle(cur->p.sprite->angle() + cur->p.angularVelocity);
        QVector3D position = cur->p.sprite->position();
        float distance = position.x() * position.x() + position.y() * position.y();
        float r = (cur->p.radiusOfArea - cur->p.scale);
        if (distance > r * r) {
            distance = r / qSqrt(distance);
            cur->p.time = 0;
            position.setX(position.x() * distance);
            position.setY(position.y() * distance);
            cur->p.sprite->setPosition(position);
        }
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
    if (countParticles > 2)
        countParticles = 2;
    for (int i=0; i<countParticles; ++i) {
        cur = new CloudParticleList;
        cur->next = _listParticles;
        _listParticles = cur;
        cur->p.sprite = _original->copy();
        cur->p.sprite->setParentEntity(_parentEntity);
        cur->p.scale = _scaleParticle + ((qrand() % 100) * 0.01f * _deltaScaleParticle);
        cur->p.sprite->setScale(cur->p.scale);
        float h = _heightArea + ((qrand() % 100) * 0.01f * _deltaHeightArea);
        cur->p.radiusOfArea = std::cos(std::asin(h / _radiusArea)) * _radiusArea;
        float d =  (cur->p.radiusOfArea - cur->p.scale);
        float a = static_cast<float>((qrand() % 100) * 0.01f * M_PI) + createAngle;
        cur->p.sprite->setPosition(std::cos(a) * d, std::sin(a) * d, h);
        cur->p.angularVelocity = static_cast<float>(((qrand() % 100) * 0.01f * 360.0f - 180.0f) * 0.1f);
        cur->p.lifeLimit = 0.0f;
        cur->p.time = _timeParticle;
    }
}

SceneShip::SceneShip(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    setAmbientColor(50, 50, 50);
    _mainEntity = new QEntity(this);
    _mainEntity->setName("Main");
    _mainEntity->setPosition(0.0f, 0.0f, 3.0f / 1.7f);
    _mainEntity->setScale(1.0f / 1.7f);
    QMesh* mesh = new QMesh(context);
    QGLPrimitiv::createSphere(mesh, _waterField.radius() + 0.2f, 16, 16);
    QEntity::QPartEntity* partEntity = _mainEntity->addPart(nullptr, nullptr);
    partEntity->setMesh(mesh);
    partEntity->setShader(new QSh_Light(nullptr, QColor(200, 200, 255, 40), 1.0f, 50.0f));
    partEntity->setAlpha(true);
    _mainEntity->addPart(partEntity);
    _ship = context->loadEntity(":/GameData/ship.3DS");
    _ship->setScale(0.01f);
    _shipAngle = 0.0f;
    _shipAngularVelocity = 0.01f;
    _shipSize = QVector2D(0.6f, 0.2f);
    _ship->setParentEntity(_mainEntity);
    /*_ship->convertShaders([] (QSh* shader) -> QSh* {
        if (shader == nullptr)
            return nullptr;
        if (shader->indexType() == QSh::Color) {
            QSceneObject3D* sceneObject = shader->sceneObject();
            if (sceneObject) {
                QScene* scene = sceneObject->scene();
                if (scene) {
                    QScrollEngineContext* context = scene->parentContext();
                    if (context) {
                        QSh_Color* shaderColor = dynamic_cast<QSh_Color*>(shader);
                        return new QSh_Light(context->emptyTexture(), shaderColor->color(), 1.0f, 10.0f);
                    }
                }
            }
        } else if (shader->indexType() == QSh::Texture1) {
            QSh_Texture1* shaderTexture = dynamic_cast<QSh_Texture1*>(shader);
            return new QSh_Light(shaderTexture->texture0(), shaderTexture->color());
        }
        return nullptr;
    });*/
    _ship->setPosition(0.0f, 0.0f, 1.5f);
    _water = new QEntity(_mainEntity);
    _water->setName("Water");
    mesh = new QMesh(this);
    mesh->setEnable_vertex_normal(true);
    mesh->setUsagePatternVertices(QOpenGLBuffer::DynamicDraw);
    mesh->setUsagePatternElements(QOpenGLBuffer::DynamicDraw);
    partEntity = _water->addPart(mesh, new QSh_Refraction_FallOff());
    partEntity->setAlpha(false);
    QLight* light = new QLight(this);
    light->setPosition(4.0f, -6.0f, 8.0f);
    light->setRadius(20.0f);
    light->setLightSoft(0.8f);
    _isoSurface.setScalarField(&_waterField);
    _isoSurface.setTValue(0.0f);
    _isoSurface.setEpsilon(0.01f);
    _isoSurface.setRegion(QVector3D(-3.4f, -3.4f, -3.4f), QVector3D(3.4f, 3.4f, 3.4f));
    _isoSurface.setCellSize(0.5f);
    _particleSystem.setRadiusArea(_waterField.radius());
    _particleSystem.setParentEntity(_mainEntity);
    QSprite* particle = new QSprite();
    particle->setAlpha(true);
    QSh_ColoredPart* shaderParticle = new QSh_ColoredPart();
    shaderParticle->setBackColor(QColor(230, 245, 255, 150));
    shaderParticle->setColor(QColor(255, 200, 100, 0));
    shaderParticle->setSmooth(0.4f);
    shaderParticle->setLimit(1.0f);
    shaderParticle->setTexturePart(context->loadTexture("CloudAlpha", ":/GameData/Cloud.jpg"));
    particle->setShader(shaderParticle);
    _particleSystem.setOriginalParticle(particle);
    _needUpdateScene = false;
    _thread = new QThread(this);
    connect(_thread, &QThread::started, this, &SceneShip::updateSceneThread);
    _thread->start();
}

SceneShip::~SceneShip()
{
    delete _thread;
}

void SceneShip::beginUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    _needUpdateScene = true;
}

void SceneShip::endUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    QScrollEngine::QSh_Refraction_FallOff* shader = static_cast<QScrollEngine::QSh_Refraction_FallOff*>(_water->shader(0));
    if (_frameProvider) {
        shader->setScreenTexture(_frameProvider->handleTexture());
        QMatrix2x2 matrixTexture = _frameProvider->matrixTexture();
        shader->setMatrixScreenTexture(matrixTexture * ((qAbs(matrixTexture(0, 0)) > 0.5f) ? 1.0f : -1.0f));
    } else {
        shader->setScreenTexture(0);
    }
    parentContext()->lock();
    QScrollEngine::QMesh* mesh = _water->part(0)->mesh();
    mesh->applyChanges();
    _updateShip();
    parentContext()->unlock();
}

void SceneShip::updateSceneThread()
{
    _mutex.lock();
    if (_needUpdateScene) {
        _waterField.update();
        _isoSurface.isoApproximate(_vertices, _normals, _triangles);
        _texcoords.resize(_vertices.size(), QVector2D(0.0, 0.0f));
        QScrollEngine::QMesh* mesh = _water->part(0)->mesh();
        mesh->swapDataOfVertices(_vertices, _texcoords, _normals);
        mesh->swapDataOfTriangles(_triangles);
        mesh->updateLocalBoundingBox();
        //mesh->updateNormals();
        float angle = _waterField.angleSource.value();
        _particleSystem.update(QVector2D(std::cos(angle), std::sin(angle)) * _waterField.speedWaves.value() * 0.1f);
        _needUpdateScene = false;
    }
    _mutex.unlock();
    QTimer::singleShot(2, this, &SceneShip::updateSceneThread);
}

void SceneShip::_updateShip()
{
    float deltaAngle = (_waterField.angleSource.value() + static_cast<float>(M_PI)) - _shipAngle;
    float deltaAngle_2 = deltaAngle - static_cast<float>(2.0f * M_PI);
    if (std::fabs(deltaAngle_2) < std::fabs(deltaAngle))
        deltaAngle = deltaAngle_2;
    float angularVelocity = std::min(std::fabs(deltaAngle), _shipAngularVelocity) * ((deltaAngle > 0.0f) ? 1.0f : -1.0f);
    _shipAngle = mod(_shipAngle + angularVelocity, static_cast<float>(2.0f * M_PI));
    QVector2D deltaX(qCos(_shipAngle), qSin(_shipAngle));
    deltaX *= _shipSize.x();
    QVector2D deltaY(- deltaX.y(), deltaX.x());
    deltaY *= _shipSize.y();
    QVector3D v[4];
    v[0] = QVector3D(- deltaX - deltaY, 0.0f);
    v[0].setZ(_waterField.getWaveHeight(v[0].x(), v[0].y()));
    v[1] = QVector3D(- deltaX + deltaY, 0.0f);
    v[1].setZ(_waterField.getWaveHeight(v[1].x(), v[1].y()));
    v[2] = QVector3D(deltaX - deltaY, 0.0f);
    v[2].setZ(_waterField.getWaveHeight(v[2].x(), v[2].y()));
    v[3] = QVector3D(deltaX + deltaY, 0.0f);
    v[3].setZ(_waterField.getWaveHeight(v[3].x(), v[3].y()));
    QVector3D axisX[2];
    axisX[0] = v[2] - v[0];
    axisX[1] = v[3] - v[1];
    axisX[0] = (axisX[0] + axisX[1]) * 0.5f;
    float length = axisX[0].length();
    if (length < 0.00005f) {
        axisX[0] = axisX[1];
        axisX[0].normalize();
    } else {
        axisX[0] /= length;
    }
    axisX[0].setZ(axisX[0].z());
    QVector3D axisY[2];
    axisY[0] = v[1] - v[0];
    axisY[1] = v[3] - v[2];
    axisY[0] = (axisY[0] + axisY[1]) * 0.5f;
    length = axisY[0].length();
    if (length < 0.00005f) {
        axisY[0] = axisY[1];
        axisY[0].normalize();
    } else {
        axisY[0] /= length;
    }
    axisY[0].setZ(axisY[0].z());
    QVector3D axisZ = QVector3D::crossProduct(axisX[0], axisY[0]);
    axisZ.normalize();
    axisY[0] = QVector3D::crossProduct(axisZ, axisX[0]);
    axisY[0].normalize();
    TMath::TMatrix<float> matrixRotation(3, 3);
    matrixRotation(0, 0) = axisX[0].x();
    matrixRotation(0, 1) = axisX[0].y();
    matrixRotation(0, 2) = axisX[0].z();
    matrixRotation(1, 0) = axisY[0].x();
    matrixRotation(1, 1) = axisY[0].y();
    matrixRotation(1, 2) = axisY[0].z();
    matrixRotation(2, 0) = axisZ.x();
    matrixRotation(2, 1) = axisZ.y();
    matrixRotation(2, 2) = axisZ.z();
    matrixRotation.invert();
    QMatrix3x3 qMatrixRotation;
    TMath::Tools<float>::convert(qMatrixRotation, matrixRotation);
    QQuaternion orientation;
    QScrollEngine::QOtherMathFunctions::matrixToQuaternion(qMatrixRotation, orientation);
    _ship->setOrientation(orientation);
    _ship->setPosition(0.0f, 0.0f, (v[0].z() + v[1].z() + v[2].z() + v[3].z()) * 0.25f);
}
