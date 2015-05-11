#ifndef QSCENEOBJECT3D_H
#define QSCENEOBJECT3D_H

#include <vector>

#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>

namespace QScrollEngine {

class QScene;
class QScrollEngineContext;
class QEntity;

class QSceneObject3D:
        public QObject
{
    Q_OBJECT
    friend class QScrollEngineContext;
    friend class QScene;

public:
    typedef struct MatrixOfObject
    {
        QMatrix4x4 world;
        QMatrix4x4 worldViewProj;
    } MatrixOfObject;

public:
    QSceneObject3D() { _transformHasChanged = true; }
    ~QSceneObject3D() { emit deleting(); }
    QEntity* parentEntity() const { return _parentEntity; }
    QMatrix4x4 matrixWorld() const { return _matrix.world; }
    QMatrix4x4 matrixWorldViewProj() const { return _matrix.worldViewProj; }
    void updateMatrxWorldViewProj(const QMatrix4x4& matrixViewProj) { _matrix.worldViewProj =
                matrixViewProj * _matrix.world; }

    QScene* scene() const { return _scene; }
    QVector3D position() const { return _position; }
    QVector3D globalPosition() const { return _globalPosition; }
    bool transformHasChanged() const { return _transformHasChanged; }
    bool getGlobalParameters(QVector3D& globalScale, QMatrix3x3& globalOrieantion) const;
    bool getGlobalParameters(QVector3D& globalScale, QQuaternion& globalOrieantion) const;
    QVector3D fromLocalToGlobal(const QVector3D& localPoint) const;
    QScrollEngineContext* parentContext();

signals:
    void deleting();

public slots:
    void setChangedTransform() { _transformHasChanged = true; }

protected:
    QEntity* _parentEntity;
    MatrixOfObject _matrix;
    QScene* _scene;
    QVector3D _position;
    QVector3D _globalPosition;
    bool _transformHasChanged;
};

}
#endif
