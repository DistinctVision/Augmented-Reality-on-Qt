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
#include <cassert>
#include <type_traits>
#include <QTime>

namespace QScrollEngine {

QScrollEngineContext::QScrollEngineContext(QOpenGLContext* context)
{
    _animationSpeed = 1.0f;
    _enableClearing = true;
    _postEffectUsed = false;
    backgroundColor.setRgb(0,0,0);
    camera = new QCamera3D();
    _openGLContext = nullptr;
    _quad = nullptr;
    clearContext();
    setOpenGLContext(context);
}

void QScrollEngineContext::clearContext()
{
    if (_openGLContext) {
        deleteObjectsOfPostProcess();
        _emptyTexture->destroy();
        delete _emptyTexture;
        for (std::map<QString, QOpenGLTexture*>::iterator it = _textures.begin(); it != _textures.end(); ++it) {
            QOpenGLTexture* texture = it->second;
            texture->release();
            texture->destroy();
            delete texture;
        }
        _textures.clear();
        for (std::vector<QScene*>::iterator it = _scenes.begin(); it != _scenes.end(); ++it)
            delete *it;
        _scenes.clear();
        _sceneOrderStep.clear();
    }
    clearShaders();
    _tempAlphaObjects.clear();
    if (_quad) {
        delete _quad;
        _quad = nullptr;
    }
    _stateTimeEvent = Play;
    _FBOs[0] = nullptr;
    _nDecreaseTexture = 5.3f;
    _postEffectUsed = false;
    _emptyTexture = nullptr;
    _openGLContext = nullptr;
}

void QScrollEngineContext::setOpenGLContext(QOpenGLContext* openGLContext)
{
    if (_openGLContext != openGLContext) {
        clearContext();
        _openGLContext = openGLContext;
        if (_openGLContext)
            initializeContext();
    }
}

void QScrollEngineContext::setOpenGLContext(QOpenGLContext* openGLContext, int width, int height)
{
    if (_openGLContext != openGLContext) {
        clearContext();
        _openGLContext = openGLContext;
        if (_openGLContext) {
            camera->setResolution(width, height);
            camera->calculateProjectionMatrix();
            _normalSize.setWidth(width);
            _normalSize.setHeight(height);
            resolveScreenQuad();
            initializeContext();
        }
    } else {
        resizeContext(width, height);
    }
}

QScrollEngineContext::~QScrollEngineContext()
{
    delete camera;
    clearContext();
}

bool QScrollEngineContext::registerShader(QSh* shaderSample)
{
    //static_assert(std::is_base_of<QSh, QSh_Class>::value, "QScrollEngine: Error. Register shader must be inherited from QSh");
    int indexType = shaderSample->indexType();
    std::map<int, _Drawing>::iterator finded = _drawings.find(indexType);
    if (finded != _drawings.end()) {
        qDebug() << "QScrollEngine: shader with this index type already exists.";
        return false;
    }
    _drawings[indexType] = _Drawing();
    shaderSample->load(this, _drawings[indexType].programs);
    _drawings[indexType].currentObjects.resize(_drawings[indexType].programs.size());
    return true;
}

void QScrollEngineContext::clearShaders()
{
    _drawings.clear();
}

void QScrollEngineContext::registerDefaultShaders()
{
    setlocale(LC_NUMERIC, "C");

    _shader_bloomMap.load(this, _shaderProgram_bloomMap);
    _shader_blur.load(this, _shaderProgram_blur);
    _shader_bloom.load(this, _shaderProgram_bloom);
    QSh_Color sh_color;
    registerShader(&sh_color);
    QSh_Texture1 sh_texture1;
    registerShader(&sh_texture1);
    QSh_Light sh_light;
    registerShader(&sh_light);
    QSh_ColoredPart sh_coloredPart;
    registerShader(&sh_coloredPart);
    QSh_Refraction_FallOff sh_refraction_FallOff;
    registerShader(&sh_refraction_FallOff);
    QSh_Sphere_Texture1 sh_sphere_texture1;
    registerShader(&sh_sphere_texture1);
    QSh_Sphere_Light sh_sphere_light;
    registerShader(&sh_sphere_light);

    setlocale(LC_ALL, "");
}

void QScrollEngineContext::_addScene(QScene* scene)
{
    unsigned int i, orderIndex = std::numeric_limits<unsigned int>::max(), currentIndex = 0;
    for (i=0; i<_sceneOrderStep.size(); ++i) {
        if (scene->order() <= _scenes[currentIndex]->order()) {
            orderIndex = i;
            break;
        }
        currentIndex += _sceneOrderStep[i];
    }
    if (orderIndex == std::numeric_limits<unsigned int>::max()) {
        _sceneOrderStep.push_back(1);
    } else {
        if (scene->order() == _scenes[currentIndex]->order()) {
            ++_sceneOrderStep[orderIndex];
        } else {
            _sceneOrderStep.insert(_sceneOrderStep.begin() + orderIndex, 1);
        }
    }
    scene->_index = currentIndex;
    _scenes.insert(_scenes.begin() + currentIndex, scene);
    for (unsigned int i=scene->_index+1; i<_scenes.size(); ++i) {
        ++_scenes[i]->_index;
    }
}

void QScrollEngineContext::_deleteScene(QScene* scene)
{
    unsigned int i, currentIndex = 0, index = scene->_index;
    for (i=0; i<_sceneOrderStep.size(); ++i) {
        if (_scenes[index]->order() == _scenes[currentIndex]->order()) {
            --_sceneOrderStep[i];
            if (_sceneOrderStep[i] <= 0)
                _sceneOrderStep.erase(_sceneOrderStep.begin() + i);
            break;
        } else if (_scenes[index]->order() < _scenes[currentIndex]->order())
            break;
        currentIndex += _sceneOrderStep[i];
    }
    for (i=index+1; i<_scenes.size(); ++i)
        _scenes[i]->_index = i - 1;
    _scenes.erase(_scenes.begin() + index);
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
    newTexture = new QOpenGLTexture(QImage(path));
    if (!newTexture->isCreated()) {
        delete newTexture;
        error(QString("QScrollEngine: Texture - '") + name + "' - " + path + " is not created.");
        return nullptr;
    }
    _textures[name] = newTexture;
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
    _textures[name] = newTexture;
    return newTexture;
}

QOpenGLTexture* QScrollEngineContext::loadBWTexture(const QString& name, const QString& path)
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
    newTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    newTexture->create();
    if (!newTexture->isCreated()) {
        error(QString("QScrollEngine: Texture - '") + name + "' - " + path + " is not created.");
        return 0;
    }
    newTexture->bind();
    float* bwImage = new float[image.width() * image.height()];
    int i, j, offset = 0;
    for (j=0; j<image.height(); ++j) {
        for (i=0; i<image.width(); ++i) {
            bwImage[offset] = qGray(image.pixel(i, image.height() - j - 1)) / 255.0f;
            qDebug() << bwImage[offset];
            offset++;
        }
    }
#ifdef GL_RED
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, image.width(), image.height(), 0, GL_RED, GL_FLOAT, bwImage);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_EXT, image.width(), image.height(), 0, GL_RED_EXT, GL_FLOAT, bwImage);
#endif
    delete[] bwImage;
    _textures[name] = newTexture;
    return newTexture;
}

