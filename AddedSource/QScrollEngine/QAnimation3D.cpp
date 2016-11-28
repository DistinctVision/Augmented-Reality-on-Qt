#include "QScrollEngine/QAnimation3D.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include <qmath.h>
#include <cassert>

namespace QScrollEngine {

float QAnimation3D::m_animationSpeed_global = 1.0f;

QAnimation3D::QAnimation3D()
{
    m_enable = true;
    m_loop = true;
    m_animKeysPosition.clear();
    m_animKeysOrientation.clear();
    m_animKeysScale.clear();
    m_currentKeyPosition = m_currentKeyOrientation = m_currentKeyScale = -1;
    m_currentTime = 0.0f;
    m_endTime = 0.0f;
    m_animationSpeed = 1.0f;
    m_countUsedEntities = 0;
}

QAnimation3D::~QAnimation3D()
{
    m_animKeysPosition.clear();
    m_animKeysOrientation.clear();
    m_animKeysScale.clear();
}

float QAnimation3D::ease(float time, float easeFrom, float easeTo)
{//TODO make epsilon?
    float k;
    float s = easeFrom + easeTo;
    if (s == 0.0f)
        return time;
    if (s > 1.0f) {
        easeFrom /= s;
        easeTo /= s;
    }
    k = 1.0f / (2.0f - s);
    if (time < easeFrom)
        return ((k / easeFrom) * time * time);
    else {
        if (time <= 1.0f - easeTo){
            return (k * (2.0f * time - easeFrom));
        } else {
            time = 1.0f - time;
            return (1.0f - (k / easeTo) * time * time);
        }
    }
}

void QAnimation3D::deleteAnimKeyPosition(int index)
{
    if (m_currentKeyPosition >= index)
        --m_currentKeyPosition;
    m_animKeysPosition.erase(m_animKeysPosition.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyPosition& key)
{
    if (key.time > m_endTime)
        m_endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(m_animKeysPosition.size()); ++index) {
        if (m_animKeysPosition[index].time == key.time) {
            m_animKeysPosition[index] = key;
            return index;
        } else if (m_animKeysPosition[index].time > key.time) {
            break;
        }
    }
    m_animKeysPosition.insert(m_animKeysPosition.begin() + index, key);
    if (m_currentKeyPosition > index)
        ++m_currentKeyPosition;
    else if (m_currentKeyPosition < 0)
        m_currentKeyPosition = 0;
    return index;
}

void QAnimation3D::deleteAnimKeyOrientation(int index)
{
    if (m_currentKeyOrientation >= index)
        --m_currentKeyOrientation;
    m_animKeysOrientation.erase(m_animKeysOrientation.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyOrientation& key)
{
    if (key.time > m_endTime)
        m_endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(m_animKeysOrientation.size()); ++index) {
        if (m_animKeysOrientation[index].time == key.time) {
            m_animKeysOrientation[index] = key;
            return index;
        } else if (m_animKeysOrientation[index].time > key.time) {
            break;
        }
    }
    m_animKeysOrientation.insert(m_animKeysOrientation.begin() + index, key);
    if (m_currentKeyOrientation > index)
        ++m_currentKeyOrientation;
    else if (m_currentKeyOrientation < 0)
        m_currentKeyOrientation = 0;
    return index;
}

void QAnimation3D::deleteAnimKeyScale(int index)
{
    if (m_currentKeyScale >= index)
        --m_currentKeyScale;
    m_animKeysScale.erase(m_animKeysScale.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyScale& key)
{
    if (key.time > m_endTime)
        m_endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(m_animKeysScale.size()); ++index) {
        if (m_animKeysScale[index].time == key.time) {
            m_animKeysScale[index] = key;
            return index;
        } else if (m_animKeysScale[index].time > key.time) {
            break;
        }
    }
    m_animKeysScale.insert(m_animKeysScale.begin() + index, key);
    if (m_currentKeyScale > index)
        ++m_currentKeyScale;
    else if (m_currentKeyScale < 0)
        m_currentKeyScale = 0;
    return index;
}

void QAnimation3D::updateFrame()
{
    setAnimationTime(m_currentTime + m_animationSpeed * m_animationSpeed_global);
}

void QAnimation3D::setAnimationTime(float time)
{
    if (time > m_endTime) {
        if (m_loop) {
            do {
               time -= m_endTime;
            } while (time > m_endTime);
        } else {
            time = m_endTime;
        }
    }
    if (time > m_currentTime) {
        m_currentTime = time;
        int nextKey;
        for(nextKey = m_currentKeyPosition + 1;
            (nextKey < static_cast<int>(m_animKeysPosition.size())) && (m_currentTime >= m_animKeysPosition[nextKey].time);
            m_currentKeyPosition = nextKey, ++nextKey);
        for(nextKey = m_currentKeyOrientation + 1;
            (nextKey < static_cast<int>(m_animKeysOrientation.size())) && (m_currentTime >= m_animKeysOrientation[nextKey].time);
            m_currentKeyOrientation = nextKey, ++nextKey);
        for(nextKey = m_currentKeyScale + 1;
            (nextKey < static_cast<int>(m_animKeysScale.size())) && (m_currentTime >= m_animKeysScale[nextKey].time);
            m_currentKeyScale = nextKey, ++nextKey);
    } else {
        m_currentTime = time;
        int prevKey;
        for(prevKey = m_currentKeyPosition - 1;
            (prevKey >= 0) && (m_currentTime < m_animKeysPosition[m_currentKeyPosition].time);
            m_currentKeyPosition = prevKey, --prevKey);
        for(prevKey = m_currentKeyOrientation - 1;
            (prevKey >= 0) && (m_currentTime < m_animKeysOrientation[m_currentKeyOrientation].time);
            m_currentKeyOrientation = prevKey, --prevKey);
        for(prevKey = m_currentKeyScale;
            (prevKey >= 0) && (m_currentTime < m_animKeysScale[m_currentKeyScale].time);
            m_currentKeyScale = prevKey, --prevKey);
    }
}

void QAnimation3D::_entityToAnimation(QEntity* entity) const
{
    float t;
    int nextKey;
    if (m_currentKeyPosition >= 0) {
        nextKey = m_currentKeyPosition + 1;
        if (nextKey < static_cast<int>(m_animKeysPosition.size())) {
            t = (m_currentTime - m_animKeysPosition[m_currentKeyPosition].time) /
                static_cast<float>(m_animKeysPosition[nextKey].time - m_animKeysPosition[m_currentKeyPosition].time);
            entity->m_position = m_animKeysPosition[m_currentKeyPosition].position +
                    (m_animKeysPosition[nextKey].position - m_animKeysPosition[m_currentKeyPosition].position) * t;
        } else
            entity->m_position = m_animKeysPosition[m_currentKeyPosition].position;
    }
    if (m_currentKeyOrientation >= 0) {
        nextKey = m_currentKeyOrientation + 1;
        if (nextKey < static_cast<int>(m_animKeysOrientation.size())) {
            t = (m_currentTime - m_animKeysOrientation[m_currentKeyOrientation].time) /
                static_cast<float>(m_animKeysOrientation[nextKey].time - m_animKeysOrientation[m_currentKeyOrientation].time);
            entity->m_orientation = QQuaternion::slerp(m_animKeysOrientation[m_currentKeyOrientation].orienation,
                    m_animKeysOrientation[nextKey].orienation, t);
        } else
            entity->m_orientation = m_animKeysOrientation[m_currentKeyOrientation].orienation;
    }
    if (m_currentKeyScale >= 0) {
        nextKey = m_currentKeyScale + 1;
        if (nextKey < static_cast<int>(m_animKeysScale.size())) {
            t = (m_currentTime - m_animKeysScale[m_currentKeyScale].time) /
                static_cast<float>(m_animKeysScale[nextKey].time - m_animKeysScale[m_currentKeyScale].time);
            entity->m_scale = m_animKeysScale[m_currentKeyScale].scale +
                    (m_animKeysScale[nextKey].scale - m_animKeysScale[m_currentKeyScale].scale) * t;
        } else
            entity->m_scale = m_animKeysScale[m_currentKeyScale].scale;
    }
}

void QAnimation3D::entityToAnimation(QEntity* entity) const
{
    _entityToAnimation(entity);
    entity->m_transformHasChanged = true;
}

}
