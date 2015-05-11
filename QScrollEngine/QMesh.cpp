#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include <QDebug>

#include <QVector4D>

#include <cassert>
#include <cfloat>

namespace QScrollEngine {

QMesh::QMesh(QScrollEngineContext* parentContext)
{
    assert(parentContext != nullptr);
    _parentContext = parentContext;
    _sizeOfElement = 3;
    _enable_vertex_normal = false;
    _countUsedParts = 0;
    _vboIds[0].create();
    _vboIds[1].create();
    _vboIds[2].create();
    _vboIds[3].create();
}

QMesh::QMesh(QScrollEngineContext* parentContext, QMesh* mesh)
{
    assert(parentContext != nullptr);
    _countUsedParts = 0;
    _parentContext = parentContext;
    _sizeOfElement = mesh->sizeOfElement();
    _vboIds[0].create();
    _vboIds[1].create();
    _vboIds[2].create();
    _vboIds[3].create();
    _enable_vertex_normal = mesh->enable_vertex_normal();
    _localBoundingBox = mesh->_localBoundingBox;
    setCountVertices(mesh->countVertices());
    setCountElements(mesh->countElements());
    setEnable_vertex_normal(mesh->enable_vertex_normal());
    unsigned int i;
    if (_enable_vertex_normal) {
        for (i=0; i<mesh->countVertices(); i++) {
            setVertexPosition(i, mesh->vertexPosition(i));
            setVertexTextureCoord(i, mesh->vertexTextureCoord(i));
            setVertexNormal(i, mesh->vertexNormal(i));
        }
    } else {
        for (i=0; i<mesh->countVertices(); i++) {
            setVertexPosition(i, mesh->vertexPosition(i));
            setVertexTextureCoord(i, mesh->vertexTextureCoord(i));
        }
    }
    for (i=0; i<mesh->countElements(); i++)
        setElement(i, mesh->element(i));
    setUsagePatternVertices(mesh->usagePatternVertices());
    setUsagePatternElements(mesh->usagePatternElements());
    applyChanges();
}

QMesh::QMesh(QScene* scene)
{
    assert(scene != nullptr);
    assert(scene->parentContext() != nullptr);
    _sizeOfElement = 3;
    _parentContext = scene->parentContext();
    _enable_vertex_normal = false;
    _countUsedParts = 0;
    _vboIds[0].create();
    _vboIds[1].create();
    _vboIds[2].create();
    _vboIds[3].create();
}

QMesh::QMesh(QMesh* mesh)
{
    _parentContext = mesh->parentContext();
    assert(_parentContext != nullptr);
    _sizeOfElement = mesh->sizeOfElement();
    _enable_vertex_normal = mesh->enable_vertex_normal();
    _countUsedParts = 0;
    _vboIds[0].create();
    _vboIds[1].create();
    _vboIds[2].create();
    _vboIds[3].create();
    _localBoundingBox = mesh->_localBoundingBox;
    setCountVertices(mesh->countVertices());
    setCountElements(mesh->countElements());
    setEnable_vertex_normal(mesh->enable_vertex_normal());
    unsigned int i;
    if (_enable_vertex_normal) {
        for (i=0; i<mesh->countVertices(); i++) {
            setVertexPosition(i, mesh->vertexPosition(i));
            setVertexTextureCoord(i, mesh->vertexTextureCoord(i));
            setVertexNormal(i, mesh->vertexNormal(i));
        }
    } else {
        for (i=0; i<mesh->countVertices(); i++) {
            setVertexPosition(i, mesh->vertexPosition(i));
            setVertexTextureCoord(i, mesh->vertexTextureCoord(i));
        }
    }
    for (i=0; i<mesh->countElements(); i++)
        setElement(i, mesh->triangle(i));
    applyChanges();
}

void QMesh::setParentContext(QScrollEngineContext* parentContext)
{
    if (_parentContext == parentContext)
        return;
    assert(parentContext != nullptr);
    _vboIds[0].destroy();
    _vboIds[1].destroy();
    _vboIds[2].destroy();
    _vboIds[3].destroy();
    _parentContext = parentContext;
    _vboIds[0].create();
    _vboIds[1].create();
    _vboIds[2].create();
    _vboIds[3].create();
    applyChanges();
}

void QMesh::_deleteMeshInEverywhere()
{
    for (int i=0; i<_parentContext->countScenes(); i++) {
        QScene* scene = _parentContext->scene(i);
        for (int j=0; j<scene->countEntities(); j++) {
            QEntity* entity = scene->entity(j);
            for (unsigned int k=0; k<entity->countParts(); k++) {
                if (entity->part(k)->mesh() == this) {
                    if (_countUsedParts == 1) {
                        _countUsedParts = 2;
                        entity->part(k)->setMesh(nullptr);
                        _countUsedParts = 0;
                        return;
                    } else {
                        entity->part(k)->setMesh(nullptr);
                        --_countUsedParts;
                    }
                }
            }
        }
    }
}

QMesh::~QMesh()
{
    _vertices.clear();
    _elements.clear();
    if (_countUsedParts > 0) {
        qDebug() << "QScrollEngine: Warning. Deleting mesh with used entities.";
        _deleteMeshInEverywhere();
    }
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Error of deleting mesh of mesh. Parent context is null.";
        return;
    }
    _vboIds[0].destroy();
    _vboIds[1].destroy();
    _vboIds[2].destroy();
    _vboIds[3].destroy();
}

