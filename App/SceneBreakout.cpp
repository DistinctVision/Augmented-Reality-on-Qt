#include "App/SceneBreakout.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <random>
#include <cfloat>
#include <cmath>
#include <QDebug>
#include "QScrollEngine/QEntity.h"

void Game_Breakout::_addQuad(QScrollEngine::QMesh* mesh, const QVector3D& axisX, const QVector3D& axisY)
{
    QVector3D axisZ = QVector3D::crossProduct(axisX, axisY);
    int shiftVertex = mesh->countVertices();
    int shiftTriangles = mesh->countTriangles();
    mesh->setCountVertices(mesh->countVertices() + 4);
    mesh->setCountTriangles(mesh->countTriangles() + 2);
    mesh->setVertexPosition(shiftVertex + 0, axisX * (- 1.0f) + axisY * 1.0f + axisZ * (1.0f));
    mesh->setVertexTextureCoord(shiftVertex + 0, QVector2D(0.0f, 0.0f));
    mesh->setVertexPosition(shiftVertex + 1, axisX * 1.0f + axisY * 1.0f + axisZ * (1.0f));
    mesh->setVertexTextureCoord(shiftVertex + 1, QVector2D(1.0f, 0.0f));
    mesh->setVertexPosition(shiftVertex + 2, axisX * 1.0f + axisY * (- 1.0f) + axisZ * (1.0f));
    mesh->setVertexTextureCoord(shiftVertex + 2, QVector2D(1.0f, 1.0f));
    mesh->setVertexPosition(shiftVertex + 3, axisX * (- 1.0f) + axisY * (- 1.0f) + axisZ * (1.0f));
    mesh->setVertexTextureCoord(shiftVertex + 3, QVector2D(0.0f, 1.0f));
    mesh->setTriangle(shiftTriangles + 0, shiftVertex + 0, shiftVertex + 2, shiftVertex + 1);
    mesh->setTriangle(shiftTriangles + 1, shiftVertex + 0, shiftVertex + 3, shiftVertex + 2);
}

void Game_Breakout::_createCornerOfBoard(QScrollEngine::QMesh* mesh)
{
    const int countSteps = 5;

    float startAngle = static_cast<float>(M_PI * 0.5);
    float stepAngle = static_cast<float>(M_PI / static_cast<double>(countSteps));

    mesh->clear();
    mesh->setSizeOfELement(3);
    mesh->setCountTriangles((countSteps + 1) * 3);
    mesh->setCountVertices((countSteps + 1) * 3 + 1);
    mesh->setVertexPosition(0, QVector3D(0.0f, 0.0f, 1.0f));
    float angle = startAngle, x = qCos(angle), y = qSin(angle);
    mesh->setVertexPosition(1, QVector3D(x, y, 1.0f));
    mesh->setVertexPosition(2, QVector3D(x, y, 1.0f));
    mesh->setVertexPosition(3, QVector3D(x, y, -1.0f));
    for (int i=1; i<=countSteps; ++i) {
        angle = startAngle + stepAngle * i;
        x = - qCos(angle);
        y = qSin(angle);
        int shiftVertex = 1 + i * 3;
        mesh->setVertexPosition(shiftVertex + 0, QVector3D(x, y, 1.0f));
        mesh->setVertexPosition(shiftVertex + 1, QVector3D(x, y, 1.0f));
        mesh->setVertexPosition(shiftVertex + 2, QVector3D(x, y, -1.0f));
        int shiftTriangle = 3 * i;
        mesh->setTriangle(shiftTriangle + 0, 0, shiftVertex + 0, shiftVertex - 3);
        mesh->setTriangle(shiftTriangle + 1, shiftVertex - 2, shiftVertex + 1, shiftVertex - 1);
        mesh->setTriangle(shiftTriangle + 2, shiftVertex + 1, shiftVertex + 2, shiftVertex - 1);
    }
    mesh->setEnable_vertex_normal(true);
    mesh->updateNormals();
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
}

