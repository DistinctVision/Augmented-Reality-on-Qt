#ifndef QANIMATION3D
#define QANIMATION3D

#include <vector>
#include <QVector3D>
#include <QQuaternion>

namespace QScrollEngine {

class QScene;
class QEntity;
class QScrollEngineContext;

class QAnimation3D
{
    friend class QEntity;
    friend class QScene;
    friend class QScrollEngineContext;
public:
    typedef struct AnimKeyPosition
    {
        qint16 time;
        QVector3D position;
    } AnimKeyPosition;

    typedef struct AnimKeyOrientation
    {
        qint16 time;
        QQuaternion orienation;
    } AnimKeyOrientation;

    typedef struct AnimKeyScale
    {
        qint16 time;
        QVector3D scale;
    } AnimKeyScale;

public:
    QAnimation3D();
    ~QAnimation3D();

    void updateFrame();
    void setAnimationTime(float time);
    void entityToAnimation(QEntity* entity) const;
    float currentTime() const { return _currentTime; }
    qint16 maxTimeKeysPosition() const { return _animKeysPosition.at(_animKeysPosition.size() - 1).time; }
    qint16 maxTimeKeysOrientation() const { return _animKeysPosition.at(_animKeysPosition.size() - 1).time; }
    qint16 maxTimeKeysScale() const { return _animKeysPosition.at(_animKeysPosition.size() - 1).time; }
    qint16 maxTimesKeys() const
    {
        qint16 m1 = maxTimeKeysPosition(), m2 = maxTimeKeysOrientation(), m3 = maxTimeKeysScale();
        if (m1 > m2) {
            if (m1 > m3)
                return m1;
            return m3;
        }
        if (m2 > m3)
            return m2;
        return m3;
    }
    qint16 minTimeKeysPosition() const { return _animKeysPosition.at(0).time; }
    qint16 minTimeKeysOrientation() const { return _animKeysOrientation.at(0).time; }
    qint16 minTimeKeysScale() const { return _animKeysScale.at(0).time; }
    qint16 minTimesKeys() const
    {
        qint16 m1 = minTimeKeysPosition(), m2 = minTimeKeysOrientation(), m3 = minTimeKeysScale();
        if (m1 < m2) {
            if (m1 < m3)
                return m1;
            return m3;
        }
        if (m2 < m3)
            return m2;
        return m3;
    }

    int countAnimKeysPosition() const { return static_cast<int>(_animKeysPosition.size()); }
    AnimKeyPosition& animKeyPosition(int index) { return _animKeysPosition.at(index); }
    void deleteAnimKeyPosition(int index);
    int addAnimKey(const AnimKeyPosition& key);
    int countAnimKeysOrientation() const { return static_cast<int>(_animKeysOrientation.size()); }
    AnimKeyOrientation& animKeyOrientation(int index) { return _animKeysOrientation.at(index); }
    void deleteAnimKeyOrientation(int index);
    int addAnimKey(const AnimKeyOrientation& key);
    int countAnimKeysScale() const { return static_cast<int>(_animKeysScale.size()); }
    AnimKeyScale& animKeyScale(int index) { return _animKeysScale.at(index); }
    void deleteAnimKeyScale(int index);
    int addAnimKey(const AnimKeyScale& key);
    void scaleTimeAnimation(float scale);

    float animationSpeed() const { return _animationSpeed; }
    void setAnimationSpeed(float speed) { _animationSpeed = speed; }
    int countUsedEntities() const { return _countUsedEntities; }
    bool enable() const { return _enable; }
    void setEnable(bool enable) { _enable = enable; }
    bool loop() const { return _loop; }
    void setLoop(bool loop) { _loop = loop; }
    static float ease(float time, float easeFrom, float easeTo);

private:
    static float _animationSpeed_global;

    bool _enable;
    bool _loop;
    std::vector<AnimKeyPosition> _animKeysPosition;
    int _currentKeyPosition;
    std::vector<AnimKeyOrientation> _animKeysOrientation;
    int _currentKeyOrientation;
    std::vector<AnimKeyScale> _animKeysScale;
    int _currentKeyScale;

    float _currentTime;
    float _animationSpeed;
    float _endTime;
    int _countUsedEntities;

    void _entityToAnimation(QEntity* entity) const;
};

}
#endif
