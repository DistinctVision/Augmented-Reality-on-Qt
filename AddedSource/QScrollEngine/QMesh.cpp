#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include <QDebug>

#include <QVector4D>

#include <cfloat>

namespace QScrollEngine {

QMesh::QMesh(QScrollEngineContext* parentContext)
{
    m_buffers.resize(5);
    m_parentContext = parentContext;
    m_sizeOfElement = 3;
    m_countUsedParts = 0;
    _initBuffers();
}

QMesh::QMesh(QScrollEngineContext* parentContext, QMesh* mesh)
{
    m_buffers.resize(5);
    m_countUsedParts = 0;
    m_parentContext = parentContext;
    m_sizeOfElement = mesh->sizeOfElement();
    _initBuffers();
    m_localBoundingBox = mesh->m_localBoundingBox;
    m_elements = mesh->elements();
    m_vertices = mesh->vertices();
    setUsagePatternOfElements(mesh->usagePatternOfElements());
    setUsagePatternOfVertexPositions(mesh->usagePatternOfVertexPositions());
    if (mesh->vertexAttributeIsEnabled(QSh::VertexAttributes::TextureCoords)) {
        enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
        m_textureCoords = mesh->textureCoords();
        setUsagePatternOfVertexAttributes(QSh::VertexAttributes::TextureCoords,
                                          mesh->usagePatternOfVertexAttributes(QSh::VertexAttributes::TextureCoords));
    }
    if (mesh->vertexAttributeIsEnabled(QSh::VertexAttributes::Normals)) {
        enableVertexAttribute(QSh::VertexAttributes::Normals);
        m_normals = mesh->normals();
        setUsagePatternOfVertexAttributes(QSh::VertexAttributes::Normals,
                                          mesh->usagePatternOfVertexAttributes(QSh::VertexAttributes::Normals));
    }
    if (mesh->vertexAttributeIsEnabled(QSh::VertexAttributes::RgbColors)) {
        enableVertexAttribute(QSh::VertexAttributes::RgbColors);
        m_rgbColors = mesh->colors();
        setUsagePatternOfVertexAttributes(QSh::VertexAttributes::RgbColors,
                                          mesh->usagePatternOfVertexAttributes(QSh::VertexAttributes::RgbColors));
    }
    applyChanges();
}

QMesh::QMesh(QScene* scene)
{
    m_buffers.resize(5);
    m_sizeOfElement = 3;
    m_parentContext = (scene != nullptr) ? scene->parentContext() : nullptr;
    m_countUsedParts = 0;
    _initBuffers();
}

QMesh::QMesh(QMesh* mesh):QMesh(mesh->parentContext(), mesh)
{
}

void QMesh::setParentContext(QScrollEngineContext* parentContext)
{
    if (m_parentContext == parentContext)
        return;
    _deleteBuffers();
    m_parentContext = parentContext;
    _initBuffers();
    applyChanges();
}

bool QMesh::_initBuffers()
{
    if (m_parentContext == nullptr)
        return false;

    m_buffers[0].vboIds = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_buffers[0].vboIds.create();
    m_buffers[0].applyChanges = [this]() {
        if (m_buffers[0].vboIds.isCreated()) {
            m_buffers[0].vboIds.bind();
            m_buffers[0].vboIds.allocate(m_elements.data(), m_elements.size() * sizeof(GLuint));
        }
    };
    m_buffers[0].clear = [this]() {
        m_elements.clear();
        if (m_buffers[0].vboIds.isCreated())
            m_buffers[0].vboIds.destroy();
    };
    m_buffers[1].vboIds= QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_buffers[1].vboIds.create();
    m_buffers[1].applyChanges = [this]() {
        if (m_buffers[1].vboIds.isCreated()) {
            m_buffers[1].vboIds.bind();
            m_buffers[1].vboIds.allocate(m_vertices.data(), m_vertices.size() * sizeof(QVector3D));
        }
    };
    m_buffers[1].clear = [this]() {
        m_vertices.clear();
        if (m_buffers[1].vboIds.isCreated())
            m_buffers[1].vboIds.destroy();
    };
    m_buffers[2].applyChanges = [this]() {
        if (m_buffers[2].vboIds.isCreated()) {
            m_buffers[2].vboIds.bind();
            m_buffers[2].vboIds.allocate(m_textureCoords.data(), m_textureCoords.size() * sizeof(QVector2D));
        }
    };
    m_buffers[2].clear = [this]() {
        m_textureCoords.clear();
        if (m_buffers[2].vboIds.isCreated())
            m_buffers[2].vboIds.destroy();
    };
    m_buffers[2].bind = [this]() {
        m_parentContext->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2].vboIds.bufferId());
        m_parentContext->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
    };
    m_buffers[3].applyChanges = [this]() {
        if (m_buffers[3].vboIds.isCreated()) {
            m_buffers[3].vboIds.bind();
            m_buffers[3].vboIds.allocate(m_normals.data(), m_normals.size() * sizeof(QVector3D));
        }
    };
    m_buffers[3].clear = [this]() {
        m_normals.clear();
        if (m_buffers[3].vboIds.isCreated())
            m_buffers[3].vboIds.destroy();
    };
    m_buffers[3].bind = [this]() {
        m_parentContext->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3].vboIds.bufferId());
        m_parentContext->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    };
    m_buffers[4].applyChanges = [this]() {
        if (m_buffers[4].vboIds.isCreated()) {
            m_buffers[4].vboIds.bind();
            m_buffers[4].vboIds.allocate(m_rgbColors.data(), m_rgbColors.size() * sizeof(RgbColor));
        }
    };
    m_buffers[4].clear = [this]() {
        m_rgbColors.clear();
        if (m_buffers[4].vboIds.isCreated())
            m_buffers[4].vboIds.destroy();
    };
    m_buffers[4].bind = [this]() {
        m_parentContext->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[4].vboIds.bufferId());
        m_parentContext->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RgbColor), nullptr);
    };

    return true;
}