void QScrollEngineContext::addTexture(QOpenGLTexture* texture, const QString& textureName)
{
    QOpenGLTexture* newTexture = QScrollEngineContext::texture(textureName);
    if (newTexture != nullptr) {
        qDebug() << QString("QScrollEngine: Texture with name '") + textureName + "' already exist!";
        return;
    }
    _textures[textureName] = texture;
}

QEntity* QScrollEngineContext::loadEntity(const QString& path, const QString& textureDir, const QString& prefixTextureName)
{
    QEntity* entity = _fileSaveLoad3DS.loadEntity(this, path, textureDir, prefixTextureName);
    if (entity == nullptr)
        error(QString("QScrollEngine: Entity - ") + path + " - is not loaded.");
    return entity;
}

bool QScrollEngineContext::saveEntity(QEntity* entity, const QString& path, const QString& textureDir, const QString& prefixTextureName)
{
    return _fileSaveLoad3DS.saveEntity(this, entity, path, textureDir, prefixTextureName);
}

QOpenGLTexture* QScrollEngineContext::texture(const QString& name)
{
    std::map<QString, QOpenGLTexture*>::iterator it = _textures.find(name);
    if (it != _textures.end())
        return it->second;
    return nullptr;
}

bool QScrollEngineContext::textureName(QString& result, const QOpenGLTexture* texture) const
{
    for (std::map<QString, QOpenGLTexture*>::const_iterator it = _textures.cbegin(); it != _textures.end(); ++it) {
        if (it->second == texture) {
            result = it->first;
            return true;
        }
    }
    return false;
}

