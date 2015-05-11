#ifndef QSH__BLOOMMAP_H
#define QSH__BLOOMMAP_H

#include "QScrollEngine/Shaders/QSh.h"

#include <QColor>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace QScrollEngine {

class QSh__BloomMap: public QSh
{
public:   
    static int locationMatrixWVP;
    static int locationScreenTexture;
    static int locationThreshold;
    static int locationGain;

private:
    float _threshold;
    float _gain;
    QMatrix4x4 _finalMatrix;

public:
    QSh__BloomMap()
    {
        _currentIndexType = -1;
        _subIndexType = 0;
        _threshold = 0.7f;
        _gain = 2.0f;
    }

    QMatrix4x4 finalMatrix() const { return _finalMatrix; }
    void setFinalMatrix(const QMatrix4x4& finalMatrix) { _finalMatrix = finalMatrix; }

    float threshold() const { return _threshold; }
    void setThreshold(float threshold) { _threshold = threshold; }
    float gain() const { return _gain; }
    void setGain(float gain) { _gain = gain; }

    void bindScreenTexture(QScrollEngineContext* parentContext, GLuint screenTexture);

    int indexType() const override { return -1; }

    bool use(QScrollEngineContext* , QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders);
    QSh__BloomMap(const QSh__BloomMap* s)
    {
        _currentIndexType = -1;
        _subIndexType = 0;
        _threshold = s->threshold();
        _gain = s->gain();
    }
    QSh* copy() const override
    {
        return new QSh__BloomMap(this);
    }
};

}
#endif
