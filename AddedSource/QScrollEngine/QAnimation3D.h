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
public:
    typedef struct AnimKeyPosition
    {
        qint16 time;
        QVector3D position;

        AnimKeyPosition() {}
        AnimKeyPosition(qint16 time, const QVector3D& position)
        {
            this->time = time;
            this->position = position;
        }
    } AnimKeyPosition;

    typedef struct AnimKeyOrientation
    {
        qint16 time;
        QQuaternion orienation;

        AnimKeyOrientation() {}
        AnimKeyOrientation(qint16 time, const QQuaternion& orienation)
        {
            this->time = time;
            this->orienation = orienation;
        }
    } AnimKeyOrientation;

    typedef struct AnimKeyScale
    {
        qint16 time;
        QVector3D scale;

        AnimKeyScale() {}
        AnimKeyScale(qint16 time, const QVector3D& scale)
        {
            this->time = time;
            this->scale = scale;
        }
    } AnimKeyScale;

public:
    QAnimation3D();
    ~QAnimation3D();

    void updateFrame();
    void setAnimationTime(float time);
    void entityToAnimation(QEntity* entity) const;
    float currentTime() const { return m_currentTime; }
    qint16 maxTimeKeysPosition() const { return m_animKeysPosition.at(m_animKeysPosition.size() - 1).time; }
    qint16 maxTimeKeysOrientation() const { return m_animKeysPosition.at(m_animKeysPosition.size() - 1).time; }
    qint16 maxTimeKeysScale() const { return m_animKeysPosition.at(m_animKeysPosition.size() - 1).time; }
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
    qint16 minTimeKeysPosition() const { return m_animKeysPosition.at(0).time; }
    qint16 minTimeKeysOrientation() const { return m_animKeysOrientation.at(0).time; }
    qint16 minTimeKeysScale() const { return m_animKeysScale.at(0).time; }
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

    int countAnimKeysPosition() const { return static_cast<int>(m_animKeysPosition.size()); }
    AnimKeyPosition& animKeyPosition(int index) { return m_animKeysPosition.at(index); }
    void deleteAnimKeyPosition(int index);
    int addAnimKey(const AnimKeyPosition& key);
    int countAnimKeysOrientation() const { return static_cast<int>(m_animKeysOrientation.size()); }
    AnimKeyOrientation& animKeyOrientation(int index) { return m_animKeysOrientation.at(index); }
    void deleteAnimKeyOrientation(int index);
    int addAnimKey(const AnimKeyOrientation& key);
    int countAnimKeysScale() const { return static_cast<int>(m_animKeysScale.size()); }
    AnimKeyScale& animKeyScale(int index) { return m_animKeysScale.at(index); }
    void deleteAnimKeyScale(int index);
    int addAnimKey(const AnimKeyScale& key);
    void scaleTimeAnimation(float scale);

    float animationSpeed() const { return m_animationSpeed; }
    void setAnimationSpeed(float speed) { m_animationSpeed = speed; }
    int countUsedEntities() const { return m_countUsedEntities; }
    bool enable() const { return m_enable; }
    void setEnable(bool enable) { m_enable = enable; }
    bool loop() const { return m_loop; }
    void setLoop(bool loop) { m_loop = loop; }
    static float ease(float time, float easeFrom, float easeTo);

private:
    friend class QEntity;
    friend class QScene;
    friend class QScrollEngineContext;

    static float m_animationSpeed_global;

    bool m_enable;
    bool m_loop;
    std::vector<AnimKeyPosition> m_animKeysPosition;
    int m_currentKeyPosition;
    std::vector<AnimKeyOrientation> m_animKeysOrientation;
    int m_currentKeyOrientation;
    std::vector<AnimKeyScale> m_animKeysScale;
    int m_currentKeyScale;

    float m_currentTime;
    float m_animationSpeed;
    float m_endTime;
    int m_countUsedEntities;

    void _entityToAnimation(QEntity* entity) const;
};

}
#endif
