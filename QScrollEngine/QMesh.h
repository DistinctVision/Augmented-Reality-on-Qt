#ifndef QMESH_H
#define QMESH_H

#include <vector>
#include <cassert>
#include <QObject>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QBoundingBox.h"
#include "QScrollEngine/Shaders/QSh_All.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QScene;
class QEntity;
class QPartEntity;
//class QQuickHull;

//#define QMESH_ASSERT(x) assert(x)
#define QMESH_ASSERT(x) static_cast<void>(x)

class QMesh:
        public QObject
{
    Q_OBJECT
    friend class QScrollEngineContext;
    friend class QScene;
    friend class QEntity;
    friend class QPartEntity;
    //friend class QQuickHull;

public:
    QMesh(QScrollEngineContext* parentContext);
    QMesh(QScrollEngineContext* parentContext, QMesh* mesh);
    QMesh(QScene* scene);
    QMesh(QMesh* mesh);
    ~QMesh();

    void updateLocalBoundingBox();
    QBoundingBox localBoundingBox() const { return _localBoundingBox; }
    QScrollEngineContext* parentContext() const { return _parentContext; }
    void setParentContext(QScrollEngineContext* parentContext);

    void setEnable_vertex_normal(bool enable) { _enable_vertex_normal = enable;
                                                if (_enable_vertex_normal) _normals.resize(_vertices.size()); }
    bool enable_vertex_normal() const { return _enable_vertex_normal; }

    void setUsagePatternVertices(QOpenGLBuffer::UsagePattern pattern) { _vboIds[1].setUsagePattern(pattern);
                                                                        _vboIds[2].setUsagePattern(pattern);
                                                                        _vboIds[3].setUsagePattern(pattern); }
    QOpenGLBuffer::UsagePattern usagePatternVertices() const { return _vboIds[1].usagePattern(); }
    void setUsagePatternElements(QOpenGLBuffer::UsagePattern pattern) { _vboIds[0].setUsagePattern(pattern); }
    QOpenGLBuffer::UsagePattern usagePatternElements() const { return _vboIds[0].usagePattern(); }

    unsigned int countVertices() const { return _vertices.size(); }
    void setCountVertices(unsigned int count);
    void setVertexPosition(unsigned int i, const QVector3D& vertex) { _vertices[i] = vertex; }
    const std::vector<QVector3D>& vertices() const { return _vertices; }
    const std::vector<QVector2D>& textureCoords() const { return _textureCoords; }
    const std::vector<QVector3D>& normals() const { return _normals; }
    QVector3D vertexPosition(unsigned int i) const { return _vertices[i]; }
    QVector2D vertexTextureCoord(unsigned int i) const { return _textureCoords[i]; }
    void setVertexTextureCoord(unsigned int i, const QVector2D& textureCoord) { _textureCoords[i] = textureCoord; }
    QVector3D vertexNormal(unsigned int i) const { return _normals[i]; }
    void setVertexNormal(unsigned int i, const QVector3D& normal) { _normals[i] = normal; }

    GLsizei sizeOfElement() const { return _sizeOfElement; }
    void setSizeOfELement(GLsizei size) { if (_sizeOfElement != size) { deleteElements(); _sizeOfElement = size; } }

    unsigned int countElements() const { return _elements.size() / _sizeOfElement; }
    void addElement(const GLuint* element)
    {
        for (int i=0; i<_sizeOfElement; ++i)
            _elements.push_back(element[i]);
    }
    void setCountElements(unsigned int count) { _elements.resize(count * _sizeOfElement); }
    void setElement(unsigned int i, const GLuint* element)
    {
        GLuint* e = this->element(i);
        for (int j=0; j<_sizeOfElement; ++j)
            e[j] = element[j];
    }
    const GLuint* element(unsigned int i) const { return &_elements.at(i * _sizeOfElement); }
    GLuint* element(unsigned int i) { return &_elements.at(i * _sizeOfElement); }
    void deleteElement(unsigned int i) {
        _elements.erase(_elements.begin() + i * _sizeOfElement); }
    void deleteElement(unsigned int i, unsigned int n) {
        _elements.erase(_elements.begin() + i * _sizeOfElement, _elements.begin() + (i + n) * _sizeOfElement); }
    void deleteElements() { _elements.clear(); }

    bool isTriangles() const { return (_sizeOfElement == 3); }
    bool isLines() const { return (_sizeOfElement == 2); }
    bool isPoints() const { return (_sizeOfElement == 1); }

    void setCountTriangles(unsigned int count) { QMESH_ASSERT(isTriangles()); _elements.resize(count * 3); }
    int countTriangles() const { QMESH_ASSERT(isTriangles()); return _elements.size() / 3; }
    void setTriangle(unsigned int i, GLuint v1, GLuint v2, GLuint v3)
    {
        QMESH_ASSERT(isTriangles());
        GLuint* triangle = &_elements[i * 3];
        triangle[0] = v1;
        triangle[1] = v2;
        triangle[2] = v3;
    }
    const GLuint* triangle(unsigned int i) const { QMESH_ASSERT(isTriangles()); return &_elements[i * 3]; }
    GLuint* triangle(unsigned int i) { QMESH_ASSERT(isTriangles()); return &_elements[i * 3]; }
    void addTriangle(GLuint v1, GLuint v2, GLuint v3)
    {
        QMESH_ASSERT(isTriangles());
        _elements.push_back(v1);
        _elements.push_back(v2);
        _elements.push_back(v3);
    }
    void deleteTriangle(unsigned int i) { QMESH_ASSERT(isTriangles()); _elements.erase(_elements.begin() + (i * 3)); }
    void deleteTriangles(unsigned int i, unsigned int n) { QMESH_ASSERT(isTriangles());
                                                           _elements.erase(_elements.begin() + (i * 3), _elements.begin() + ((i + n) * 3)); }

    void setCountLines(unsigned int count) { QMESH_ASSERT(isLines()); _elements.resize(count * 2); }
    int countLines() const { QMESH_ASSERT(isLines()); return _elements.size() / 2; }
    void setLine(unsigned int i, GLuint v1, GLuint v2)
    {
        QMESH_ASSERT(isLines());
        GLuint* triangle = &_elements.at(i * 2);
        triangle[0] = v1;
        triangle[1] = v2;
    }
    const GLuint* line(unsigned int i) const { QMESH_ASSERT(isLines()); return &_elements.at(i * 2); }
    GLuint* line(unsigned int i) { QMESH_ASSERT(isLines()); return &_elements.at(i * 2); }
    void addLine(GLuint v1, GLuint v2)
    {
        QMESH_ASSERT(isLines());
        _elements.push_back(v1);
        _elements.push_back(v2);
    }
    void deleteLine(unsigned int i) { QMESH_ASSERT(isLines()); _elements.erase(_elements.begin() + (i * 2)); }
    void deleteLines(unsigned int i, unsigned int n) { QMESH_ASSERT(isLines());
                                                       _elements.erase(_elements.begin() + i * 2, _elements.begin() + ((i + n) * 2)); }

    void setCountPoints(unsigned int count) { QMESH_ASSERT(isPoints()); _elements.resize(count); }
    int countPoints() const { QMESH_ASSERT(isPoints()); return _elements.size() / 2; }
    void setPoint(unsigned int i, GLuint v1)
    {
        QMESH_ASSERT(isPoints());
        _elements[i] = v1;
    }
    const GLuint* point(unsigned int i) const { QMESH_ASSERT(isPoints()); return &_elements.at(i); }
    GLuint* point(unsigned int i) { QMESH_ASSERT(isPoints()); return &_elements.at(i); }
    void addPoint(GLuint v1)
    {
        QMESH_ASSERT(isPoints());
        _elements.push_back(v1);
    }
    void deletePoint(unsigned int i) { QMESH_ASSERT(isPoints()); _elements.erase(_elements.begin() + i); }
    void deletePoints(unsigned int i, unsigned int n) { QMESH_ASSERT(isPoints());
                                                       _elements.erase(_elements.begin() + i, _elements.begin() + (i + n)); }

    bool checkIndicesOfElements();
    void deleteVertices() { _vertices.clear(); _textureCoords.clear(); _normals.clear(); }
    void clear() { deleteVertices(); deleteElements(); }

    void moveVertices(const QVector3D& deltaPosition);
    void moveVertices(float x, float y, float z) { moveVertices(QVector3D(x, y, z)); }
    void scaleVertices(const QVector3D& scale);
    void scaleVertices(float x, float y, float z) { scaleVertices(QVector3D(x, y, z)); }
    void scaleVertices(float scale) { scaleVertices(QVector3D(scale, scale, scale)); }
    void transformVertices(const QMatrix4x4& transform);
    void moveTextureCoords(float x, float y);
    void moveTextureCoords(const QVector2D& deltaPosition) { moveTextureCoords(deltaPosition.x(), deltaPosition.y()); }
    void scaleTextureCoords(float x, float y);
    void scaleTextureCoords(const QVector2D& scale) { scaleTextureCoords(scale.x(), scale.y()); }
    void scaleTextureCoords(float scale) { scaleTextureCoords(scale, scale); }
    void transformTextureCoords(const QMatrix3x3& transform);
    void flipTriangles();
    void flipNormals();
    void updateNormals();
    void swapDataOfVertices(std::vector<QVector3D>& positions, std::vector<QVector2D>& textureCoords, std::vector<QVector3D>& normals);
    void swapDataOfTriangles(std::vector<GLuint>& triangles);
    void applyChanges();
    void applyChangesOfVertexPositions();
    void applyChangesOfVertexTextureCoords();
    void applyChangesOfVertexNormals();
    void applyChangesOfElements();
    int countUsedParts() const { return _countUsedParts; }
    bool pickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                 const QVector3D& rayPoint, const QVector3D& rayDir) const;
    bool fastPickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                 const QVector3D& rayPoint, const QVector3D& rayDir) const;


signals:
    void changedLocalBoundingBox();

private:
    QScrollEngineContext* _parentContext;
    QOpenGLBuffer _vboIds[4];
    bool _enable_vertex_normal;
    std::vector<QVector3D> _vertices;
    std::vector<QVector2D> _textureCoords;
    std::vector<QVector3D> _normals;
    std::vector<GLuint> _elements;
    GLsizei _sizeOfElement;
    int _countUsedParts;
    QBoundingBox _localBoundingBox;

    void _deleteMeshInEverywhere();
};

}
#endif
