#ifndef QFILELOAD3DS_H
#define QFILELOAD3DS_H

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QColor>
#include <vector>
#include <cassert>
#include <memory>
#include "QScrollEngine/QAnimation3D.h"
#include "QScrollEngine/QMesh.h"

#define   BitIsSet(reg, bit)       ((reg & (1<<bit)) != 0)

namespace QScrollEngine {

class QEntity;
class QScrollEngineContext;

class QFileLoad3DS
{

public:
    QEntity* loadEntity(QScrollEngineContext* context, const QString& filename,
                        const QString& textureDir = "!", const QString& prefixTextureName = "");

protected:
    enum EMesh3DSChunks
    {
        CHUNK_MAIN3DS       = 0x4D4D,
            CHUNK_VERSION           = 0x0002,

            CHUNK_COLOR_F32         = 0x0010,
            CHUNK_COLOR_U8          = 0x0011,
            CHUNK_PERCENT_U8        = 0x0030,
            CHUNK_PERCENT_F32       = 0x0031,

            CHUNK_EDIT              = 0x3D3D,
                CHUNK_EDIT_OBJECT       = 0x4000,
                    CHUNK_OBJECT_MESH       = 0x4100,
                        CHUNK_MESH_VERTICES     = 0x4110,
                        CHUNK_MESH_TRIANGLES    = 0x4120,
                            CHUNK_MESH_MATERIAL     = 0x4130,
                            CHUNK_MESH_SMOOTHING_GROUP = 0x4150,
                        CHUNK_MESH_TEXCOORDS    = 0x4140,
                        CHUNK_MESH_MATRIX       = 0x4160,

                CHUNK_EDIT_MATERIAL      = 0xAFFF,
                    CHUNK_MATERIAL_NAME         = 0xA000,
                    CHUNK_MATERIAL_DIFFUSE      = 0xA020,
                    CHUNK_MATERIAL_SHININESS      = 0xA040,
                    CHUNK_MATERIAL_COLORMAP     = 0xA200,
                        CHUNK_TEXTURE_FILE          = 0xA300,

                CHUNK_KEYFRAME          = 0xB000,
                    CHUNK_KEYFRAME_NAME = 0xB00A,
                    CHUNK_KEYFRAME_START_END = 0xB008,
                    CHUNK_KEYFRAME_CURTIME  = 0xB009,
                    CHUNK_KEYFRAME_TRACK    = 0xB002,
                        CHUNK_TRACK_BONENAME    = 0xB010,
                        CHUNK_TRACK_PIVOTPOINT  = 0xB013,
                        CHUNK_TRACK_BOUNDBOX    = 0xB014,   //not used
                        CHUNK_TRACK_BONEPOS     = 0xB020,
                        CHUNK_TRACK_BONEROT     = 0xB021,
                        CHUNK_TRACK_BONESCL     = 0xB022,
                        CHUNK_TRACK_NODE_ID     = 0xB030,
        CHUNK_END       = 0xFFFF

    };

    typedef struct
    {
        QString name;
        QColor diffuse;
        float shininess;
        bool isLightMaterial;
        QString textureName;
    } Material3DS;

    typedef struct
    {
        qint16 index;
        qint16 id;
        qint16 parentId;
        QString name;
        QAnimation3D* animation;
        QVector3D pivot;
    } TempInfo;

    class QFileReader
    {
    private:
        QByteArray _data;
        int _pos;

    public:
        QFileReader(QFile* file)
        {
            _data = file->readAll();
            _pos = 0;
        }
        bool atEnd() const
        {
            return (_pos >= _data.size());
        }
        void toBegin()
        {
            _pos = 0;
        }
        int pos() const { return _pos; }
        void setPos(int pos) { _pos = pos; }
        char readNext()
        {
            if (atEnd())
                return 0;
            ++_pos;
            return _data[_pos - 1];
        }
        void ignore(int size)
        {
            _pos = std::min(_pos + size, _data.size());
        }
        template<typename Type>
        QFileReader& operator >> (Type& p)
        {
            int size = sizeof(Type);
            char* a = new char[size];
            for (int i=0; i<size; ++i)
                a[i] = readNext();
            p = *(reinterpret_cast<Type*>(a));
            delete[] a;
            return (*this);
        }
        ~QFileReader()
        {
            _data.clear();
        }
    };

