#include "QScrollEngine/QFileSaveLoad3DS.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/QEntity.h"
#include <functional>
#include <algorithm>

namespace QScrollEngine {

bool QFileSaveLoad3DS::saveEntity(QScrollEngineContext* context, QEntity* entity, const QString& fileName,
                                  const QString& textureDir, const QString& prefixTextureName)
{
    m_saveTexture = (textureDir != "!");
    m_entities.clear();
    m_textureInfo.clear();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << QString("QScrollEngine: Error. Failed to write file '") + fileName + "'.";
        return false;
    }
    _entitiesToVector(entity);
    ChunkData chunkData;
    chunkData.id = CHUNK_EDIT;
    chunkData.data.clear();
    _writeEntities(context, chunkData, prefixTextureName);
    chunkData.length = 6 + chunkData.data.size();
    QFileData data;
    data.clear();
    data << static_cast<quint16>(CHUNK_MAIN3DS) << static_cast<quint32>(6 + 6 + 4 + chunkData.length) <<
            static_cast<quint16>(CHUNK_VERSION) << static_cast<quint32>(6 + 4) << static_cast<qint32>(3) <<
            chunkData.id << chunkData.length << chunkData.data;
    //data << static_cast<quint16>(CHUNK_END) << static_cast<quint32>(6);
    data.writeToFile(file);
    _writeTextures(context, textureDir);
    m_entities.clear();
    m_textureInfo.clear();
    file.close();
    return true;
}

void QFileSaveLoad3DS::_entitiesToVector(QEntity* entity)
{
    m_entities.push_back(entity);
    for (unsigned int i=0; i<entity->countEntityChilds(); ++i)
        _entitiesToVector(entity->childEntity(i));
}

int QFileSaveLoad3DS::_findEntityInVector(QEntity* entity)
{
    for (unsigned int i=0; i<m_entities.size(); ++i) {
        if (m_entities[i] == entity)
            return static_cast<int>(i);
    }
    return -1;
}

void QFileSaveLoad3DS::_write(QFileData& data, const QString& string)
{
    for (int i=0; i<string.size(); ++i)
        data << string[i].toLatin1();
    data << '\0';
}

