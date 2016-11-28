#ifndef QSH__BLOOM_H
#define QSH__BLOOM_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QSize>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh__Bloom: public QSh
{
public:
    QSh__Bloom()
    {
        m_currentTypeIndex = -3;
        m_subTypeIndex = 0;
        m_intensity = 0.8f;
        setSize(640, 480);
    }

    QMatrix4x4 finalMatrix() const { return m_finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { m_finalMatrix = finalMatrix; }

    float intensity() const { return m_intensity; }
    void setIntensity(float intensity) { m_intensity = intensity; }

    void bindScreenTexture(QScrollEngineContext* parentContext, GLuint screenTexture);
    void bindBloomMapTexture(QScrollEngineContext* parentContext, GLuint bloomMapTexture);

    void setSize(int width, int height)
    {
        m_texel.setX(m_D.x() / (float) width);
        m_texel.setY(m_D.y() / (float) height);
    }
    void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSize size() const { return QSize(m_D.x() / m_texel.x(), m_D.y() / m_texel.y()); }

    QVector3D texelParam() const { return QVector3D(m_texel.x(), m_texel.y(), m_intensity); }

    QSh__Bloom(const QSh__Bloom* s)
    {
        m_currentTypeIndex = -3;
        m_subTypeIndex = 0;
        m_intensity = s->intensity();
        setSize(s->size());
        setFinalMatrix(s->finalMatrix());
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh__Bloom(this));
    }

    int typeIndex() const override { return -3; }
    bool use(QScrollEngineContext*, QOpenGLShaderProgram* program, const QDrawObject3D* ) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        return attrs;
    }


private:
    float m_intensity;
    QVector2D m_texel;
    QMatrix4x4 m_finalMatrix;

public:
    class UniformLocation
    {
    public:
        void bindParameters(QOpenGLShaderProgram* program, const QSh__Bloom* shader) const;
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int matrixWVP;
        int screenTexture;
        int bloomMapTexture;
        int texelParam;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static const QVector2D m_D;
    static UniformLocation m_locations;
};

}
#endif
