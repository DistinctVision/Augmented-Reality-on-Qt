#ifndef QSKINNEDMESH_H
#define QSKINNEDMESH_H

#include "QScrollEngine/QMesh.h"
#include <QVector4D>
#include <QList>

namespace QScrollEngine {

class QSkinnedMesh:
        public QMesh
{
    Q_OBJECT

public:
    struct Bone {
        QEntity* entity;
        float radius;
        float vertexA;
        float vertexB;
    };

    QSkinnedMesh(QScrollEngineContext* parentContext);
    QSkinnedMesh(QScrollEngineContext* parentContext, QMesh* mesh);
    QSkinnedMesh(QScene* scene);
    QSkinnedMesh(QMesh* mesh);
    ~QSkinnedMesh();

    void setCountVertices(std::size_t count) override;
    void deleteVertices() override;

    void setBones(QEntity* bone, QEntity* entity);

    std::size_t countBones() const;
    Bone getBone(std::size_t index) const;

public slots:
    void deleteBone(QSceneObject3D* object);
    void update();

private:
    struct SkinnedVertex {
        std::size_t boneIndex;
        QVector3D positionOnBone;
        float weight;
    };

    QEntity* m_mainBone;
    QEntity* m_entity;
    std::vector<std::vector<SkinnedVertex>> m_skinnedVertices;
    std::vector<Bone> m_bones;

    void _addBone(QEntity* bone);
    void _computeSkin(const QMatrix4x4& originVertexTransform, const QVector3D& scale);
    void _clearBones();
    void _deleteBone(std::size_t boneIndex);

    QVector3D _getPosition(const std::vector<SkinnedVertex>& s) const;
    void _update(const QMatrix4x4& invOriginVertexTransform);
};

}

#endif // QSKINNEDMESH_H