int QFileSaveLoad3DS::_findTexture(const QString& name)
{
    for (unsigned int i=0; i<m_textureInfo.size(); ++i) {
        if (m_textureInfo[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

int QFileSaveLoad3DS::_findTexture(QOpenGLTexture* texture)
{
    for (unsigned int i=0; i<m_textureInfo.size(); ++i) {
        if (m_textureInfo[i].texture == texture)
            return static_cast<int>(i);
    }
    return -1;
}

void QFileSaveLoad3DS::_checkTextureName(QString& name, const QString& prefixTextureName)
{
    if (name.left(prefixTextureName.size()) == prefixTextureName)
        name = name.right(name.size() - prefixTextureName.size());
    if (!m_saveTexture)
        return;
    if ((name.right(4) != ".jpg") &&
        (name.right(4) != ".JPG") &&
        (name.right(4) != ".bmp") &&
        (name.right(4) != ".BMP") &&
        (name.right(4) != ".png")) {
        name += ".jpg";
    }
    name = name.right(12);
}

QString QFileSaveLoad3DS::_writeMaterial(QScrollEngineContext* context, ChunkData& chunkData,
                                         const QString& prefixTextureName, QSh* shader)
{
    QString materialName = "";
    ChunkData chunkDataMaterial, chunkDataTemp;
    chunkDataMaterial.id = CHUNK_EDIT_MATERIAL;
    chunkDataMaterial.data.clear();
    if (shader->typeIndex() == static_cast<int>(QSh::Type::Color)) {
        QSh_Color* shaderColor = static_cast<QSh_Color*>(shader);
        chunkDataTemp.id = CHUNK_MATERIAL_NAME;
        chunkDataTemp.data.clear();
        materialName = QString("ShaderColor") + QString::number(m_currentIdShader);
        ++m_currentIdShader;
        _write(chunkDataTemp.data, materialName);
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        chunkDataTemp.id = CHUNK_MATERIAL_DIFFUSE;
        chunkDataTemp.data.clear();
        QColor color = shaderColor->color();
        float r = color.redF();
        float g = color.greenF();
        float b = color.blueF();
        chunkDataTemp.data << static_cast<quint16>(CHUNK_COLOR_F32) << static_cast<quint32>(6 + 3 * 4) << r << g << b;
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
    } else if (shader->typeIndex() == static_cast<int>(QSh::Type::Texture)) {
        QSh_Texture* shaderTexture1 = dynamic_cast<QSh_Texture*>(shader);
        int findedTextureIndex = -1;
        if (shaderTexture1->texture()) {
            findedTextureIndex = _findTexture(shaderTexture1->texture());
            if (findedTextureIndex < 0) {
                TextureInfo ti;
                ti.texture = shaderTexture1->texture();
                if (!context->textureName(ti.name, ti.texture)) {
                    ti.name = QString("Texture") + QString::number(m_currentIdTexture);
                    ++m_currentIdTexture;
                }
                findedTextureIndex = static_cast<int>(m_textureInfo.size());
                _checkTextureName(ti.name, prefixTextureName);
                m_textureInfo.push_back(ti);
            }
        }
        chunkDataTemp.id = CHUNK_MATERIAL_NAME;
        chunkDataTemp.data.clear();
        materialName = QString("ShaderTexture") + QString::number(m_currentIdShader);
        ++m_currentIdShader;
        _write(chunkDataTemp.data, materialName);
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        chunkDataTemp.id = CHUNK_MATERIAL_DIFFUSE;
        chunkDataTemp.data.clear();
        QColor color = shaderTexture1->color();
        float r = color.redF();
        float g = color.greenF();
        float b = color.blueF();
        chunkDataTemp.data << static_cast<quint16>(CHUNK_COLOR_F32) << static_cast<quint32>(6 + 3 * 4) << r << g << b;
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        if (findedTextureIndex >= 0) {
            chunkDataTemp.id = CHUNK_TEXTURE_FILE;
            chunkDataTemp.data.clear();
            _write(chunkDataTemp.data, m_textureInfo[findedTextureIndex].name);
            chunkDataTemp.length = 6 + chunkDataTemp.data.size();
            chunkDataMaterial.data << static_cast<quint16>(CHUNK_MATERIAL_COLORMAP) <<
                                      static_cast<quint32>(chunkDataTemp.length + 6) <<
                                      chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        }
    } else if (shader->typeIndex() == static_cast<int>(QSh::Type::Light)) {
        QSh_Light* shaderLight = dynamic_cast<QSh_Light*>(shader);
        int findedTextureIndex = -1;
        if (shaderLight->texture()) {
            findedTextureIndex = _findTexture(shaderLight->texture());
            if (findedTextureIndex < 0) {
                TextureInfo ti;
                ti.texture = shaderLight->texture();
                if (!context->textureName(ti.name, ti.texture)) {
                    ti.name = QString("Texture") + QString::number(m_currentIdTexture);
                    ++m_currentIdTexture;
                }
                findedTextureIndex = static_cast<int>(m_textureInfo.size());
                _checkTextureName(ti.name, prefixTextureName);
                m_textureInfo.push_back(ti);
            }
        }
        chunkDataTemp.id = CHUNK_MATERIAL_NAME;
        chunkDataTemp.data.clear();
        materialName = QString("ShaderLight") + QString::number(m_currentIdShader);
        ++m_currentIdShader;
        _write(chunkDataTemp.data, materialName);
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        chunkDataTemp.id = CHUNK_MATERIAL_DIFFUSE;
        chunkDataTemp.data.clear();
        QColor color = shaderLight->color();
        float r = color.redF();
        float g = color.greenF();
        float b = color.blueF();
        chunkDataTemp.data << static_cast<quint16>(CHUNK_COLOR_F32) << static_cast<quint32>(6 + 3 * 4) << r << g << b;
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMaterial.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        if (findedTextureIndex >= 0) {
            chunkDataTemp.id = CHUNK_TEXTURE_FILE;
            chunkDataTemp.data.clear();
            _write(chunkDataTemp.data, m_textureInfo[findedTextureIndex].name);
            chunkDataTemp.length = 6 + chunkDataTemp.data.size();
            chunkDataMaterial.data << static_cast<quint16>(CHUNK_MATERIAL_COLORMAP) <<
                                      static_cast<quint32>(chunkDataTemp.length + 6) <<
                                      chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        }
        chunkDataMaterial.data << static_cast<quint16>(CHUNK_MATERIAL_SHININESS) << static_cast<quint32>(6 + 6 + 4) <<
                                  static_cast<quint16>(CHUNK_PERCENT_F32) << static_cast<quint32>(6 + 4) <<
                                  static_cast<float>((shaderLight->specularIntensity() > 0.0f) ? shaderLight->specularPower() : 0.0f);
    }
    chunkDataMaterial.length = 6 + chunkDataMaterial.data.size();
    chunkData.data << chunkDataMaterial.id << chunkDataMaterial.length << chunkDataMaterial.data;
    return materialName;
}

void QFileSaveLoad3DS::_writeMesh(ChunkData& chunkData, QMesh* mesh, const QString& name, const QString& materialName)
{
    if (!mesh->isTriangles()) {
        qDebug() << "QScrollEngine: Mesh is not triangles mesh. Not saved.";
        return;
    }
    ChunkData chunkDataMesh;
    chunkDataMesh.id = CHUNK_OBJECT_MESH;
    chunkDataMesh.data.clear();
    if (mesh == nullptr) {
        chunkDataMesh.data << static_cast<quint16>(CHUNK_MESH_VERTICES) << static_cast<quint32>(6 + 2) << static_cast<quint16>(0);
        if (!materialName.isEmpty()) {
            chunkDataMesh.data << static_cast<quint16>(CHUNK_MESH_TRIANGLES) <<
                                  static_cast<quint32>(6 + 2 + (6 + materialName.size() + 1 + 2)) << static_cast<quint16>(0) <<
                                  static_cast<quint16>(CHUNK_MESH_MATERIAL) <<
                                  static_cast<quint32>(6 + materialName.size() + 1 + 2);
            _write(chunkDataMesh.data, materialName);
            chunkDataMesh.data << static_cast<quint16>(0);
        }
    } else {
        ChunkData chunkDataTemp;
        chunkDataTemp.id = CHUNK_MESH_VERTICES;
        chunkDataTemp.data.clear();
        quint16 count = static_cast<quint16>(mesh->countVertices());
        chunkDataTemp.data << static_cast<quint16>(count);
        quint16 j;
        for (j=0; j<count; ++j) {
            QVector3D v = mesh->vertexPosition(j);
            chunkDataTemp.data << (v.x()) << (v.y()) << (v.z());
        }
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMesh.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        chunkDataTemp.id = CHUNK_MESH_TEXCOORDS;
        chunkDataTemp.data.clear();
        chunkDataTemp.data << static_cast<quint16>(count);
        for (j=0; j<count; ++j) {
            QVector2D v = mesh->vertexTextureCoord(j);
            chunkDataTemp.data << (v.x()) << (1.0f - v.y());
        }
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMesh.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
        chunkDataTemp.data.clear();
        chunkDataTemp.id = CHUNK_MESH_TRIANGLES;
        count = static_cast<quint16>(mesh->countTriangles());
        chunkDataTemp.data << count;
        GLuint* triangle;
        quint16 iv1, iv2, iv3;
        qint16 flag = 0;
        for (j=0; j<count; ++j) {
            triangle = mesh->triangle(j);
            iv1 = triangle[0];
            iv2 = triangle[1];
            iv3 = triangle[2];
            chunkDataTemp.data << iv1 << iv2 << iv3 << flag;
        }
        if (!materialName.isEmpty()) {
            quint16 id = CHUNK_MESH_MATERIAL;
            quint32 size = (6) + (materialName.size() + 1) + (2 + count * 2);
            chunkDataTemp.data << id << size;
            _write(chunkDataTemp.data, materialName);
            chunkDataTemp.data << count;
            for (j=0; j<count; ++j)
                chunkDataTemp.data << static_cast<quint16>(j);
        }
        chunkDataTemp.length = 6 + chunkDataTemp.data.size();
        chunkDataMesh.data << chunkDataTemp.id << chunkDataTemp.length << chunkDataTemp.data;
    }
    chunkDataMesh.length = 6 + chunkDataMesh.data.size();
    chunkData.data << static_cast<quint16>(CHUNK_EDIT_OBJECT) << static_cast<quint32>(6 + name.size() + 1 + chunkDataMesh.length);
    _write(chunkData.data, name);
    chunkData.data << chunkDataMesh.id << chunkDataMesh.length << chunkDataMesh.data;

}

void QFileSaveLoad3DS::_writeAnimation(ChunkData& chunkData, int indexEntity, const QString& name)
{
    QEntity* entity = m_entities[indexEntity];
    QAnimation3D* animation = entity->animation();
    ChunkData chunkDataAnimation;
    chunkDataAnimation.id = CHUNK_KEYFRAME_TRACK;
    chunkDataAnimation.data.clear();
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_NODE_ID) << static_cast<quint32>(6 + 2) <<
                               static_cast<qint16>(indexEntity);
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONENAME) << static_cast<quint32>(6 + (name.size() + 1) + 4 + 2);
    _write(chunkDataAnimation.data, name);
    chunkDataAnimation.data << static_cast<qint32>(16384);
    QEntity* parentEntity = entity->parentEntity();
    if (parentEntity == nullptr)
        chunkDataAnimation.data << static_cast<qint16>(-1);
    else
        chunkDataAnimation.data << static_cast<qint16>(_findEntityInVector(parentEntity));
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_PIVOTPOINT) << static_cast<quint32>(6 + 4 * 3) <<
                               static_cast<float>(0.0f) << static_cast<float>(0.0f) << static_cast<float>(0.0f);
    float angle;
    QVector3D axis;
    QQuaternion prev, turn;
    qint16 count = 1;
    if (animation == nullptr) {
        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONEPOS) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 3 * 4));
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        QVector3D p = entity->position();
        chunkDataAnimation.data << static_cast<qint16>(0) << static_cast<qint32>(0) << (p.x()) << (p.y()) << (p.z());

        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONEROT) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 4 * 4));
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        QOtherMathFunctions::quaternionToAngleAxis(entity->orientation(), angle, axis);
        chunkDataAnimation.data << static_cast<qint16>(0) << static_cast<qint32>(0) << (angle) << (- axis.x()) << (- axis.y()) << (- axis.z());

        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONESCL) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 3 * 4));
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        p = entity->scale();
        chunkDataAnimation.data << static_cast<qint16>(0) << static_cast<qint32>(0) << (p.x()) << (p.z()) << (p.y());
    } else {
        count = animation->countAnimKeysPosition();
        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONEPOS) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 3 * 4) * count);
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(50*0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        qint16 i;
        for (i=0; i<count; ++i) {
            QAnimation3D::AnimKeyPosition key = animation->animKeyPosition(i);
            chunkDataAnimation.data << static_cast<qint16>(key.time) << static_cast<qint32>(0) <<
                                       key.position.x() << key.position.y() << key.position.z();
        }

        count = animation->countAnimKeysOrientation();
        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONEROT) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 4 * 4) * count);
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(41*0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        for (i=0; i<count; ++i) {
            QAnimation3D::AnimKeyOrientation key = animation->animKeyOrientation(i);
            turn = key.orienation * prev.conjugate();
            prev = key.orienation;
            QOtherMathFunctions::quaternionToAngleAxis(turn, angle, axis);
            chunkDataAnimation.data << static_cast<qint16>(key.time) << static_cast<qint32>(0) <<
                                       (angle) << (- axis.x()) << (- axis.y()) << (- axis.z());
        }

        count = animation->countAnimKeysScale();
        chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONESCL) << static_cast<quint32>(6 + 2 + 8 + 2 + 2 +
                                                                                                     (2 + 4 + 3 * 4) * count);
        chunkDataAnimation.data << static_cast<quint16>(0);
        chunkDataAnimation.data << static_cast<quint32>(0) << static_cast<quint32>(50*0);
        chunkDataAnimation.data << count << static_cast<qint16>(0);
        for (i=0; i<count; ++i) {
            QAnimation3D::AnimKeyScale key = animation->animKeyScale(i);
            chunkDataAnimation.data << static_cast<qint16>(key.time) << static_cast<qint32>(0) <<
                                       (key.scale.x()) << (key.scale.y()) << (key.scale.z());
        }
    }
    chunkDataAnimation.length = 6 + chunkDataAnimation.data.size();
    chunkData.data << static_cast<quint16>(CHUNK_KEYFRAME) << static_cast<quint32>(6 + chunkDataAnimation.length);
    chunkData.data << chunkDataAnimation.id << chunkDataAnimation.length << chunkDataAnimation.data;
}

