#include "QScrollEngine/QSkinnedMesh.h"
#include "QScrollEngine/QScrollEngineContext.h"

namespace QScrollEngine {

QSkinnedMesh::QSkinnedMesh(QScrollEngineContext* parentContext):
    QMesh(parentContext), m_mainBone(nullptr), m_entity(nullptr)
{
}

QSkinnedMesh::QSkinnedMesh(QScrollEngineContext* parentContext, QMesh* mesh):
    QMesh(parentContext, mesh), m_mainBone(nullptr), m_entity(nullptr)
{
    m_skinnedVertices.resize(m_vertices.size());
}

QSkinnedMesh::QSkinnedMesh(QScene* scene):
    QMesh(scene), m_mainBone(nullptr), m_entity(nullptr)
{
}

QSkinnedMesh::QSkinnedMesh(QMesh* mesh):
    QSkinnedMesh(mesh->parentContext(), mesh)
{
}

QSkinnedMesh::~QSkinnedMesh()
{
    _clearBones();
}

void QSkinnedMesh::deleteVertices()
{
    QMesh::deleteVertices();
    m_skinnedVertices.clear();
}

void QSkinnedMesh::setCountVertices(std::size_t count)
{
    QMesh::setCountVertices(count);
    m_skinnedVertices.resize(count);
}

void QSkinnedMesh::setBones(QEntity* bone, QEntity* entity)
{
    m_mainBone = bone;
    m_entity = entity;
    _clearBones();
    _addBone(m_mainBone);
    m_bones.shrink_to_fit();
    m_entity->updateTransform();
    /*QVector3D scale = bone->scale();
    scale.setX((std::fabs(scale.x() > 5e-3f) ? 1.0f / scale.x() : 0.0f));
    scale.setY((std::fabs(scale.y() > 5e-3f) ? 1.0f / scale.y() : 0.0f));
    scale.setZ((std::fabs(scale.z() > 5e-3f) ? 1.0f / scale.z() : 0.0f));*/
    _computeSkin(m_entity->matrixWorld(), bone->scale());
    _update(m_entity->matrixWorld().inverted());
    connect(m_mainBone, SIGNAL(onSceneUpdate()), this, SLOT(update()));
}

void QSkinnedMesh::deleteBone(QSceneObject3D* object)
{
    if (object == m_mainBone) {
        _clearBones();
        m_mainBone = nullptr;
        return;
    }
    for (std::size_t i = 0; i < m_bones.size(); ++i) {
        if (m_bones[i].entity == object) {
            _deleteBone(i);
            return;
        }
    }
}

void QSkinnedMesh::update()
{
    if (m_mainBone == nullptr)
        return;
    if (m_entity == nullptr)
        return;
    if (!m_mainBone->transformHasChanged())
        return;
    m_entity->updateTransform();
    _update(m_entity->matrixWorld().inverted());
}

void QSkinnedMesh::_addBone(QEntity* bone)
{
    connect(bone, SIGNAL(deleting(QSceneObject3D*)), this, SLOT(deleteBone(QSceneObject3D*)), Qt::DirectConnection);
    bone->updateTransform();
    QBoundingBox bb = bone->boundingBox();
    float radius = 1.0f;
    QSh_Light* sh = (bone->countParts() > 0) ? dynamic_cast<QSh_Light*>(bone->shader(0).data()) : nullptr;
    if (sh != nullptr)
        radius = sh->specularIntensity();
    m_bones.push_back({ bone, radius, bb.min().z(), bb.max().z() });
    for (std::size_t i = 0; i < bone->countEntityChilds(); ++i) {
        _addBone(bone->childEntity(i));
    }
}

void QSkinnedMesh::_computeSkin(const QMatrix4x4& originVertexTransform, const QVector3D& scale)
{
    SkinnedVertex sv;
    std::vector<QMatrix4x4> invBoneTransforms;
    invBoneTransforms.resize(m_bones.size());
    std::size_t i, j;
    for (i = 0; i < m_bones.size(); ++i) {
        invBoneTransforms[i] = m_bones[i].entity->matrixWorld().inverted() * originVertexTransform;
    }
    for (i = 0; i < m_vertices.size(); ++i) {
        std::vector<SkinnedVertex>& s = m_skinnedVertices[i];
        s.clear();
        for (j = 0; j < m_bones.size(); ++j) {
            const Bone& bone = m_bones[j];
            sv.positionOnBone = invBoneTransforms[j] * m_vertices[i];
            float x = sv.positionOnBone.x() * scale.x();
            float y = sv.positionOnBone.y() * scale.y();
            if (sv.positionOnBone.z() < bone.vertexA) {
                float z = (bone.vertexA - sv.positionOnBone.z()) * scale.z();
                sv.weight = std::sqrt(x * x + y * y + z * z);
            } else if (sv.positionOnBone.z() > bone.vertexB) {
                float z = (sv.positionOnBone.z() - bone.vertexB) * scale.z();
                sv.weight = std::sqrt(x * x + y * y + z * z);
            } else {
                sv.weight = std::sqrt(x * x + y * y);
            }
            if (sv.weight < bone.radius) {
                sv.boneIndex = j;
                s.push_back(sv);
            }
        }
        if (!s.empty()) {
            if (s.size() == 1) {
                s[0].weight = 1.0f;
            } else {
                float sum = 0.0f;
                for (j = 0; j < s.size(); ++j) {
                    sum += s[j].weight;
                }
                if (sum < 5e-3f) {
                    for (j = 0; j < s.size(); ++j) {
                        s[j].weight = 1.0f / (float) s.size();
                    }
                } else {
                    for (j = 0; j < s.size(); ++j) {
                        s[j].weight = 1.0f - s[j].weight / sum;
                    }
                }
            }
        }
    }
}

void QSkinnedMesh::_clearBones()
{
    m_bones.clear();
    disconnect(this);
    for (auto it = m_skinnedVertices.begin(); it != m_skinnedVertices.end(); ++it)
        it->clear();
}

void QSkinnedMesh::_deleteBone(std::size_t boneIndex)
{
    for (auto it = m_skinnedVertices.begin(); it != m_skinnedVertices.end(); ++it) {
        for (auto jt = it->begin(); jt != it->end(); ) {
            if (jt->boneIndex == boneIndex) {
                jt = it->erase(jt);
            } else if (jt->boneIndex > boneIndex) {
                --jt->boneIndex;
            }
            ++jt;
        }
    }
    m_bones.erase(m_bones.begin() + boneIndex);
}

QVector3D QSkinnedMesh::_getPosition(const std::vector<SkinnedVertex>& s) const
{
    auto it = s.cbegin();
    QVector3D v = (m_bones[it->boneIndex].entity->matrixWorld() * it->positionOnBone) * it->weight;
    ++it;
    while (it != s.cend()) {
        v += (m_bones[it->boneIndex].entity->matrixWorld() * it->positionOnBone) * it->weight;
        ++it;
    }
    return v;
}

void QSkinnedMesh::_update(const QMatrix4x4& invOriginVertexTransform)
{
    for (auto bit = m_bones.begin(); bit != m_bones.end(); ++bit)
        bit->entity->updateTransform();
    auto vit = m_vertices.begin();
    auto it = m_skinnedVertices.cbegin();
    for (; it != m_skinnedVertices.end(); ) {
        if (!it->empty()) {
            *vit = invOriginVertexTransform * _getPosition(*it);
        }
        ++it;
        ++vit;
    }
    updateLocalBoundingBox();
    if (vertexAttributeIsEnabled(QSh::VertexAttributes::Normals))
        updateNormals();
    applyChangesOfVertexPositions();
}

}
