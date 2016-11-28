#ifndef QSH_REFRACTION_FALLOFF_H
#define QSH_REFRACTION_FALLOFF_H

#include "QScrollEngine/Shaders/QSh.h"
#include <QColor>
#include <QMatrix2x2>
#include <qgl.h>

namespace QScrollEngine {

class QSh_Refraction_FallOff:
        public QSh
{
public:
    QSh_Refraction_FallOff()
    {

        m_currentTypeIndex = static_cast<int>(Type::Refraction_FallOff);
        m_subTypeIndex = 0;
        m_screenTexture = 0;
        m_refract = 0.07f;
        m_fallOffColor = QColor(0, 155, 255, 255);
        m_fallOffInt = 1.0f;
        m_fallOffSoft = 1.0f;
    }
    QSh_Refraction_FallOff(GLint screenTexture, float refract = 0.7f,
                           const QColor& fallOffColor = QColor(0, 155, 255, 255),
                           float fallOffIntensity = 1.0f,
                           float fallOffSoft = 1.0f,
                           const QMatrix2x2& matrixScreenTexture = QMatrix2x2())
    {
        m_currentTypeIndex = static_cast<int>(Type::Refraction_FallOff);
        m_subTypeIndex = 0;
        m_screenTexture = screenTexture;
        m_refract = refract;
        m_fallOffColor = fallOffColor;
        m_fallOffInt = fallOffIntensity;
        m_fallOffSoft = fallOffSoft;
        m_matrixScreenTexture = matrixScreenTexture;
    }

    GLuint screenTexture() const { return m_screenTexture; }
    void setScreenTexture(GLuint texture) { m_screenTexture = texture; }
    float refract() const { return m_refract; }
    void setRefract(float refract) { m_refract = refract; }
    QColor fallOffColor() const { return m_fallOffColor; }
    void setFallOffColor(const QColor& color) { m_fallOffColor = color; }
    float fallOffIntensity() const { return m_fallOffInt; }
    void setFallOffIntensity(float intensity) { m_fallOffInt = intensity; }
    float fallOffSoft() const { return m_fallOffSoft; }
    void setFallOffSoft(float soft) { m_fallOffSoft = soft; }
    QMatrix2x2 matrixScreenTexture() const { return m_matrixScreenTexture; }
    void setMatrixScreenTexture(const QMatrix2x2& matrix) { m_matrixScreenTexture = matrix; }

    QSh_Refraction_FallOff(const QSh_Refraction_FallOff* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::Refraction_FallOff);
        m_subTypeIndex = 0;
        m_screenTexture = s->screenTexture();
        m_refract = s->refract();
        m_fallOffColor = s->fallOffColor();
        m_fallOffInt = s->fallOffIntensity();
        m_fallOffSoft = s->fallOffSoft();
        m_matrixScreenTexture = s->matrixScreenTexture();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_Refraction_FallOff(this));
    }

    int typeIndex() const override { return static_cast<int>(Type::Refraction_FallOff); }
    bool use(QScrollEngineContext*, QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    std::vector<VertexAttributes> attributes() const override
    {
        std::vector<VertexAttributes> attrs;
        attrs.push_back(VertexAttributes::TextureCoords);
        attrs.push_back(VertexAttributes::Normals);
        return attrs;
    }

protected:
    GLuint m_screenTexture;
    float m_refract;
    QColor m_fallOffColor;
    float m_fallOffInt;
    float m_fallOffSoft;
    QMatrix2x2 m_matrixScreenTexture;

public:
    class UniformLocation
    {
    public:
        void bindParameters(QScrollEngineContext* context,
                            QOpenGLShaderProgram* program,
                            const QSh_Refraction_FallOff* shader,
                            const QSceneObject3D* sceneObject) const;
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int matrixWVP;
        int matrixW;
        int matrixScreenTexture;
        int worldCameraPosition;
        int screenTexture;
        int refract;
        int fallOffColor;
        int fallOffIntensity;
        int fallOffSoft;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static UniformLocation m_locations;
};

}
#endif // QSH_REFRACTION_FALLOFF_H
