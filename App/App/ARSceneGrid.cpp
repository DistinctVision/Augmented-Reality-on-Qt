#include "ARSceneGrid.h"
#include "AR/ARSystem.h"
#include "AR/Painter.h"

ARSceneGrid::ARSceneGrid()
{
    m_textureRenderer = nullptr;
}

QScrollEngine::QEntity* ARSceneGrid::_createHexGrid(float cellSize, int countX, int countY)
{
    using namespace QScrollEngine;
    QMesh* mesh = new QMesh(scene()->parentContext());
    mesh->setSizeOfELement(2);
    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);
    std::vector<QVector3D>& vertices = mesh->vertices();
    std::vector<QVector2D>& textureCoords = mesh->textureCoords();
    std::vector<QVector3D>& normals = mesh->normals();
    float startY = 0.0f;
    float curX, curY;
    std::size_t index;
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
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    QEntity* entity = new QEntity(scene());
    QEntity::Part* part = entity->addPart(mesh, QShPtr(new QSh_Light(nullptr, QColor(55, 155, 255), 1.0f, 1.0f)));
    part->setDrawMode(GL_LINES);
    return entity;
}

QScrollEngine::QMesh* ARSceneGrid::_createArrow(float length, float lengthArrow)
{
    using namespace QScrollEngine;
    QMesh* mesh = new QMesh(scene()->parentContext());
    mesh->setSizeOfELement(2);
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);
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
    for (std::size_t i=0; i<mesh->countVertices(); ++i)
        mesh->setVertexNormal(i, QVector3D(0.0f, 0.0f, 1.0f));
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    return mesh;
}

void ARSceneGrid::initScene()
{
    using namespace QScrollEngine;

    scene()->setAmbientColor(70, 70, 70);
    scene()->parentContext()->glLineWidth(3.0f);
    //connect(scene(), SIGNAL(beginDrawing()), this, SLOT(drawGrid()), Qt::DirectConnection);
    /*m_textureRenderer = new TextureRenderer();
    m_tempFrameTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    connect(scene(), &QScene::deleting, [=]() {
        delete m_tempFrameTexture;
        delete m_textureRenderer;
    });*/
    QEntity* entity = _createHexGrid(0.5f, 8, 8);
    entity->setPosition(-2.0f, -2.0f, 0.0f);
    QEntity* arrows = new QEntity(scene());
    arrows->setPosition(0.0f, 0.0f, 0.01f);
    arrows->setScale(0.8f);
    QMesh* meshArrow = _createArrow(2.0f, 0.5f);
    QEntity* arrowX = new QEntity(arrows);
    QEntity::Part* part = arrowX->addPart(meshArrow, QShPtr(new QSh_Color(QColor(255, 0, 0, 255))));
    part->setDrawMode(GL_LINES);
    QEntity* arrowY = new QEntity(arrows);
    part = arrowY->addPart(meshArrow, QShPtr(new QSh_Color(QColor(0, 255, 0, 255))));
    arrowY->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f));
    arrowY->setScale(-1.0f, 1.0f, 1.0f);
    part->setDrawMode(GL_LINES);
    QEntity* arrowZ = new QEntity(arrows);
    part = arrowZ->addPart(meshArrow, QShPtr(new QSh_Color(QColor(0, 0, 255, 255))));
    arrowZ->setOrientation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 270.0f));
    arrowZ->setScale(1.0f, 1.0f, 1.0f);
    part->setDrawMode(GL_LINES);
    QLight* light = new QLight(scene());
    light->setPosition(0.0f, 0.0f, 3.0f);
    light->setRadius(20.0f);
    /*entity = new QEntity(scene());
    entity->setPosition(0.0f, 0.0f, 0.0f);
    entity->addPart(scene()->parentContext(), QGLPrimitiv::Primitives::Cube,
                    QShPtr(new QSh_Light(nullptr, QColor(255, 255, 255))));*/
}

void ARSceneGrid::_endUpdate()
{
    scene()->setEnabled((arTracker()->trackingState() == ARTracker::Tracking));
}

