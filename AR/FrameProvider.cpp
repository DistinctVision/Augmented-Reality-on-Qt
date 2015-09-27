#include "AR/FrameProvider.h"
#include <cassert>
#include <QTime>

namespace AR {

const GLfloat FrameProvider::gl_vertex_data[8] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, -1.0f
};

const GLfloat FrameProvider::gl_texture_data[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

QList<QVideoFrame::PixelFormat> FrameProvider::supportedPixelFormats(QAbstractVideoBuffer::HandleType type)
{
    if (type == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_BGR32;
    } else if (type == QAbstractVideoBuffer::GLTextureHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_BGR32;
    }
    return QList<QVideoFrame::PixelFormat>();
}

FrameProvider* FrameProvider::createFrameProvider(const QVideoFrame::PixelFormat& pixelFormat)
{
    FrameProvider* frameProvider;
    switch (pixelFormat) {
    case QVideoFrame::Format_RGB32: {
        frameProvider = new FrameProvider_RGB32();
    }
        break;
    case QVideoFrame::Format_BGR32: {
        frameProvider = new FrameProvider_BGR32();
    }
        break;
    default:
        frameProvider = nullptr;
    }
    return frameProvider;
}

FrameProvider::FrameProvider()
{
    _context = nullptr;
    _needToDeleteTexture = false;
    _handleSourceTexture = 0;
    _internalFormat = 4;
    _format = GL_RGBA;
    _type = GL_UNSIGNED_BYTE;
    _handleType = QAbstractVideoBuffer::NoHandle;
    _pixelFormat = QVideoFrame::Format_Invalid;
    _matrixTexture.setToIdentity();
    _usedTransform = false;
}

void FrameProvider::_clearContextData()
{
    if (_context == nullptr)
        return;
    if (_needToDeleteTexture) {
        _context->glDeleteTextures(1, &_handleSourceTexture);
        _needToDeleteTexture = false;
    }
    _handleSourceTexture = 0;
    _context = nullptr;
}

FrameProvider::~FrameProvider()
{
    _clearContextData();
}

void FrameProvider::setHandleTexture(QScrollEngine::QScrollEngineContext* context, GLuint handleTexture, int textureWidth, int textureHeight)
{
    assert((handleTexture > 0) && (context != nullptr));
    if (_handleSourceTexture == handleTexture)
        return;
    if (_context != context) {
        _clearContextData();
        _context = context;
    } else if (_needToDeleteTexture) {
        _context->glDeleteTextures(1, &_handleSourceTexture);
    }
    _needToDeleteTexture = false;
    _handleSourceTexture = handleTexture;
    _context->glBindTexture(GL_TEXTURE_2D, _handleSourceTexture);
    _originalTextureSize.x = static_cast<GLuint>(textureWidth);
    _originalTextureSize.y = static_cast<GLuint>(textureHeight);
}

void FrameProvider::setDataTexture(QScrollEngine::QScrollEngineContext* context, const GLvoid* data, int textureWidth, int textureHeight)
{
    assert(context != nullptr);
    if (_context != context) {
        _clearContextData();
        _context = context;
    }
    if (_handleSourceTexture == 0) {
        if (_needToDeleteTexture)
            _context->glDeleteTextures(1, &_handleSourceTexture);
        _context->glGenTextures(1, &_handleSourceTexture);
        _needToDeleteTexture = true;
        _originalTextureSize = AR::IPoint::ZERO;
        _context->glBindTexture(GL_TEXTURE_2D, _handleSourceTexture);
    } else {
        _context->glBindTexture(GL_TEXTURE_2D, _handleSourceTexture);
    }
    if ((_originalTextureSize.x != textureWidth) || (_originalTextureSize.y != textureHeight)) {
        _originalTextureSize.x = textureWidth;
        _originalTextureSize.y = textureHeight;
        _context->glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, static_cast<GLuint>(_originalTextureSize.x), static_cast<GLuint>(_originalTextureSize.y),
                               0, _format, _type, data);
        _context->glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        _context->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _originalTextureSize.x, _originalTextureSize.y, _format, _type, data);
        _context->glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void FrameProvider::bindColorShader(QOpenGLFunctions* glFunctions)
{
    assert(_shaderColor.isLinked());
    _shaderColor.bind();
    _shaderColor.enableAttributeArray(0);
    _shaderColor.enableAttributeArray(1);
    if (_usedTransform) {
        _shaderColor.setUniformValue(_locationMatrixTexture_color, _matrixTexture);
    } else {
        _shaderColor.setUniformValue(_locationMatrixTexture_color, QMatrix2x2());
    }
    _shaderColor.setUniformValue(_locationTexture_color, 0);
    glFunctions->glActiveTexture(GL_TEXTURE0);
    glFunctions->glBindTexture(GL_TEXTURE_2D, _handleSourceTexture);
    glFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), gl_vertex_data);
    glFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), gl_texture_data);
    _otherDataToColorShader();
}