void QScrollEngineContext::deleteTexture(const QString& name)
{
    std::map<QString, QOpenGLTexture*>::iterator it = _textures.find(name);
    if (it != _textures.end()) {
        QOpenGLTexture* texture = it->second;
        texture->release();
        texture->destroy();
        delete texture;
        _textures.erase(it);
    }
}

void QScrollEngineContext::deleteTexture(QOpenGLTexture* texture)
{
    for (std::map<QString, QOpenGLTexture*>::iterator it = _textures.begin(); it != _textures.end(); ++it)
        if (it->second == texture) {
            texture->release();
            texture->destroy();
            _textures.erase(it);
            delete texture;
            return;
        }
}

void QScrollEngineContext::deleteAllTextures()
{
    for (std::map<QString, QOpenGLTexture*>::iterator it = _textures.begin(); it != _textures.end(); ++it) {
        QOpenGLTexture* texture = it->second;
        texture->release();
        texture->destroy();
        delete texture;
    }
    _textures.clear();
}

void QScrollEngineContext::initializeContext()
{
    initializeOpenGLFunctions();
    glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    /*// Set nearest filtering mode for texture minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set bilinear filtering mode for texture magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    registerDefaultShaders();
    if (_quad)
        delete _quad;
    _quad = new QMesh(this);
    QGLPrimitiv::createQuad(_quad);
    resolveScreenQuad();
    initObjectsOfPostProcess();

    if (_emptyTexture != nullptr) {
        _emptyTexture->release();
        _emptyTexture->destroy();
        delete _emptyTexture;
    }
    QImage emptyImage = QImage(1, 1, QImage::Format_RGB32);
    emptyImage.setPixel(0, 0, qRgb(255, 255, 255));
    _emptyTexture = new QOpenGLTexture(emptyImage);
    assert(_emptyTexture->isCreated());
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

bool QScrollEngineContext::checkBindShader(QOpenGLShaderProgram* program, QString name)
{
    if (!program->bind()) {
        error(QString("Don't bind shader - " + name));
        return false;
    }
    program->release();
    return true;
}

void QScrollEngineContext::resolveScreenQuad()
{
    const float zNear = 1.0f, zFar = 3.0f, Z = 2.0f;
    _quadFinalMatrix.ortho(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar);
    QMatrix4x4 m = _quadFinalMatrix.inverted();
    QVector2D d(2.0f * (1.0f) - 1.0f, 1.0f - 2.0f * (0.0f));
    QVector3D resultNear = m * QVector3D(d, -1.0f);
    QVector3D resultFar = m * QVector3D(d, 1.0f);
    float t = (Z - zNear) / (zFar - zNear);
    QVector3D result = resultNear + (resultFar - resultNear) * t;
    _quadFinalMatrix.translate(0.0f, 0.0f, -Z);
    _quadFinalMatrix.scale(result.x() * 2.0f, result.y() * 2.0f);
    _shader_bloomMap.setFinalMatrix(_quadFinalMatrix);
    _shader_blur.setFinalMatrix(_quadFinalMatrix);
    _shader_bloom.setFinalMatrix(_quadFinalMatrix);
}

void QScrollEngineContext::initObjectsOfPostProcess()
{
    if (_postEffectUsed) {
        _decreaseSize.setWidth(qRound(_normalSize.width() / _nDecreaseTexture));
        _decreaseSize.setHeight(qRound(_normalSize.height() / _nDecreaseTexture));
        _shader_blur.setSize(_decreaseSize);
        _shader_bloom.setSize(_normalSize);
        if (_normalSize.width() < 10 || _normalSize.height() < 10) {
            _normalSize.setWidth(10);
            _normalSize.setHeight(10);
        }
        _FBOs[0] = new QOpenGLFramebufferObject(_normalSize, QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D);
        _FBOs[1] = new QOpenGLFramebufferObject(_decreaseSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, _FBOs[1]->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        _FBOs[2] = new QOpenGLFramebufferObject(_decreaseSize, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, _FBOs[2]->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void QScrollEngineContext::deleteObjectsOfPostProcess()
{
    if (_FBOs[0]) {
        delete _FBOs[0];
        delete _FBOs[1];
        delete _FBOs[2];
        _FBOs[0] = nullptr;
    }
}

QOpenGLShaderProgram* QScrollEngineContext::shaderProgram(int indexType, int subIndexType)
{
    std::map<int, _Drawing>::iterator it = _drawings.find(indexType);
    if (it == _drawings.end())
        return nullptr;
    return it->second.programs[subIndexType].data();
}

void QScrollEngineContext::resizeContext(int width, int height)
{
    if ((_normalSize.width() == width) && (_normalSize.height() == height))
        return;
    glViewport(0, 0, width, height);
    camera->setResolution(width, height);
    camera->calculateProjectionMatrix();
    _normalSize.setWidth(width);
    _normalSize.setHeight(height);
    resolveScreenQuad();
    deleteObjectsOfPostProcess();
    initObjectsOfPostProcess();
}

void QScrollEngineContext::drawSprite(const QSprite* sprite)
{
    QSh* shaderParameters = sprite->shader();
    if (!shaderParameters)
        return;
    shaderParameters->preprocess();
    QOpenGLShaderProgram* program = _drawings[shaderParameters->currentIndexType()].programs[shaderParameters->subIndexType()].data();
    if (!program->bind())
        return;
    if (!shaderParameters->use(this, program)) {
        program->release();
        return;
    }
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad->_vboIds[0].bufferId());
    glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[1].bufferId());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[2].bufferId());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
    if ((sprite->_scale.x() > 0.0f && sprite->_scale.y() > 0.0f) ||
            (sprite->_scale.x() < 0.0f && sprite->_scale.y() < 0.0f)) {
        glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
    } else {
        glCullFace(GL_CW);
        glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
        glCullFace(GL_CCW);
    }
    program->disableAttributeArray(1);
    program->disableAttributeArray(0);
    program->release();
}

void QScrollEngineContext::drawPartEntity(const QEntity::QPartEntity* part)
{
    QSh* shaderParameters = part->shader();
    if (shaderParameters == nullptr)
        return;
    shaderParameters->preprocess();
    QOpenGLShaderProgram* program = _drawings[shaderParameters->currentIndexType()].programs[shaderParameters->subIndexType()].data();
    if (!program->bind())
        return;
    if (!shaderParameters->use(this, program)) {
        program->release();
        return;
    }
    program->enableAttributeArray(0);
    program->enableAttributeArray(1);
    drawMesh(part->drawMode(), part->mesh(), program);
    program->disableAttributeArray(1);
    program->disableAttributeArray(0);
    program->release();
}

void QScrollEngineContext::drawMesh(GLenum drawMode, const QMesh* mesh, QOpenGLShaderProgram* program)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_vboIds[0].bufferId());
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_vboIds[1].bufferId());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_vboIds[2].bufferId());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
    if (mesh->_enable_vertex_normal) {
        program->enableAttributeArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->_vboIds[3].bufferId());
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glDrawElements(drawMode, mesh->_elements.size(), GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(2);
    } else {
        glDrawElements(drawMode, mesh->_elements.size(), GL_UNSIGNED_INT, nullptr);
    }
}

void QScrollEngineContext::drawEntity(QEntity* entity)
{
    for (std::vector<QEntity::QPartEntity*>::iterator it = entity->_parts.begin(); it != entity->_parts.end(); ++it)
        drawPartEntity(*it);
}

void QScrollEngineContext::drawLines(const QVector2D* lineVertices, unsigned int countVertices, const QColor& color,
                                     const QMatrix4x4& matrixWorldViewProj)
{
    QOpenGLShaderProgram* program = _drawings[QSh::Color].programs[0].data();
    program->bind();
    program->enableAttributeArray(0);
    program->setUniformValue(QSh_Color::locationMatrixWVP, matrixWorldViewProj);
    program->setUniformValue(QSh_Color::locationColor, color);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), lineVertices);
    glDrawArrays(GL_LINES, 0, countVertices);
}

void QScrollEngineContext::_addTempAlphaObject(const TempAlphaObject& object)
{
    if (_tempAlphaObjects.empty()) {
        _tempAlphaObjects.push_back(object);
        return;
    }
    for (std::list<TempAlphaObject>::iterator it = _tempAlphaObjects.begin(); it != _tempAlphaObjects.end(); ++it) {
        if (object.zDistance > it->zDistance) {
            _tempAlphaObjects.insert(it, object);
            return;
        }
    }
    _tempAlphaObjects.push_back(object);
}

void QScrollEngineContext::_sortingTempAlphaObjects()
{
    float minZDis;
    TempAlphaObject temp;
    std::list<TempAlphaObject>::iterator it, c_it, begin = _tempAlphaObjects.begin();

    while (begin != _tempAlphaObjects.end()) {
        it = begin;
        minZDis = it->zDistance;
        c_it = it;
        for (++it; it != _tempAlphaObjects.end(); ++it) {
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
    QEntity::QPartEntity* part;
    QOpenGLShaderProgram* program;
    unsigned int k;
    for (std::map<int, _Drawing>::iterator it = _drawings.begin(); it != _drawings.end(); ++it) {
        _Drawing& drawing = it->second;
        for (k=0; k<drawing.programs.size(); k++) {
            if (!drawing.currentObjects[k].sprites.empty() || !drawing.currentObjects[k].partEntities.empty()) {
                program = drawing.programs[k].data();
                if (program->bind()) {
                    program->enableAttributeArray(0);
                    program->enableAttributeArray(1);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad->_vboIds[0].bufferId());
                    glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[1].bufferId());
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
                    glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[2].bufferId());
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
                    QSprite* sprite;
                    for (std::vector<QSprite*>::iterator it = drawing.currentObjects[k].sprites.begin();
                         it != drawing.currentObjects[k].sprites.end(); ++it) {
                        sprite = *it;
                        shaderOfObject = sprite->shader();
                        if (shaderOfObject->use(this, program)) {
                            if ((sprite->_scale.x() > 0.0f && sprite->_scale.y() > 0.0f) ||
                                    (sprite->_scale.x() < 0.0f && sprite->_scale.y() < 0.0f)) {
                                glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
                            } else {
                                glCullFace(GL_FRONT);
                                glDrawElements(sprite->drawMode(), 6, GL_UNSIGNED_INT, nullptr);
                                glCullFace(GL_BACK);
                            }
                        }
                    }
                    drawing.currentObjects[k].sprites.clear();
                    for (std::vector<QEntity::QPartEntity*>::iterator it = drawing.currentObjects[k].partEntities.begin();
                         it != drawing.currentObjects[k].partEntities.end(); ++it) {
                        part = *it;
                        shaderOfObject = part->shader();
                        if (shaderOfObject->use(this, program)) {
                            drawMesh(part->drawMode(), part->mesh(), program);
                        }
                    }
                    drawing.currentObjects[k].partEntities.clear();
                    program->disableAttributeArray(1);
                    program->disableAttributeArray(0);
                    program->release();
                }
            }
        }
    }
}

void QScrollEngineContext::beginPaint()
{
    if (_postEffectUsed) {
        glViewport(0, 0, _normalSize.width(), _normalSize.height());
        _FBOs[0]->bind();
    }
    if (_enableClearing) {
        glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void QScrollEngineContext::drawScenes()
{
    lock();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    unsigned int i, j, begin = 0, end;
    QAnimation3D::_animationSpeed_global = _animationSpeed;
    for (i=0; i<_sceneOrderStep.size(); ++i) {
        end = begin + _sceneOrderStep[i];
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        for (j=begin; j<end; ++j) {
            _scenes[j]->_update();
            unlock();
            emit _scenes[j]->beginDrawing();
            lock();
        }
        //Draw objects without alpha
        _drawCurrent();
        //Draw objects with alpha
        unlock();
        for (j=begin; j<end; ++j)
            emit _scenes[j]->beginDrawingAlphaObjects();
        lock();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        //glDisable(GL_DEPTH_TEST);
        for (std::list<TempAlphaObject>::iterator it = _tempAlphaObjects.begin(); it != _tempAlphaObjects.end(); ++it)
            it->drawObject->draw(this);
        _tempAlphaObjects.clear();
        unlock();
        for (j=begin; j<end; ++j) {
            emit _scenes[j]->endDrawing();
        }
        lock();
        begin = end;
    }
    unlock();
}

void QScrollEngineContext::endPaint(int defaultFBOId)
{
    if (_postEffectUsed) {
        glFrontFace(GL_CCW);
        _FBOs[1]->bind();
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glViewport(0, 0, _decreaseSize.width(), _decreaseSize.height());
        QOpenGLShaderProgram* program = _shaderProgram_bloomMap[0].data();
        program->bind();
        _shader_bloomMap.use(this, program);
        _shader_bloomMap.bindScreenTexture(this, _FBOs[0]->texture());
        //glGenerateMipmap(GL_TEXTURE_2D);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad->_vboIds[0].bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[1].bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[2].bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        //program->release();

        _FBOs[2]->bind();
        program = _shaderProgram_blur[0].data();
        program->bind();
        _shader_blur.setByX();
        _shader_blur.use(this, program);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad->_vboIds[0].bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[1].bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[2].bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        _shader_blur.bindScreenTexture(this, _FBOs[1]->texture());
        //glGenerateMipmap(GL_TEXTURE_2D);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        _FBOs[1]->bind();
        _shader_blur.setByY();
        _shader_blur.use(this, program);
        _shader_blur.bindScreenTexture(this, _FBOs[2]->texture());
        //glGenerateMipmap(GL_TEXTURE_2D);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        //program->release();
        glViewport(0, 0, _normalSize.width(), _normalSize.height());
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
        program = _shaderProgram_bloom[0].data();
        program->bind();
        _shader_bloom.use(this, program);
        _shader_bloom.bindBloomMapTexture(this, _FBOs[1]->texture());
        _shader_bloom.bindScreenTexture(this, _FBOs[0]->texture());
        //glGenerateMipmap(GL_TEXTURE_2D);
        program->enableAttributeArray(0);
        program->enableAttributeArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad->_vboIds[0].bufferId());
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[1].bufferId());
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, _quad->_vboIds[2].bufferId());
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        program->disableAttributeArray(1);
        program->disableAttributeArray(0);
        program->release();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
}

void QScrollEngineContext::paintContext()
{
    if (_openGLContext == nullptr)
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
