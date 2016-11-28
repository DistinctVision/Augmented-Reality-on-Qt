#include "ARSceneShip.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include <QOpenGLShader>
#include <QTimer>
#include <cmath>
#include <QtMath>
#include <cassert>
#include "TMath/TMath.h"

int Sh_WarpRiple::locationMatrixWVP[2];
int Sh_WarpRiple::locationFrameTexture[2];
int Sh_WarpRiple::locationBorderSize[2];
int Sh_WarpRiple::locationRadius[2];
int Sh_WarpRiple::locationSourceRipple[2];
int Sh_WarpRiple::locationHeightRipple[2];
int Sh_WarpRiple::locationOffset[2];
int Sh_WarpRiple::locationTextureMatrix[2];
int Sh_WarpRiple::locationShadowPower[2];

Sh_WarpRiple::Sh_WarpRiple():QSh()
{
    m_currentTypeIndex = 1411;
    m_egl = false;
    m_frameTextureId = std::numeric_limits<GLuint>::max();
    m_radius = 1.0f;
    m_borderSize = QVector2D(1.0f, 1.0f);
    m_heightRipple = 1.0f;
    m_offset = 0.0f;
    m_shadowPower = 0.4f;
}

QVector3D Sh_WarpRiple::transform(const QVector3D& point) const
{
    float b = qMax(qMin(qMin((m_borderSize.x() - qAbs(point.x())) / m_borderSize.x(),
                        (m_borderSize.y() - qAbs(point.y())) / m_borderSize.y()), 1.0f), 0.0f);
    float h = qCos((m_sourceRipple - point.toVector2D()).length() / m_radius + m_offset) *  m_heightRipple;
    return QVector3D(point.x(), point.y(), point.z() + h * qSqrt(b));
}

void Sh_WarpRiple::preprocess(const QScrollEngine::QDrawObject3D* )
{
#if defined(GL_TEXTURE_EXTERNAL_OES)
    m_subTypeIndex = m_egl ? 1 : 0;
#endif
}

bool Sh_WarpRiple::use(QScrollEngine::QScrollEngineContext* context, QOpenGLShaderProgram* program,
                       const QScrollEngine::QDrawObject3D* drawObject)
{
    if (m_frameTextureId == std::numeric_limits<GLuint>::max())
        return false;
    const QScrollEngine::QSceneObject3D* sceneObject = drawObject->sceneObject();
    QSH_ASSERT(sceneObject != nullptr);

#if defined(GL_TEXTURE_EXTERNAL_OES)
    if (m_egl) {
        program->setUniformValue(locationMatrixWVP[1], sceneObject->matrixWorldViewProj());
        context->glActiveTexture(GL_TEXTURE0);
        context->glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_frameTextureId);
        program->setUniformValue(locationFrameTexture[1], 0);
        program->setUniformValue(locationBorderSize[1], m_borderSize);
        program->setUniformValue(locationRadius[1], m_radius);
        program->setUniformValue(locationSourceRipple[1], m_sourceRipple);
        program->setUniformValue(locationHeightRipple[1], m_heightRipple);
        program->setUniformValue(locationOffset[1], m_offset);
        program->setUniformValue(locationTextureMatrix[1], m_textureMatrix);
        program->setUniformValue(locationShadowPower[1], m_shadowPower);
    } else {
#endif
        program->setUniformValue(locationMatrixWVP[0], sceneObject->matrixWorldViewProj());
        context->glActiveTexture(GL_TEXTURE0);
        context->glBindTexture(GL_TEXTURE_2D, m_frameTextureId);
        program->setUniformValue(locationFrameTexture[0], 0);
        program->setUniformValue(locationBorderSize[0], m_borderSize);
        program->setUniformValue(locationRadius[0], m_radius);
        program->setUniformValue(locationSourceRipple[0], m_sourceRipple);
        program->setUniformValue(locationHeightRipple[0], m_heightRipple);
        program->setUniformValue(locationOffset[0], m_offset);
        program->setUniformValue(locationTextureMatrix[0], m_textureMatrix);
        program->setUniformValue(locationShadowPower[0], m_shadowPower);
#if defined(GL_TEXTURE_EXTERNAL_OES)
    }
#endif
    return true;
}

