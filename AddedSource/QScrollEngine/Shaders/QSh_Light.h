#ifndef QSH_LIGHT_H
#define QSH_LIGHT_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Texture.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QSharedPointer>

#include "QScrollEngine/QSceneObject3D.h"
#include "QScrollEngine/QDrawObject3D.h"
#include "QScrollEngine/QLight.h"

namespace QScrollEngine {

class QSh_Light: virtual public QSh_Texture
{
public:
    QSh_Light()
    {
        m_currentTypeIndex = static_cast<int>(Type::Light);
        m_subTypeIndex = 0;
        m_texture = nullptr;
        m_color.setRgb(255, 255, 255, 255);
        m_specularIntensity = 0.0f;
        m_specularPower = 1.0f;
        m_lights[0] = m_lights[1] = nullptr;
    }
    QSh_Light(QOpenGLTexture* texture, const QColor& color = QColor(255, 255, 255, 255),
              float specularIntensity = 0.0f, float specularPower = 1.0f)
    {
        m_currentTypeIndex = static_cast<int>(Type::Light);
        m_subTypeIndex = 0;
        m_texture = texture;
        m_color = color;
        m_specularIntensity = specularIntensity;
        m_specularPower = specularPower;
        m_lights[0] = m_lights[1] = nullptr;
    }

    float specularIntensity() const { return m_specularIntensity; }
    void setSpecularIntensity(float intensity) { m_specularIntensity = intensity; }
    float specularPower() const { return m_specularPower; }
    void setSpecularPower(float power) { m_specularPower = power; }

    QLight* light0() const { return m_lights[0]; }
    QLight* light1() const { return m_lights[1]; }

    QSh_Light(const QSh_Light* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::Light);
        m_subTypeIndex = 0;
        m_texture = s->texture();
        m_color = s->color();
        m_specularIntensity = s->specularIntensity();
        m_specularPower = s->specularPower();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_Light(this));
    }

    int typeIndex() const override { return static_cast<int>(Type::Light); }
    void preprocess(const QDrawObject3D* drawObject) override;
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        attrs.push_back(VertexAttributes::Normals);
        return attrs;
    }

protected:
    QLight* m_lights[2];
    float m_specularIntensity;
    float m_specularPower;

    void _findLights(QScene* scene, const QDrawObject3D* drawObject);

public:
    class UniformLocationOmniOmni: public QSh_Texture::UniformLocation
    {
    public:
        void bindParameters(QScrollEngineContext* context,
                            QOpenGLShaderProgram* program,
                            const QSh_Light* shader,
                            const QSceneObject3D* sceneObject) const;
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        int matrixW;
        int ambientColor;
        int light0_color;
        int light0_position;
        int light0_radius;
        int light0_soft;
        int light1_color;
        int light1_position;
        int light1_radius;
        int light1_soft;

        virtual void bindLightParameters(QScrollEngineContext* context,
                                         QOpenGLShaderProgram* program,
                                         const QSh_Light* shader,
                                         const QSceneObject3D* sceneObject) const;
    };

    class UniformLocationOmniSpot: public UniformLocationOmniOmni
    {
    public:
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        int light1_dir;
        int light1_spotCutOff;

        void bindLightParameters(QScrollEngineContext* context,
                                 QOpenGLShaderProgram* program,
                                 const QSh_Light* shader,
                                 const QSceneObject3D* sceneObject) const override;
    };

    class UniformLocationSpotSpot: public UniformLocationOmniSpot
    {
    public:
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        int light0_dir;
        int light0_spotCutOff;

        void bindLightParameters(QScrollEngineContext* context,
                                 QOpenGLShaderProgram* program,
                                 const QSh_Light* shader,
                                 const QSceneObject3D* sceneObject) const override;
    };

    class UniformLocationSpecular
    {
    protected:
        int specularIntensity;
        int specularPower;
        int viewPosition;

        void bindSpecularParameters(QOpenGLShaderProgram* program,
                                    const QSh_Light* shader,
                                    const QScene* scene) const;
    };

    class UniformLocationOmniOmniSpecular: public UniformLocationOmniOmni,
                                           public UniformLocationSpecular
    {
    public:
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        void bindLightParameters(QScrollEngineContext* context,
                                 QOpenGLShaderProgram* program,
                                 const QSh_Light* shader,
                                 const QSceneObject3D* sceneObject) const override;
    };

    class UniformLocationOmniSpotSpecular: public UniformLocationOmniSpot,
                                           public UniformLocationSpecular
    {
    public:
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        void bindLightParameters(QScrollEngineContext* context,
                                 QOpenGLShaderProgram* program,
                                 const QSh_Light* shader,
                                 const QSceneObject3D* sceneObject) const override;
    };

    class UniformLocationSpotSpotSpecular: public UniformLocationSpotSpot,
                                           public UniformLocationSpecular
    {
    public:
        void loadLocations(QOpenGLShaderProgram* shader);
    protected:
        void bindLightParameters(QScrollEngineContext* context,
                                 QOpenGLShaderProgram* program,
                                 const QSh_Light* shader,
                                 const QSceneObject3D* sceneObject) const override;
    };

    static const QSharedPointer<UniformLocationOmniOmni>* getLocations() { return m_locations; }

private:
    static QSharedPointer<UniformLocationOmniOmni> m_locations[6];
};

}
#endif
