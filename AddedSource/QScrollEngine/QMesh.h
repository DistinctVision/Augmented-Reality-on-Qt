#ifndef QMESH_H
#define QMESH_H

#include <vector>
#include <cassert>
#include <functional>
#include <QObject>
#include <QVector>
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

public:
    typedef struct RgbColor
    {
        float red, green, blue;

        RgbColor() { red = green = blue = 255; }
        RgbColor(float red, float green, float blue)
        {
            this->red = red;
            this->green = green;
            this->blue = blue;
        }
    } RgbColor;

    QMesh(QScrollEngineContext* parentContext);
    QMesh(QScrollEngineContext* parentContext, QMesh* mesh);
    QMesh(QScene* scene);
    QMesh(QMesh* mesh);
    ~QMesh();

    void updateLocalBoundingBox();
    void setLocalBoundingBox(const QBoundingBox& boundingBox);
    QBoundingBox localBoundingBox() const { return m_localBoundingBox; }
    QScrollEngineContext* parentContext() const { return m_parentContext; }
    void setParentContext(QScrollEngineContext* parentContext);

    std::size_t countVertices() const { return m_vertices.size(); }
    virtual void setCountVertices(std::size_t count);
    void setVertexPosition(std::size_t i, const QVector3D& vertex) { m_vertices[i] = vertex; }
    const std::vector<GLuint>& elements() const { return m_elements; }
    const std::vector<QVector3D>& vertices() const { return m_vertices; }
    const std::vector<QVector2D>& textureCoords() const { return m_textureCoords; }
    const std::vector<QVector3D>& normals() const { return m_normals; }
    const std::vector<RgbColor>& colors() const { return m_rgbColors; }
    std::vector<GLuint>& elements() { return m_elements; }
    std::vector<QVector3D>& vertices() { return m_vertices; }
    std::vector<QVector2D>& textureCoords() { return m_textureCoords; }
    std::vector<QVector3D>& normals() { return m_normals; }
    std::vector<RgbColor>& colors() { return m_rgbColors; }
    void deleteVertexPositions() { m_vertices.clear(); }
    void deleteTextureCoords() { m_textureCoords.clear(); }
    void deleteVertexNormals() { m_normals.clear(); }
    void deleteRgbColors() { m_rgbColors.clear(); }
    virtual void deleteVertices()
    {
        deleteVertexPositions();
        deleteTextureCoords();
        deleteVertexNormals();
        deleteRgbColors();
    }
    void clear();

    QVector3D vertexPosition(std::size_t i) const { return m_vertices[i]; }
    QVector2D vertexTextureCoord(std::size_t i) const { return m_textureCoords[i]; }
    void setVertexTextureCoord(std::size_t i, const QVector2D& textureCoord) { m_textureCoords[i] = textureCoord; }
    QVector3D vertexNormal(std::size_t i) const { return m_normals[i]; }
    void setVertexNormal(std::size_t i, const QVector3D& normal) { m_normals[i] = normal; }
    RgbColor vertexRgbColor(std::size_t i) const { return m_rgbColors[i]; }
    void setVertexRgbColor(std::size_t i, const RgbColor& color) { m_rgbColors[i] = color; }

    GLsizei sizeOfElement() const { return m_sizeOfElement; }
    void setSizeOfELement(GLsizei size) { if (m_sizeOfElement != size) { deleteElements(); m_sizeOfElement = size; } }

    std::size_t countElements() const { return m_elements.size() / m_sizeOfElement; }
    void addElement(const GLuint* element)
    {
        for (int i=0; i<m_sizeOfElement; ++i)
            m_elements.push_back(element[i]);
    }
    void setCountElements(std::size_t count) { m_elements.resize(count * m_sizeOfElement); }
    void setElement(std::size_t i, const GLuint* element)
    {
        GLuint* e = this->element(i);
        for (int j=0; j<m_sizeOfElement; ++j)
            e[j] = element[j];
    }
    const GLuint* element(std::size_t i) const { return &m_elements.at(i * m_sizeOfElement); }
    GLuint* element(std::size_t i) { return &m_elements.at(i * m_sizeOfElement); }
    void deleteElement(std::size_t i) {
        m_elements.erase(m_elements.begin() + i * m_sizeOfElement); }
    void deleteElement(std::size_t i, std::size_t n) {
        m_elements.erase(m_elements.begin() + i * m_sizeOfElement, m_elements.begin() + (i + n) * m_sizeOfElement); }
    void deleteElements() { m_elements.clear(); }

    bool isTriangles() const { return (m_sizeOfElement == 3); }
    bool isLines() const { return (m_sizeOfElement == 2); }
    bool isPoints() const { return (m_sizeOfElement == 1); }

    void setCountTriangles(std::size_t count) { QMESH_ASSERT(isTriangles()); m_elements.resize(count * 3); }
    int countTriangles() const { QMESH_ASSERT(isTriangles()); return m_elements.size() / 3; }
    void setTriangle(std::size_t i, GLuint v1, GLuint v2, GLuint v3)
    {
        QMESH_ASSERT(isTriangles());
        GLuint* triangle = &m_elements[i * 3];
        triangle[0] = v1;
        triangle[1] = v2;
        triangle[2] = v3;
    }
    const GLuint* triangle(std::size_t i) const { QMESH_ASSERT(isTriangles()); return &m_elements[i * 3]; }
    GLuint* triangle(std::size_t i) { QMESH_ASSERT(isTriangles()); return &m_elements[i * 3]; }
    void addTriangle(GLuint v1, GLuint v2, GLuint v3)
    {
        QMESH_ASSERT(isTriangles());
        m_elements.push_back(v1);
        m_elements.push_back(v2);
        m_elements.push_back(v3);
    }
    void deleteTriangle(std::size_t i) { QMESH_ASSERT(isTriangles()); m_elements.erase(m_elements.begin() + (i * 3)); }
    void deleteTriangles(std::size_t i, unsigned int n) { QMESH_ASSERT(isTriangles());
                                                           m_elements.erase(m_elements.begin() + (i * 3),
                                                                           m_elements.begin() + ((i + n) * 3)); }

    void setCountLines(std::size_t count) { QMESH_ASSERT(isLines()); m_elements.resize(count * 2); }
    int countLines() const { QMESH_ASSERT(isLines()); return m_elements.size() / 2; }
    void setLine(std::size_t i, GLuint v1, GLuint v2)
    {
        QMESH_ASSERT(isLines());
        GLuint* triangle = &m_elements.at(i * 2);
        triangle[0] = v1;
        triangle[1] = v2;
    }
    const GLuint* line(std::size_t i) const { QMESH_ASSERT(isLines()); return &m_elements.at(i * 2); }
    GLuint* line(std::size_t i) { QMESH_ASSERT(isLines()); return &m_elements.at(i * 2); }
    void addLine(GLuint v1, GLuint v2)
    {
        QMESH_ASSERT(isLines());
        m_elements.push_back(v1);
        m_elements.push_back(v2);
    }
    void deleteLine(std::size_t i) { QMESH_ASSERT(isLines()); m_elements.erase(m_elements.begin() + (i * 2)); }
    void deleteLines(std::size_t i, std::size_t n) { QMESH_ASSERT(isLines());
                                                     m_elements.erase(m_elements.begin() + i * 2, m_elements.begin() + ((i + n) * 2)); }

    void setCountPoints(std::size_t count) { QMESH_ASSERT(isPoints()); m_elements.resize(count); }
    int countPoints() const { QMESH_ASSERT(isPoints()); return m_elements.size() / 2; }
    void setPoint(std::size_t i, GLuint v1)
    {
        QMESH_ASSERT(isPoints());
        m_elements[i] = v1;
    }
    const GLuint* point(std::size_t i) const { QMESH_ASSERT(isPoints()); return &m_elements.at(i); }
    GLuint* point(std::size_t i) { QMESH_ASSERT(isPoints()); return &m_elements.at(i); }
    void addPoint(GLuint v1)
    {
        QMESH_ASSERT(isPoints());
        m_elements.push_back(v1);
    }
    void deletePoint(std::size_t i) { QMESH_ASSERT(isPoints()); m_elements.erase(m_elements.begin() + i); }
    void deletePoints(std::size_t i, std::size_t n) { QMESH_ASSERT(isPoints());
                                                      m_elements.erase(m_elements.begin() + i, m_elements.begin() + (i + n)); }

    bool checkIndicesOfElements();
    bool checkElements() const;
    bool checkVertices() const;

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
    void transformNormals(const QMatrix3x3& transform);
    void updateNormals();
    void setUsagePatternOfElements(QOpenGLBuffer::UsagePattern usagePattern);
    QOpenGLBuffer::UsagePattern usagePatternOfElements() const;
    void setUsagePatternOfVertexPositions(QOpenGLBuffer::UsagePattern usagePattern);
    QOpenGLBuffer::UsagePattern usagePatternOfVertexPositions() const;
    void setUsagePatternOfVertexAttributes(QSh::VertexAttributes attribute, QOpenGLBuffer::UsagePattern usagePattern);
    QOpenGLBuffer::UsagePattern usagePatternOfVertexAttributes(QSh::VertexAttributes attribute) const;
    void enableVertexAttribute(QSh::VertexAttributes attribute);
    void disableAttribute(QSh::VertexAttributes attribute);
    bool vertexAttributeIsEnabled(QSh::VertexAttributes attribute);
    void disableAllVertexAttributes();
    void applyChanges();
    void applyChangesOfElements();
    void applyChangesOfVertexPositions();
    void applyChangesOfAttrtibute(QSh::VertexAttributes attribute);
    bool bind(const std::vector<QSh::VertexAttributes>& attributes) const;

    int countUsedParts() const { return m_countUsedParts; }
    bool pickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                 const QVector3D& rayPoint, const QVector3D& rayDir) const;
    bool fastPickRay(QVector3D& resultPoint, QVector3D& resultTriangleDir,
                 const QVector3D& rayPoint, const QVector3D& rayDir) const;

signals:
    void changedLocalBoundingBox();

protected:
    friend class QScrollEngineContext;
    friend class QScene;
    friend class QEntity;
    friend class QPartEntity;
    //friend class QQuickHull;

    typedef struct Buffer {
        QOpenGLBuffer vboIds;
        std::function<void()> applyChanges;
        std::function<void()> clear;
        std::function<void()> bind;
    } Buffer;

    QScrollEngineContext* m_parentContext;
    QVector<Buffer> m_buffers;
    std::vector<GLuint> m_elements;
    std::vector<QVector3D> m_vertices;
    std::vector<QVector2D> m_textureCoords;
    std::vector<QVector3D> m_normals;
    std::vector<RgbColor> m_rgbColors;
    GLsizei m_sizeOfElement;
    int m_countUsedParts;
    QBoundingBox m_localBoundingBox;

    virtual bool _initBuffers();
    void _deleteMeshInEverywhere();
    void _deleteBuffers();
};

}

#endif
