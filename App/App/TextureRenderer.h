#ifndef TEXTURERENDERER_H
#define TEXTURERENDERER_H

#include <qopengl.h>
#include <QMatrix3x3>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include "QScrollEngine/QScrollEngineContext.h"

class TextureRenderer
{
public:
    static const GLfloat gl_vertex_data[8];
    static const GLfloat gl_texCoord_data[8];

public:
    TextureRenderer();
    ~TextureRenderer();

    bool isBlackWhite() const;
    void setBlackWhite(bool v);

    void draw(GLuint textureId, const QMatrix3x3& textureMatrix);

    void clear();

private:
    QOpenGLShaderProgram* m_shaderRGB;
    QOpenGLShaderProgram* m_shaderBW;
    bool m_isBlackWhite;
    QOpenGLContext* m_glContext;
    QOpenGLFunctions* m_gl;
};

#endif // TEXTURERENDERER_H