Game_Breakout::Game_Breakout(QScrollEngine::QScrollEngineContext* context)
{
    using namespace QScrollEngine;
    _initialized = false;
    _colorBlocks[0] = QColor(0, 80, 255);
    _colorBlocks[1] = QColor(0, 255, 80);
    _colorBlocks[2] = QColor(255, 255, 0);
    _colorBlocks[3] = QColor(255, 0, 0);

    _minBorder = QVector2D(-10.0f, -2.0f);
    _maxBorder = QVector2D(10.0f, 12.0f);
    _borderAdded = 5.0f;
    QMesh* mesh;

    _all = new QEntity();
    _all->setName("All");

    _background = new QEntity(_all);
    _background->addPart(context, QGLPrimitiv::Quad, new QSh_Light(context->emptyTexture(), QColor(150, 200, 255, 255), 1.0f, 1.0f));

    _blockEntity = new QEntity();
    _blockEntity->setName("Block");
    mesh = new QMesh(context);
    mesh->setSizeOfELement(3);
    Game_Breakout::_addQuad(mesh, QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    Game_Breakout::_addQuad(mesh, QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f));
    Game_Breakout::_addQuad(mesh, QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f));
    Game_Breakout::_addQuad(mesh, QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f));
    Game_Breakout::_addQuad(mesh, QVector3D(0.0f, 0.0f, - 1.0f), QVector3D(0.0f, 1.0f, 0.0f));
    mesh->setEnable_vertex_normal(true);
    mesh->updateNormals();
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    _blockEntity->addPart(mesh, new QSh_Light(context->emptyTexture(), QColor(255, 255, 255, 255), 1.0f, 10.0f));

    for (int i=0; i<4; ++i) {
        _borderEntity[i] = _blockEntity->clone();
        _borderEntity[i]->setParentEntity(_all);
        QSh_Color* sh_color = dynamic_cast<QSh_Color*>(_borderEntity[i]->part(0)->shader());
        sh_color->setColorF(1.0f, 0.3f, 0.3f, 1.0f);
    }

    _board.entity = new QEntity();
    mesh = new QMesh(context);
    Game_Breakout::_createCornerOfBoard(mesh);
    QEntity* rightCorner = new QEntity(_board.entity);
    rightCorner->setName("RightCorner");
    rightCorner->addPart(mesh, new QSh_Light(context->emptyTexture(), QColor(0, 100, 255, 255), 0.5f, 1.0f));
    rightCorner->setPosition(1.0f, 0.0f, 0.0f);
    QEntity* leftCorner = new QEntity(_board.entity);
    leftCorner->setName("LeftCorner");
    leftCorner->addPart(mesh, new QSh_Light(context->emptyTexture(), QColor(0, 100, 255, 255), 0.5f, 1.0f));
    leftCorner->setPosition(-1.0f, 0.0f, 0.0f);
    leftCorner->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 180.0f));
    QEntity* entity = _blockEntity->clone();
    entity->setParentEntity(_board.entity);
    _board.maxVelocity = 1.0f;
    _board.radius = 0.25f;
    _board.width = 2.0f;
    _cursor = new QEntity();
    QOpenGLTexture* textureGlow = context->loadTexture("Glow", ":/GameData/Glow.png");
    textureGlow->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    _cursor->addPart(context, QGLPrimitiv::Quad, new QSh_Texture1(textureGlow));
    mesh = _cursor->part(0)->mesh();
    mesh->setVertexPosition(0, QVector3D(-0.5f, -0.1f, -1.0f));
    mesh->setVertexPosition(1, QVector3D(0.5f, -0.1f, -1.0f));
    mesh->setVertexPosition(2, QVector3D(0.5f, 3.0f, -1.0f));
    mesh->setVertexPosition(3, QVector3D(-0.5f, 3.0f, -1.0f));
    _cursor->part(0)->mesh()->applyChangesOfVertexPositions();
    _cursor->part(0)->mesh()->updateLocalBoundingBox();
    _cursor->setAlpha(true);

    _ballMaxVelocity = 1.0f;
    _sampleBall = new QEntity();
    _sampleBall->setName("Ball");
    mesh = new QMesh(context);
    QGLPrimitiv::createSphere(mesh, 0.5f, 8, 8);
    _sampleBall->addPart(mesh, new QSh_Light(context->emptyTexture(), QColor(255, 255, 255, 255), 1.0f, 10.0f));
}

Game_Breakout::~Game_Breakout()
{
    clear();
    delete _blockEntity;
    delete _board.entity;
    delete _sampleBall;
}

