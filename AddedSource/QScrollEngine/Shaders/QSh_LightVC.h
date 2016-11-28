#ifndef QSH_LIGHTVC_H
#define QSH_LIGHTVC_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Light.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>

#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QLight.h"

namespace QScrollEngine {

class QSh_LightVC:
        public QSh_Light
{
public:
    QSh_LightVC()
    {
        m_currentTypeIndex = static_cast<int>(Type::LightVC);
        m_subTypeIndex = 0;
        m_texture = nullptr;
        m_color.setRgb(255, 255, 255, 255);
        m_specularIntensity = 0.0f;
        m_specularPower = 1.0f;
        m_lights[0] = m_lights[1] = nullptr;
    }
    QSh_LightVC(QOpenGLTexture* texture, const QColor& color = QColor(255, 255, 255, 255),
                float specularIntensity = 0.0f, float specularPower = 1.0f)
    {
        m_currentTypeIndex = static_cast<int>(Type::LightVC);
        m_subTypeIndex = 0;
        m_texture = texture;
        m_color = color;
        m_specularIntensity = specularIntensity;
        m_specularPower = specularPower;
        m_lights[0] = m_lights[1] = nullptr;
    }

    int typeIndex() const override { return static_cast<int>(Type::LightVC); }
    void preprocess(const QDrawObject3D* drawObject) override;
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        attrs.push_back(VertexAttributes::Normals);
        attrs.push_back(VertexAttributes::RgbColors);
        return attrs;
    }

    QSh_LightVC(const QSh_Light* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::LightVC);
        m_subTypeIndex = 0;
        m_texture = s->texture();
        m_color = s->color();
        m_specularIntensity = s->specularIntensity();
        m_specularPower = s->specularPower();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_LightVC(this));
    }

protected:
    const QSharedPointer<UniformLocationOmniOmni>* getLocations() const { return m_locations; }

private:
    static QSharedPointer<UniformLocationOmniOmni> m_locations[6];
};

}
#endif
