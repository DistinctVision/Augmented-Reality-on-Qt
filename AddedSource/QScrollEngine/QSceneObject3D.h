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

public:
    struct MatrixOfObject
    {
        QMatrix4x4 world;
        QMatrix4x4 worldViewProj;
    };

public:
    QSceneObject3D();
    ~QSceneObject3D();
    QEntity* parentEntity() const;
    QMatrix4x4 matrixWorld() const;
    QMatrix4x4 matrixWorldViewProj() const;
    void updateMatrxWorldViewProj(const QMatrix4x4& matrixViewProj);

    QScene* scene() const;
    QVector3D position() const;
    QVector3D globalPosition() const;
    bool transformHasChanged() const;
    bool getGlobalParameters(QVector3D& globalScale, QMatrix3x3& globalOrieantion) const;
    bool getGlobalParameters(QVector3D& globalScale, QQuaternion& globalOrieantion) const;
    QVector3D fromLocalToGlobal(const QVector3D& localPoint) const;
    QScrollEngineContext* parentContext();

signals:
    void onDelete(QSceneObject3D* object);
    void onSceneUpdate();

public slots:
    void setChangedTransform();

protected:
    friend class QScrollEngineContext;
    friend class QScene;

    QEntity* m_parentEntity;
    MatrixOfObject m_matrix;
    QScene* m_scene;
    QVector3D m_position;
    QVector3D m_globalPosition;
    bool m_transformHasChanged;
};

}
#endif
