#ifndef QSH_TEXTURE1_H
#define QSH_TEXTURE1_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Color.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh_Texture: public QSh_Color
{
public:
    QSh_Texture()
    {
        m_currentTypeIndex = static_cast<int>(Type::Texture);
        m_subTypeIndex = 0;
        m_color.setRgb(255, 255, 255, 255);
        m_texture = nullptr;
    }
    QSh_Texture(QOpenGLTexture* texture, const QColor& color = QColor(255, 255, 255, 255))
    {
        m_currentTypeIndex = static_cast<int>(Type::Texture);
        m_subTypeIndex = 0;
        m_color = color;
        m_texture = texture;
    }

    QOpenGLTexture* texture() const { return m_texture; }
    void setTexture(QOpenGLTexture* texture) { m_texture = texture; }
    QSh_Texture(const QSh_Texture* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::Texture);
        m_subTypeIndex = 0;
        m_color = s->color();
        m_texture = s->texture();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_Texture(this));
    }
    int typeIndex() const override { return static_cast<int>(Type::Texture); }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        return attrs;
    }

protected:
    QOpenGLTexture* m_texture;

public:
    class UniformLocation: public QSh_Color::UniformLocation
    {
    public:
        void bindParameters(QScrollEngineContext* context,
                            QOpenGLShaderProgram* program,
                            const QSh_Texture* shader,
                            const QSceneObject3D* sceneObject) const;
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        int texture;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static UniformLocation m_locations;
};

}
#endif