void Sh_WarpRiple::load(QScrollEngine::QScrollEngineContext* context,
                        std::vector<QSharedPointer<QOpenGLShaderProgram>>& programs)
{
    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    vertexShader.compileSourceCode("#ifdef GL_ES\n" \
                                   "precision highp int;\n" \
                                   "precision highp float;\n" \
                                   "#endif\n" \
                                   "attribute vec4 vertex_position;\n" \
                                   "uniform mat4 matrix_wvp;\n" \
                                   "uniform mat3 textureMatrix;\n" \
                                   "uniform vec2 borderSize;\n" \
                                   "uniform vec2 sourceRipple;\n" \
                                   "uniform float heightRipple;\n" \
                                   "uniform float radius;\n" \
                                   "uniform float offset;\n" \
                                   "uniform float shadowPower;\n" \
                                   "varying highp vec2 textureCoords;\n" \
                                   "varying highp float kWrap;\n" \
                                   "void main() {\n" \
                                   "    vec4 p = matrix_wvp * vertex_position;\n" \
                                   "    textureCoords = (p.w > 0.0) ? ((p.xy / p.w) + vec2(1, 1)) * 0.5 : vec2(0, 0);\n" \
                                   "    textureCoords = (textureMatrix * vec3(textureCoords, 1)).xy;\n" \
                                   "    float h = cos(length(sourceRipple - vertex_position.xy) / radius + offset);\n" \
                                   "    h *= sqrt(clamp(min((borderSize.x - abs(vertex_position.x)) / borderSize.x, "
                                   "(borderSize.y - abs(vertex_position.y)) / borderSize.y), 0.0, 1.0));\n" \
                                   "    kWrap = 1.0 - max((h * shadowPower), 0.0);\n" \
                                   "    gl_Position = matrix_wvp * vec4(vertex_position.xy, "
                                   "vertex_position.z + h * heightRipple, 1);\n" \
                                   "}");
    QOpenGLShaderProgram* programGL = new QOpenGLShaderProgram();
    programGL->addShader(&vertexShader);
    programGL->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "#ifdef GL_ES\n" \
                                       "precision mediump int;\n" \
                                       "precision mediump float;\n" \
                                       "#endif\n" \
                                       "varying highp vec2 textureCoords;\n" \
                                       "uniform sampler2D frameTexture;\n" \
                                       "varying highp float kWrap;\n" \
                                       "void main() {\n" \
                                       "    gl_FragColor = vec4(texture2D(frameTexture, "
                                       "textureCoords).rgb * kWrap, 1);\n" \
                                       "}");
    programGL->bindAttributeLocation("vertex_position", 0);
    if (!programGL->link())
        context->error("Sh_WarpRipple: " + programGL->log());
    locationMatrixWVP[0] = programGL->uniformLocation("matrix_wvp");
    locationFrameTexture[0] = programGL->uniformLocation("frameTexture");
    locationBorderSize[0] = programGL->uniformLocation("borderSize");
    locationRadius[0] = programGL->uniformLocation("radius");
    locationSourceRipple[0] = programGL->uniformLocation("sourceRipple");
    locationHeightRipple[0] = programGL->uniformLocation("heightRipple");
    locationOffset[0] = programGL->uniformLocation("offset");
    locationTextureMatrix[0] = programGL->uniformLocation("textureMatrix");
    locationShadowPower[0] = programGL->uniformLocation("shadowPower");

