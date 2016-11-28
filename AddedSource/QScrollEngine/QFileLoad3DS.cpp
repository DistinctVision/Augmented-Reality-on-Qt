#include "QScrollEngine/QFileLoad3DS.h"
#include "QScrollEngine/QMesh.h"
#include "QScrollEngine/QEntity.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include <QOpenGLTexture>
#include <QObjectUserData>
#include <QSharedPointer>
#include <utility>
#include <list>
#include <climits>
#include <cassert>

#include <QMatrix4x4>

#include <QDebug>

namespace QScrollEngine {

QEntity* QFileLoad3DS::loadEntity(QScrollEngineContext* context,
                                  const QString& filename, const QString& textureDir, const QString& prefixTextureName)
{
    m_entities.clear();
    m_materials.clear();
    m_tempInfo.clear();
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return nullptr;
    QFileReader reader(&file);
    if (!_readHead(reader))
        return nullptr;
    _readMain(context, reader);
    QEntity* entity = _getFinishEntity(context, textureDir, prefixTextureName);
    m_entities.clear();
    m_tempInfo.clear();
    m_materials.clear();
    file.close();
    return entity;
}

QFileLoad3DS::Chunk QFileLoad3DS::_readChunk(QFileReader& reader)
{
    Chunk chunk;
    reader >> chunk.id;
    reader >> chunk.length;
    chunk.length -= 6;
    return chunk;
}

QString QFileLoad3DS::_readString(QFileReader& reader)
{
    QString str;
    qint8 c;
    int i=0;
    do {
        reader >> c;
        if (c == '\0') {
            break;
        }
        str += QChar(c);
        i++;
    } while (i < 100);
    return str;
}

QColor QFileLoad3DS::_readColor(QFileReader& reader)
{
    QColor color;
    color.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    Chunk chunk;
    chunk = _readChunk(reader);
    switch (chunk.id) {
    case CHUNK_COLOR_F32:
    {
        float r, g, b;
        reader >> r >> g >> b;
        color.setRgbF(r, g, b, 1.0f);
    }
        break;
    case CHUNK_COLOR_U8:
    {
        uchar r, g, b;
        reader >> r >> g >> b;
        color.setRgb(r, g, b, 255);
    }
        break;
    default:
        reader.ignore(chunk.length);
    }
    return color;
}

float QFileLoad3DS::_readPercent(QFileReader& reader)
{
    float percent = 0.0f;
    Chunk chunk;
    chunk = _readChunk(reader);
    switch (chunk.id) {
    case CHUNK_PERCENT_F32:
    {
        reader >> percent;
    }
        break;
    case CHUNK_PERCENT_U8:
    {
        quint16 percentu16;
        reader >> percentu16;
        percent = static_cast<float>(percentu16);
    }
        break;
    default:
        reader.ignore(chunk.length);
    }
    return percent;
}

bool QFileLoad3DS::_readHead(QFileReader& reader)
{
    Chunk chunk = _readChunk(reader);
    return (chunk.id == CHUNK_MAIN3DS);
}

void QFileLoad3DS::_readMain(QScrollEngineContext* context, QFileReader& reader)
{
    Chunk chunk;
    while (!reader.atEnd()) {
        chunk = _readChunk(reader);
        switch (chunk.id) {
        case CHUNK_VERSION:
        {
            qint32 version;
            reader >> version;
        }
            break;
        case CHUNK_EDIT:
        {
        }
            break;
        case CHUNK_EDIT_OBJECT:
        {
            QEntity* entity = _readEntity(context, reader, chunk);
            m_entities.push_back(entity);
        }
            break;
        case CHUNK_EDIT_MATERIAL:
        {
            Material3DS material = _readMaterial(reader, chunk);
            m_materials.push_back(material);
        }
            break;
        case CHUNK_KEYFRAME:
        {
            _readAnimations(reader, chunk);
        }
            break;
        case CHUNK_END:
        {
            return;
        }
        default:
            reader.ignore(chunk.length);
        }
    }
}

QEntity* QFileLoad3DS::_readEntity(QScrollEngineContext* context, QFileReader& reader, Chunk& parentChunk)
{
    QEntity* entity = new QEntity();
    QString name = _readString(reader);
    entity->setName(name);
    quint32 readed = name.length() + 1;
    Chunk chunk;
    while ((!reader.atEnd()) && (readed < parentChunk.length)) {
        chunk = _readChunk(reader);
        readed += 6 + chunk.length;
        switch (chunk.id) {
        case CHUNK_OBJECT_MESH:
        {
            QMesh* mesh = _readMesh(context, reader, chunk);
            entity->addPart(mesh, QShPtr(new QSh_Color()), true);
        }
            break;
        case CHUNK_END:
        {
            return entity;
        }
        default:
            reader.ignore(chunk.length);
        }
    }
    return entity;
}

QMesh* QFileLoad3DS::_readMesh(QScrollEngineContext* context, QFileReader& reader, Chunk& parentChunk)
{
    QMatrix4x4 transform;
    QMesh* mesh = new QMesh(context);
    quint32 readed = 0;
    std::vector<quint32> smoothGroups;
    Chunk chunk;
    while ((!reader.atEnd()) && (readed < parentChunk.length)) {
        chunk = _readChunk(reader);
        readed += 6 + chunk.length;
        switch (chunk.id) {
        case CHUNK_MESH_VERTICES:
        {
            quint16 countVertices;
            reader >> countVertices;
            mesh->setCountVertices(countVertices);
            float x, y, z;
            for (int i=0; i<countVertices; i++) {
                reader >> x >> y >> z;
                mesh->setVertexPosition(i, QVector3D(x, y, z));
            }
        }
            break;
        case CHUNK_MESH_TRIANGLES:
        {
            quint16 countTriangles;
            reader >> countTriangles;
            mesh->setCountTriangles(countTriangles);
            quint16 iv1, iv2, iv3;
            qint16 flag;
            for (int i=0; i<countTriangles; i++) {
                reader >> iv1 >> iv2 >> iv3;
                /*if (iv1 >= mesh->countVertices())
                    iv1 = 0;
                if (iv2 >= mesh->countVertices())
                    iv2 = 0;
                if (iv3 >= mesh->countVertices())
                    iv3 = 0;*/
                // Read but ignore the flags;
                reader >> flag;
                mesh->setTriangle(i, iv1, iv2, iv3);
            }
            readed += ((2 + countTriangles * (6 + 2)) - chunk.length);
        }
            break;
        case CHUNK_MESH_TEXCOORDS:
        {
            quint16 newCountVertices, oldCountVertices = mesh->countVertices();
            mesh->enableVertexAttribute(QSh::VertexAttributes::TextureCoords);
            mesh->textureCoords().resize(oldCountVertices);
            reader >> newCountVertices;
            float u, v;
            int i, count = std::min(newCountVertices, oldCountVertices);
            for (i=0; i<count; i++) {
                reader >> u >> v;
                mesh->setVertexTextureCoord(i, QVector2D(u, 1.0f - v));
            }
            for (i=count; i<newCountVertices; i++) {
                reader >> u >> v;
            }
        }
            break;
        case CHUNK_MESH_MATRIX:
        {
            float t;
            int i, j;
            for (i=0; i<4; ++i) {
                for (j=0; j<3; ++j) {
                    reader >> t;
                    transform(j, i) = t;
                }
            }
        }
            break;
        case CHUNK_MESH_MATERIAL:
        {
            QString* materialName = new QString();
            *materialName = _readString(reader);
            quint16 countTriangles, triangle;
            reader >> countTriangles;
            for (quint16 i=0; i<countTriangles; ++i)
                reader >> triangle;
            mesh->setUserData(0, reinterpret_cast<QObjectUserData*>(materialName));
        }
            break;
        case CHUNK_MESH_SMOOTHING_GROUP:
        {
            quint16 countTriangles = static_cast<quint16>(mesh->countElements());
            quint32 group;
            smoothGroups.resize(countTriangles);
            for (quint16 i=0; i<countTriangles; ++i) {
                reader >> group;
                smoothGroups[i] = group;
            }
        }
            break;
        default:
            reader.ignore(chunk.length);
        }
    }
    bool success;
    transform = transform.inverted(&success);
    if (success) {
        mesh->transformVertices(transform);
    } else {
        assert(false);
    }
    if (smoothGroups.size() >= mesh->countElements()) {
        mesh->updateNormals();
        //applySmoothGroups(mesh, smoothGroups);
    } else if (!smoothGroups.empty()) {
        qDebug() << "smoothGroups is not valid or not founded.";
    }
    mesh->applyChanges();
    mesh->updateLocalBoundingBox();
    /*if (mesh->userData(0) == nullptr) {
        if (!_materials.empty()) {
            QString* materialName = new QString();
            *materialName = _materials[_materials.size()-1].name;
            mesh->setUserData(0, reinterpret_cast<QObjectUserData*>(materialName));
        }
    }*/
    return mesh;
}

QFileLoad3DS::Material3DS QFileLoad3DS::_readMaterial(QFileReader& reader, Chunk& parentChunk)
{
    Material3DS material;
    material.diffuse.setRgbF(1.0f, 1.0f, 1.0f, 1.0f);
    material.shininess = 0.0f;
    material.isLightMaterial = false;
    quint32 readed = 0;
    Chunk chunk;
    while ((!reader.atEnd()) && (readed < parentChunk.length)) {
        chunk = _readChunk(reader);
        readed += 6 + chunk.length;
        switch (chunk.id) {
        case CHUNK_MATERIAL_NAME:
        {
            material.name = _readString(reader);
        }
            break;
        case CHUNK_MATERIAL_DIFFUSE:
        {
            material.diffuse = _readColor(reader);
        }
            break;
        case CHUNK_MATERIAL_SHININESS:
        {
            material.shininess = _readPercent(reader);
            material.isLightMaterial = true;
        }
            break;
        case CHUNK_MATERIAL_COLORMAP:
        {
            readed -= chunk.length;
        }
            break;
        case CHUNK_TEXTURE_FILE:
        {
            material.textureName = _readString(reader);
        }
            break;
        default:
            reader.ignore(chunk.length);
        }
    }
    return material;
}

void QFileLoad3DS::_readAnimations(QFileReader& reader, Chunk& parentChunk)
{
    quint32 readed = 0;
    Chunk chunk;
    while ((!reader.atEnd()) && (readed < parentChunk.length)) {
        chunk = _readChunk(reader);
        readed += 6 + chunk.length;
        switch (chunk.id) {
        case CHUNK_KEYFRAME_NAME:
        {
            qint16 revision;
            reader >> revision;
            QString name = _readString(reader);
            Q_UNUSED(name);
            qint32 animationLength;
            reader >> animationLength;
            revision++;
        }
            break;
        case CHUNK_KEYFRAME_START_END:
        {
            qint32 start, end;
            reader >> start >> end;
            start += 1;
        }
            break;
        case CHUNK_KEYFRAME_CURTIME:
        {
            qint32 a;
            reader >> a;
            ++a;
        }
            break;
        case CHUNK_KEYFRAME_TRACK:
        {
            _readAnimation(reader, chunk);
        }
            break;
        default:
            reader.ignore(chunk.length);
        }
    }
}

void QFileLoad3DS::_readAnimation(QFileReader& reader, Chunk& parentChunk)
{
    TempInfo temp;
    temp.index = -1;
    temp.parentId = -1;
    temp.name.clear();
    temp.id = -1;
    temp.animation = new QAnimation3D();
    quint32 readed = 0;
    Chunk chunk;
    qint16 count;
    while ((!reader.atEnd()) && (readed < parentChunk.length)) {
        chunk = _readChunk(reader);
        readed += 6 + chunk.length;
        switch (chunk.id) {
        case CHUNK_TRACK_BONENAME:
        {
            temp.name = _readString(reader);
            reader.ignore(2*sizeof(qint16));//ignore flags
            reader >> temp.parentId;
        }
            break;
        case CHUNK_TRACK_PIVOTPOINT:
        {
            float x, y, z;
            reader >> x >> y >> z;
            temp.pivot.setX(x); temp.pivot.setY(y); temp.pivot.setZ(z);
        }
            break;
        case CHUNK_TRACK_BONEPOS:
        {
            reader.ignore(sizeof(quint16));//ignore flags
            reader.ignore(sizeof(quint8) * 8);//unknow
            reader >> count;
            reader.ignore(sizeof(qint16));
            float x, y, z;
            QAnimation3D::AnimKeyPosition key;
            for (qint16 i=0; i<count; ++i) {
                reader >> key.time;
                reader.ignore(sizeof(qint32));
                reader >> x >> y >> z;
                key.position.setX(x); key.position.setY(y); key.position.setZ(z);
                temp.animation->addAnimKey(key);
            }
        }
            break;
        case CHUNK_TRACK_BONEROT:
        {
            reader.ignore(sizeof(quint16));//ignore flags
            reader.ignore(sizeof(quint8) * 8);//unknow
            reader >> count;
            reader.ignore(sizeof(qint16));
            float x, y, z, angle;
            QAnimation3D::AnimKeyOrientation key;
            QQuaternion prev, current;
            for (qint16 i=0; i<count; ++i) {
                reader >> key.time;
                reader.ignore(sizeof(qint32));
                reader >> angle >> x >> y >> z;
                current = QQuaternion::fromAxisAndAngle(- x, - y, - z, angle * RADIAN_TO_EULER);
                key.orienation = current * prev;
                prev = key.orienation;
                temp.animation->addAnimKey(key);
            }
        }
            break;
        case CHUNK_TRACK_BONESCL:
        {
            reader.ignore(sizeof(quint16));//ignore flags
            reader.ignore(sizeof(quint8) * 8);//unknow
            reader >> count;
            reader.ignore(sizeof(qint16));
            float x, y, z;
            QAnimation3D::AnimKeyScale key;
            for (qint16 i=0; i<count; ++i) {
                reader >> key.time;
                reader.ignore(sizeof(qint32));
                reader >> x >> y >> z;
                key.scale.setX(x); key.scale.setY(y); key.scale.setZ(z);
                temp.animation->addAnimKey(key);
            }
        }
            break;
        case CHUNK_TRACK_NODE_ID:
        {
            reader >> temp.id;
        }
            break;
        default:
            reader.ignore(chunk.length);
        }
    }
    //delete temp.animation;
    //temp.animation = nullptr;
    m_tempInfo.push_back(temp);
}

QEntity* QFileLoad3DS::_getFinishEntity(QScrollEngineContext* context, const QString& textureDir,
                                        const QString& prefixTextureName)
{
    QOpenGLTexture* texture;
    std::size_t i, j;
    for (i = 0; i < m_entities.size(); ++i) {
        QEntity* entity = m_entities[i];
        for (j = 0; j < entity->countParts(); ++j) {
            QEntity::Part* part = entity->part(j);
            if (part->mesh()->userData(0) != nullptr) {
                QString* materialName = reinterpret_cast<QString*>(part->mesh()->userData(0));
                for (std::size_t k = 0; k < m_materials.size(); ++k) {
                    if (*materialName == m_materials[k].name) {
                        texture = nullptr;
                        QString textureName = prefixTextureName + m_materials[k].textureName;
                        if ((textureDir != "!") && (!m_materials[k].textureName.isEmpty())) {
                            texture = context->texture(textureName);
                            if (texture == nullptr)
                                texture = context->loadTexture(textureName, textureDir + m_materials[k].textureName);
                        }
                        if (texture) {
                            if (m_materials[k].isLightMaterial) {
                                float specularIntensity = (m_materials[k].shininess > 0.0f) ? 1.0f : 0.0f;
                                part->setShader(QShPtr(
                                                    new QSh_Light(texture, m_materials[k].diffuse, specularIntensity,
                                                              m_materials[k].shininess)));
                            } else {
                                part->setShader(QShPtr(new QSh_Texture(texture, m_materials[k].diffuse)));
                            }
                        } else {
                            if (m_materials[k].isLightMaterial) {
                                float specularIntensity = (m_materials[k].shininess > 0.0f) ? 1.0f : 0.0f;
                                part->setShader(QShPtr(
                                                    new QSh_Light(nullptr, m_materials[k].diffuse, specularIntensity,
                                                              m_materials[k].shininess)));
                            } else {
                                static_cast<QSh_Color*>(part->shader().data())->setColor(m_materials[k].diffuse);
                            }
                            if ((!m_materials[k].textureName.isEmpty()) && (textureDir != "!")) {
                                qDebug() << QString("QScrollEngine: Failed to load texture - '") + textureDir +
                                            m_materials[k].textureName + "'";
                            }
                        }
                        break;
                    }
                }
                delete materialName;
                part->mesh()->setUserData(0, nullptr);
            }
        }
        if (!entity->name().isEmpty()) {
            for (j = 0; j < m_tempInfo.size(); ++j) {
                if (m_tempInfo[j].index < 0) {
                    if (m_tempInfo[j].name == entity->name()) {
                        m_tempInfo[j].index = i;
                        for (std::size_t k = 0; k < entity->countParts(); ++k) {
                            QEntity::Part* part = m_entities[i]->part(k);
                            part->mesh()->moveVertices(-m_tempInfo[j].pivot);
                            part->mesh()->applyChanges();
                            part->mesh()->updateLocalBoundingBox();
                        }
                        entity->setPosition(m_tempInfo[j].pivot);
                        QAnimation3D* animation = m_tempInfo[j].animation;
                        if (animation) {
                            if ((animation->countAnimKeysPosition() <= 1) &&
                                    (animation->countAnimKeysOrientation() <= 1) &&
                                    (animation->countAnimKeysScale() <= 1)) {
                                if ((animation->countAnimKeysPosition() == 0) &&
                                        (animation->countAnimKeysOrientation() == 0) &&
                                        (animation->countAnimKeysScale() == 0)) {
                                    entity->setUserData(0, reinterpret_cast<QObjectUserData*>(1));
                                } else {
                                    animation->setAnimationTime(0.0f);
                                    animation->entityToAnimation(entity);
                                }
                                delete animation;
                            } else
                                entity->setAnimation(m_tempInfo[j].animation);
                        }
                    }
                }
            }
        }
    }
    for (i = 0; i < m_tempInfo.size(); ++i) {
        if (m_tempInfo[i].index >= 0) {
            if (m_tempInfo[i].parentId >= 0) {
                for (j = 0; j < m_tempInfo.size(); ++j) {
                    if (m_tempInfo[j].id == m_tempInfo[i].parentId) {
                        if (m_tempInfo[j].index >= 0) {
                            m_entities[m_tempInfo[i].index]->setParentEntity(m_entities[m_tempInfo[j].index]);
                        }
                        break;
                    }
                }
            }
        }
    }
    for (i = 0; i < m_tempInfo.size(); ++i) {
        if (m_tempInfo[i].index >= 0) {
            QEntity* entity = m_entities[m_tempInfo[i].index];
            if (entity->userData(0) == reinterpret_cast<QObjectUserData*>(1)) {
                entity->setUserData(0, nullptr);
                QEntity* parentEntity = entity->parentEntity();
                if ((parentEntity) && (entity->countEntityChilds() == 0) && (entity->name().left(4) == "Mesh")) {
                    for (std::size_t j = 0; j < entity->countParts(); ++j) {
                        QEntity::Part* part = entity->part(j);
                        parentEntity->addPart(part->mesh(), part->shader()->copy(), true);
                    }
                    delete entity;
                    m_entities[m_tempInfo[i].index] = nullptr;
                }
            }
        }
    }
    QEntity* rootEntity = nullptr;
    int countRootEntity = 0;
    for (i = 0; i < m_entities.size(); ++i) {
        if (m_entities[i]) {
            if (m_entities[i]->parentEntity() == nullptr) {
                ++countRootEntity;
                rootEntity = m_entities[i];
            }
        }
    }
    if (countRootEntity == 1) {
        if (rootEntity->animation()) {
            QEntity* newRootEntity = new QEntity();
            rootEntity->setParentEntity(newRootEntity);
            return newRootEntity;
        } else
            return rootEntity;
    } else if (countRootEntity == 0) {
        assert(false);
        for (i=0; i<m_entities.size(); ++i) {
            if (m_entities[i])
                m_entities[i]->setParentEntity(nullptr);
        }
        for (i=0; i<m_entities.size(); ++i) {
            if (m_entities[i])
                delete m_entities[i];
        }
        return nullptr;
    }
    rootEntity = new QEntity();
    for (i = 0; i < m_entities.size(); ++i) {
        if (m_entities[i]) {
            if (m_entities[i]->parentEntity() == nullptr)
                m_entities[i]->setParentEntity(rootEntity);
        }
    }
    return rootEntity;
}

void QFileLoad3DS::applySmoothGroups(QMesh* mesh, const std::vector<quint32>& smoothGroups)
{
    float epsilon = 0.0005f;
    float negEpsilon = 1.0f - epsilon;
    QMESH_ASSERT(mesh->isTriangles());
    mesh->enableVertexAttribute(QSh::VertexAttributes::Normals);
    std::vector<GLuint>& triangles = mesh->elements();
    std::vector<QVector3D>& vertices = mesh->vertices();
    std::vector<QVector2D>& texCoords = mesh->textureCoords();
    std::vector<std::size_t> linkToNewVertices;
    linkToNewVertices.resize(vertices.size(), std::numeric_limits<std::size_t>::max());

    std::list<_Vertex*> newVertices;
    std::vector<_Triangle> newTriangles;
    newTriangles.resize(triangles.size() / 3);

    std::list<_Edge*> newEdges;

    QVector3D deltaVertices;
    QVector2D deltaTexCoords;
    std::size_t i, j, indexA, indexB;
    if (mesh->vertexAttributeIsEnabled(QSh::VertexAttributes::TextureCoords)) {
        if (vertices.size() != texCoords.size())
            texCoords.resize(vertices.size());
        for (i=0; i<vertices.size(); ++i) {
            if (linkToNewVertices[i] != std::numeric_limits<std::size_t>::max())
                continue;
            linkToNewVertices[i] = i;
            for (j=i+1; j<vertices.size(); ++j) {
                deltaVertices = vertices[i] - vertices[j];
                deltaTexCoords = texCoords[i] - texCoords[j];
                if (std::fmod(qAbs(deltaTexCoords.x()), 1.0f) > negEpsilon)
                    deltaTexCoords.setX(0.0f);
                if (std::fmod(qAbs(deltaTexCoords.y()), 1.0f) > negEpsilon)
                    deltaTexCoords.setY(0.0f);
                if ((qAbs(deltaVertices.x()) < epsilon) && (qAbs(deltaVertices.y()) < epsilon) &&
                        (qAbs(deltaVertices.z()) < epsilon) &&
                        (qAbs(deltaTexCoords.x()) < epsilon) && (qAbs(deltaTexCoords.y()) < epsilon)) {
                    linkToNewVertices[j] = i;
                }
            }
        }
    } else {
        for (i=0; i<vertices.size(); ++i) {
            if (linkToNewVertices[i] != std::numeric_limits<std::size_t>::max())
                continue;
            linkToNewVertices[i] = i;
            for (j=i+1; j<vertices.size(); ++j) {
                deltaVertices = vertices[i] - vertices[j];
                if ((qAbs(deltaVertices.x()) < epsilon) && (qAbs(deltaVertices.y()) < epsilon) &&
                        (qAbs(deltaVertices.z()) < epsilon)) {
                    linkToNewVertices[j] = i;
                }
            }
        }
    }
    for (i=0; i<newTriangles.size(); ++i) {
        _Triangle& newTriangle = newTriangles[i];
        GLuint* oldTriangle = &triangles[i * 3];
        oldTriangle[0] = linkToNewVertices[oldTriangle[0]];
        oldTriangle[1] = linkToNewVertices[oldTriangle[1]];
        oldTriangle[2] = linkToNewVertices[oldTriangle[2]];
        for (j=0; j<3; ++j) {
            indexA = oldTriangle[j];
            indexB = oldTriangle[(j + 1) % 3];
            _Edge* edge = _findEdge(newEdges, indexB, indexA);
            if ((edge == nullptr) || (!_canJoin(smoothGroups, edge->triangles[0], i))) {
                edge = new _Edge();
                edge->triangles[0] = i;
                edge->indexJointOfTriangle[0] = j;
                edge->triangles[1] = std::numeric_limits<std::size_t>::max();
                _Vertex* newVertexA;
                if (j > 0) {
                    _Edge* prevEdge = newTriangle.edges[(j + 2) % 3];
                    newVertexA = (prevEdge->triangles[0] == i) ? prevEdge->vertices[1] : prevEdge->vertices[0];
                } else {
                    newVertexA = new _Vertex();
                    newVertexA->data->oldVertex = indexA;
                    newVertices.push_back(newVertexA);
                }
                _addEdgeToList(newVertexA->data->edges, edge);
                _Vertex* newVertexB;
                if (j > 1) {
                    _Edge* nextEdge = newTriangle.edges[(j + 1) % 3];
                    newVertexB = (nextEdge->triangles[0] == i) ? nextEdge->vertices[0] : nextEdge->vertices[1];
                } else {
                    newVertexB = new _Vertex();
                    newVertexB->data->oldVertex = indexB;
                    newVertices.push_back(newVertexB);
                }
                _addEdgeToList(newVertexB->data->edges, edge);
                //assert(newVertexA->data->oldVertex == indexA);
                //assert(newVertexB->data->oldVertex == indexB);
                edge->vertices[0] = newVertexA;
                edge->vertices[1] = newVertexB;
                newEdges.push_back(edge);
            } else {
                //assert(edge->triangles[1] == std::numeric_limits<std::size_t>::max());
                edge->triangles[1] = i;
                edge->indexJointOfTriangle[1] = j;
                _Vertex* newVertexA;
                if (j > 0) {
                    _Edge* prevEdge = newTriangle.edges[(j + 2) % 3];
                    _Vertex* prevVertex = ((prevEdge->triangles[0] == i) ? prevEdge->vertices[1] : prevEdge->vertices[0]);
                    newVertexA = _weldVertices(edge->vertices[1], prevVertex);
                } else {
                    newVertexA = edge->vertices[1];
                }
                _addEdgeToList(newVertexA->data->edges, edge);
                _Vertex* newVertexB;
                if (j > 1) {
                    _Edge* nextEdge = newTriangle.edges[(j + 1) % 3];
                    _Vertex* nextVertex = (nextEdge->triangles[0] == i) ? nextEdge->vertices[0] : nextEdge->vertices[1];
                    newVertexB = _weldVertices(edge->vertices[0], nextVertex);
                } else {
                    newVertexB = edge->vertices[0];
                }
                _addEdgeToList(newVertexB->data->edges, edge);
                //assert(newVertexA->data->oldVertex == indexA);
                //assert(newVertexB->data->oldVertex == indexB);
            }
            newTriangle.edges[j] = edge;
        }
        /*assert((newTriangle.edges[0]->triangles[0] == i) || (newTriangle.edges[0]->triangles[1] == i));
        assert((newTriangle.edges[1]->triangles[0] == i) || (newTriangle.edges[1]->triangles[1] == i));
        assert((newTriangle.edges[2]->triangles[0] == i) || (newTriangle.edges[2]->triangles[1] == i));
        assert((newTriangle.edges[0]->vertices[(newTriangle.edges[0]->triangles[0] == i)]->data) ==
                (newTriangle.edges[1]->vertices[(newTriangle.edges[1]->triangles[1] == i)]->data));
        assert((newTriangle.edges[1]->vertices[(newTriangle.edges[1]->triangles[0] == i)]->data) ==
                (newTriangle.edges[2]->vertices[(newTriangle.edges[2]->triangles[1] == i)]->data));
        assert((newTriangle.edges[2]->vertices[(newTriangle.edges[2]->triangles[0] == i)]->data) ==
                (newTriangle.edges[0]->vertices[(newTriangle.edges[0]->triangles[1] == i)]->data));*/
    }
    std::vector<QVector3D> resultVertices;
    std::vector<QVector2D> resultTexCoords;
    resultVertices.reserve(vertices.size());
    if (mesh->vertexAttributeIsEnabled(QSh::VertexAttributes::TextureCoords)) {
        for (std::list<_Vertex*>::iterator it = newVertices.begin(); it != newVertices.end(); ++it) {
            _Vertex* vertex = *it;
            if (vertex->data->edges.empty()) {
                vertex->data->index = std::numeric_limits<std::size_t>::max();
            } else {
                vertex->data->index = resultVertices.size();
                resultVertices.push_back(vertices[vertex->data->oldVertex]);
                resultTexCoords.push_back(texCoords[vertex->data->oldVertex]);
            }
        }
    } else {
        for (std::list<_Vertex*>::iterator it = newVertices.begin(); it != newVertices.end(); ++it) {
            _Vertex* vertex = *it;
            if (vertex->data->edges.empty()) {
                vertex->data->index = std::numeric_limits<std::size_t>::max();
            } else {
                vertex->data->index = resultVertices.size();
                resultVertices.push_back(vertices[vertex->data->oldVertex]);
            }
        }
    }
    for (i=0; i<newTriangles.size(); ++i) {
        _Triangle& newTriangle = newTriangles[i];
        _Vertex* vertex1 = (newTriangle.edges[0]->triangles[0] == i) ?
                    newTriangle.edges[0]->vertices[0] : newTriangle.edges[0]->vertices[1];
        _Vertex* vertex2 = (newTriangle.edges[1]->triangles[0] == i) ?
                    newTriangle.edges[1]->vertices[0] : newTriangle.edges[1]->vertices[1];
        _Vertex* vertex3 = (newTriangle.edges[2]->triangles[0] == i) ?
                    newTriangle.edges[2]->vertices[0] : newTriangle.edges[2]->vertices[1];
        //assert(vertex1->data->index != std::numeric_limits<std::size_t>::max());
        //assert(vertex2->data->index != std::numeric_limits<std::size_t>::max());
        //assert(vertex3->data->index != std::numeric_limits<std::size_t>::max());
        GLuint* triangle = &triangles[i * 3];
        triangle[0] = vertex1->data->index;
        triangle[1] = vertex2->data->index;
        triangle[2] = vertex3->data->index;
    }
    mesh->vertices() = std::move(resultVertices);
    mesh->textureCoords() = std::move(resultTexCoords);
    for (std::list<_Vertex*>::iterator it = newVertices.begin(); it != newVertices.end(); ++it)
        delete *it;
    for (std::list<_Edge*>::iterator it = newEdges.begin(); it != newEdges.end(); ++it)
        delete *it;
    mesh->updateNormals();
}

}
