#ifndef QSH_COLOR_H
#define QSH_COLOR_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

namespace QScrollEngine {

class QPlanarShadows;

class QSh_Color: public QSh
{
    friend class QScrollEngineContext;

public:
    QSh_Color()
    {
        m_currentTypeIndex = static_cast<int>(Type::Color);
        m_subTypeIndex = 0;
        m_color.setRgb(255, 255, 255, 255);
    }
    QSh_Color(const QColor& color)
    {
        m_currentTypeIndex = static_cast<int>(Type::Color);
        m_subTypeIndex = 0;
        m_color = color;
    }

    QColor color() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    void setColor(int r, int g, int b, int a = 255) { m_color.setRgb(r, g, b, a); }
    void setColorF(float r, float g, float b, float a = 1.0f) { m_color.setRgbF(r, g, b, a); }
    QSh_Color(const QSh_Color* s)
    {
        m_currentTypeIndex = static_cast<int>(Type::Color);
        m_subTypeIndex = 0;
        m_color = s->color();
    }
    QShPtr copy() const override
    {
        return QShPtr(new QSh_Color(this));
    }
    int typeIndex() const override { return static_cast<int>(Type::Color); }
    bool use(QScrollEngineContext* , QOpenGLShaderProgram* program, const QDrawObject3D* drawObject) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& program) override;

    std::vector<VertexAttributes> attributes() const override { return std::vector<VertexAttributes>(); }

protected:
    QColor m_color;

public:
    class UniformLocation
    {
    public:
        virtual ~UniformLocation() {}

        void bindParameters(QOpenGLShaderProgram* program, const QSh_Color* shader,
                            const QSceneObject3D* sceneObject) const;
        void bindParameters(QOpenGLShaderProgram* program, const QColor& color, const QMatrix4x4& matrixWVP);
        void loadLocations(QOpenGLShaderProgram* shader);

    protected:
        int matrixWVP;
        int color;
    };

    static const UniformLocation& getLocations() { return m_locations; }

private:
    static UniformLocation m_locations;
};

}
#endif