#if defined(GL_TEXTURE_EXTERNAL_OES)
    QOpenGLShaderProgram* programEGL = new QOpenGLShaderProgram();
    programEGL->addShader(&vertexShader);
    programEGL->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#extension GL_OES_EGL_image_external : require\n" \
                                        "#ifdef GL_ES\n"
                                        "precision highp int;\n"
                                        "precision highp float;\n"
                                        "#endif\n"
                                        "varying highp vec2 textureCoords;\n" \
                                        "uniform samplerExternalOES frameTexture;\n"
                                        "varying highp float kWrap;\n" \
                                        "void main() {\n" \
                                        "    gl_FragColor = vec4(texture2D(frameTexture, "
                                        "textureCoords).rgb * kWrap, 1);\n" \
                                        "}");
    programEGL->bindAttributeLocation("vertex_position", 0);
    programEGL->link();
    if (!programEGL->link())
        context->error("Sh_WarpRipple: " + programEGL->log());
    locationMatrixWVP[1] = programEGL->uniformLocation("matrix_wvp");
    locationFrameTexture[1] = programEGL->uniformLocation("frameTexture");
    locationBorderSize[1] = programEGL->uniformLocation("borderSize");
    locationRadius[1] = programEGL->uniformLocation("radius");
    locationSourceRipple[1] = programEGL->uniformLocation("sourceRipple");
    locationHeightRipple[1] = programEGL->uniformLocation("heightRipple");
    locationOffset[1] = programEGL->uniformLocation("offset");
    locationTextureMatrix[1] = programEGL->uniformLocation("textureMatrix");
    locationShadowPower[1] = programEGL->uniformLocation("shadowPower");

    programs.resize(2);
    programs[0] = QSharedPointer<QOpenGLShaderProgram>(programGL);
    programs[1] = QSharedPointer<QOpenGLShaderProgram>(programEGL);
#else
    programs.resize(1);
    programs[0] = QSharedPointer<QOpenGLShaderProgram>(programGL);
#endif
}


QScrollEngine::QEntity* ARSceneShip::createGrid(QScrollEngine::QScene* scene, const QVector2D& size, const QSize& countSteps)
{
    using namespace QScrollEngine;
    QVector2D halfSize = size * 0.5f;
    QVector2D step(size.x() / (float)countSteps.width(), size.y() / (float)countSteps.height());
    QEntity* grid = new QEntity(scene);
    QMesh* mesh = new QMesh(scene);
    mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
    mesh->setSizeOfELement(2);
    mesh->setCountLines((countSteps.width() + 1) + (countSteps.height() + 1));
    mesh->setCountVertices(mesh->countLines() * 2);
    float x, y;
    int i, indexVertex;
    for (i=0, indexVertex=0; i<countSteps.height(); ++i, indexVertex += 2) {
        y = - halfSize.y() + step.y() * i;
        mesh->setVertexPosition(indexVertex, QVector3D(-halfSize.x(), y, 0.0f));
        mesh->setVertexTextureCoord(indexVertex, QVector2D(0.0f, i / (float)countSteps.height()));
        mesh->setVertexPosition(indexVertex + 1, QVector3D(halfSize.x(), y, 0.0f));
        mesh->setVertexTextureCoord(indexVertex + 1, QVector2D(1.0f, i / (float)countSteps.height()));
        mesh->setLine(i, indexVertex, indexVertex + 1);
    }
    {
        y = halfSize.y();
        mesh->setVertexPosition(indexVertex, QVector3D(-halfSize.x(), y, 0.0f));
        mesh->setVertexTextureCoord(indexVertex, QVector2D(0.0f, 1.0f));
        mesh->setVertexPosition(indexVertex + 1, QVector3D(halfSize.x(), y, 0.0f));
        mesh->setVertexTextureCoord(indexVertex + 1, QVector2D(1.0f, 1.0f));
        mesh->setLine(i, indexVertex, indexVertex + 1);
        indexVertex += 2;
    }
    for (i=0; i<countSteps.width(); ++i, indexVertex += 2) {
        x = - halfSize.x() + step.x() * i;
        mesh->setVertexPosition(indexVertex, QVector3D(x, -halfSize.y(), 0.0f));
        mesh->setVertexTextureCoord(indexVertex, QVector2D(i / (float)countSteps.width(), 0.0f));
        mesh->setVertexPosition(indexVertex + 1, QVector3D(x, halfSize.y(), 0.0f));
        mesh->setVertexTextureCoord(indexVertex + 1, QVector2D(i / (float)countSteps.width(), 1.0f));
        mesh->setLine(countSteps.height() + 1 + i, indexVertex, indexVertex + 1);
    }
    {
        x = halfSize.x();
        mesh->setVertexPosition(indexVertex, QVector3D(x, -halfSize.y(), 0.0f));
        mesh->setVertexTextureCoord(indexVertex, QVector2D(1.0f, 0.0f));
        mesh->setVertexPosition(indexVertex + 1, QVector3D(x, halfSize.y(), 0.0f));
        mesh->setVertexTextureCoord(indexVertex + 1, QVector2D(1.0f, 1.0f));
        mesh->setLine(countSteps.height() + 1 + i, indexVertex, indexVertex + 1);
    }
    mesh->updateLocalBoundingBox();
    mesh->applyChanges();
    QSh_ColoredPart* sh = new QSh_ColoredPart();
    QOpenGLTexture* textureGradient = scene->parentContext()->texture("linearGradient");
    if (textureGradient == nullptr)
        textureGradient = scene->parentContext()->loadTexture("linearGradient", ":/Data/linearGradient.png");
    textureGradient->setWrapMode(QOpenGLTexture::ClampToEdge);
    sh->setTexturePart(textureGradient);
    sh->setColorF(0.0f, 0.5f, 1.0f, 0.0);
    sh->setBackColorF(1.0f, 1.0f, 1.0f, 1.0f);
    sh->setSmooth(0.1f);
    sh->setLimit(0.0f);
    QEntity::Part* part = grid->addPart(mesh, QShPtr(sh));
    part->setAlpha(true);
    part->setDrawMode(GL_LINES);
    return grid;
}