void Game_Breakout::clear()
{
    if (_initialized) {
        for (QList<StaticBlock>::iterator it  = _staticBlocks.begin(); it != _staticBlocks.end(); ++it)
            delete it->entity;
        _initialized = false;
    }
    _staticBlocks.clear();
    _board.entity->setParentScene(nullptr);
    delete _planarShadows.light();
    _planarShadows.clear();
}

void Game_Breakout::turnAllToCamera(QScrollEngine::QScene::CameraInfo cameraInfo)
{
    cameraInfo.localX = - cameraInfo.localX;
    cameraInfo.localX.setZ(0.0f);
    cameraInfo.localX.normalize();
    cameraInfo.localZ = QVector3D(0.0f, 0.0f, 1.0f);
    cameraInfo.localY = QVector3D::crossProduct(cameraInfo.localZ, cameraInfo.localX);
#ifdef __ANDROID__
    std::swap(cameraInfo.localX, cameraInfo.localY);
    cameraInfo.localY = - cameraInfo.localY;
#endif
    QMatrix3x3 matrix;
    matrix(0, 0) = cameraInfo.localX.x();
    matrix(0, 1) = cameraInfo.localX.y();
    matrix(0, 2) = cameraInfo.localX.z();
    matrix(1, 0) = cameraInfo.localY.x();
    matrix(1, 1) = cameraInfo.localY.y();
    matrix(1, 2) = cameraInfo.localY.z();
    matrix(2, 0) = cameraInfo.localZ.x();
    matrix(2, 1) = cameraInfo.localZ.y();
    matrix(2, 2) = cameraInfo.localZ.z();
#ifdef __ANDROID__
    matrix = matrix.transposed();
#endif
    QQuaternion orientation;
    QScrollEngine::QOtherMathFunctions::matrixToQuaternion(matrix, orientation);
    _all->setOrientation(orientation);
}

void Game_Breakout::updateBoard(const QVector3D& cameraPosition, const QVector3D& cameraLocalZ)
{
    if (!_initialized)
        return;
    QVector3D localZ = _all->getInverseTransformDir(cameraLocalZ);
    QVector3D localP = _all->getInverseTransformPoint(cameraPosition);
    QVector2D dir(localZ.x(), localZ.y()), p(localP.x(), localP.y());
    float l = dir.length(), x;
    if (l > 0.000005f) {
        dir /= l;
        if (qAbs(dir.y()) > 0.000005f) {
            x = dir.x() * (- p.y() / dir.y()) + p.x();
        } else {
            x = localP.x();
        }
    } else {
        x = localP.x();
    }
    if (x < _minBorder.x())
        x = _minBorder.x();
    if (x > _maxBorder.x())
        x = _maxBorder.x();
    QVector3D prev = _cursor->position();
    _cursor->setPosition(x, prev.y(), prev.z());
    float dB = _board.width * 0.5f + _board.radius;
    if (x < _minBorder.x() + dB)
        x = _minBorder.x() + dB;
    if (x > _maxBorder.x() - dB)
        x = _maxBorder.x() - dB;
    float dx = x - _board.position.x();
    if (dx > _board.maxVelocity)
        dx = _board.maxVelocity;
    if (dx < - _board.maxVelocity)
        dx = - _board.maxVelocity;
    _board.position.setX(_board.position.x() + dx);
    prev = _board.entity->position();
    _board.entity->setPosition(_board.position.x(), prev.y(), prev.z());
    for (QList<Ball>::iterator it = _balls.begin(); it != _balls.end(); ++it) {
        _solveCollisionBallWithBoard(*it, _board);
    }
}

