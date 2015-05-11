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
    static int locationMatrixWVP;
    static int locationColor;
    static int locationBackColor;
    static int locationTexturePart;
    static int locationPartParam;

protected:
    QColor _backColor;
    QOpenGLTexture* _texturePart;
    QVector2D _partParam;

public:
    QSh_ColoredPart()
    {
        _currentIndexType = ColoredPart;
        _subIndexType = 0;
        _color.setRgb(255, 255, 255, 255);
        _backColor.setRgb(0, 0, 0, 255);
        _texturePart = nullptr;
        _partParam.setX(0.3f);
        _partParam.setY(0.5f);
    }
    QSh_ColoredPart(QOpenGLTexture* texturePart, const QColor& color = QColor(255, 255, 255, 255),
                    const QColor& backColor = QColor(0, 0, 0, 255), float limit = 0.3f, float soft = 0.5f)
    {
        _currentIndexType = ColoredPart;
        _texturePart = texturePart;
        _color = color;
        _backColor = backColor;
        _partParam.setX(limit);
        _partParam.setY(soft);
    }
    QColor backColor() const { return _backColor; }
    void setBackColor(const QColor& color) { _backColor = color; }
    void setBackColor(int r, int g, int b, int a = 255) { _backColor.setRgb(r, g, b, a); }
    void setBackColorF(float r, float g, float b, float a = 1.0f) { _backColor.setRgbF(r, g, b, a); }
    QOpenGLTexture* texturePart() const { return _texturePart; }
    void setTexturePart(QOpenGLTexture* bwTexture) { _texturePart = bwTexture; }
    float limit() const { return _partParam.x(); }
    void setLimit(float value) { _partParam.setX(value); }
    float smooth() const { return _partParam.y(); }
    void setSmooth(float value) { assert(value >= 0.0f); _partParam.setY(value); }
    int indexType() const override { return ColoredPart; }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    QSh_ColoredPart(const QSh_ColoredPart* s)
    {
        _currentIndexType = ColoredPart;
        _subIndexType = 0;
        _color = s->color();
        _backColor = s->backColor();
        _texturePart = s->texturePart();
        _partParam.setX(s->limit());
        _partParam.setY(s->smooth());
    }
    QSh* copy() const override
    {
        return new QSh_ColoredPart(this);
    }
};

}
#endif
