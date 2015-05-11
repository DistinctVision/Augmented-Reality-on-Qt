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
    static int locationMatrixWVP;
    static int locationMatrixW;
    static int locationMatrixScreenTexture;
    static int locationWorldCameraPosition;
    static int locationScreenTexture;
    static int locationRefract;
    static int locationFallOffColor;
    static int locationFallOffIntensity;
    static int locationFallOffSoft;

public:
    QSh_Refraction_FallOff()
    {

        _currentIndexType = Refraction_FallOff;
        _subIndexType = 0;
        _screenTexture = 0;
        _refract = 0.07f;
        _fallOffColor = QColor(0, 155, 255, 255);
        _fallOffInt = 1.0f;
        _fallOffSoft = 1.0f;
    }
    QSh_Refraction_FallOff(GLint screenTexture, float refract = 0.7f,
                           const QColor& fallOffColor = QColor(0, 155, 255, 255),
                           float fallOffIntensity = 1.0f,
                           float fallOffSoft = 1.0f,
                           const QMatrix2x2& matrixScreenTexture = QMatrix2x2())
    {
        _currentIndexType = Refraction_FallOff;
        _subIndexType = 0;
        _screenTexture = screenTexture;
        _refract = refract;
        _fallOffColor = fallOffColor;
        _fallOffInt = fallOffIntensity;
        _fallOffSoft = fallOffSoft;
        _matrixScreenTexture = matrixScreenTexture;
    }

    GLuint screenTexture() const { return _screenTexture; }
    void setScreenTexture(GLuint texture) { _screenTexture = texture; }
    float refract() const { return _refract; }
    void setRefract(float refract) { _refract = refract; }
    QColor fallOffColor() const { return _fallOffColor; }
    void setFallOffColor(const QColor& color) { _fallOffColor = color; }
    float fallOffIntensity() const { return _fallOffInt; }
    void setFallOffIntensity(float intensity) { _fallOffInt = intensity; }
    float fallOffSoft() const { return _fallOffSoft; }
    void setFallOffSoft(float soft) { _fallOffSoft = soft; }
    QMatrix2x2 matrixScreenTexture() const { return _matrixScreenTexture; }
    void setMatrixScreenTexture(const QMatrix2x2& matrix) { _matrixScreenTexture = matrix; }

    QSh_Refraction_FallOff(const QSh_Refraction_FallOff* s)
    {
        _currentIndexType = Refraction_FallOff;
        _subIndexType = 0;
        _screenTexture = s->screenTexture();
        _refract = s->refract();
        _fallOffColor = s->fallOffColor();
        _fallOffInt = s->fallOffIntensity();
        _fallOffSoft = s->fallOffSoft();
        _matrixScreenTexture = s->matrixScreenTexture();
    }
    int indexType() const override { return Refraction_FallOff; }
    bool use(QScrollEngineContext* context, QOpenGLShaderProgram* program) override;
    void load(QScrollEngineContext* context, std::vector<QSharedPointer<QOpenGLShaderProgram>>& shaders) override;
    QSh* copy() const override
    {
        return new QSh_Refraction_FallOff(this);
    }

protected:
    GLuint _screenTexture;
    float _refract;
    QColor _fallOffColor;
    float _fallOffInt;
    float _fallOffSoft;
    QMatrix2x2 _matrixScreenTexture;
};

}
#endif // QSH_REFRACTION_FALLOFF_H