void FrameProvider::bindLuminanceShader(QOpenGLFunctions* glFunctions)
{
    assert(_shaderLuminace.isLinked());
    _shaderLuminace.bind();
    _shaderLuminace.enableAttributeArray(0);
    _shaderLuminace.enableAttributeArray(1);
    if (_usedTransform) {
        _shaderColor.setUniformValue(_locationMatrixTexture_luminance, _matrixTexture);
    } else {
        _shaderColor.setUniformValue(_locationMatrixTexture_luminance, QMatrix2x2());
    }
    _shaderLuminace.setUniformValue(_locationTexture_luminance, 0);
    glFunctions->glActiveTexture(GL_TEXTURE0);
    glFunctions->glBindTexture(GL_TEXTURE_2D, _handleSourceTexture);
    glFunctions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), gl_vertex_data);
    glFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), gl_texture_data);
    _otherDataToLuminanceShader();
}

void FrameProvider::getColorImage(QOpenGLFunctions* glFunctions, Image<Rgba>& image)
{
    glFunctions->glViewport(0, 0, image.width(), image.height());
    glFunctions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glFunctions->glGenerateMipmap(GL_TEXTURE_2D);
    glFunctions->glReadPixels(0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(image.data()));
    image.data();
}

void FrameProvider::getLuminanceImage(QOpenGLFunctions* glFunctions, Image<uchar>& image)
{
    QTime timer;
    timer.start();
    glFunctions->glDisable(GL_BLEND);
    glFunctions->glViewport(0, 0, image.width() / 4, image.height());
    _shaderLuminace.setUniformValue(_locationTexelX_luminance, 1.0f / static_cast<float>(image.width()));
    glFunctions->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glFunctions->glFlush();
    glFunctions->glReadPixels(0, 0, image.width() / 4, image.height(), GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(image.data()));
    _timeProvideLuminanceFrame = timer.elapsed();
}

