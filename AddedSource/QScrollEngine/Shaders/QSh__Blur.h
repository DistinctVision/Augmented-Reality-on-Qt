#ifndef QSH__BLUR_H
#define QSH__BLUR_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QSize>

namespace QScrollEngine {

class QSh__Blur: public QSh
{
public:
    QSh__Blur()
    {
        m_currentTypeIndex = -2;
        m_subTypeIndex = 0;
        m_texelParam.setZ(8.0f);
        setSize(640, 480);
        m_axisX = true;
    }

    QMatrix4x4 finalMatrix() const { return m_finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { m_finalMatrix = finalMatrix; }

    void setSize(int width, int height)
    {
        m_texelParam.setX(m_D.x() / (float) width);
        m_texelParam.setY(m_D.y() / (float) height);
    }
    void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSize size() const { return QSize(m_D.x() / m_texelParam.x(), m_D.y() / m_texelParam.y()); }
    void setRadius(float radius) { m_texelParam.setZ(radius); }
    float radius() const { return m_texelParam.z(); }
    bool byX() const { return m_axisX; }
    bool byY() const { return !m_axisX; }
    void setByX() { m_axisX = true; }
    void setByY() { m_axisX = false; }

    QVector3D texelParam() const
    {
        if (m_axisX)
            return QVector3D(m_texelParam.x(), 0.0f, m_texelParam.z());
        return QVector3D(0.0f, m_texelParam.y(), m_texelParam.z());
    }

    void bindScreenTexture(QScrollEngineContext* context, GLuint screenTexture);

    QSh__Blur(const QSh__Blur* s)
    {
        m_currentTypeIndex = -2;
        m_subTypeIndex = 0;
        m_texelParam.setZ(s->radius());
        setSize(s->size());
        m_axisX = byX();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh__Blur(this));
    }

    int typeIndex() const override { return -2; }
    bool use(QScrollEngineContext*, QOpenGLShaderProgram* program, const QDrawObject3D* ) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders);
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        return attrs;
    }

private:
    QVector3D m_texelParam;
    QMatrix4x4 m_finalMatrix;
    bool m_axisX;

public:
    class UniformLocation
    {
    public:
        void bindParameters(QOpenGLShaderProgram* program, const QSh__Blur* shader) const;
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int matrixWVP;
        int screenTexture;
        int texelParam;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static const QVector2D m_D;
    static UniformLocation m_locations;
};

}
#endif
