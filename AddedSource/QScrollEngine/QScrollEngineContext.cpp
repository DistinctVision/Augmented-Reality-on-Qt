#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QScene.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QSpotLight.h"
#include "QScrollEngine/QGLPrimitiv.h"
#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QLight.h"
#include "QScrollEngine/QFileLoad3DS.h"
#include "QScrollEngine/QCamera3D.h"
#include <QSurface>
#include <QSurfaceFormat>
#include <cassert>
#include <type_traits>
#include <QTime>

namespace QScrollEngine {

QScrollEngineContext::QScrollEngineContext(QOpenGLContext* context)
{
    m_animationSpeed = 1.0f;
    m_enableClearing = true;
    m_postEffectUsed = false;
    backgroundColor.setRgb(0,0,0);
    camera = new QCamera3D();
    m_openGLContext = nullptr;
    m_quad = nullptr;
    setOpenGLContext(context);
}

void QScrollEngineContext::clearContext()
{
    if (m_openGLContext) {
        deleteObjectsOfPostProcess();
        m_emptyTexture->destroy();
        delete m_emptyTexture;
        for (std::map<QString, QOpenGLTexture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it) {
            QOpenGLTexture* texture = it->second;
            texture->release();
            texture->destroy();
            delete texture;
        }
        m_textures.clear();
        while (m_scenes.size() > 0) {
            delete m_scenes[m_scenes.size() - 1];
        }
        m_scenes.clear();
        m_sceneOrderStep.clear();
    }
    clearShaders();
    m_tempAlphaObjects.clear();
    if (m_quad) {
        delete m_quad;
        m_quad = nullptr;
    }
    m_stateTimeEvent = Play;
    m_FBOs[0] = nullptr;
    m_nDecreaseTexture = 6.0f;
    m_postEffectUsed = false;
    m_emptyTexture = nullptr;
    if (m_openGLContext) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    m_openGLContext = nullptr;
}

void QScrollEngineContext::setOpenGLContext(QOpenGLContext* openGLContext)
{
    if (m_openGLContext != openGLContext) {
        clearContext();
        m_openGLContext = openGLContext;
        if (m_openGLContext)
            initializeContext();
    }
}

void QScrollEngineContext::setOpenGLContext(QOpenGLContext* openGLContext, int width, int height)
{
    if (m_openGLContext != openGLContext) {
        clearContext();
        m_openGLContext = openGLContext;
        if (m_openGLContext) {
            camera->setResolution(width, height);
            camera->calculateProjectionMatrix();
            m_normalSize.setWidth(width);
            m_normalSize.setHeight(height);
            resolveScreenQuad();
            initializeContext();
        }
    } else {
        resizeContext(width, height);
    }
}

QScrollEngineContext::~QScrollEngineContext()
{
    clearContext();
    delete camera;
}

bool QScrollEngineContext::registerShader(QSh* shaderSample)
{
    //static_assert(std::is_base_of<QSh, QSh_Class>::value,
    //              "QScrollEngine: Error. Register shader must be inherited from QSh");
    int typeIndex = shaderSample->typeIndex();
    std::map<int, _Drawing>::iterator finded = m_drawings.find(typeIndex);
    if (finded != m_drawings.end()) {
        qDebug() << "QScrollEngine: shader with this type index already exists.";
        return false;
    }
    m_drawings[typeIndex] = _Drawing();
    shaderSample->load(this, m_drawings[typeIndex].programs);
    m_drawings[typeIndex].currentObjects.resize(m_drawings[typeIndex].programs.size());
    m_drawings[typeIndex].attributes = shaderSample->attributes();

    for (std::vector<QSharedPointer<QOpenGLShaderProgram>>::iterator it = m_drawings[typeIndex].programs.begin();
         it != m_drawings[typeIndex].programs.end();
         ++it) {
        QOpenGLShaderProgram* program = it->data();
        program->bindAttributeLocation("vertex_position", 0);
        for (std::size_t i=0; i<m_drawings[typeIndex].attributes.size(); ++i) {
            QSh::VertexAttributes vert_attr = m_drawings[typeIndex].attributes[i];
            switch (vert_attr) {
                case QSh::VertexAttributes::TextureCoords: {
                    program->bindAttributeLocation("vertex_texcoord", static_cast<int>(vert_attr) - 1);
                } break;
                case QSh::VertexAttributes::Normals: {
                    program->bindAttributeLocation("vertex_normal", static_cast<int>(vert_attr) - 1);
                } break;
                case QSh::VertexAttributes::RgbColors: {
                    program->bindAttributeLocation("vertex_rgbColor", static_cast<int>(vert_attr) - 1);
                } break;
                default:
                    break;
            }
        }

        if (!program->bind()) {
            error(QString("Don't bind shader - " +
                          QString::fromUtf8(typeid(*shaderSample).name())));
            return false;
        }
        program->release();
    }
    return true;

    return true;
}

void QScrollEngineContext::clearShaders()
{
    m_drawings.clear();
}

void QScrollEngineContext::registerDefaultShaders()
{
    //setlocale(LC_NUMERIC, "C");

    m_shader_bloomMap.load(this, m_shaderProgram_bloomMap);
    m_shader_blur.load(this, m_shaderProgram_blur);
    m_shader_bloom.load(this, m_shaderProgram_bloom);
    QSh_Color sh_color;
    registerShader(&sh_color);
    QSh_Texture sh_texture1;
    registerShader(&sh_texture1);
    QSh_Light sh_light;
    registerShader(&sh_light);
    QSh_ColoredPart sh_coloredPart;
    registerShader(&sh_coloredPart);
    QSh_Refraction_FallOff sh_refraction_FallOff;
    registerShader(&sh_refraction_FallOff);
    QSh_LightVC sh_lightVC;
    registerShader(&sh_lightVC);

    //setlocale(LC_ALL, "");
}

void QScrollEngineContext::_addScene(QScene* scene)
{
    unsigned int i, orderIndex = std::numeric_limits<unsigned int>::max(), currentIndex = 0;
    for (i=0; i<m_sceneOrderStep.size(); ++i) {
        if (scene->order() <= m_scenes[currentIndex]->order()) {
            orderIndex = i;
            break;
        }
        currentIndex += m_sceneOrderStep[i];
    }
    if (orderIndex == std::numeric_limits<unsigned int>::max()) {
        m_sceneOrderStep.push_back(1);
    } else {
        if (scene->order() == m_scenes[currentIndex]->order()) {
            ++m_sceneOrderStep[orderIndex];
        } else {
            m_sceneOrderStep.insert(m_sceneOrderStep.begin() + orderIndex, 1);
        }
    }
    scene->m_index = currentIndex;
    m_scenes.insert(m_scenes.begin() + currentIndex, scene);
    for (unsigned int i=scene->m_index+1; i<m_scenes.size(); ++i) {
        ++m_scenes[i]->m_index;
    }
}

void QScrollEngineContext::_deleteScene(QScene* scene)
{
    unsigned int i, currentIndex = 0, index = scene->m_index;
    for (i=0; i<m_sceneOrderStep.size(); ++i) {
        if (m_scenes[index]->order() == m_scenes[currentIndex]->order()) {
            --m_sceneOrderStep[i];
            if (m_sceneOrderStep[i] <= 0)
                m_sceneOrderStep.erase(m_sceneOrderStep.begin() + i);
            break;
        } else if (m_scenes[index]->order() < m_scenes[currentIndex]->order())
            break;
        currentIndex += m_sceneOrderStep[i];
    }
    for (i=index+1; i<m_scenes.size(); ++i)
        m_scenes[i]->m_index = i - 1;
    m_scenes.erase(m_scenes.begin() + index);
}

QOpenGLTexture* QScrollEngineContext::loadTexture(const QString& name, const QString& path)
{
    QOpenGLTexture* newTexture = texture(name);
    if (newTexture != nullptr) {
        qDebug() << QString("QScrollEngine: Texture with name '") + name + "' already exist!";
        return newTexture;
    }
    QImage image(path);
    if (image.isNull()) {
        error(QString("QScrollEngine: Texture - '") + name + "' - " + path + " is not loaded.");
        return nullptr;
    }
    newTexture = new QOpenGLTexture(image);
    if (!newTexture->isCreated()) {
        delete newTexture;
        error(QString("QScrollEngine: Texture - '") + name + "' - " + path + " is not created.");
        return nullptr;
    }
    m_textures[name] = newTexture;
    return newTexture;
}

QOpenGLTexture* QScrollEngineContext::loadTexture(const QString& name, const QImage& image)
{
    QOpenGLTexture* newTexture = texture(name);
    if (newTexture != nullptr) {
        qDebug() << QString("QScrollEngine: Texture with name '") + name + "' already exist!";
        return newTexture;
    }
    newTexture = new QOpenGLTexture(image);
    if (!newTexture->isCreated()) {
        delete newTexture;
        error(QString("QScrollEngine: Texture - '") + name + "' - [from image] is not created.");
        return nullptr;
    }
    newTexture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_textures[name] = newTexture;
    return newTexture;
}

void QScrollEngineContext::addTexture(const QString& name, QOpenGLTexture* texture)
{
    QOpenGLTexture* newTexture = QScrollEngineContext::texture(name);
    if (newTexture != nullptr) {
        qDebug() << QString("QScrollEngine: Texture with name '") + name + "' already exist!";
        return;
    }
    m_textures[name] = texture;
}

QEntity* QScrollEngineContext::loadEntity(const QString& path, const QString& textureDir, const QString& prefixTextureName)
{
    QEntity* entity = m_fileSaveLoad3DS.loadEntity(this, path, textureDir, prefixTextureName);
    if (entity == nullptr)
        error(QString("QScrollEngine: Entity - ") + path + " - is not loaded.");
    return entity;
}

bool QScrollEngineContext::saveEntity(QEntity* entity, const QString& path, const QString& textureDir,
                                      const QString& prefixTextureName)
{
    return m_fileSaveLoad3DS.saveEntity(this, entity, path, textureDir, prefixTextureName);
}

QOpenGLTexture* QScrollEngineContext::texture(const QString& name)
{
    std::map<QString, QOpenGLTexture*>::iterator it = m_textures.find(name);
    if (it != m_textures.end())
        return it->second;
    return nullptr;
}

bool QScrollEngineContext::textureName(QString& result, const QOpenGLTexture* texture) const
{
    for (std::map<QString, QOpenGLTexture*>::const_iterator it = m_textures.cbegin(); it != m_textures.end(); ++it) {
        if (it->second == texture) {
            result = it->first;
            return true;
        }
    }
    return false;
}

void QScrollEngineContext::deleteTexture(const QString& name)
{
    std::map<QString, QOpenGLTexture*>::iterator it = m_textures.find(name);
    if (it != m_textures.end()) {
        QOpenGLTexture* texture = it->second;
        texture->release();
        texture->destroy();
        delete texture;
        m_textures.erase(it);
    }
}

void QScrollEngineContext::deleteTexture(QOpenGLTexture* texture)
{
    for (std::map<QString, QOpenGLTexture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
        if (it->second == texture) {
            texture->release();
            texture->destroy();
            m_textures.erase(it);
            delete texture;
            return;
        }
}

void QScrollEngineContext::deleteAllTextures()
{
    for (std::map<QString, QOpenGLTexture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it) {
        QOpenGLTexture* texture = it->second;
        texture->release();
        texture->destroy();
        delete texture;
    }
    m_textures.clear();
}

void QScrollEngineContext::initializeContext()
{
    initializeOpenGLFunctions();

    glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    /*// Set nearest filtering mode for texture minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set bilinear filtering mode for texture magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    registerDefaultShaders();
    if (m_quad)
        delete m_quad;
    m_quad = new QMesh(this);
    QGLPrimitiv::createQuad(m_quad);

    resolveScreenQuad();

    initObjectsOfPostProcess();

    if (m_emptyTexture != nullptr) {
        m_emptyTexture->release();
        m_emptyTexture->destroy();
        delete m_emptyTexture;
    }
    QImage emptyImage = QImage(1, 1, QImage::Format_RGB32);
    emptyImage.setPixel(0, 0, qRgb(255, 255, 255));
    m_emptyTexture = new QOpenGLTexture(emptyImage);
    assert(m_emptyTexture->isCreated());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool QScrollEngineContext::loadShader(QOpenGLShaderProgram* program, QString nameVertex, QString nameFragment)
{
    program->create();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/" + nameVertex + ".vsh")) {
        error(QString("Don't open - " + nameVertex + ".vsh\n") + program->log());
        return false;
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/" + nameFragment + ".fsh")) {
        error(QString("Don't open - " + nameFragment + ".fsh\n") + program->log());
        return false;
    }
    if (!program->link()) {
        error(QString("Don't link - Shader - " + nameVertex + "|" + nameFragment + "\n") + program->log());
        return false;
    }
    return true;
}

void QScrollEngineContext::resolveScreenQuad()
{
    const float zNear = 1.0f, zFar = 3.0f, Z = 2.0f;
    m_quadFinalMatrix.ortho(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar);
    QMatrix4x4 m = m_quadFinalMatrix.inverted();
    QVector2D d(2.0f * (1.0f) - 1.0f, 1.0f - 2.0f * (0.0f));
    QVector3D resultNear = m * QVector3D(d, -1.0f);
    QVector3D resultFar = m * QVector3D(d, 1.0f);
    float t = (Z - zNear) / (zFar - zNear);
    QVector3D result = resultNear + (resultFar - resultNear) * t;
    m_quadFinalMatrix.translate(0.0f, 0.0f, -Z);
    m_quadFinalMatrix.scale(result.x() * 2.0f, result.y() * 2.0f);
    m_shader_bloomMap.setFinalMatrix(m_quadFinalMatrix);
    m_shader_blur.setFinalMatrix(m_quadFinalMatrix);
    m_shader_bloom.setFinalMatrix(m_quadFinalMatrix);
}

void QScrollEngineContext::initObjectsOfPostProcess()
{
    if (m_postEffectUsed) {
        m_decreaseSize.setWidth(qRound(m_normalSize.width() / m_nDecreaseTexture));
        m_decreaseSize.setHeight(qRound(m_normalSize.height() / m_nDecreaseTexture));
        m_shader_blur.setSize(m_decreaseSize);
        m_shader_bloom.setSize(m_normalSize);
        if (m_normalSize.width() < 10 || m_normalSize.height() < 10) {
            m_normalSize.setWidth(10);
            m_normalSize.setHeight(10);
        }
        m_FBOs[0] = new QOpenGLFramebufferObject(m_normalSize, QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D);
        m_FBOs[1] = new QOpenGLFramebufferObject(m_decreaseSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_FBOs[1]->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_FBOs[2] = new QOpenGLFramebufferObject(m_decreaseSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_FBOs[2]->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void QScrollEngineContext::deleteObjectsOfPostProcess()
{
    if (m_FBOs[0]) {
        delete m_FBOs[0];
        delete m_FBOs[1];
        delete m_FBOs[2];
        m_FBOs[0] = nullptr;
    }
}

QOpenGLShaderProgram* QScrollEngineContext::shaderProgram(int indexType, int subIndexType)
{
    std::map<int, _Drawing>::iterator it = m_drawings.find(indexType);
    if (it == m_drawings.end())
        return nullptr;
    return it->second.programs[subIndexType].data();
}

QOpenGLShaderProgram* QScrollEngineContext::shaderProgram(QSh::Type type, int subIndexType)
{
    return shaderProgram(static_cast<int>(type), subIndexType);
}

const std::vector<QSh::VertexAttributes>& QScrollEngineContext::vertexAttributesOfShader(int indexType) const
{
    std::map<int, _Drawing>::const_iterator it = m_drawings.find(indexType);
    if (it == m_drawings.cend())
        return m_drawings.at(static_cast<int>(QSh::Type::Color)).attributes;
    return it->second.attributes;
}

const std::vector<QSh::VertexAttributes>& QScrollEngineContext::vertexAttributesOfShader(QSh::Type type) const
{
    return vertexAttributesOfShader(static_cast<int>(type));
}

void QScrollEngineContext::resizeContext(int width, int height)
{
    if ((m_normalSize.width() == width) && (m_normalSize.height() == height))
        return;
    glViewport(0, 0, width, height);
    camera->setResolution(width, height);
    camera->calculateProjectionMatrix();
    m_normalSize.setWidth(width);
    m_normalSize.setHeight(height);
    resolveScreenQuad();
    deleteObjectsOfPostProcess();
    initObjectsOfPostProcess();
}

void QScrollEngineContext::drawSprite(const QSprite* sprite)
{
    QSh* shaderParameters = sprite->shader().data();
    if (!shaderParameters)
        return;
    shaderParameters->preprocess(sprite);
    _Drawing& drawing = m_drawings[shaderParameters->currentTypeIndex()];
    QOpenGLShaderProgram* program = drawing.programs[shaderParameters->subTypeIndex()].data();
    if (!program->bind())
        return;
    _enableVertexAttributes(program, drawing.attributes);
    m_quad->bind(drawing.attributes);
    if (!shaderParameters->use(this, program, sprite)) {
        _disableVertexAttributes(program, drawing.attributes);
        program->release();
        return;
    }
    m_quad->bind(drawing.attributes);
    if ((sprite->m_scale.x() > 0.0f && sprite->m_scale.y() > 0.0f) ||
            (sprite->m_scale.x() < 0.0f && sprite->m_scale.y() < 0.0f)) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    } else {
        glCullFace(GL_CW);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glCullFace(GL_CCW);
    }
    program->release();
    _disableVertexAttributes(program, drawing.attributes);
}

void QScrollEngineContext::drawMesh(const QMesh* mesh, const QDrawObject3D* drawObject, QSh* shader)
{
    shader->preprocess(drawObject);
    _Drawing& drawing = m_drawings[shader->currentTypeIndex()];
    QOpenGLShaderProgram* program = drawing.programs[shader->subTypeIndex()].data();
    if (!program->bind())
        return;
    if (!shader->use(this, program, drawObject)) {
        program->release();
        return;
    }
    _enableVertexAttributes(program, drawing.attributes);
    if (!mesh->bind(drawing.attributes)) {
        _disableVertexAttributes(program, drawing.attributes);
        program->release();
        return;
    }
    glDrawElements(drawObject->drawMode(), mesh->m_elements.size(), GL_UNSIGNED_INT, nullptr);
    _disableVertexAttributes(program, drawing.attributes);
    program->release();
}

void QScrollEngineContext::drawPartEntity(const QEntity::Part* part)
{
    QSh* shaderParameters = part->shader().data();
    if (shaderParameters == nullptr)
        return;
    shaderParameters->preprocess(part);
    _Drawing& drawing = m_drawings[shaderParameters->currentTypeIndex()];
    QOpenGLShaderProgram* program = drawing.programs[shaderParameters->subTypeIndex()].data();
    if (!program->bind())
        return;
    if (!shaderParameters->use(this, program, part)) {
        program->release();
        return;
    }
    QMesh* mesh = part->mesh();
    _enableVertexAttributes(program, drawing.attributes);
    if (!mesh->bind(drawing.attributes)) {
        _disableVertexAttributes(program, drawing.attributes);
        program->release();
        return;
    }
    glDrawElements(part->drawMode(), mesh->m_elements.size(), GL_UNSIGNED_INT, nullptr);
    _disableVertexAttributes(program, drawing.attributes);
    program->release();
}

void QScrollEngineContext::drawEntity(QEntity* entity)
{
    for (std::vector<QEntity::Part*>::iterator it = entity->m_parts.begin(); it != entity->m_parts.end(); ++it)
        drawPartEntity(*it);
}

void QScrollEngineContext::drawLines(const QVector2D* lineVertices, std::size_t countVertices, const QColor& color,
                                     const QMatrix4x4& matrixWorldViewProj)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    QOpenGLShaderProgram* program = m_drawings[static_cast<int>(QSh::Type::Color)].programs[0].data();
    program->bind();
    program->enableAttributeArray(0);
    program->setUniformValue("matrix_wvp", matrixWorldViewProj);
    program->setUniformValue("color", color);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), lineVertices);
    glDrawArrays(GL_LINES, 0, countVertices);
    program->disableAttributeArray(0);
}

void QScrollEngineContext::_enableVertexAttributes(QOpenGLShaderProgram* program,
                                                   const std::vector<QSh::VertexAttributes>& attributes)
{
    program->enableAttributeArray(0);
    for (std::vector<QSh::VertexAttributes>::const_iterator it = attributes.cbegin();
         it != attributes.cend();
         ++it) {
        program->enableAttributeArray(static_cast<int>(*it) - 1);
    }
}

void QScrollEngineContext::_disableVertexAttributes(QOpenGLShaderProgram* program,
                                                    const std::vector<QSh::VertexAttributes>& attributes)
{
    program->disableAttributeArray(0);
    for (std::vector<QSh::VertexAttributes>::const_iterator it = attributes.cbegin();
         it != attributes.cend();
         ++it) {
        program->disableAttributeArray(static_cast<int>(*it) - 1);
    }
}

void QScrollEngineContext::_addTempAlphaObject(const TempAlphaObject& object)
{
    if (m_tempAlphaObjects.empty()) {
        m_tempAlphaObjects.push_back(object);
        return;
    }
    for (std::list<TempAlphaObject>::iterator it = m_tempAlphaObjects.begin(); it != m_tempAlphaObjects.end(); ++it) {
        if (object.zDistance > it->zDistance) {
            m_tempAlphaObjects.insert(it, object);
            return;
        }
    }
    m_tempAlphaObjects.push_back(object);
}

void QScrollEngineContext::_sortingTempAlphaObjects()
{
    float minZDis;
    TempAlphaObject temp;
    std::list<TempAlphaObject>::iterator it, c_it, begin = m_tempAlphaObjects.begin();

    while (begin != m_tempAlphaObjects.end()) {
        it = begin;
        minZDis = it->zDistance;
        c_it = it;
        for (++it; it != m_tempAlphaObjects.end(); ++it) {
            if (it->zDistance < minZDis) {
                minZDis = it->zDistance;
                c_it = it;
            }
        }
        temp = (*begin);
        (*begin) = (*c_it);
        (*c_it) = temp;
        ++begin;
    }
}

void QScrollEngineContext::_drawCurrent()
{
    QSh* shaderOfObject;
    QEntity::Part* part;
    QOpenGLShaderProgram* program;
    std::size_t k;
    for (std::map<int, _Drawing>::iterator it = m_drawings.begin(); it != m_drawings.end(); ++it) {
        _Drawing& drawing = it->second;
        for (k=0; k<drawing.programs.size(); ++k) {
            if (!drawing.currentObjects[k].sprites.empty() || !drawing.currentObjects[k].partEntities.empty()) {
                program = drawing.programs[k].data();
                if (program->bind()) {
                    _enableVertexAttributes(program, drawing.attributes);
                    m_quad->bind(drawing.attributes);
                    QSprite* sprite;
                    for (std::vector<QSprite*>::iterator it = drawing.currentObjects[k].sprites.begin();
                         it != drawing.currentObjects[k].sprites.end();
                         ++it) {
                        sprite = *it;
                        shaderOfObject = sprite->shader().data();
                        if (shaderOfObject->use(this, program, sprite)) {
                            if ((sprite->m_scale.x() > 0.0f && sprite->m_scale.y() > 0.0f) ||
                                    (sprite->m_scale.x() < 0.0f && sprite->m_scale.y() < 0.0f)) {
                                glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
                            } else {
                                glCullFace(GL_CW);
                                glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
                                glCullFace(GL_CCW);
                            }
                        }
                    }
                    drawing.currentObjects[k].sprites.clear();
                    for (std::vector<QEntity::Part*>::iterator it = drawing.currentObjects[k].partEntities.begin();
                         it != drawing.currentObjects[k].partEntities.end();
                         ++it) {
                        part = *it;
                        shaderOfObject = part->shader().data();
                        if (shaderOfObject->use(this, program, part)) {
                            QMesh* mesh = part->mesh();
                            if (mesh->bind(drawing.attributes))
                                glDrawElements(part->drawMode(), mesh->m_elements.size(), GL_UNSIGNED_INT, nullptr);
                        }
                    }
                    drawing.currentObjects[k].partEntities.clear();
                    _disableVertexAttributes(program, drawing.attributes);
                    program->release();
                }
            }
        }
    }
}

void QScrollEngineContext::beginPaint()
{
    if (m_postEffectUsed) {
        glViewport(0, 0, m_normalSize.width(), m_normalSize.height());
        m_FBOs[0]->bind();
    }
    if (m_enableClearing) {
        glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void QScrollEngineContext::drawScenes()
{
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    std::size_t i, j, begin = 0, end;
    QAnimation3D::m_animationSpeed_global = m_animationSpeed;
    for (i = 0; i < m_sceneOrderStep.size(); ++i) {
        end = begin + m_sceneOrderStep[i];
        for (j = begin; j < end; ++j) {
            if (m_scenes[j]->enabled()) {
                emit m_scenes[j]->beginDrawing();
                m_scenes[j]->_update();
            }
        }
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        //Draw objects without alpha
        _drawCurrent();
        //Draw objects with alpha
        for (j = begin; j < end; ++j) {
            if (m_scenes[j]->enabled()) {
                emit m_scenes[j]->beginDrawingAlphaObjects();
            }
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (std::list<TempAlphaObject>::iterator it = m_tempAlphaObjects.begin(); it != m_tempAlphaObjects.end(); ++it)
            it->drawObject->draw(this);
        m_tempAlphaObjects.clear();
        for (j = begin; j < end; ++j) {
            if (m_scenes[j]->enabled()) {
                emit m_scenes[j]->endDrawing();
            }
        }
        begin = end;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QScrollEngineContext::endPaint(int defaultFBOId)
{
    if (m_postEffectUsed) {
        glFrontFace(GL_CCW);
        m_FBOs[1]->bind();
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glViewport(0, 0, m_decreaseSize.width(), m_decreaseSize.height());
        QOpenGLShaderProgram* program = m_shaderProgram_bloomMap[0].data();
        program->bind();
        m_shader_bloomMap.use(this, program, nullptr);
        m_shader_bloomMap.bindScreenTexture(this, m_FBOs[0]->texture());
        glGenerateMipmap(GL_TEXTURE_2D);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quad->m_buffers[0].vboIds.bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[1].vboIds.bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[2].vboIds.bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        //program->release();

        m_FBOs[2]->bind();
        program = m_shaderProgram_blur[0].data();
        program->bind();
        m_shader_blur.setByX();
        m_shader_blur.use(this, program, nullptr);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quad->m_buffers[0].vboIds.bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[1].vboIds.bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[2].vboIds.bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        m_shader_blur.bindScreenTexture(this, m_FBOs[1]->texture());
        glGenerateMipmap(GL_TEXTURE_2D);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        m_FBOs[1]->bind();
        m_shader_blur.setByY();
        m_shader_blur.use(this, program, nullptr);
        m_shader_blur.bindScreenTexture(this, m_FBOs[2]->texture());
        glGenerateMipmap(GL_TEXTURE_2D);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        //program->release();
        glViewport(0, 0, m_normalSize.width(), m_normalSize.height());
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
        program = m_shaderProgram_bloom[0].data();
        program->bind();
        m_shader_bloom.use(this, program, nullptr);
        m_shader_bloom.bindBloomMapTexture(this, m_FBOs[1]->texture());
        m_shader_bloom.bindScreenTexture(this, m_FBOs[0]->texture());
        glGenerateMipmap(GL_TEXTURE_2D);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quad->m_buffers[0].vboIds.bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[1].vboIds.bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad->m_buffers[2].vboIds.bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        program->release();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QScrollEngineContext::paintContext()
{
    if (m_openGLContext == nullptr)
        return;
    //lock();

    GLint defaultFBOId = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBOId);

    beginPaint();

    drawScenes();

    endPaint(defaultFBOId);

    //unlock();
}

}