void Game_Breakout::initialize(QScrollEngine::QScene* scene)
{
    using namespace QScrollEngine;
    _planarShadows.setScene(scene);
    _planarShadows.setLight(new QLight(_all));
    _planarShadows.light()->setPosition(0.0f, 8.0f, 15.0f);
    _planarShadows.light()->setRadius(60.0f);
    _planarShadows.setPlanePos(QVector3D(0.0f, 0.0f, -1.0f));
    _planarShadows.setPlaneDir(QVector3D(0.0f, 0.0f, 1.0f));
    _planarShadows.setColorShadows(QColor(0, 0, 0, 100));
    //_planarShadows.setAlpha(true);
    for (QList<StaticBlock>::iterator it = _staticBlocks.begin(); it != _staticBlocks.end(); ++it) {
        it->entity = _blockEntity->clone();
        it->entity->setParentEntity(_all);
        it->entity->setPosition(it->position.x(), it->position.y(), 0.0f);
        it->entity->setScale(it->size.x(), it->size.y(), 1.0f);
        _planarShadows.addEntity(it->entity);
        for (unsigned int i=0; i<it->entity->countParts(); ++i) {
            QSh_Color* sh_color = dynamic_cast<QSh_Color*>(it->entity->part(i)->shader());
            sh_color->setColor(_colorBlocks[it->type]);
        }
    }
    _updateBoardEntity();
    _board.entity->setParentEntity(_all);
    _planarShadows.addEntity(_board.entity);
    _deleteBalls();
    Ball ball;
    ball.entity = _sampleBall->clone();
    ball.entity->setParentEntity(_all);
    ball.radius = 0.5f;
    ball.radiusSquared = 0.25f;
    ball.velocity = QVector2D(0.4f, 0.4f);
    _balls.push_back(ball);
    _planarShadows.addEntity(ball.entity);
    _all->setParentScene(scene);
    _all->setAlpha(true);
    scene->updateCameraInfo(scene->parentContext()->camera);
    turnAllToCamera(scene->cameraInfo());
    _cursor->setParentEntity(_all);
    for (int i=0; i<4; ++i)
        _planarShadows.addEntity(_borderEntity[i]);
    updateBackgroundAndBorder();
    _initialized = true;
}

void Game_Breakout::updateBackgroundAndBorder()
{
    const float sizeBorder = 0.25f;
    const float z = -1.05f;
    _background->setParentEntity(_all);
    QVector2D center = (_minBorder + _maxBorder) * 0.5f;
    QVector2D delta = _maxBorder - _minBorder;
    QVector2D scale = delta + QVector2D(_borderAdded, _borderAdded);
    _background->setPosition(center.x(), center.y(), z);
    _background->setScale(scale.x(), scale.y(), 1.0f);
    delta *= 0.5f;
    _borderEntity[0]->setPosition(_minBorder.x() - sizeBorder, center.y(), sizeBorder + z);
    _borderEntity[0]->setScale(sizeBorder, delta.y() + sizeBorder * 2.0f, sizeBorder * 2.0f);
    _borderEntity[1]->setPosition(_maxBorder.x() + sizeBorder, center.y(), sizeBorder + z);
    _borderEntity[1]->setScale(sizeBorder, delta.y() + sizeBorder * 2.0f, sizeBorder * 2.0f);
    _borderEntity[2]->setPosition(center.x(), _minBorder.y() - sizeBorder, sizeBorder + z);
    _borderEntity[2]->setScale(delta.x() + sizeBorder * 2.0f, sizeBorder, sizeBorder * 2.0f);
    _borderEntity[3]->setPosition(center.x(), _maxBorder.y() + sizeBorder, sizeBorder + z);
    _borderEntity[3]->setScale(delta.x() + sizeBorder * 2.0f, sizeBorder, sizeBorder * 2.0f);
}

void Game_Breakout::updateGame()
{
    using namespace QScrollEngine;
    if (!_initialized)
        return;
    QVector2D minA, maxA;
    for (QList<Ball>::iterator it = _balls.begin(); it != _balls.end(); ++it) {
        it->position += it->velocity;
        minA = it->position - QVector2D(it->radius, it->radius);
        maxA = it->position + QVector2D(it->radius, it->radius);
        for (QList<StaticBlock>::iterator itBlock = _staticBlocks.begin(); itBlock != _staticBlocks.end(); ) {
            if (_collisionAABB(minA, maxA, itBlock->position - itBlock->size, itBlock->position + itBlock->size))
                if (_solveCollisionBallWithBlock(*it, itBlock->position, itBlock->size)) {
                    int newType = itBlock->type - 1;
                    if (newType < 0) {
                        delete itBlock->entity;
                        if (itBlock == _staticBlocks.begin()) {
                            _staticBlocks.erase(itBlock);
                            itBlock = _staticBlocks.begin();
                        } else {
                            QList<StaticBlock>::iterator prev = itBlock - 1;
                            _staticBlocks.erase(itBlock);
                            itBlock = prev + 1;
                        }
                        continue;
                    } else {
                        itBlock->type = TypeBlock(newType);
                        QSh_Color* sh_color = dynamic_cast<QSh_Color*>(itBlock->entity->shader(0));
                        sh_color->setColor(_colorBlocks[newType]);
                    }
                }
            ++itBlock;
        }
        for (QList<Ball>::iterator itBall = it + 1; itBall != _balls.end(); ++itBall) {
            _solveCollisionBallWithBall(*it, *itBall);
        }
        _solveCollisionBallWithBorders(*it);
        it->entity->setPosition(it->position.x(), it->position.y(), it->entity->position().z());
        float velocity = it->velocity.length();
        float d = velocity / _ballMaxVelocity;
        if (d > 1.0f) {
            it->velocity /= d;
            d = 1.0f;
        }
        QVector3D color;
        if (d < 0.5f) {
            color = QVector3D(0, 1.0f - d * 2.0f, d * 2.0f);
        } else {
            color = QVector3D(1.0f - (d - 0.5f) * 2.0f, (d - 0.5f) * 2.0f, 0.0f);
        }
        QSh_Color* sh_color = dynamic_cast<QSh_Color*>(it->entity->shader(0));
        if (sh_color)
            sh_color->setColorF(color.x(), color.y(), color.z());

    }
}

