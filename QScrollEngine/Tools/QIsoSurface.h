#ifndef QISOSURFACE_H
#define QISOSURFACE_H

#include <QObject>
#include <QVector3D>
#include "QScrollEngine/QMesh.h"
#include <qgl.h>

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

    void isoApproximate(QMesh* mesh);
    void isoApproximate(std::vector<QVector3D>& vertices, std::vector<QVector3D>& normals, std::vector<GLuint>& triangles);

    float cellSize() const { return _cellSize; }
    void setCellSize(float cellSize) { _cellSize = cellSize; }
    QVector3D start() const { return _start; }
    QVector3D end() const { return _end; }
    void setRegion(QVector3D start, QVector3D end) { _start = start; _end = end; }

    float tValue() const { return _tValue; }
    void setTValue(float tValue) { _tValue = tValue; }

    float epsilon() const { return _epsilon; }
    void setEpsilon(float epsilon) { _epsilon = epsilon; }

    ScalarField* scalarField() const { return _scalarField; }
    void setScalarField(ScalarField* scalarField) { _scalarField = scalarField; }

private:
    typedef struct GridCell
    {
        QVector3D vertices[8];
        float values[8];
    } GridCell;

    static int _edgeTable[256];
    static signed char _triangleTable[256][16];

    ScalarField* _scalarField;

    unsigned int _countX, _countY, _countZ;
    QVector3D _start, _end, _diff;
    float _cellSize;

    float _tValue;

    float _epsilon;

    void _polygonizeGrids(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles,
                          float* topVals, float* bottomVals, int topZ);
    void _fillGrid(float* grid, int z);

    QVector3D _calcGradient(const QVector3D& pos);
    void _marchingCube(std::vector<QVector3D>& vertices, std::vector<GLuint>& triangles, const GridCell& cell);
    QVector3D _vertexInterpolate(const QVector3D& vertexA, const QVector3D& vertexB, float valueA, float valueB, float value) const
    {
        return (vertexA + ((value - valueA) / (valueB - valueA)) * (vertexB - vertexA));
    }
};

}

#endif // QISOSURFACE_H
