#ifndef QSH_COLOR_H
#define QSH_COLOR_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

namespace QScrollEngine {

class QSh_Color: public QSh
{
public:
    static int locationMatrixWVP;
    static int locationColor;

protected:
    QColor _color;

public:
    QSh_Color()
    {
        _currentIndexType = Color;
        _subIndexType = 0;
        _color.setRgb(255, 255, 255, 255);
    }
    QSh_Color(const QColor& color)
    {
        _currentIndexType = Color;
        _subIndexType = 0;
        _color = color;
    }

    QColor color() const { return _color; }
    void setColor(const QColor& color) { _color = color; }
    void setColor(int r, int g, int b, int a = 255) { _color.setRgb(r, g, b, a); }
    void setColorF(float r, float g, float b, float a = 1.0f) { _color.setRgbF(r, g, b, a); }
    int indexType() const override { return Color; }
    bool use(QScrollEngineContext* , QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& program) override;
    QSh_Color(const QSh_Color* s)
    {
        _currentIndexType = Color;
        _subIndexType = 0;
        _color = s->color();
    }
    QSh* copy() const override
    {
        return new QSh_Color(this);
    }
};

}
#endif