QScrollEngine::QEntity* ARSceneShip::_createWater(const QVector2D& size, const QSize& countSteps)
{
    using namespace QScrollEngine;
    QVector2D step(size.x() / (float)countSteps.width(), size.y() / (float)countSteps.height());
    QEntity* water = new QEntity(scene());
    QMesh* mesh = new QMesh(scene());
    mesh->setCountVertices((countSteps.width() + 1) * (countSteps.height() + 1));
    mesh->setSizeOfELement(3);
    mesh->setCountTriangles(countSteps.width() * countSteps.height() * 2);
    QVector2D start = - size * 0.5f;
    int i, j;
    for (j=0; j<=countSteps.height(); ++j) {
        for (i=0; i<=countSteps.width(); ++i) {
            mesh->setVertexPosition((countSteps.width() + 1) * j + i,
                                    QVector3D(start.x() + step.x() * i, start.y() + step.y() * j, 0.0f));
        }
    }
    for (j=0; j<countSteps.height(); ++j) {
        for (i=0; i<countSteps.width(); ++i) {
            std::size_t indexTriangle = (countSteps.width() * j + i) * 2;
            std::size_t indexVertex = (countSteps.width() + 1) * j + i;
            std::size_t indexVertexOfNextLine = (countSteps.width() + 1) * (j + 1) + i;
            mesh->setTriangle(indexTriangle, indexVertex, indexVertex + 1, indexVertexOfNextLine);
            mesh->setTriangle(indexTriangle + 1, indexVertex + 1, indexVertexOfNextLine + 1, indexVertexOfNextLine);
        }
    }
    mesh->applyChanges();
    Sh_WarpRiple* sh = new Sh_WarpRiple();
    sh->setBorderSize(size * 0.4f);
    sh->setRadius(0.5f);
    sh->setHeightRipple(0.0f);
    mesh->updateLocalBoundingBox();
    QBoundingBox bb = mesh->localBoundingBox();
    bb.expand(sh->heightRipple());
    mesh->setLocalBoundingBox(bb);
    water->addPart(mesh, QShPtr(sh));
    //water->addPart(mesh, QShPtr(new QSh_Color(QColor(255, 0, 0))));
    return water;
}

ARSceneShip::~ARSceneShip()
{
}

