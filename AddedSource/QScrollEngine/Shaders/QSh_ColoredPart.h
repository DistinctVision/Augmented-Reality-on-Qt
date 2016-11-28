#ifndef QSH_COLOREDPART_H
#define QSH_COLOREDPART_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Color.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>

#include <cassert>

namespace QScrollEngine {

class QSh_ColoredPart: public QSh_Color
{
public:
    QSh_ColoredPart()
    {
        m_currentTypeIndex = static_cast<int>(Type::ColoredPart);
        m_subTypeIndex = 0;
        m_color.setRgb(255, 255, 255, 255);
        m_backColor.setRgb(0, 0, 0, 255);
        m_texturePart = nullptr;
        m_partParam.setX(0.3f);
        m_partParam.setY(0.5f);
    }
    QSh_ColoredPart(QOpenGLTexture* texturePart, const QColor& color = QColor(255, 255, 255, 255),
                    const QColor& backColor = QColor(0, 0, 0, 255), float limit = 0.3f, float soft = 0.5f)
    {
        m_currentTypeIndex = static_cast<int>(Type::ColoredPart);
        m_texturePart = texturePart;
        m_color = color;
        m_backColor = backColor;
        m_partParam.setX(limit);
        m_partParam.setY(soft);
    }
    QColor backColor() const { return m_backColor; }
    void setBackColor(const QColor& color) { m_backColor = color; }
    void setBackColor(int r, int g, int b, int a = 255) { m_backColor.setRgb(r, g, b, a); }
    void setBackColorF(float r, float g, float b, float a = 1.0f) { m_backColor.setRgbF(r, g, b, a); }
    QOpenGLTexture* texturePart() const { return m_texturePart; }
    void setTexturePart(QOpenGLTexture* bwTexture) { m_texturePart = bwTexture; }
    float limit() const { return m_partParam.x(); }
    void setLimit(float value) { m_partParam.setX(value); }
    float smooth() const { return m_partParam.y(); }
    void setSmooth(float value) { assert(value >= 0.0f); m_partParam.setY(value); }
    QVector2D partParams() const { return QVector2D(m_partParam.x() * (1.0f + m_partParam.y()), m_partParam.y()); }

    QSh_ColoredPart(const QSh_ColoredPart* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::ColoredPart);
        m_subTypeIndex = 0;
        m_color = s->color();
        m_backColor = s->backColor();
        m_texturePart = s->texturePart();
        m_partParam.setX(s->limit());
        m_partParam.setY(s->smooth());
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_ColoredPart(this));
    }

    int typeIndex() const override { return (int)Type::ColoredPart; }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        return attrs;
    }

protected:
    QColor m_backColor;
    QOpenGLTexture* m_texturePart;
    QVector2D m_partParam;

public:
    class UniformLocation: public QSh_Color::UniformLocation
    {
    public:
        void bindParameters(QScrollEngineContext* context,
                            QOpenGLShaderProgram* program,
                            const QSh_ColoredPart* shader,
                            const QSceneObject3D* sceneObject) const;
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int backColor;
        int texturePart;
        int partParam;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static UniformLocation m_locations;
};

}
#endif
