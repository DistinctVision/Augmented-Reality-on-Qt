#ifndef SCENEBREAKOUT_H
#define SCENEBREAKOUT_H

#include <vector>
#include <QList>
#include <QVector2D>
#include <QVector3D>
#include "QScrollEngine/QScene.h"
#include "App/ARCameraQml.h"
#include <QThread>
#include <QColor>
#include <QMutex>
#include <QMutexLocker>
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Tools/PlanarShadows.h"


class Game_Breakout:
        public QObject
{
    Q_OBJECT
public:
    enum TypeBlock:int
    {
        Blue = 0,
        Green = 1,
        Yellow = 2,
        Red = 3
    };

    Game_Breakout(QScrollEngine::QScrollEngineContext* context);
    ~Game_Breakout();

    bool initialized() const { return _initialized; }
    QVector2D minBorder() const { return _minBorder; }
    QVector2D maxBorder() const { return _maxBorder; }
    void setMinMaxBorder(const QVector2D& minBorder, const QVector2D& maxBorder) { _minBorder = minBorder; _maxBorder = maxBorder; }

    void clear();
    void initialize(QScrollEngine::QScene* scene);
    void turnAllToCamera(QScrollEngine::QScene::CameraInfo cameraInfo);
    void updateGame();

    void addStaticBlock(TypeBlock type, const QVector2D& size, const QVector2D& position);
    void addStaticBlocks(TypeBlock type, const QVector2D& size, const QVector2D& startPosition, const QVector2D& step, int countSteps);

    void updateBoard(const QVector3D& cameraPosition, const QVector3D& cameraLocalZ);
    void updateBackgroundAndBorder();

    QScrollEngine::PlanarShadows& planarShadows() { return _planarShadows; }

signals:
    void winGame();
    void loseGame();

private:
    typedef struct StaticBlock {
        QScrollEngine::QEntity* entity;
        TypeBlock type;
        QVector2D position;
        QVector2D size;
    } StaticBlock;

    typedef struct Board {
        QScrollEngine::QEntity* entity;
        float radius;
        float width;
        float maxVelocity;
        QVector2D position;
    } Board;

    typedef struct Ball
    {
        QScrollEngine::QEntity* entity;
        QVector2D position;
        QVector2D velocity;
        float radius;
        float radiusSquared;
    } Ball;

    QColor _colorBlocks[4];
    bool _initialized;
    QScrollEngine::QEntity* _all;
    QList<StaticBlock> _staticBlocks;
    QScrollEngine::QEntity* _blockEntity;
    QScrollEngine::QEntity* _sampleBall;
    Board _board;
    float _ballMaxVelocity;
    QList<Ball> _balls;
    QVector2D _minBorder, _maxBorder;
    QScrollEngine::QEntity* _borderEntity[4];
    QScrollEngine::QEntity* _cursor;
    QScrollEngine::QEntity* _background;
    QScrollEngine::PlanarShadows _planarShadows;
    float _borderAdded;

    static void _addQuad(QScrollEngine::QMesh* mesh, const QVector3D& axisX, const QVector3D& axisY);
    inline static bool _collisionAABB(const QVector2D& minA, const QVector2D& maxA, const QVector2D& minB, const QVector2D& maxB)
    {
        if (minA.x() < maxB.x())
            if (minA.y() < maxB.y())
                if (minB.x() < maxA.x())
                    if (minB.y() < maxA.y())
                        return true;
        return false;
    }
    static void _createCornerOfBoard(QScrollEngine::QMesh* mesh);
    void _updateBoardEntity();
    void _deleteBalls();
    bool _solveCollisionBallWithBlock(Ball& ball, const QVector2D& blockPosition, const QVector2D& blockSize);
    bool _solveCollisionBallWithBall(Ball& ballA, Ball& ballB);
    bool _solveCollisionBallWithCircle(Ball& ball, const QVector2D& circleCenter, float radius);
    bool _solveCollisionBallWithBoard(Ball& ball, Board& board);
    bool _solveCollisionBallWithBorders(Ball& ball);
};

class SceneBreakout:
        public SceneInterface
{
    Q_OBJECT
public:
    SceneBreakout(QScrollEngine::QScrollEngineContext* context, int order = 0);
    ~SceneBreakout();

    void beginUpdate() override;
    void endUpdate() override;

public slots:
    void updateSceneThread();

private:
    Game_Breakout* _game;
    QThread* _thread;
    QMutex _mutex;
    bool _needUpdateScene;

    void _createLevel1();
};

#endif // SCENEWATER_H