FrameProvider_RGB32::FrameProvider_RGB32():
    FrameProvider()
{
    _internalFormat = 4;
    _type = GL_UNSIGNED_BYTE;
    _format = GL_RGBA;
    _pixelFormat = QVideoFrame::Format_RGB32;
    _handleType = QAbstractVideoBuffer::NoHandle;
    const char* textVertex =
            "attribute highp vec2 vertex;\n"
            "attribute highp vec2 texCoord;\n"
            "varying mediump vec2 texc;\n"
            "uniform mediump mat2 matrixTexture;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(vertex, 0.0, 1.0);\n"
            "    texc = matrixTexture * (texCoord - vec2(0.5, 0.5)) + vec2(0.5, 0.5);\n"
            "}\n";
    const char* textFragmentColor =
        "uniform sampler2D texture;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(texture, texc).rgb;\n"
        "    gl_FragColor = vec4(color.b, color.g, color.r, 1.0);\n"
        "}\n";
    _shaderColor.addShaderFromSourceCode(QOpenGLShader::Vertex, textVertex);
    _shaderColor.addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentColor);
    _shaderColor.bindAttributeLocation("vertex", 0);
    _shaderColor.bindAttributeLocation("texCoord", 1);
    _shaderColor.link();
    _locationMatrixTexture_color = _shaderColor.uniformLocation("matrixTexture");
    _locationTexture_color = _shaderColor.uniformLocation("texture");
    const char* textFragmentLuminace =
        "uniform sampler2D texture;\n"
        "uniform highp float texelX;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec4 result;\n"
        "    highp vec2 textureCoord = texc;\n"
        "    highp vec3 color = texture2D(texture, textureCoord).rgb;\n"
        "    result.r = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.g = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.b = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.a = (color.r + color.g + color.b) / 3.0;\n"
        "    gl_FragColor = result;\n"
        "}\n";
    _shaderLuminace.addShaderFromSourceCode(QOpenGLShader::Vertex, textVertex);
    _shaderLuminace.addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentLuminace);
    _shaderLuminace.bindAttributeLocation("vertex", 0);
    _shaderLuminace.bindAttributeLocation("texCoord", 1);
    _shaderLuminace.link();
    _locationMatrixTexture_luminance = _shaderLuminace.uniformLocation("matrixTexture");
    _locationTexture_luminance = _shaderLuminace.uniformLocation("texture");
    _locationTexelX_luminance = _shaderLuminace.uniformLocation("texelX");
}

FrameProvider_BGR32::FrameProvider_BGR32()
{
    _internalFormat = 4;
    _type = GL_UNSIGNED_BYTE;
    _format = GL_RGBA;
    _pixelFormat = QVideoFrame::Format_BGR32;
    _handleType = QAbstractVideoBuffer::NoHandle;
    const char* textVertex =
            "attribute highp vec2 vertex;\n"
            "attribute highp vec2 texCoord;\n"
            "varying mediump vec2 texc;\n"
            "uniform mediump mat2 matrixTexture;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(vertex, 0.0, 1.0);\n"
            "    texc = matrixTexture * (texCoord - vec2(0.5, 0.5)) + vec2(0.5, 0.5);\n"
            "}\n";
    const char* textFragmentColor =
        "uniform sampler2D texture;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(texture, texc).rgb;\n"
        "    gl_FragColor = vec4(color.r, color.g, color.b, 1.0);\n"
        "}\n";
    _shaderColor.addShaderFromSourceCode(QOpenGLShader::Vertex, textVertex);
    _shaderColor.addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentColor);
    _shaderColor.bindAttributeLocation("vertex", 0);
    _shaderColor.bindAttributeLocation("texCoord", 1);
    _shaderColor.link();
    _locationMatrixTexture_color = _shaderColor.uniformLocation("matrixTexture");
    _locationTexture_color = _shaderColor.uniformLocation("texture");
    const char* textFragmentLuminace =
        "uniform sampler2D texture;\n"
        "uniform highp float texelX;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec4 result;\n"
        "    highp vec2 textureCoord = texc;\n"
        "    highp vec3 color = texture2D(texture, textureCoord).rgb;\n"
        "    result.r = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.g = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.b = (color.r + color.g + color.b) / 3.0;\n"
        "    textureCoord.x += texelX;\n"
        "    color = texture2D(texture, textureCoord).rgb;\n"
        "    result.a = (color.r + color.g + color.b) / 3.0;\n"
        "    gl_FragColor = result;\n"
        "}\n";
    _shaderLuminace.addShaderFromSourceCode(QOpenGLShader::Vertex, textVertex);
    _shaderLuminace.addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentLuminace);
    _shaderLuminace.bindAttributeLocation("vertex", 0);
    _shaderLuminace.bindAttributeLocation("texCoord", 1);
    _shaderLuminace.link();
    _locationMatrixTexture_luminance = _shaderLuminace.uniformLocation("matrixTexture");
    _locationTexture_luminance = _shaderLuminace.uniformLocation("texture");
    _locationTexelX_luminance = _shaderLuminace.uniformLocation("texelX");
}

}
