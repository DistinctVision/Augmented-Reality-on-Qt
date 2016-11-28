#include "QScrollEngine/QGLPrimitiv.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/Shaders/QSh.h"

#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <qmath.h>

namespace QScrollEngine {

void QGLPrimitiv::createQuad(QMesh* mesh)
{
    mesh->setSizeOfELement(3);
    mesh->clear();

    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);

    mesh->setCountVertices(4);
    mesh->setVertexPosition(0, QVector3D(-0.5f, -0.5f, 0.0f));
    mesh->setVertexTextureCoord(0, QVector2D(0.0f, 0.0f));
    mesh->setVertexNormal(0, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->setVertexPosition(1, QVector3D(0.5f, -0.5f, 0.0f));
    mesh->setVertexTextureCoord(1, QVector2D(1.0f, 0.0f));
    mesh->setVertexNormal(1, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->setVertexPosition(2, QVector3D(0.5f, 0.5f, 0.0f));
    mesh->setVertexTextureCoord(2, QVector2D(1.0f, 1.0f));
    mesh->setVertexNormal(2, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->setVertexPosition(3, QVector3D(-0.5f, 0.5f, 0.0f));
    mesh->setVertexTextureCoord(3, QVector2D(0.0f, 1.0f));
    mesh->setVertexNormal(3, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->addTriangle(0, 1, 2);
    mesh->addTriangle(0, 2, 3);

    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
}

void QGLPrimitiv::createCube(QMesh* mesh)
{
    mesh->setSizeOfELement(3);
    mesh->clear();

    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);

    mesh->setCountVertices(24);
    QVector3D nx(1.0f, 0.0f, 0.0f), ny(0.0f, 1.0f, 0.0f), nz(0.0f, 0.0f, 1.0f);
    for (int i=0; i<6; i++) {
        if (i == 0) {

        } else if (i == 1) {
            nx = nz;
            nz = QVector3D::crossProduct(nx, ny);
        } else if (i == 2) {
            nx = nz;
            nz = QVector3D::crossProduct(nx, ny);
        } else if (i == 3) {
            nx = nz;
            nz = QVector3D::crossProduct(nx, ny);
        } else if (i == 4) {
            ny = nx;
            nx = nz;
            nz = QVector3D::crossProduct(nx, ny);
        } else if (i == 5) {
            nz = - nz;
            nx = - nx;
        }
        int indexShift = i * 4;
        mesh->setVertexPosition(indexShift, nx * (-0.5f) + ny * (-0.5f) + nz * 0.5f);
        mesh->setVertexTextureCoord(indexShift, QVector2D(0.0f, 0.0f));
        mesh->setVertexNormal(indexShift, -nz);
        mesh->setVertexPosition(indexShift+1, nx * 0.5f + ny * (-0.5f) + nz * 0.5f);
        mesh->setVertexTextureCoord(indexShift+1, QVector2D(1.0f, 0.0f));
        mesh->setVertexNormal(indexShift+1, -nz);
        mesh->setVertexPosition(indexShift+2, nx * 0.5f + ny * 0.5f + nz * 0.5f);
        mesh->setVertexTextureCoord(indexShift+2, QVector2D(1.0f, 1.0f));
        mesh->setVertexNormal(indexShift+2, -nz);
        mesh->setVertexPosition(indexShift+3, nx * (-0.5f) + ny * 0.5f + nz * 0.5f);
        mesh->setVertexTextureCoord(indexShift+3, QVector2D(0.0f, 1.0f));
        mesh->setVertexNormal(indexShift+3, -nz);
        mesh->addTriangle(indexShift, indexShift+1, indexShift+2);
        mesh->addTriangle(indexShift, indexShift+2, indexShift+3);
    }

    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
}

void QGLPrimitiv::createSphere(QMesh* mesh, float radius, int rings, int sectors)
{
    mesh->setSizeOfELement(3);
    mesh->clear();

    float const R = 1.0f / static_cast<float>(rings-1);
    float const S = 1.0f / static_cast<float>(sectors-1);
    int r, s;

    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);

    mesh->setCountVertices(rings * sectors);
    QVector3D dir;
    int i = 0;
    for(r = 0; r < rings; r++) {
        for(s = 0; s < sectors; s++) {
            dir.setY(qSin(-M_PI_2 + M_PI * r * R));
            dir.setX(qCos(2.0f * M_PI * s * S) * qSin(M_PI * r * R));
            dir.setZ(qSin(2.0f * M_PI * s * S) * qSin(M_PI * r * R));

            mesh->setVertexPosition(i, dir * radius);
            mesh->setVertexTextureCoord(i, QVector2D(s*S, r*R));
            mesh->setVertexNormal(i, - dir);

            ++i;
        }
    }

    i = 0;
    GLuint iv1, iv2, iv3;
    mesh->setCountTriangles(rings * sectors * 2);
    for(r = 0; r < rings-1; r++) {
        for(s = 0; s < sectors-1; s++) {
            iv3 = r * sectors + s;
            iv2 = r * sectors + s + 1;
            iv1 = (r + 1) * sectors + s + 1;
            mesh->setTriangle(i, iv1, iv2, iv3);
            ++i;
            iv2 = iv1;
            iv1 = (r + 1) * sectors + s;
            mesh->setTriangle(i, iv1, iv2, iv3);
            ++i;
        }
    }

    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
}

void QGLPrimitiv::createCylinder(QMesh* mesh, float radius, float height, int lengthsegs, int heightsegs)
{
    using namespace std;

    // but if there is data, this will append it correctly
    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);
    mesh->setCountVertices((lengthsegs + 1) * (heightsegs + 1));
    float heightstride = height / (float)(heightsegs);
    float h = - height * 0.5f;
    float a = 0.0f;
    for(int i = 0; i <= heightsegs; ++i) {
        for (int j = 0; j <= lengthsegs; ++j){
            mesh->setVertexPosition(i * (lengthsegs + 1) + j, QVector3D(cos(a) * radius,
                                                                        sin(a) * radius,
                                                                        h));
            mesh->setVertexTextureCoord(i * (lengthsegs + 1) + j, QVector2D((float)(j) / (float)(lengthsegs),
                                                                            (float)(i) / (float)(heightsegs)));
            a += (float)(M_PI * 2.0f) / (float)(lengthsegs);
        }
        a = 0.0f;
        h += heightstride;
    }
    std::vector<GLuint>& indices = mesh->elements();
    indices.resize(6 * lengthsegs * heightsegs);
    std::size_t off = 0;
    for (int i = 0;  i < heightsegs; ++i){
        for (int j = 0; j < lengthsegs; ++j){
            indices[off    ] = i * (lengthsegs + 1) + j;
            indices[off + 1] = i * (lengthsegs + 1) + j + 1;
            indices[off + 2] = (i + 1) * (lengthsegs + 1) + j;
            indices[off + 3] = i * (lengthsegs + 1) + j + 1;
            indices[off + 4] = (i + 1) * (lengthsegs + 1) + j + 1;
            indices[off + 5] = (i + 1) * (lengthsegs + 1) + j;
            off += 6;
        }
    }
    mesh->updateNormals();
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
}

}