void Game_Breakout::addStaticBlock(TypeBlock type, const QVector2D& size, const QVector2D& position)
{
    if (_initialized)
        return;
    StaticBlock block;
    block.entity = nullptr;
    block.type = type;
    block.position = position;
    block.size = size;
    _staticBlocks.push_back(block);
}

void Game_Breakout::addStaticBlocks(TypeBlock type, const QVector2D& size, const QVector2D& startPosition,
                                    const QVector2D& step, int countSteps)
{
    if (_initialized)
        return;
    StaticBlock block;
    block.entity = nullptr;
    block.type = type;
    block.size = size;
    block.position = startPosition;
    for (int i=0; i<countSteps; ++i) {
        _staticBlocks.push_back(block);
        block.position += step;
    }
}

bool Game_Breakout::_solveCollisionBallWithBlock(Ball& ball, const QVector2D& blockPosition, const QVector2D& blockSize)
{
    QVector2D ballLocalPos = ball.position - blockPosition;
    QVector2D normal, point;
    if (ballLocalPos.x() < - blockSize.x()) {
        if (ballLocalPos.y() < - blockSize.y()) {
            point = - blockSize;
            normal = ballLocalPos - point;
            float distance = normal.lengthSquared();
            if (distance > ball.radiusSquared)
                return false;
            distance = qSqrt(distance);
            if (distance < 0.0005f)
                return false;
            normal /= distance;
        } else if (ballLocalPos.y() > blockSize.y()) {
            point = QVector2D(- blockSize.x(), blockSize.y());
            normal = ballLocalPos - point;
            float distance = normal.lengthSquared();
            if (distance > ball.radiusSquared)
                return false;
            distance = qSqrt(distance);
            if (distance < 0.0005f)
                return false;
            normal /= distance;
        } else {
            point = - blockSize;
            normal = QVector2D(-1.0f, 0.0f);
        }
    } else if (ballLocalPos.x() > blockSize.x()) {
        if (ballLocalPos.y() < - blockSize.y()) {
            point = QVector2D(blockSize.x(), - blockSize.y());
            normal = ballLocalPos - point;
            float distance = normal.lengthSquared();
            if (distance > ball.radiusSquared)
                return false;
            distance = qSqrt(distance);
            if (distance < 0.0005f)
                return false;
            normal /= distance;
        } else if (ballLocalPos.y() > blockSize.y()) {
            point = blockSize;
            normal = ballLocalPos - point;
            float distance = normal.lengthSquared();
            if (distance > ball.radiusSquared)
                return false;
            distance = qSqrt(distance);
            if (distance < 0.0005f)
                return false;
            normal /= distance;
        } else {
            point = blockSize;
            normal = QVector2D(1.0f, 0.0f);
        }
    } else {
        if (ballLocalPos.y() < - blockSize.y()) {
            point = - blockSize;
            normal = QVector2D(0.0f, - 1.0f);
        } else if (ballLocalPos.y() > blockSize.y()) {
            point = blockSize;
            normal = QVector2D(0.0f, 1.0f);
        } else {
            point = - blockSize;
            normal = QVector2D(0.0f, - 1.0f);
        }
    }
    float depth = ball.radius - QVector2D::dotProduct(ballLocalPos - point, normal);
    ball.position += normal * (depth + 0.001f);
    float localVelocity = - QVector2D::dotProduct(ball.velocity, normal);
    if (localVelocity < 0.0f)
        localVelocity = 0.0f;
    ball.velocity += normal * localVelocity * 2.0f;
    return true;
}