void QFileSaveLoad3DS::_writeMeshAnimation(ChunkData& chunkData, QString& name, int meshId, int parentEntityId)
{
    ChunkData chunkDataAnimation;
    chunkDataAnimation.id = CHUNK_KEYFRAME_TRACK;
    chunkDataAnimation.data.clear();
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_NODE_ID) << static_cast<quint32>(6 + 2) <<
                               static_cast<qint16>(meshId);
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_BONENAME) << static_cast<quint32>(6 + (name.size() + 1) + 4 + 2);
    _write(chunkDataAnimation.data, name);
    chunkDataAnimation.data << static_cast<qint32>(16384);
    chunkDataAnimation.data << static_cast<qint16>(parentEntityId);
    chunkDataAnimation.data << static_cast<quint16>(CHUNK_TRACK_PIVOTPOINT) << static_cast<quint32>(6 + 4 * 3) <<
                               static_cast<float>(0.0f) << static_cast<float>(0.0f) << static_cast<float>(0.0f);
    chunkDataAnimation.length = 6 + chunkDataAnimation.data.size();
    chunkData.data << static_cast<quint16>(CHUNK_KEYFRAME) << static_cast<quint32>(6 + chunkDataAnimation.length);
    chunkData.data << chunkDataAnimation.id << chunkDataAnimation.length << chunkDataAnimation.data;
}

