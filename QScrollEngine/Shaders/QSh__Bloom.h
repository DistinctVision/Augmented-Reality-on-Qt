#ifndef QSH__BLOOM_H
#define QSH__BLOOM_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QSize>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace QScrollEngine {

class QSh__Bloom: public QSh
{
public:
    static int locationMatrixWVP;
    static int locationScreenTexture;
    static int locationBloomMapTexture;
    static int locationTexelParam;

private:
    static const QVector2D _D;

    float _intensity;
    QVector2D _texel;
    QMatrix4x4 _finalMatrix;

public:
    QSh__Bloom()
    {
        _currentIndexType = -3;
        _subIndexType = 0;
        _intensity = 0.8f;
        setSize(640, 480);
    }

    QMatrix4x4 finalMatrx() const { return _finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { _finalMatrix = finalMatrix; }

    float intensity() const { return _intensity; }
    void setIntensity(float intensity) { _intensity = intensity; }

    void bindScreenTexture(QScrollEngineContext* parentContext, GLuint screenTexture);
    void bindBloomMapTexture(QScrollEngineContext* parentContext, GLuint bloomMapTexture);

    void setSize(int width, int height)
    {
        _texel.setX(_D.x() / (float) width);
        _texel.setY(_D.y() / (float) height);
    }
    void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSize size() const { return QSize(_D.x() / _texel.x(), _D.y() / _texel.y()); }

    int indexType() const override { return -3; }
    bool use(QScrollEngineContext*, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;

    QSh__Bloom(const QSh__Bloom* s)
    {
        _currentIndexType = -3;
        _subIndexType = 0;
        _intensity = s->intensity();
        setSize(s->size());
        setFinalMatrix(s->finalMatrx());
    }
    QSh* copy() const override
    {
        return new QSh__Bloom(this);
    }
};

}
#endif
