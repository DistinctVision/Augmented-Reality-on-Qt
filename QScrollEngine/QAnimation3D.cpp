#include "QScrollEngine/QAnimation3D.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include <qmath.h>
#include <cassert>

namespace QScrollEngine {

float QAnimation3D::_animationSpeed_global = 1.0f;

QAnimation3D::QAnimation3D()
{
    _enable = true;
    _loop = true;
    _animKeysPosition.clear();
    _animKeysOrientation.clear();
    _animKeysScale.clear();
    _currentKeyPosition = _currentKeyOrientation = _currentKeyScale = -1;
    _currentTime = 0.0f;
    _endTime = 0.0f;
    _animationSpeed = 1.0f;
    _countUsedEntities = 0;
}

QAnimation3D::~QAnimation3D()
{
    _animKeysPosition.clear();
    _animKeysOrientation.clear();
    _animKeysScale.clear();
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
    if (_currentKeyPosition >= index)
        --_currentKeyPosition;
    _animKeysPosition.erase(_animKeysPosition.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyPosition& key)
{
    if (key.time > _endTime)
        _endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(_animKeysPosition.size()); ++index) {
        if (_animKeysPosition[index].time == key.time) {
            _animKeysPosition[index] = key;
            return index;
        } else if (_animKeysPosition[index].time > key.time) {
            break;
        }
    }
    _animKeysPosition.insert(_animKeysPosition.begin() + index, key);
    if (_currentKeyPosition > index)
        ++_currentKeyPosition;
    else if (_currentKeyPosition < 0)
        _currentKeyPosition = 0;
    return index;
}

void QAnimation3D::deleteAnimKeyOrientation(int index)
{
    if (_currentKeyOrientation >= index)
        --_currentKeyOrientation;
    _animKeysOrientation.erase(_animKeysOrientation.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyOrientation& key)
{
    if (key.time > _endTime)
        _endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(_animKeysOrientation.size()); ++index) {
        if (_animKeysOrientation[index].time == key.time) {
            _animKeysOrientation[index] = key;
            return index;
        } else if (_animKeysOrientation[index].time > key.time) {
            break;
        }
    }
    _animKeysOrientation.insert(_animKeysOrientation.begin() + index, key);
    if (_currentKeyOrientation > index)
        ++_currentKeyOrientation;
    else if (_currentKeyOrientation < 0)
        _currentKeyOrientation = 0;
    return index;
}

void QAnimation3D::deleteAnimKeyScale(int index)
{
    if (_currentKeyScale >= index)
        --_currentKeyScale;
    _animKeysScale.erase(_animKeysScale.begin() + index);
}

int QAnimation3D::addAnimKey(const AnimKeyScale& key)
{
    if (key.time > _endTime)
        _endTime = key.time;
    int index = 0;
    for (; index<static_cast<int>(_animKeysScale.size()); ++index) {
        if (_animKeysScale[index].time == key.time) {
            _animKeysScale[index] = key;
            return index;
        } else if (_animKeysScale[index].time > key.time) {
            break;
        }
    }
    _animKeysScale.insert(_animKeysScale.begin() + index, key);
    if (_currentKeyScale > index)
        ++_currentKeyScale;
    else if (_currentKeyScale < 0)
        _currentKeyScale = 0;
    return index;
}

void QAnimation3D::updateFrame()
{
    setAnimationTime(_currentTime + _animationSpeed * _animationSpeed_global);
}

void QAnimation3D::setAnimationTime(float time)
{
    if (time > _endTime) {
        if (_loop) {
            do {
               time -= _endTime;
            } while (time > _endTime);
        } else {
            time = _endTime;
        }
    }
    if (time > _currentTime) {
        _currentTime = time;
        int nextKey;
        for(nextKey = _currentKeyPosition + 1;
            (nextKey < static_cast<int>(_animKeysPosition.size())) && (_currentTime >= _animKeysPosition[nextKey].time);
            _currentKeyPosition = nextKey, ++nextKey);
        for(nextKey = _currentKeyOrientation + 1;
            (nextKey < static_cast<int>(_animKeysOrientation.size())) && (_currentTime >= _animKeysOrientation[nextKey].time);
            _currentKeyOrientation = nextKey, ++nextKey);
        for(nextKey = _currentKeyScale + 1;
            (nextKey < static_cast<int>(_animKeysScale.size())) && (_currentTime >= _animKeysScale[nextKey].time);
            _currentKeyScale = nextKey, ++nextKey);
    } else {
        _currentTime = time;
        int prevKey;
        for(prevKey = _currentKeyPosition - 1;
            (prevKey >= 0) && (_currentTime < _animKeysPosition[_currentKeyPosition].time);
            _currentKeyPosition = prevKey, --prevKey);
        for(prevKey = _currentKeyOrientation - 1;
            (prevKey >= 0) && (_currentTime < _animKeysOrientation[_currentKeyOrientation].time);
            _currentKeyOrientation = prevKey, --prevKey);
        for(prevKey = _currentKeyScale;
            (prevKey >= 0) && (_currentTime < _animKeysScale[_currentKeyScale].time);
            _currentKeyScale = prevKey, --prevKey);
    }
}

void QAnimation3D::_entityToAnimation(QEntity* entity) const
{
    float t;
    int nextKey;
    if (_currentKeyPosition >= 0) {
        nextKey = _currentKeyPosition + 1;
        if (nextKey < static_cast<int>(_animKeysPosition.size())) {
            t = (_currentTime - _animKeysPosition[_currentKeyPosition].time) /
                static_cast<float>(_animKeysPosition[nextKey].time - _animKeysPosition[_currentKeyPosition].time);
            entity->_position = _animKeysPosition[_currentKeyPosition].position +
                    (_animKeysPosition[nextKey].position - _animKeysPosition[_currentKeyPosition].position) * t;
        } else
            entity->_position = _animKeysPosition[_currentKeyPosition].position;
    }
    if (_currentKeyOrientation >= 0) {
        nextKey = _currentKeyOrientation + 1;
        if (nextKey < static_cast<int>(_animKeysOrientation.size())) {
            t = (_currentTime - _animKeysOrientation[_currentKeyOrientation].time) /
                static_cast<float>(_animKeysOrientation[nextKey].time - _animKeysOrientation[_currentKeyOrientation].time);
            entity->_orientation = QQuaternion::slerp(_animKeysOrientation[_currentKeyOrientation].orienation,
                    _animKeysOrientation[nextKey].orienation, t);
        } else
            entity->_orientation = _animKeysOrientation[_currentKeyOrientation].orienation;
    }
    if (_currentKeyScale >= 0) {
        nextKey = _currentKeyScale + 1;
        if (nextKey < static_cast<int>(_animKeysScale.size())) {
            t = (_currentTime - _animKeysScale[_currentKeyScale].time) /
                static_cast<float>(_animKeysScale[nextKey].time - _animKeysScale[_currentKeyScale].time);
            entity->_scale = _animKeysScale[_currentKeyScale].scale +
                    (_animKeysScale[nextKey].scale - _animKeysScale[_currentKeyScale].scale) * t;
        } else
            entity->_scale = _animKeysScale[_currentKeyScale].scale;
    }
}

void QAnimation3D::entityToAnimation(QEntity* entity) const
{
    _entityToAnimation(entity);
    entity->_transformHasChanged = true;
}

}