void QFileSaveLoad3DS::_writeEntities(QScrollEngineContext* context, ChunkData& chunkData, const QString& prefixTextureName)
{
    std::vector<QString> usedNames;
    QString entityName, materialName, meshName;
    m_currentIdTexture = m_currentIdShader = 0;
    int currentEntityId = 0;
    int currentMeshId = 0;
    unsigned int i;
    for (i=0; i<m_entities.size(); ++i) {
        QEntity* entity = m_entities[i];
        entityName = entity->name().left(10);
        if (entityName.isEmpty()) {
            entityName = "Entity" + QString::number(currentEntityId);
            ++currentEntityId;
        }
        while (std::find(usedNames.begin(), usedNames.end(), entityName) != usedNames.end()) {
            entityName = "Entity" + QString::number(currentEntityId);
            ++currentEntityId;
        }
        usedNames.push_back(entityName);
        if (entity->countParts() == 0) {
            _writeMesh(chunkData, nullptr, entityName, "");
            if (entity->parentEntity() == nullptr)
                _writeMeshAnimation(chunkData, entityName, static_cast<int>(i), -1);
            else
                _writeMeshAnimation(chunkData, entityName, static_cast<int>(i), _findEntityInVector(entity->parentEntity()));
            continue;
        }
        {
            QEntity::Part* part = entity->part(0);
            if (part->shader())
                materialName = _writeMaterial(context, chunkData, prefixTextureName, part->shader().data());
            else
                materialName = "";
            _writeMesh(chunkData, part->mesh(), entityName, materialName);
            _writeAnimation(chunkData, static_cast<int>(i), entityName);
        }
        for (unsigned int j=1; j<entity->countParts(); ++j) {
            QEntity::Part* part = entity->part(j);
            if (part->shader())
                materialName = _writeMaterial(context, chunkData, prefixTextureName, part->shader().data());
            else
                materialName = "";
            meshName = "Mesh" + QString::number(currentMeshId);
            ++currentMeshId;
            while (std::find(usedNames.begin(), usedNames.end(), meshName) != usedNames.end()) {
                meshName = "Mesh" + QString::number(currentMeshId);
                ++currentMeshId;
            }
            _writeMesh(chunkData, part->mesh(), meshName, materialName);
            _writeMeshAnimation(chunkData, meshName, static_cast<int>(m_entities.size() + currentMeshId), static_cast<int>(i));
        }
    }
}