void QMesh::applyChanges()
{
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    applyChangesOfVertexPositions();
    applyChangesOfVertexTextureCoords();
    applyChangesOfVertexNormals();
    applyChangesOfElements();
}

void QMesh::applyChangesOfElements()
{
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    _vboIds[0].bind();
    _vboIds[0].allocate(_elements.data(), _elements.size() * sizeof(GLuint));
}

void QMesh::applyChangesOfVertexPositions()
{
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    _vboIds[1].bind();
    _vboIds[1].allocate(_vertices.data(), _vertices.size() * sizeof(QVector3D));
}

void QMesh::applyChangesOfVertexTextureCoords()
{
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    _vboIds[2].bind();
    _vboIds[2].allocate(_textureCoords.data(), _textureCoords.size() * sizeof(QVector2D));
}

void QMesh::applyChangesOfVertexNormals()
{
    if (_parentContext == nullptr) {
        qDebug() << "QScrollEngine: Failed to apply changes of mesh. Parent context is null.";
        return;
    }
    if (_enable_vertex_normal) {
        _vboIds[3].bind();
        _vboIds[3].allocate(_normals.data(), _normals.size() * sizeof(QVector3D));
    }
}

void QMesh::swapDataOfVertices(std::vector<QVector3D>& positions, std::vector<QVector2D>& textureCoords, std::vector<QVector3D>& normals)
{
    assert((positions.size() == textureCoords.size()) &&
           (((_enable_vertex_normal) && (normals.size() == positions.size())) || (!_enable_vertex_normal)));
    _vertices.swap(positions);
    _textureCoords.swap(textureCoords);
    if (_enable_vertex_normal)
        _normals.swap(normals);
}

void QMesh::swapDataOfTriangles(std::vector<GLuint>& triangles)
{
    _elements.swap(triangles);
}

void QMesh::setCountVertices(unsigned int count)
{
    //unsigned int prevCount = _vertices.size();
    _vertices.resize(count);
    _textureCoords.resize(count);
    if (_enable_vertex_normal) {
        _normals.resize(count);
    }
}

void QMesh::moveVertices(const QVector3D& deltaPosition)
{
    for (std::vector<QVector3D>::iterator it = _vertices.begin(); it != _vertices.end(); ++it) {
        *it += deltaPosition;
    }
}

void QMesh::scaleVertices(const QVector3D& scale)
{
    for (std::vector<QVector3D>::iterator it = _vertices.begin(); it != _vertices.end(); ++it) {
        *it *= scale;
    }
}

void QMesh::transformVertices(const QMatrix4x4& transform)
{
    for (std::vector<QVector3D>::iterator it = _vertices.begin(); it != _vertices.end(); ++it) {
        *it = QOtherMathFunctions::transform(transform, *it);
    }
}

void QMesh::moveTextureCoords(float x, float y)
{
    for (std::vector<QVector2D>::iterator it = _textureCoords.begin(); it != _textureCoords.end(); ++it) {
        it->setX(it->x() + x);
        it->setY(it->y() + y);
    }
}


void QMesh::scaleTextureCoords(float x, float y)
{
    for (std::vector<QVector2D>::iterator it = _textureCoords.begin(); it != _textureCoords.end(); ++it) {
        it->setX(it->x() * x);
        it->setY(it->y() * y);
    }
}

void QMesh::transformTextureCoords(const QMatrix3x3& transform)
{
    for (std::vector<QVector2D>::iterator it = _textureCoords.begin(); it != _textureCoords.end(); ++it) {
        *it = QOtherMathFunctions::transform(transform, *it);
    }
}