    void applySmoothGroups(QMesh* mesh, const std::vector<quint32>& smoothGroups);

protected:
    typedef struct
    {
        quint16 id;
        quint32 length;
    } Chunk;

    struct _Edge;
    typedef struct
    {
        std::list<_Edge*> edges;
        unsigned int oldVertex;
        unsigned int index;
    } _VertexData;
    typedef struct _Vertex
    {
        _Vertex() { data = std::make_shared<_VertexData>(); }
        std::shared_ptr<_VertexData> data;
    } _Vertex;
    typedef struct _Edge
    {
        _Vertex* vertices[2];
        unsigned int triangles[2];
        int indexJointOfTriangle[2];
    } _Edge;
    typedef struct
    {
        _Edge* edges[3];
    } _Triangle;

    std::vector<QEntity*> m_entities;
    std::vector<Material3DS> m_materials;
    std::vector<TempInfo> m_tempInfo;

    inline _Edge* _findEdge(std::list<_Edge*>& newEdges, unsigned int oldVertexA, unsigned int oldVertexB)
    {
        _Edge* edge;
        for (std::list<_Edge*>::iterator it = newEdges.begin(); it != newEdges.end(); ++it) {
            edge = *it;
            if ((edge->vertices[0]->data->oldVertex == oldVertexA) && (edge->vertices[1]->data->oldVertex == oldVertexB))
                return edge;
        }
        return nullptr;
    }
    inline _Vertex* _weldVertices(_Vertex* vertexA, _Vertex* vertexB)
    {
        if (vertexA->data == vertexB->data)
            return vertexA;
        //assert(vertexA->data->oldVertex == vertexB->data->oldVertex);
        for (std::list<_Edge*>::iterator it = vertexB->data->edges.begin(); it != vertexB->data->edges.end(); ++it) {
            _Edge* edge = *it;
            if (edge->vertices[0]->data == vertexB->data) {
                edge->vertices[0] = vertexA;
            } else if (edge->vertices[1]->data == vertexB->data) {
                edge->vertices[1] = vertexA;
            } else {
                assert(false);
            }
        }
        vertexA->data->edges.splice(vertexA->data->edges.begin(), vertexB->data->edges);
        vertexB->data = vertexA->data;
        return vertexA;
    }
    inline void _addEdgeToList(std::list<_Edge*>& list, _Edge* edge)
    {
        for (std::list<_Edge*>::iterator it = list.begin(); it != list.end(); ++it) {
            if ((*it) == edge)
                return;
        }
        list.push_back(edge);
    }
    inline bool _canJoin(const std::vector<quint32>& smoothGroups, unsigned int indexTriangleA, unsigned int indexTriangleB)
    {
        //if (indexTriangleA >= smoothGroups.size())
        //    return true;
        quint16 groupA = smoothGroups[indexTriangleA];
        if (groupA == 0)
            return true;
        //if (indexTriangleB >= smoothGroups.size())
        //    return true;
        quint16 groupB = smoothGroups[indexTriangleB];
        if (groupB == 0)
            return true;
        return (groupA & groupB);
    }

    Chunk _readChunk(QFileReader& reader);
    QString _readString(QFileReader& reader);
    QColor _readColor(QFileReader& reader);
    float _readPercent(QFileReader& reader);
    bool _readHead(QFileReader& reader);
    void _readMain(QScrollEngineContext* context, QFileReader& reader);
    QEntity* _readEntity(QScrollEngineContext* context, QFileReader& reader, Chunk& parentChunk);
    QMesh* _readMesh(QScrollEngineContext* context, QFileReader& reader, Chunk& parentChunk);
    Material3DS _readMaterial(QFileReader& reader, Chunk& parentChunk);
    void _readAnimations(QFileReader& reader, Chunk& parentChunk);
    void _readAnimation(QFileReader& reader, Chunk& parentChunk);
    QEntity* _getFinishEntity(QScrollEngineContext* context, const QString& textureDir, const QString& prefixTextureName);
};

}
#endif