void QFileSaveLoad3DS::_writeTextures(QScrollEngineContext* context, const QString& textureDir)
{
    if (!m_saveTexture)
        return;
    GLint defaultFBOId = 0;
    context->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBOId);
    QImage image;
    uchar* data;
    GLuint frameBuffer = 0;
    context->glGenFramebuffers(1, &frameBuffer);
    context->glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    if (frameBuffer == 0) {
        qDebug() << QString("QScrollEngine: Error. Failed to create frmae buffer for saving textures.");
        context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
        return;
    }
    const int maxMemSize = 1024 * 1024 * 4;
    data = new uchar[maxMemSize];
    for (unsigned int i=0; i<m_textureInfo.size(); ++i) {
        QOpenGLTexture* texture = m_textureInfo[i].texture;
        if ((texture->width() * texture->height() * 4) > maxMemSize) {
            qDebug() << QString("QScrollEngine: Error. Too big texture - '") + m_textureInfo[i].name + "'.";
            continue;
        }
        context->glBindTexture(GL_TEXTURE_2D, texture->textureId());
        context->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->textureId(), 0);
        if (context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            qDebug() << QString("QScrollEngine: Error. Failed to use frame buffer for texture - '") + m_textureInfo[i].name + "'.";
            continue;
        }
        context->glReadPixels(0, 0, texture->width(), texture->height(), GL_RGBA, GL_UNSIGNED_BYTE, data);
        image = QImage(data, texture->width(), texture->height(), QImage::Format_RGBA8888);
        if (!image.save(textureDir + m_textureInfo[i].name)) {
            qDebug() << QString("QStrollEngine: Error. Failed to save the texture - '") + m_textureInfo[i].name +
                        "' to path - '" + textureDir + m_textureInfo[i].name + "'.";
        }
    }
    delete[] data;
    context->glDeleteFramebuffers(1, &frameBuffer);
    context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
}

}