void QMesh::_deleteMeshInEverywhere()
{
    for (std::size_t i = 0; i < m_parentContext->countScenes(); ++i) {
        QScene* scene = m_parentContext->scene(i);
        for (std::size_t j = 0; j < scene->countEntities(); ++j) {
            QEntity* entity = scene->entity(j);
            for (std::size_t k = 0; k < entity->countParts(); ++k) {
                if (entity->part(k)->mesh() == this) {
                    if (m_countUsedParts == 1) {
                        m_countUsedParts = 2;
                        entity->part(k)->setMesh(nullptr);
                        m_countUsedParts = 0;
                        return;
                    } else {
                        entity->part(k)->setMesh(nullptr);
                        --m_countUsedParts;
                    }
                }
            }
        }
    }
}

QMesh::~QMesh()
{
    _deleteBuffers();
    if (m_countUsedParts > 0) {
        qDebug() << "QScrollEngine: Warning. Deleting mesh with used entities.";
        _deleteMeshInEverywhere();
    }
}

void QMesh::_deleteBuffers()
{
    if (m_parentContext == nullptr) {
        //qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    for (int i = 0; i < m_buffers.size(); ++i)
        m_buffers[i].clear();
}

void QMesh::applyChanges()
{
    if (m_parentContext == nullptr) {
        //qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    applyChangesOfElements();
    applyChangesOfVertexPositions();
    for (int i = 2; i < m_buffers.size(); ++i)
        applyChangesOfAttrtibute(QSh::VertexAttributes(i));
}

void QMesh::applyChangesOfElements()
{
    if (m_parentContext == nullptr) {
        //qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    m_buffers[0].applyChanges();
}

void QMesh::applyChangesOfVertexPositions()
{
    if (m_parentContext == nullptr) {
        //qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    m_buffers[1].applyChanges();
}

void QMesh::applyChangesOfAttrtibute(QSh::VertexAttributes attribute)
{
    if (m_parentContext == nullptr) {
        //qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    m_buffers[(int)attribute].applyChanges();
}

bool QMesh::bind(const std::vector<QSh::VertexAttributes>& attributes) const
{
    QMESH_ASSERT(m_parentContext != nullptr);
    QMESH_ASSERT(checkVertices() && checkElements());
    m_parentContext->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[0].vboIds.bufferId());
    m_parentContext->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1].vboIds.bufferId());
    m_parentContext->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    for (std::vector<QSh::VertexAttributes>::const_iterator it = attributes.cbegin();
         it != attributes.cend();
         ++it) {
        if (!m_buffers[(int)(*it)].vboIds.isCreated())
            return false;
        m_buffers[(int)(*it)].bind();
    }
    return true;
}

void QMesh::setUsagePatternOfElements(QOpenGLBuffer::UsagePattern usagePattern)
{
    m_buffers[0].vboIds.setUsagePattern(usagePattern);
}

QOpenGLBuffer::UsagePattern QMesh::usagePatternOfElements() const
{
    return m_buffers[0].vboIds.usagePattern();
}

void QMesh::setUsagePatternOfVertexPositions(QOpenGLBuffer::UsagePattern usagePattern)
{
    m_buffers[1].vboIds.setUsagePattern(usagePattern);
}

QOpenGLBuffer::UsagePattern QMesh::usagePatternOfVertexPositions() const
{
    return m_buffers[1].vboIds.usagePattern();
}

void QMesh::setUsagePatternOfVertexAttributes(QSh::VertexAttributes attribute, QOpenGLBuffer::UsagePattern usagePattern)
{
    m_buffers[(int)attribute].vboIds.setUsagePattern(usagePattern);
}

QOpenGLBuffer::UsagePattern QMesh::usagePatternOfVertexAttributes(QSh::VertexAttributes attribute) const
{
    return m_buffers[(int)attribute].vboIds.usagePattern();
}

void QMesh::enableVertexAttribute(QSh::VertexAttributes attribute)
{
    if (!m_buffers[(int)attribute].vboIds.isCreated()) {
        m_buffers[(int)attribute].vboIds = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_buffers[(int)attribute].vboIds.create();
    }
}

void QMesh::disableAttribute(QSh::VertexAttributes attribute)
{
    if (m_buffers[(int)attribute].vboIds.isCreated())
        m_buffers[(int)attribute].vboIds.destroy();
}

bool QMesh::vertexAttributeIsEnabled(QSh::VertexAttributes attribute)
{
    return m_buffers[(int)attribute].vboIds.isCreated();
}

void QMesh::disableAllVertexAttributes()
{
    for (int i = 2; i < m_buffers.size(); ++i)
        m_buffers[i].vboIds.destroy();
}

void QMesh::clear()
{
    deleteElements();
    deleteVertices();
    disableAllVertexAttributes();
}

void QMesh::setCountVertices(std::size_t count)
{
    //std::size_t prevCount = _vertices.size();
    m_vertices.resize(count);
    if (m_buffers[2].vboIds.isCreated())
        m_textureCoords.resize(count);
    if (m_buffers[3].vboIds.isCreated())
        m_normals.resize(count);
    if (m_buffers[4].vboIds.isCreated())
        m_rgbColors.resize(count);
}

void QMesh::moveVertices(const QVector3D& deltaPosition)
{
    for (std::vector<QVector3D>::iterator it = m_vertices.begin(); it != m_vertices.end(); ++it) {
        *it += deltaPosition;
    }
}

void QMesh::scaleVertices(const QVector3D& scale)
{
    for (std::vector<QVector3D>::iterator it = m_vertices.begin(); it != m_vertices.end(); ++it) {
        *it *= scale;
    }
}

void QMesh::transformVertices(const QMatrix4x4& transform)
{
    for (std::vector<QVector3D>::iterator it = m_vertices.begin(); it != m_vertices.end(); ++it) {
        *it = QOtherMathFunctions::transform(transform, *it);
    }
}

void QMesh::moveTextureCoords(float x, float y)
{
    for (std::vector<QVector2D>::iterator it = m_textureCoords.begin(); it != m_textureCoords.end(); ++it) {
        it->setX(it->x() + x);
        it->setY(it->y() + y);
    }
}

void QMesh::scaleTextureCoords(float x, float y)
{
    for (std::vector<QVector2D>::iterator it = m_textureCoords.begin(); it != m_textureCoords.end(); ++it) {
        it->setX(it->x() * x);
        it->setY(it->y() * y);
    }
}

void QMesh::transformTextureCoords(const QMatrix3x3& transform)
{
    for (std::vector<QVector2D>::iterator it = m_textureCoords.begin(); it != m_textureCoords.end(); ++it) {
        *it = QOtherMathFunctions::transform(transform, *it);
    }
}

void QMesh::flipTriangles()
{
    QMESH_ASSERT(isTriangles());
    std::size_t count = countTriangles();
    for (std::size_t i=0; i<count; ++i)
        std::swap(m_elements[i * 3], m_elements[i * 3 + 1]);
}

bool QMesh::checkIndicesOfElements()
{
    bool isRight = true, isCurRight;
    std::size_t countVertices = m_vertices.size();
    std::vector<GLuint> elements;
    elements.reserve(m_elements.size());
    std::size_t countElements = m_elements.size() / m_sizeOfElement;
    if (m_elements.size() != (countElements * m_sizeOfElement)) {
        m_elements.resize(countElements * m_sizeOfElement);
        isRight = false;
    }
    GLuint* curElement;
    GLuint i;
    GLsizei j;
    for (i=0; i<countElements; ++i) {
        curElement = &m_elements[i * m_sizeOfElement];
        isCurRight = true;
        for (j = 0; j < m_sizeOfElement; ++j) {
            if (curElement[j] >= countVertices) {
                isCurRight = false;
                break;
            }
        }
        if (isCurRight) {
            for (j = 0; j < m_sizeOfElement; ++j)
                elements.push_back(curElement[j]);
        } else {
            isRight = false;
        }
    }
    elements.shrink_to_fit();
    m_elements.swap(elements);
    return isRight;
}

bool QMesh::checkVertices() const
{
    if (m_buffers[2].vboIds.isCreated()) {
        if (m_textureCoords.size() != m_vertices.size())
            return false;
    }
    if (m_buffers[3].vboIds.isCreated()) {
        if (m_normals.size() != m_vertices.size())
            return false;
    }
    if (m_buffers[4].vboIds.isCreated()) {
        if (m_rgbColors.size() != m_vertices.size())
            return  false;
    }
    return true;
}

bool QMesh::checkElements() const
{
    return ((m_elements.size() % m_sizeOfElement) == 0);
}

void QMesh::flipNormals()
{
    for (std::vector<QVector3D>::iterator it = m_normals.begin(); it != m_normals.end(); ++it)
        *it = - *it;
}

void QMesh::transformNormals(const QMatrix3x3& transform)
{
    for (std::vector<QVector3D>::iterator it = m_normals.begin(); it != m_normals.end(); ++it) {
        *it = QOtherMathFunctions::transform(transform, *it);
    }
}

void QMesh::updateNormals()
{
    if (isTriangles()) {
        enableVertexAttribute(QSh::VertexAttributes::Normals);
        if (m_normals.size() != m_vertices.size())
            m_normals.resize(m_vertices.size());
        std::size_t i;
        for (i=0; i<m_normals.size(); i++)
            m_normals[i] = QVector3D(0.0f, 0.0f, 0.0f);
        QVector3D dir, p0;
        std::size_t countTr = countTriangles();
        for (i=0; i<countTr; ++i) {
            GLuint* tr = triangle(i);
            p0 = m_vertices[tr[1]];
            dir = QVector3D::crossProduct(m_vertices[tr[0]] - p0, m_vertices[tr[2]] - p0);
            dir.normalize();
            m_normals[tr[0]] += dir;
            m_normals[tr[1]] += dir;
            m_normals[tr[2]] += dir;
        }
        for (i=0; i<m_vertices.size(); i++)
            m_normals[i].normalize();
    } else {
        qDebug() << "QScrollEngine: Error. Update normals - just only for triangles.";
    }
}

void QMesh::updateLocalBoundingBox()
{
    if (m_vertices.size() == 0) {
        m_localBoundingBox.deactivate();
        return;
    }
    std::vector<QVector3D>::iterator it = m_vertices.begin();
    m_localBoundingBox.toPoint(*it);
    for (++it; it != m_vertices.end(); ++it)
        m_localBoundingBox.addPoint(*it);
    emit changedLocalBoundingBox();
}


void QMesh::setLocalBoundingBox(const QBoundingBox& boundingBox)
{
    m_localBoundingBox = boundingBox;
    emit changedLocalBoundingBox();
}

bool QMesh::pickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                    const QVector3D& rayPoint, const QVector3D& rayDir) const
{
    if (isTriangles()) {
        float minDis = FLT_MAX, dis;
        bool finded = false;
        std::size_t i, resultIndex;
        std::size_t countTr = countTriangles();
        const GLuint* tr;
        for (i = 0; i < countTr; ++i) {
            tr = triangle(i);
            if (QOtherMathFunctions::collisionTriangleRay(resultPoint, m_vertices[tr[0]],
                                                                       m_vertices[tr[1]],
                                                                       m_vertices[tr[2]], rayPoint, rayDir)) {
                dis = (resultPoint - rayPoint).lengthSquared();
                if (dis < minDis) {
                    resultIndex = i;
                    minDis = dis;
                    finded = true;
                }
            }
        }
        if (!finded)
            return false;
        tr = triangle(resultIndex);
        resultTriangleDir = QVector3D::crossProduct(m_vertices[tr[0]] - m_vertices[tr[1]],
                                                    m_vertices[tr[2]] - m_vertices[tr[1]]);
        return true;
    }
    qDebug() << "QScrollEngine: Error. Pick ray - just only for triangles.";
    return false;
}

bool QMesh::fastPickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                    const QVector3D& rayPoint, const QVector3D& rayDir) const
{
    if (isTriangles()) {
        bool finded = false;
        std::size_t i, resultIndex;
        std::size_t countTr = countTriangles();
        const GLuint* tr;
        for (i=0; i<countTr; ++i) {
            tr = triangle(i);
            if (QOtherMathFunctions::collisionTriangleRay(resultPoint, m_vertices[tr[0]],
                                                                       m_vertices[tr[1]],
                                                                       m_vertices[tr[2]], rayPoint, rayDir)) {
                resultIndex = i;
                finded = true;
                break;
            }
        }
        if (!finded)
            return false;
        tr = triangle(resultIndex);
        resultTriangleDir = QVector3D::crossProduct(m_vertices[tr[0]] - m_vertices[tr[1]],
                                                    m_vertices[tr[2]] - m_vertices[tr[1]]);
        return true;
    }
    qDebug() << "QScrollEngine: Error. Fast pick ray - just only for triangles.";
    return false;
}

}