bool Game_Breakout::_solveCollisionBallWithBall(Ball& ballA, Ball& ballB)
{
    float sumRadius = ballA.radius + ballB.radius;
    float radiusSquared = sumRadius * sumRadius;
    QVector2D delta = ballB.position - ballA.position;
    float distance = delta.lengthSquared();
    if (distance > radiusSquared)
        return false;
    distance = qSqrt(distance);
    if (distance < 0.0005f)
        return false;
    delta /= distance;
    float depth = (sumRadius - distance) * 0.5f;
    QVector2D force = delta * (depth + 0.001f);
    ballA.position -= force;
    ballB.position += force;
    float localVelocity = - QVector2D::dotProduct(ballB.velocity - ballA.velocity, delta);
    if (localVelocity < 0.0f)
        localVelocity = 0.0f;
    force = delta * (localVelocity);
    ballA.velocity -= force;
    ballB.velocity += force;
    return true;
}

bool Game_Breakout::_solveCollisionBallWithCircle(Ball& ball, const QVector2D& circleCenter, float radius)
{
    float sumRadius = ball.radius + radius;
    float radiusSquared = sumRadius * sumRadius;
    QVector2D delta = ball.position - circleCenter;
    float distance = delta.lengthSquared();
    if (distance > radiusSquared)
        return false;
    distance = qSqrt(distance);
    if (distance < 0.0005f)
        return false;
    delta /= distance;
    float depth = sumRadius - distance;
    QVector2D force = delta * (depth + 0.001f);
    ball.position += force;
    float localVelocity = - QVector2D::dotProduct(ball.velocity, delta);
    if (localVelocity < 0.0f)
        localVelocity = 0.0f;
    force = delta * (localVelocity * 2.0f);
    ball.velocity += force;
    return true;
}

bool Game_Breakout::_solveCollisionBallWithBoard(Ball& ball, Board& board)
{
    float h1 = (board.position.y() + board.radius) - (ball.position.y() - ball.radius);
    if (h1 < 0.0f)
        return false;
    float h2 = (ball.position.y() + ball.radius) - (board.position.y() - board.radius);
    if (h2 < 0.0f)
        return false;
    float localX = ball.position.x() - board.position.x();
    float d = board.width * 0.5f + board.radius;
    if ((localX + ball.radius) < - d)
        return false;
    if ((localX - ball.radius) > d)
        return false;
    if ((localX > - d) && (localX < d)) {
        float t = (localX + d) / (2.0f * d);
        QVector2D dir = QVector2D(-1.0f, 1.0f) * (1.0f - t) + QVector2D(1.0f, 1.0f) * t;
        dir.normalize();
        if (h1 < h2) {
            ball.position.setY(ball.position.y() + (h1 + 0.001f));
            //ball.velocity.setY(qAbs(ball.velocity.y()));
            ball.velocity = dir * ball.velocity.length();
        } else {
            ball.position.setY(ball.position.y() - (h2 + 0.001f));
            //ball.velocity.setY(- qAbs(ball.velocity.y()));
            dir.setY( - dir.y());
            ball.velocity = dir * ball.velocity.length();
        }
        return true;
    }
    if (_solveCollisionBallWithCircle(ball, QVector2D(board.position.x() - board.width * 0.5f, board.position.y()), board.radius))
        return true;
    if (_solveCollisionBallWithCircle(ball, QVector2D(board.position.x() + board.width * 0.5f, board.position.y()), board.radius))
        return true;
    return false;
}