void ARSceneShip::initScene()
{
    using namespace QScrollEngine;
    QScrollEngineContext* context = scene()->parentContext();
    scene()->setAmbientColor(50, 50, 50);

    {
        Sh_WarpRiple rs;
        if (context->shaderProgram(rs.typeIndex()) == nullptr)
            context->registerShader(&rs);
    }
    context->glLineWidth(2.0f);
    m_grid = createGrid(scene(), QVector2D(5.0f, 5.0f), QSize(10, 10));
    m_grid->setVisibled(false);

    m_water = _createWater(QVector2D(5.0f, 5.0f) / 0.8f, QSize(10, 10));
    m_water->setVisibled(false);

    m_ship = context->loadEntity(":/Data/ship.3DS");
    m_ship->setParentScene(scene());
    m_ship->setScale(0.008f);
    m_ship->convertShaders([&context] (QSh* shader) -> QSh* {
        if (shader == nullptr)
            return nullptr;
        if (shader->typeIndex() == (int)QSh::Type::Color) {
            QSh_Color* shaderColor = static_cast<QSh_Color*>(shader);
            return new QSh_Light(context->emptyTexture(), shaderColor->color(), 1.0f, 10.0f);
        } else if (shader->typeIndex() == (int)QSh::Type::Texture) {
            QSh_Texture* shaderTexture = static_cast<QSh_Texture*>(shader);
            return new QSh_Light(shaderTexture->texture(), shaderTexture->color());
        } else if (shader->typeIndex() == (int)QSh::Type::Light) {
            return shader;
        }
        return nullptr;
    });
    m_ship->setPosition(0.0f, 0.0f, 1.5f);
    m_ship->setVisibled(false);
    m_shipAngle = 0.0f;
    m_shipAngularDamping = 0.01f;
    m_shipSize = QVector2D(0.6f, 0.3f);
    m_shipStartDeep = - 2.0f;
    m_shipCurrentDeep = m_shipStartDeep;
    m_shipDeepVelocity = 0.03f;

    QLight* light = new QLight(scene());
    light->setPosition(0.0f, 0.0f, 10.0f);
    light->setRadius(30.0f);

    m_state = 0.0f;
    m_speedOfChangeState = 0.05f;
}

void ARSceneShip::setFrameTexture(GLuint frameTexture, const QMatrix3x3& textureMatrix, bool egl)
{
    Sh_WarpRiple* sh = dynamic_cast<Sh_WarpRiple*>(m_water->shader(0).data());
    if (sh) {
        sh->setFrameTexture(frameTexture, egl, textureMatrix);
        if (frameTexture != std::numeric_limits<GLuint>::max()) {
            QScrollEngine::QScrollEngineContext* context = scene()->parentContext();
#if defined(GL_TEXTURE_EXTERNAL_OES)
            if (egl) {
                context->glBindTexture(GL_TEXTURE_EXTERNAL_OES, frameTexture);
                context->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                context->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                context->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                context->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
#endif
                context->glBindTexture(GL_TEXTURE_2D, frameTexture);
                context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(GL_TEXTURE_EXTERNAL_OES)
            }
#endif
        }
    }
}

void ARSceneShip::_beginUpdate()
{
    if (arTracker()->trackingState() != ARTracker::Tracking)
        return;
    using namespace QScrollEngine;
    if (m_state < 1.0f) {
        m_ship->setVisibled(false);
        m_water->setVisibled(false);
        m_grid->setVisibled(true);
        QSh_ColoredPart* sh = dynamic_cast<QSh_ColoredPart*>(m_grid->part(0)->shader().data());
        QColor color = sh->backColor();
        if (m_state < 0.5f) {
            sh->setLimit(1.0f - m_state * 2.0f);
            color.setAlphaF(1.0f);
            sh->setBackColor(color);
        } else {
            sh->setLimit(0.0f);
            color.setAlphaF((1.0f - m_state) * 2.0f);
            sh->setBackColor(color);
        }
        m_state += m_speedOfChangeState;
    } else {
        m_grid->setVisibled(false);
        m_water->setVisibled(true);
        m_ship->setVisibled(true);

        scene()->updateCameraInfo(scene()->parentContext()->camera);
        QVector2D camPos = scene()->cameraPosition().toVector2D();
        Sh_WarpRiple* sh = dynamic_cast<Sh_WarpRiple*>(m_water->part(0)->shader().data());
        float prevSourceOffset = fmodf(sh->sourceRipple().length() / sh->radius(), (float)(2.0f * M_PI));
        sh->setSourceRipple(sh->sourceRipple() * 0.95f + camPos * 0.05f);
        float sourceOffset = fmodf(sh->sourceRipple().length() / sh->radius(), (float)(2.0f * M_PI));
        sh->setOffset(- (sourceOffset + std::fmod(- (sh->offset() + prevSourceOffset) + 0.1f, (float)(2.0f * M_PI))));
        sh->setHeightRipple(qMin(sh->heightRipple() + 0.03f, 0.3f));
        _updateShip();
    }
}

