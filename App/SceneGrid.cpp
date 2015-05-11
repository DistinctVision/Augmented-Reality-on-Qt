#include "App/SceneGrid.h"
#include <QOpenGLTexture>
#include <QQuaternion>
#include <random>
#include <cfloat>
#include <cmath>
#include <QDebug>
#include "QScrollEngine/QEntity.h"

QScrollEngine::QEntity* SceneGrid::_createHexGrid(float cellSize, int countX, int countY)
{
    using namespace QScrollEngine;
    QMesh* mesh = new QMesh(parentContext());
    mesh->setSizeOfELement(2);
    mesh->setEnable_vertex_normal(true);
    std::vector<QVector3D> vertices;
    std::vector<QVector2D> textureCoords;
    std::vector<QVector3D> normals;
    float startY = 0.0f;
    float curX, curY;
    unsigned int index;
    for (int i=0; i<countY; ++i) {
        curX = ((i % 2) == 0) ? cellSize * 0.5f : 0.0f;
        curY = startY;
        index = vertices.size();
        vertices.resize(index + 3);
        mesh->addLine(index, index + 1);
        mesh->addLine(index + 1, index + 2);
        vertices.at(index) = QVector3D(curX, curY + cellSize * 0.333f, 0.0f);
        vertices.at(index + 1) = QVector3D(curX, curY + cellSize, 0.0f);
        vertices.at(index + 2) = QVector3D(curX + cellSize * 0.5f, curY + cellSize * 1.3333f, 0.0f);
        for (int j=0; j<countX; ++j) {
            index = vertices.size();
            vertices.resize(index + 4);
            mesh->addLine(index, index + 1);
            mesh->addLine(index + 1, index + 2);
            mesh->addLine(index + 2, index + 3);
            vertices.at(index) = QVector3D(curX, curY + cellSize * 0.3333f, 0.0f);
            vertices.at(index + 1) = QVector3D(curX + cellSize * 0.5f, curY, 0.0f);
            vertices.at(index + 2) = QVector3D(curX + cellSize, curY + cellSize * 0.333f, 0.0f);
            vertices.at(index + 3) = QVector3D(curX + cellSize, curY + cellSize, 0.0f);
            curX += cellSize;
        }
        index = vertices.size();
        vertices.resize(index + 2);
        mesh->addLine(index, index + 1);
        vertices.at(index) = QVector3D(curX - cellSize * 0.5f, curY + cellSize * 1.3333f, 0.0f);
        vertices.at(index + 1) = QVector3D(curX, curY + cellSize, 0.0f);
        startY += cellSize;
    }
    curX = ((countX % 2) == 0) ? cellSize * 0.5f : cellSize;
    curY = startY;
    for (int j=1; j<countX; ++j) {
        index = vertices.size();
        vertices.resize(index + 3);
        mesh->addLine(index, index + 1);
        mesh->addLine(index + 1, index + 2);
        vertices.at(index) = QVector3D(curX, curY + cellSize * 0.3334f, 0.0f);
        vertices.at(index + 1) = QVector3D(curX + cellSize * 0.5f, curY, 0.0f);
        vertices.at(index + 2) = QVector3D(curX + cellSize, curY + cellSize * 0.3334f, 0.0f);
        curX += cellSize;
    }
    textureCoords.resize(vertices.size(), QVector2D(0.0f, 0.0f));
    normals.resize(vertices.size(), QVector3D(0.0f, 0.0f, 1.0f));
    mesh->swapDataOfVertices(vertices, textureCoords, normals);
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    QEntity* entity = new QEntity(this);
    QEntity::QPartEntity* part = entity->addPart(mesh, new QSh_Light(nullptr, QColor(55, 155, 255), 1.0f, 1.0f));
    part->setDrawMode(GL_LINES);
    return entity;
}

QScrollEngine::QMesh* SceneGrid::_createArrow(float length, float lengthArrow)
{
    using namespace QScrollEngine;
    QMesh* mesh = new QMesh(parentContext());
    mesh->setSizeOfELement(2);
    mesh->setCountVertices(5);
    mesh->setVertexPosition(0, QVector3D(0.0f, 0.0f, 0.0f));
    mesh->setVertexPosition(1, QVector3D(length, 0.0f, 0.0f));
    mesh->setVertexPosition(2, QVector3D(length - lengthArrow, 0.0f, lengthArrow * 0.5f));
    mesh->setVertexPosition(3, QVector3D(length - lengthArrow, 0.0f, 0.0f));
    mesh->setVertexPosition(4, QVector3D(length - lengthArrow, - lengthArrow * 0.5f, 0.0f));
    mesh->addLine(0, 1);
    for (GLuint i=1; i<5; ++i)
        for (GLuint j=i+1; j<5; ++j)
            mesh->addLine(i, j);
    mesh->setEnable_vertex_normal(true);
    for (unsigned int i=0; i<mesh->countVertices(); ++i)
        mesh->setVertexNormal(i, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    return mesh;
}

SceneGrid::SceneGrid(QScrollEngine::QScrollEngineContext* context, int order)
{
    using namespace QScrollEngine;
    setOrder(order);
    setParentContext(context);
    setAmbientColor(70, 70, 70);
    context->glLineWidth(3.0f);
    QEntity* entity = _createHexGrid(0.5f, 8, 8);
    entity->setPosition(-2.0f, -2.0f, 0.0f);
    QEntity* arrows = new QEntity(this);
    arrows->setPosition(0.0f, 0.0f, 0.01f);
    arrows->setScale(0.8f);
    QMesh* meshArrow = _createArrow(2.0f, 0.5f);
    QEntity* arrowX = new QEntity(arrows);
    QEntity::QPartEntity* part = arrowX->addPart(meshArrow, new QSh_Color(QColor(255, 0, 0, 255)));
    part->setDrawMode(GL_LINES);
    QEntity* arrowY = new QEntity(arrows);
    part = arrowY->addPart(meshArrow, new QSh_Color(QColor(0, 255, 0, 255)));
    arrowY->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f));
    arrowY->setScale(-1.0f, 1.0f, 1.0f);
    part->setDrawMode(GL_LINES);
    QEntity* arrowZ = new QEntity(arrows);
    part = arrowZ->addPart(meshArrow, new QSh_Color(QColor(0, 0, 255, 255)));
    arrowZ->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 270.0f));
    arrowZ->setScale(1.0f, 1.0f, -1.0f);
    part->setDrawMode(GL_LINES);
    QLight* light = new QLight(this);
    light->setPosition(0.0f, 0.0f, 4.0f);
    light->setRadius(10.0f);
}

SceneGrid::~SceneGrid()
{
}
