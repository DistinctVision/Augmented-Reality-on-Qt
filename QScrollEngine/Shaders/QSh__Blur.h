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
    static int locationMatrixWVP;
    static int locationScreenTexture;
    static int locationTexelParam;

private:
    static const QVector2D _D;

    QVector3D _texelParam;
    QMatrix4x4 _finalMatrix;
    bool _axisX;

public:
    QSh__Blur()
    {
        _currentIndexType = -2;
        _subIndexType = 0;
        _texelParam.setZ(4.0f);
        setSize(640, 480);
        _axisX = true;
    }

    QMatrix4x4 finalMatrix() const { return _finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { _finalMatrix = finalMatrix; }

    void setSize(int width, int height)
    {
        _texelParam.setX(_D.x() / (float) width);
        _texelParam.setY(_D.y() / (float) height);
    }
    void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSize size() const { return QSize(_D.x() / _texelParam.x(), _D.y() / _texelParam.y()); }
    void setRadius(float radius) { _texelParam.setZ(radius); }
    float radius() const { return _texelParam.z(); }
    bool byX() const { return _axisX; }
    bool byY() const { return !_axisX; }
    void setByX() { _axisX = true; }
    void setByY() { _axisX = false; }

    void bindScreenTexture(QScrollEngineContext* context, GLuint screenTexture);

    int indexType() const override { return -2; }
    bool use(QScrollEngineContext*, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders);
    QSh__Blur(const QSh__Blur* s)
    {
        _currentIndexType = -2;
        _subIndexType = 0;
        _texelParam.setZ(s->radius());
        setSize(s->size());
        _axisX = byX();
    }
    QSh* copy() const override
    {
        return new QSh__Blur(this);
    }
};

}
#endif
