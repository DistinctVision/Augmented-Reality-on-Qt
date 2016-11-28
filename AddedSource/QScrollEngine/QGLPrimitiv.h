#ifndef QGLPRIMITIV_H
#define QGLPRIMITIV_H

namespace QScrollEngine {

class QMesh;

class QGLPrimitiv
{
public:
    enum class Primitives
    {
        None,
        Quad,
        Cube,
        Sphere,
        Cylinder
    };

    static void createQuad(QMesh* mesh);

    static void createCube(QMesh* mesh);

    static void createSphere(QMesh* mesh, float radius = 0.5f, int rings = 16, int sectors = 16);

    static void createCylinder(QMesh* mesh, float radius = 0.5f, float height = 1.0f,
                               int lengthsegs = 16, int heightsegs = 1);
};

}

#endif
