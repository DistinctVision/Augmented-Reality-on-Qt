#include "TextureRenderer.h"
#include <cassert>

const GLfloat TextureRenderer::gl_vertex_data[8] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f
};

const GLfloat TextureRenderer::gl_texCoord_data[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

TextureRenderer::TextureRenderer()
{
    m_glContext = nullptr;
    m_gl = nullptr;
    m_isBlackWhite = false;
    m_shaderRGB = nullptr;
    m_shaderBW = nullptr;
}

bool TextureRenderer::isBlackWhite() const
{
    return m_isBlackWhite;
}

void TextureRenderer::setBlackWhite(bool v)
{
    m_isBlackWhite = v;
}

void TextureRenderer::draw(GLuint textureId, const QMatrix3x3& textureMatrix)
{
    if (QOpenGLContext::currentContext() != m_glContext) {
        clear();
        m_glContext = QOpenGLContext::currentContext();
        m_gl = new QOpenGLFunctions(m_glContext);
        m_gl->initializeOpenGLFunctions();
        QOpenGLShader vertexShader(QOpenGLShader::Vertex);
        const char* textVertex =
                "attribute mediump vec2 vertex;\n"
                "attribute mediump vec2 texCoord;\n"
                "uniform mat3 textureMatrix;\n"
                "varying mediump vec2 texc;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = vec4(vertex, 0, 1);\n"
                "    texc = (textureMatrix * vec3(texCoord, 1)).xy;\n"
                "}\n";
        vertexShader.compileSourceCode(textVertex);
        const char* textFragmentRGB =
                "uniform sampler2D texture;\n"
                "varying mediump vec2 texc;\n"
                "void main(void)\n"
                "{\n"
                "    gl_FragColor = vec4(texture2D(texture, texc).rgb, 1.0);\n"
                "}\n";
        const char* textFragmentBW =
                "uniform sampler2D texture;\n"
                "varying mediump vec2 texc;\n"
                "void main(void)\n"
                "{\n"
                "    mediump float v = dot(texture2D(texture, texc).rgb, vec3(0.334, 0.334, 0.334));\n"
                "    gl_FragColor = vec4(v, v, v, 1);\n"
                "}\n";
        m_shaderRGB = new QOpenGLShaderProgram();
        m_shaderRGB->addShader(&vertexShader);
        m_shaderRGB->addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentRGB);
        m_shaderRGB->bindAttributeLocation("vertex", 0);
        m_shaderRGB->bindAttributeLocation("texCoord", 1);
        m_shaderRGB->link();

        m_shaderBW = new QOpenGLShaderProgram();
        m_shaderBW->addShader(&vertexShader);
        m_shaderBW->addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentBW);
        m_shaderBW->bindAttributeLocation("vertex", 0);
        m_shaderBW->bindAttributeLocation("texCoord", 1);
        m_shaderBW->link();
    }

    m_gl->glDisable(GL_DEPTH_TEST);
    m_gl->glDepthMask(GL_FALSE);

    m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (m_isBlackWhite) {
        m_shaderBW->bind();

        m_shaderBW->enableAttributeArray(0);
        m_shaderBW->enableAttributeArray(1);

        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_vertex_data);
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_texCoord_data);
        m_gl->glActiveTexture(GL_TEXTURE0);
        m_gl->glBindTexture(GL_TEXTURE_2D, textureId);
        m_shaderBW->setUniformValue("texture", 0);
        m_shaderBW->setUniformValue("textureMatrix", textureMatrix);

        m_gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_shaderBW->disableAttributeArray(1);
        m_shaderBW->disableAttributeArray(0);
    } else {
        m_shaderRGB->bind();

        m_shaderRGB->enableAttributeArray(0);
        m_shaderRGB->enableAttributeArray(1);

        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_vertex_data);
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_texCoord_data);
        m_gl->glActiveTexture(GL_TEXTURE0);
        m_gl->glBindTexture(GL_TEXTURE_2D, textureId);
        m_shaderRGB->setUniformValue("texture", 0);
        m_shaderRGB->setUniformValue("textureMatrix", textureMatrix);

        m_gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_shaderRGB->disableAttributeArray(1);
        m_shaderRGB->disableAttributeArray(0);
    }
}

void TextureRenderer::clear()
{
    if (m_glContext != nullptr) {
        assert(QOpenGLContext::currentContext() == m_glContext);
        m_shaderRGB->bind();
        m_shaderRGB->removeAllShaders();
        delete m_shaderRGB;
        m_shaderRGB = nullptr;
        m_shaderBW->bind();
        m_shaderBW->removeAllShaders();
        delete m_shaderBW;
        m_shaderBW = nullptr;
        delete m_gl;
        m_gl = nullptr;
        m_glContext = nullptr;
    }
}

TextureRenderer::~TextureRenderer()
{
    clear();
}

