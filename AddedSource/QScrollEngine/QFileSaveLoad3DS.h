#ifndef QFILESAVELOAD3DS_H
#define QFILESAVELOAD3DS_H

#include <QChar>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <vector>
#include <QOpenGLTexture>
#include "QScrollEngine/QFileLoad3DS.h"
#include "QScrollEngine/Shaders/QSh.h"

namespace QScrollEngine {

class QScrollEngineContext;
class QEntity;

class QFileSaveLoad3DS : public QFileLoad3DS
{
public:
    bool saveEntity(QScrollEngineContext* context, QEntity* entity, const QString& fileName,
                    const QString& textureDir = "!", const QString& prefixTextureName = "");

private:
    typedef struct
    {
        QString name;
        QOpenGLTexture* texture;
    } TextureInfo;

    class QFileData
    {
    public:
        void clear() { _data.clear(); }
        unsigned int size() const { return _data.size(); }

        template<typename Type>
        QFileData& operator << (const Type& p)
        {
            unsigned int size = sizeof(p);
            const char* data = reinterpret_cast<const char*>(&p);
            unsigned int begin = _data.size();
            _data.resize(_data.size() + size);
            for (unsigned int i=0; i<size; ++i)
                _data[begin+i] = data[i];
            return (*this);
        }

        QFileData& operator << (const QFileData& fileData)
        {
            _data.insert(_data.end(), fileData._data.begin(), fileData._data.end());
            return (*this);
        }

        void writeToFile(QFile& file)
        {
            file.write(_data.data(), _data.size());
        }

    private:
        std::vector<char> _data;
    };

    typedef struct
    {
        quint16 id;
        quint32 length;
        QFileData data;
    } ChunkData;

    std::vector<TextureInfo> m_textureInfo;
    bool m_saveTexture;
    int m_currentIdTexture;
    int m_currentIdShader;

    int _findTexture(const QString& name);
    int _findTexture(QOpenGLTexture* texture);
    void _checkTextureName(QString& name, const QString& prefixTextureName);
    void _write(QFileData& data, const QString& string);
    int _findEntityInVector(QEntity* entity);
    void _entitiesToVector(QEntity *entity);
    QString _writeMaterial(QScrollEngineContext* context, ChunkData& chunkData, const QString& prefixTextureName, QSh* shader);
    void _writeMesh(ChunkData& chunkData, QMesh* mesh, const QString& name, const QString& materialName);
    void _writeAnimation(ChunkData& chunkData, int indexEntity, const QString& name);
    void _writeMeshAnimation(ChunkData& chunkData, QString& name, int meshId, int parentEntityId);
    void _writeEntities(QScrollEngineContext* context, ChunkData& chunkData, const QString& prefixTextureName);
    void _writeTextures(QScrollEngineContext* context, const QString& textureDir);
};

}

#endif // QFILESAVELOAD3DS_H