/*void ARSceneGrid::drawGrid()
{
    using namespace AR;
    using namespace TMath;
    using namespace QScrollEngine;

    if (scene() == nullptr)
        return;

    QScrollEngineContext* context = scene()->parentContext();
    return;
    if (context == nullptr)
        return;

    context->glDepthMask(GL_FALSE);
    context->glDisable(GL_DEPTH_TEST);
    context->glDisable(GL_BLEND);

    Image<uchar> image = arTracker()->arSystem().lastImage().copy();

    for (auto it = MapProjector::debug.begin(); it != MapProjector::debug.end(); ++it) {
        Painter::drawImage(image, it->patch.convert<uchar>([](const Point2i, const float& v) { return v; }),
                           it->p - (it->patch.size() - Point2i(1, 1)).cast<float>() * (it->scale * 0.5f),
                           it->patch.size().cast<float>() * it->scale);
    }

    if (m_tempFrameTexture->isCreated()) {
        if ((m_tempFrameTexture->width() != image.width()) ||
                (m_tempFrameTexture->height() != image.height())) {
            m_tempFrameTexture->bind();
            m_tempFrameTexture->destroy();
            m_tempFrameTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
            m_tempFrameTexture->setSize(image.width(), image.height(), 4);
            m_tempFrameTexture->allocateStorage();
        } else {
            m_tempFrameTexture->bind();
        }
    } else {
        m_tempFrameTexture->create();
        m_tempFrameTexture->bind();
        m_tempFrameTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        m_tempFrameTexture->setSize(image.width(), image.height(), 4);
        m_tempFrameTexture->allocateStorage();
    }
    m_tempFrameTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                                image.convert<Rgba>([](const Point2i& , const uchar& v) { return Rgba(v, v, v); }).data());
    m_tempFrameTexture->generateMipMaps();

    m_textureRenderer->draw(context, m_tempFrameTexture->textureId(), arTracker()->textureMatrix());

//    QMatrix4x4 textureMatrix;
//    textureMatrix.fill(0.0f);
//    for (int i=0; i<2; ++i) {
//        textureMatrix(i, 0) = m_textureMatrix(i, 0);
//        textureMatrix(i, 1) = m_textureMatrix(i, 1);
//        textureMatrix(i, 3) = m_textureMatrix(i, 2);
//    }
//    textureMatrix(3, 0) = m_textureMatrix(2, 0);
//    textureMatrix(3, 1) = m_textureMatrix(2, 1);
//    textureMatrix(3, 3) = m_textureMatrix(2, 2);

    QMatrix4x4 t;
    t.setToIdentity();
    t(0, 0) = 0.5f;
    t(1, 1) = -0.5f;
    t(0, 3) = 0.5f;
    t(1, 3) = 0.5f;

    Point2i imageSize = arTracker()->imageSize();

    QMatrix4x4 transformMatrix;
    transformMatrix.ortho(0.0f, (float)(imageSize.x),
                          0.0f, (float)(imageSize.y), 0.0f, 1.0f);
    //transformMatrix = t.inverted() * textureMatrix * t * transformMatrix;

    context->glLineWidth(4.0f);

    Camera camera;
    camera.setCameraParameters(arTracker()->arSystem().cameraParameters());
    camera.setImageSize(imageSize);
    TMatrixd rotation = arTracker()->arSystem().currentRotation();
    TVectord translation = arTracker()->arSystem().currentTranslation();

    QVector2D verts[4];

    TVectord v = rotation * TVectord::create(-2.0, -2.0, 0.0) + translation;
    Point2d p = camera.project(Point2d(v(0) / v(2), v(1) / v(2)));
    verts[0] = QVector2D((float)p.x, (float)p.y);

    v = rotation * TVectord::create(2.0, -2.0, 0.0) + translation;
    p = camera.project(Point2d(v(0) / v(2), v(1) / v(2)));
    verts[1] = QVector2D((float)p.x, (float)p.y);

    v = rotation * TVectord::create(2.0, 2.0, 0.0) + translation;
    p = camera.project(Point2d(v(0) / v(2), v(1) / v(2)));
    verts[2] = QVector2D((float)p.x, (float)p.y);

    v = rotation * TVectord::create(-2.0, 2.0, 0.0) + translation;
    p = camera.project(Point2d(v(0) / v(2), v(1) / v(2)));
    verts[3] = QVector2D((float)p.x, (float)p.y);

    std::vector<QVector2D> vv;
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            vv.push_back(verts[i]);
            vv.push_back(verts[j]);
        }
    }

    scene()->parentContext()->drawLines(vv.data(), vv.size(), QColor(0, 0, 255), transformMatrix);
}*/
