#ifndef QSH_TEXTURE1_H
#define QSH_TEXTURE1_H

#include "QScrollEngine/Shaders/QSh.h"
#include "QScrollEngine/Shaders/QSh_Color.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh_Texture1: public QSh_Color
{
public:
    static int locationMatrixWVP;
    static int locationColor;
    static int locationTexture0;

protected:
    QOpenGLTexture* _texture0;

public:
    QSh_Texture1()
    {
        _currentIndexType = Texture1;
        _subIndexType = 0;
        _color.setRgb(255, 255, 255, 255);
        _texture0 = nullptr;
    }
    QSh_Texture1(QOpenGLTexture* texture0, const QColor& color = QColor(255, 255, 255, 255))
    {
        _currentIndexType = Texture1;
        _subIndexType = 0;
        _color = color;
        _texture0 = texture0;
    }

    QOpenGLTexture* texture0() const { return _texture0; }
    void setTexture0(QOpenGLTexture* texture0) { _texture0 = texture0; }
    QSh_Texture1(const QSh_Texture1* s)
    {
        _currentIndexType = Texture1;
        _subIndexType = 0;
        _color = s->color();
        _texture0 = s->texture0();
    }
    QSh* copy() const override
    {
        return new QSh_Texture1(this);
    }
    int indexType() const override { return Texture1; }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
};

}
#endif