bool Game_Breakout::_solveCollisionBallWithBorders(Ball& ball)
{
    float d1 = _minBorder.x() - (ball.position.x() - ball.radius);
    float d2 = (ball.position.x() + ball.radius) - _maxBorder.x();
    float d3 = _minBorder.y() - (ball.position.y() - ball.radius);
    float d4 = (ball.position.y() + ball.radius) - _maxBorder.y();
    bool collision = false;
    if (d1 > 0.0f) {
        collision = true;
        ball.position.setX(ball.position.x() + d1);
        ball.velocity.setX(qAbs(ball.velocity.x()));
    }
    if (d2 > 0.0f) {
        collision = true;
        ball.position.setX(ball.position.x() - d2);
        ball.velocity.setX(- qAbs(ball.velocity.x()));
    }
    if (d3 > 0.0f) {
        collision = true;
        ball.position.setY(ball.position.y() + d3);
        ball.velocity.setY(qAbs(ball.velocity.y()));
    }
    if (d4 > 0.0f) {
        collision = true;
        ball.position.setY(ball.position.y() - d4);
        ball.velocity.setY(- qAbs(ball.velocity.y()));
    }
    return collision;
}

void Game_Breakout::_updateBoardEntity()
{
    using namespace QScrollEngine;
    QEntity* entity = _board.entity->findChild("RightCorner");
    QSh_Color* sh_color = dynamic_cast<QSh_Color*>(entity->part(0)->shader());
    sh_color->setColor(0, 100, 255);
    entity->setPosition(_board.width * 0.5f, 0.0f, 0.0f);
    entity->setScale(_board.radius, _board.radius, 1.0f);
    entity = _board.entity->findChild("LeftCorner");
    sh_color = dynamic_cast<QSh_Color*>(entity->part(0)->shader());
    sh_color->setColor(0, 100, 255);
    entity->setPosition(- _board.width * 0.5f, 0.0f, 0.0f);
    entity->setScale(_board.radius, _board.radius, 1.0f);
    entity = _board.entity->findChild("Block");
    sh_color = dynamic_cast<QSh_Color*>(entity->part(0)->shader());
    sh_color->setColor(0, 100, 255);
    entity->setScale(_board.width * 0.5f, _board.radius, 1.0f);
}

void Game_Breakout::_deleteBalls()
{
    for (QList<Ball>::iterator it = _balls.begin(); it != _balls.end(); ++it)
        delete it->entity;
    _balls.clear();
}

SceneBreakout::SceneBreakout(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    setAmbientColor(70, 70, 70);
    context->glLineWidth(2.0f);
    scale = QVector3D(0.2f, 0.2f, 0.2f);
    _arSystem = nullptr;
    _game = new Game_Breakout(context);
    _needUpdateScene = false;
    _thread = new QThread(this);
    connect(_thread, &QThread::started, this, &SceneBreakout::updateSceneThread);
    _thread->start();
}

void SceneBreakout::_createLevel1()
{
    _game->addStaticBlocks(Game_Breakout::Red, QVector2D(1.0f, 0.5f), QVector2D(-7.0f, 10.5f), QVector2D(2.0f, 0.0f), 8);
    _game->addStaticBlocks(Game_Breakout::Yellow, QVector2D(1.0f, 0.5f), QVector2D(-7.0f, 9.0f), QVector2D(2.0f, 0.0f), 8);
    _game->addStaticBlocks(Game_Breakout::Green, QVector2D(1.0f, 0.5f), QVector2D(-7.0f, 7.5f), QVector2D(2.0f, 0.0f), 8);
    _game->addStaticBlocks(Game_Breakout::Blue, QVector2D(1.0f, 0.5f), QVector2D(-7.0f, 6.0f), QVector2D(2.0f, 0.0f), 8);
}

void SceneBreakout::updateSceneThread()
{
    _mutex.lock();
    if (_needUpdateScene) {
        if (_arSystem->state() == AR::ARSystem::TrackingNow)
            _game->updateGame();
        else if (_arSystem->state() == AR::ARSystem::NotTracking)
            _game->clear();
        _needUpdateScene = false;
    }
    _mutex.unlock();
    QTimer::singleShot(2, this, &SceneBreakout::updateSceneThread);
}

void SceneBreakout::beginUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    _needUpdateScene = true;
}

void SceneBreakout::endUpdate()
{
    QMutexLocker mutexLocker(&_mutex);
    if (_arSystem->state() == AR::ARSystem::TrackingNow) {
        if (!_game->initialized()) {
            _createLevel1();
            _game->initialize(this);
        }
        updateCameraInfo(parentContext()->camera);
        _game->updateBoard(cameraPosition(), cameraLocalZ());
    }
}

SceneBreakout::~SceneBreakout()
{
    delete _game;
}
