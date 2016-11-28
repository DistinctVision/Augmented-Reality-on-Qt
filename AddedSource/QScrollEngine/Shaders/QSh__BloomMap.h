#ifndef QSH__BLOOMMAP_H
#define QSH__BLOOMMAP_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace QScrollEngine {

class QSh__BloomMap: public QSh
{
public:
    QSh__BloomMap()
    {
        m_currentTypeIndex = -1;
        m_subTypeIndex = 0;
        m_threshold = 0.6f;
        m_gain = 2.0f;
    }

    QMatrix4x4 finalMatrix() const { return m_finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { m_finalMatrix = finalMatrix; }

    float threshold() const { return m_threshold; }
    void setThreshold(float threshold) { m_threshold = threshold; }
    float gain() const { return m_gain; }
    void setGain(float gain) { m_gain = gain; }

    void bindScreenTexture(QScrollEngineContext* parentContext, GLuint screenTexture);

    QSh__BloomMap(const QSh__BloomMap* s)
    {
        m_currentTypeIndex = -1;
        m_subTypeIndex = 0;
        m_threshold = s->threshold();
        m_gain = s->gain();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh__BloomMap(this));
    }

    int typeIndex() const override { return -1; }
    bool use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* ) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders);
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        return attrs;
    }

private:
    float m_threshold;
    float m_gain;
    QMatrix4x4 m_finalMatrix;

public:
    class UniformLocation
    {
    public:
        void bindParameters(QOpenGLShaderProgram* program, const QSh__BloomMap* shader) const;
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int matrixWVP;
        int screenTexture;
        int threshold;
        int gain;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static UniformLocation m_locations;
};

}
#endif
