#ifndef QISOSURFACE_H
#define QISOSURFACE_H

#include <QObject>
#include <QVector3D>
#include "QScrollEngine/QMesh.h"
#include <qgl.h>
#include <functional>
#include <algorithm>

namespace QScrollEngine {

class QIsoSurface:
        public QObject
{
    Q_OBJECT
public:
    class ScalarField
    {
    public:
        virtual float value(const QVector3D& point) = 0;
    };

signals:
    void updateProgress(float progress);

public:
    QIsoSurface();
    ~QIsoSurface();

    void isoApproximate(QMesh* mesh, ScalarField* scalarField, bool normals = true);
    void isoApproximate(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles,
                        ScalarField* scalarField);
    void isoApproximate(std::vector<QVector3D>& vertices, std::vector<QVector3D>& normals, std::vector<GLuint>& triangles,
                        ScalarField* scalarField);

    void isoApproximate(QMesh* mesh, const std::function<float(const QVector3D& point)>& scalarField, bool normals = true);
    void isoApproximate(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles,
                        const std::function<float(const QVector3D& point)>& scalarField);
    void isoApproximate(std::vector<QVector3D>& vertices, std::vector<QVector3D>& normals, std::vector<GLuint>& triangles,
                        const std::function<float(const QVector3D& point)>& scalarField);

    float cellSize() const { return m_cellSize; }
    void setCellSize(float cellSize) { m_cellSize = cellSize; }
    QVector3D start() const { return m_start; }
    QVector3D end() const { return m_end; }
    void setRegion(QVector3D start, QVector3D end) { m_start = start; m_end = end; }

    float tValue() const { return m_tValue; }
    void setTValue(float tValue) { m_tValue = tValue; }

    float epsilon() const { return m_epsilon; }
    void setEpsilon(float epsilon) { m_epsilon = epsilon; }

private:
    typedef struct GridCell
    {
        QVector3D vertices[8];
        float values[8];
    } GridCell;

    static int m_edgeTable[256];
    static signed char m_triangleTable[256][16];

    unsigned int m_countX, m_countY, m_countZ;
    QVector3D m_start, m_end, m_diff;
    float m_cellSize;

    float m_tValue;

    float m_epsilon;

    void _polygonizeGrids(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles,
                          float* topVals, float* bottomVals, int topZ);
    void _fillGrid(ScalarField* scalarField, float* grid, int z);
    QVector3D _calcGradient(ScalarField* scalarField, const QVector3D& pos);
    void _fillGrid(const std::function<float(const QVector3D& point)>& scalarField, float* grid, int z);
    QVector3D _calcGradient(const std::function<float(const QVector3D& point)>& scalarField, const QVector3D& pos);
    void _marchingCube(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles, const GridCell& cell);

    QVector3D _vertexInterpolate(const QVector3D& vertexA, const QVector3D& vertexB, float valueA, float valueB, float value) const
    {
        return (vertexA + ((value - valueA) / (valueB - valueA)) * (vertexB - vertexA));
    }
};

}

#endif // QISOSURFACE_H