void QMesh::flipTriangles()
{
    QMESH_ASSERT(isTriangles());
    unsigned int count = countTriangles();
    for (unsigned int i=0; i<count; ++i)
        std::swap(_elements[i * 3], _elements[i * 3 + 1]);
}

bool QMesh::checkIndicesOfElements()
{
    bool isRight = true, isCurRight;
    unsigned int countVertices = _vertices.size();
    std::vector<GLuint> elements;
    elements.reserve(_elements.size());
    unsigned int countElements = _elements.size() / _sizeOfElement;
    if (_elements.size() - countElements != 0)
        _elements.resize(countElements * _sizeOfElement);
    GLuint* curElement;
    GLuint i;
    GLsizei j;
    for (i=0; i<countElements; ++i) {
        curElement = &_elements[i * _sizeOfElement];
        isCurRight = true;
        for (j=0; j<_sizeOfElement; ++j) {
            if (curElement[j] >= countVertices) {
                isCurRight = false;
                break;
            }
        }
        if (isCurRight) {
            for (j=0; j<_sizeOfElement; ++j)
                elements.push_back(curElement[j]);
        } else {
            isRight = false;
        }
    }
    elements.shrink_to_fit();
    _elements.swap(elements);
    return isRight;
}

void QMesh::flipNormals()
{
    for (std::vector<QVector3D>::iterator it = _normals.begin(); it != _normals.end(); ++it)
        *it = - *it;
}

void QMesh::updateNormals()
{
    setEnable_vertex_normal(true);
    if (isTriangles()) {
        unsigned int i;
        for (i=0; i<_normals.size(); i++)
            _normals[i] = QVector3D(0.0f, 0.0f, 0.0f);
        QVector3D dir, p0;
        unsigned int countTr = countTriangles();
        for (i=0; i<countTr; ++i) {
            GLuint* tr = triangle(i);
            p0 = _vertices[tr[1]];
            dir = QVector3D::crossProduct(_vertices[tr[0]] - p0, _vertices[tr[2]] - p0);
            dir.normalize();
            _normals[tr[0]] += dir;
            _normals[tr[1]] += dir;
            _normals[tr[2]] += dir;
        }
        for (i=0; i<_vertices.size(); i++)
            _normals[i].normalize();
    } else {
        qDebug() << "QScrollEngine: Error. Update normals - just only for triangles.";
    }
}

void QMesh::updateLocalBoundingBox()
{
    if (_vertices.size() == 0) {
        _localBoundingBox.deactivate();
        return;
    }
    std::vector<QVector3D>::iterator it = _vertices.begin();
    _localBoundingBox.toPoint(*it);
    for (++it; it != _vertices.end(); ++it)
        _localBoundingBox.addPoint(*it);
    emit changedLocalBoundingBox();
}

bool QMesh::pickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                    const QVector3D& rayPoint, const QVector3D& rayDir) const
{
    if (isTriangles()) {
        float minDis = FLT_MAX, dis;
        bool finded = false;
        unsigned int i, resultIndex;
        unsigned int countTr = countTriangles();
        const GLuint* tr;
        for (i=0; i<countTr; ++i) {
            tr = triangle(i);
            if (QOtherMathFunctions::collisionTriangleRay(resultPoint, _vertices[tr[0]],
                                                                       _vertices[tr[1]],
                                                                       _vertices[tr[2]], rayPoint, rayDir)) {
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
        resultTriangleDir = QVector3D::crossProduct(_vertices[tr[0]] - _vertices[tr[1]],
                                                    _vertices[tr[2]] - _vertices[tr[1]]);
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
        unsigned int i, resultIndex;
        unsigned int countTr = countTriangles();
        const GLuint* tr;
        for (i=0; i<countTr; ++i) {
            tr = triangle(i);
            if (QOtherMathFunctions::collisionTriangleRay(resultPoint, _vertices[tr[0]],
                                                                       _vertices[tr[1]],
                                                                       _vertices[tr[2]], rayPoint, rayDir)) {
                resultIndex = i;
                finded = true;
                break;
            }
        }
        if (!finded)
            return false;
        tr = triangle(resultIndex);
        resultTriangleDir = QVector3D::crossProduct(_vertices[tr[0]] - _vertices[tr[1]],
                                                    _vertices[tr[2]] - _vertices[tr[1]]);
        return true;
    }
    qDebug() << "QScrollEngine: Error. Fast pick ray - just only for triangles.";
    return false;
}

}