void ARSceneShip::_endUpdate()
{
    scene()->setEnabled((arTracker()->trackingState() == ARTracker::Tracking));
}

void ARSceneShip::_updateShip()
{
    m_shipCurrentDeep = std::min(m_shipCurrentDeep + m_shipDeepVelocity, 0.0f);
    QQuaternion deepOrientation = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f,
                                             90.0f * qPow(m_shipCurrentDeep / m_shipStartDeep, 0.9f));
    Sh_WarpRiple* sh = dynamic_cast<Sh_WarpRiple*>(m_water->part(0)->shader().data());
    assert(sh != nullptr);
    float angleSource = qAtan2(sh->sourceRipple().y(), sh->sourceRipple().x());
    float deltaAngle = (angleSource + static_cast<float>(M_PI)) - m_shipAngle;
    float deltaAngle_2 = deltaAngle - static_cast<float>(2.0f * M_PI);
    if (std::fabs(deltaAngle_2) < std::fabs(deltaAngle))
        deltaAngle = deltaAngle_2;
    float angularVelocity = std::fabs(deltaAngle) * ((deltaAngle > 0.0f) ? 1.0f : -1.0f) * m_shipAngularDamping;
    m_shipAngle = std::fmod(m_shipAngle + angularVelocity, static_cast<float>(2.0f * M_PI));
    QVector2D deltaX(std::cos(m_shipAngle), std::sin(m_shipAngle));
    deltaX *= m_shipSize.x();
    QVector2D deltaY(- deltaX.y(), deltaX.x());
    deltaY *= m_shipSize.y();
    QVector3D v[4];
    v[0] = sh->transform(QVector3D(- deltaX - deltaY, 0.0f));
    v[1] = sh->transform(QVector3D(- deltaX + deltaY, 0.0f));
    v[2] = sh->transform(QVector3D(deltaX - deltaY, 0.0f));
    v[3] = sh->transform(QVector3D(deltaX + deltaY, 0.0f));
    QVector3D axisX[2];
    axisX[0] = v[2] - v[0];
    axisX[1] = v[3] - v[1];
    axisX[0] = (axisX[0] + axisX[1]) * 0.5f;
    float length = axisX[0].length();
    if (length < 0.00005f) {
        axisX[0] = axisX[1];
        axisX[0].normalize();
    } else {
        axisX[0] /= length;
    }
    axisX[0].setZ(axisX[0].z());
    QVector3D axisY[2];
    axisY[0] = v[1] - v[0];
    axisY[1] = v[3] - v[2];
    axisY[0] = (axisY[0] + axisY[1]) * 0.5f;
    length = axisY[0].length();
    if (length < 0.00005f) {
        axisY[0] = axisY[1];
        axisY[0].normalize();
    } else {
        axisY[0] /= length;
    }
    axisY[0].setZ(axisY[0].z());
    QVector3D axisZ = QVector3D::crossProduct(axisX[0], axisY[0]);
    axisZ.normalize();
    axisY[0] = QVector3D::crossProduct(axisZ, axisX[0]);
    axisY[0].normalize();
    TMath::TMatrixf matrixRotation(3, 3);
    matrixRotation(0, 0) = axisX[0].x();
    matrixRotation(0, 1) = axisX[0].y();
    matrixRotation(0, 2) = axisX[0].z();
    matrixRotation(1, 0) = axisY[0].x();
    matrixRotation(1, 1) = axisY[0].y();
    matrixRotation(1, 2) = axisY[0].z();
    matrixRotation(2, 0) = axisZ.x();
    matrixRotation(2, 1) = axisZ.y();
    matrixRotation(2, 2) = axisZ.z();
    matrixRotation.invert();
    QMatrix3x3 qMatrixRotation;
    TMath::TTools::convert(qMatrixRotation, matrixRotation);
    QQuaternion orientation;
    QScrollEngine::QOtherMathFunctions::matrixToQuaternion(qMatrixRotation, orientation);
    m_ship->setOrientation(orientation * deepOrientation);
    m_ship->setPosition(0.0f, 0.0f, (v[0].z() + v[1].z() + v[2].z() + v[3].z()) * 0.25f + m_shipCurrentDeep);
}
