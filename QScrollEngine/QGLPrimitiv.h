#ifndef QGLPRIMITIV_H
#define QGLPRIMITIV_H

namespace QScrollEngine {

class QMesh;

class QGLPrimitiv
{
public:
    enum Primitives
    {
        None,
        Quad,
        Cube,
        Sphere
    };

    static void createQuad(QMesh* mesh);

    static void createCube(QMesh* mesh);

    static void createSphere(QMesh* mesh, float radius = 0.5f, int rings = 16, int sectors = 16);
};

}

#endif
