#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <stdint.h>

#include <QVideoFrame>
#include <QAbstractVideoBuffer>
#include <QMatrix2x2>
#include <QVector2D>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include "QScrollEngine/QScrollEngineContext.h"

#include "AR/Image.h"

namespace AR {

class FrameProvider
{
public:
    static const GLfloat gl_vertex_data[8];
    static const GLfloat gl_texture_data[8];

    static QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle);
    static FrameProvider* createFrameProvider(const QVideoFrame::PixelFormat& pixelFormat);

public:
    FrameProvider();
    virtual ~FrameProvider();

    QScrollEngine::QScrollEngineContext* context() { return _context; }
    GLuint handleTexture() const { return _handleSourceTexture; }
    void setHandleTexture(QScrollEngine::QScrollEngineContext* context, GLuint hangleTexture, int textureWidth, int textureHeight);
    void setDataTexture(QScrollEngine::QScrollEngineContext* context, const GLvoid* textureData, int textureWidth, int textureHeight);

    QAbstractVideoBuffer::HandleType handleType() const { return _handleType; }
    QVideoFrame::PixelFormat pixelFormat() const { return _pixelFormat; }

    void setMatrixTexture(const QMatrix2x2& matrix) { _matrixTexture = matrix; }
    QMatrix2x2 matrixTexture() const { return _matrixTexture; }
    void setUsedTransform(bool usedTransform) { _usedTransform = usedTransform; }
    bool usedTransform() const { return _usedTransform; }

    void bindColorShader(QOpenGLFunctions* glFunctions);
    void bindLuminanceShader(QOpenGLFunctions* glFunctions);
    void getColorImage(QOpenGLFunctions* glFunctions, Image<Rgba> &image);
    void getLuminanceImage(QOpenGLFunctions* glFunctions, Image<uchar>& image);

    int timeProvideLuminanceFrame() const { return _timeProvideLuminanceFrame; }

    AR::IPoint originalTextureSize() const { return _originalTextureSize; }

protected:
    QScrollEngine::QScrollEngineContext* _context;
    bool _needToDeleteTexture;
    GLuint _handleSourceTexture;
    AR::IPoint _originalTextureSize;
    GLint _internalFormat;
    GLenum _format;
    GLenum _type;
    QAbstractVideoBuffer::HandleType _handleType;
    QVideoFrame::PixelFormat _pixelFormat;
    int _timeProvideLuminanceFrame;

    QOpenGLShaderProgram _shaderColor;
    int _locationMatrixTexture_color;
    int _locationTexture_color;
    QOpenGLShaderProgram _shaderLuminace;
    int _locationMatrixTexture_luminance;
    int _locationTexture_luminance;
    int _locationTexelX_luminance;
    QMatrix2x2 _matrixTexture;
    bool _usedTransform;

    void _clearContextData();

    virtual void _otherDataToColorShader() {}
    virtual void _otherDataToLuminanceShader() {}
};

class FrameProvider_RGB32:
        public FrameProvider
{
public:
    FrameProvider_RGB32();
};

class FrameProvider_BGR32:
        public FrameProvider
{
public:
    FrameProvider_BGR32();
};

/*#include "AR/ARSystem.h"

class ColorConverter
{
public:
    virtual ~ColorConverter() {}
    void convert(const uchar* scr, ARSystem& arSystem, int width, int height)
    {
        if ((arSystem._sourceImage.width() != width) || (arSystem._sourceImage.height() != height)) {
            arSystem._sourceImage.resize(IPoint(width, height));
            arSystem._sourceBWImage.resize(arSystem._sourceImage.size());
        }
        _convert(scr, arSystem._sourceImage.data(), arSystem._sourceBWImage.data(), arSystem._sourceImage.size());
    }

protected:
    virtual void _convert(const uchar* scr, Rgb* dstRGB, uchar* dstBW, const IPoint& size) = 0;
};

class ColorConverter_RGB32_To_RGB:
        public ColorConverter
{
protected:
    inline void _decodePixel(const uchar* rgb32, Rgb* dstRGB, int& currentPixels, int& cursor)
    {
        dstRGB[currentPixels].red = rgb32[cursor + 2];
        dstRGB[currentPixels].green = rgb32[cursor + 1];
        dstRGB[currentPixels].blue = rgb32[cursor];
        ++currentPixels;
        cursor += 4;
    }
    void _convert(const uchar* rgb32, Rgb* dstRGB, uchar* dstBW, const IPoint& size)
    {
        int cursor = 0;
        int cursorBW = 0;
        int currentPixels = 0;
        int i, j;
        for (i = 0; i < size.y; ++i) {
            for (j = 0; j < size.x; ++j) {
                dstBW[cursorBW] = (rgb32[cursor + 2] + rgb32[cursor + 1] + rgb32[cursor]) / 3;
                ++cursorBW;
                _decodePixel(rgb32, dstRGB, currentPixels, cursor);
            }
        }
    }
};

class ColorConverter_YUV_NV21_To_RGB:
        public ColorConverter
{
protected:
    inline uchar _decodePixel(const uchar* yuv, Rgb* dstRGB, int cX, int& currentPixels, int& baseOffsetA, int& baseOffsetB)
    {
        uchar y = qMin(qMax(0xff & ((int) yuv[baseOffsetA + cX]), 16), 240);
        int offset = baseOffsetB + (cX & ~1);
        int v = (0xff & ((int) yuv[offset])) - 128;
        int u = (0xff & ((int) yuv[offset + 1])) - 128;

        dstRGB[currentPixels].red = static_cast<uchar>(y + 1.13983f * v);
        dstRGB[currentPixels].green = static_cast<uchar>(y - 0.39465f * u - 0.5806f * v);
        dstRGB[currentPixels].blue = static_cast<uchar>(y + 2.03211f * u);
        ++currentPixels;
        return y;
    }
    void _convert(const uchar* yuv, Rgb* dstRGB, uchar* dstBW, const IPoint& size)
    {
        const int frameSize = size.y * size.x;

        int baseOffsetA, baseOffsetB;

        int cursorBW = 0;
        int currentPixels = 0;
        int i, j;
        for (i = 0; i < size.y; ++i) {
            baseOffsetA = i * size.x;
            baseOffsetB = frameSize + (i >> 1) * size.x;
            for (j = 0; j < size.x; ++j) {
                dstBW[cursorBW] = _decodePixel(yuv, dstRGB, j, currentPixels, baseOffsetA, baseOffsetB);
                ++cursorBW;
            }
        }
    }
};*/


#ifdef  __ARM_NEON__

/*
* ARM NEON optimized implementation of UYVY -> RGB16 convertor
*/
void uyvy422_to_rgb16_line_neon (uint8_t * dst, const uint8_t * src, int n)
{
     /* and this is the NEON code itself */
     static __attribute__ ((aligned (16))) uint16_t acc_r[8] = {
       22840, 22840, 22840, 22840, 22840, 22840, 22840, 22840,
     };
     static __attribute__ ((aligned (16))) uint16_t acc_g[8] = {
       17312, 17312, 17312, 17312, 17312, 17312, 17312, 17312,
     };
     static __attribute__ ((aligned (16))) uint16_t acc_b[8] = {
       28832, 28832, 28832, 28832, 28832, 28832, 28832, 28832,
     };
     /*
      * Registers:
      * q0, q1 : d0, d1, d2, d3  - are used for initial loading of YUV data
      * q2     : d4, d5          - are used for storing converted RGB data
      * q3     : d6, d7          - are used for temporary storage
      *
      * q6     : d12, d13        - are used for converting to RGB16
      * q7     : d14, d15        - are used for storing RGB16 data
      * q4-q5 - reserved
      *
      * q8, q9 : d16, d17, d18, d19  - are used for expanded Y data
      * q10    : d20, d21
      * q11    : d22, d23
      * q12    : d24, d25
      * q13    : d26, d27
      * q13, q14, q15            - various constants (#16, #149, #204, #50, #104, #154)
      */
     asm volatile (".macro convert_macroblock size\n"
         /* load up to 16 source pixels in UYVY format */
         ".if \\size == 16\n"
         "pld [%[src], #128]\n"
         "vld1.32 {d0, d1, d2, d3}, [%[src]]!\n"
         ".elseif \\size == 8\n"
         "vld1.32 {d0, d1}, [%[src]]!\n"
         ".elseif \\size == 4\n"
         "vld1.32 {d0}, [%[src]]!\n"
         ".elseif \\size == 2\n"
         "vld1.32 {d0[0]}, [%[src]]!\n"
         ".else\n" ".error \"unsupported macroblock size\"\n" ".endif\n"
         /* convert from 'packed' to 'planar' representation */
         "vuzp.8      d0, d1\n"    /* d1 - separated Y data (first 8 bytes) */
         "vuzp.8      d2, d3\n"    /* d3 - separated Y data (next 8 bytes) */
         "vuzp.8      d0, d2\n"    /* d0 - separated U data, d2 - separated V data */
         /* split even and odd Y color components */
         "vuzp.8      d1, d3\n"    /* d1 - evenY, d3 - oddY */
         /* clip upper and lower boundaries */
         "vqadd.u8    q0, q0, q4\n"
         "vqadd.u8    q1, q1, q4\n"
         "vqsub.u8    q0, q0, q5\n"
         "vqsub.u8    q1, q1, q5\n"
         "vshr.u8     d4, d2, #1\n"    /* d4 = V >> 1 */
         "vmull.u8    q8, d1, d27\n"       /* q8 = evenY * 149 */
         "vmull.u8    q9, d3, d27\n"       /* q9 = oddY * 149 */
         "vld1.16     {d20, d21}, [%[acc_r], :128]\n"      /* q10 - initialize accumulator for red */
         "vsubw.u8    q10, q10, d4\n"      /* red acc -= (V >> 1) */
         "vmlsl.u8    q10, d2, d28\n"      /* red acc -= V * 204 */
         "vld1.16     {d22, d23}, [%[acc_g], :128]\n"      /* q11 - initialize accumulator for green */
         "vmlsl.u8    q11, d2, d30\n"      /* green acc -= V * 104 */
         "vmlsl.u8    q11, d0, d29\n"      /* green acc -= U * 50 */
         "vld1.16     {d24, d25}, [%[acc_b], :128]\n"      /* q12 - initialize accumulator for blue */
         "vmlsl.u8    q12, d0, d30\n"      /* blue acc -= U * 104 */
         "vmlsl.u8    q12, d0, d31\n"      /* blue acc -= U * 154 */
         "vhsub.s16   q3, q8, q10\n"       /* calculate even red components */
         "vhsub.s16   q10, q9, q10\n"      /* calculate odd red components */
         "vqshrun.s16 d0, q3, #6\n"        /* right shift, narrow and saturate even red components */
         "vqshrun.s16 d3, q10, #6\n"       /* right shift, narrow and saturate odd red components */
         "vhadd.s16   q3, q8, q11\n"       /* calculate even green components */
         "vhadd.s16   q11, q9, q11\n"      /* calculate odd green components */
         "vqshrun.s16 d1, q3, #6\n"        /* right shift, narrow and saturate even green components */
         "vqshrun.s16 d4, q11, #6\n"       /* right shift, narrow and saturate odd green components */
         "vhsub.s16   q3, q8, q12\n"       /* calculate even blue components */
         "vhsub.s16   q12, q9, q12\n"      /* calculate odd blue components */
         "vqshrun.s16 d2, q3, #6\n"        /* right shift, narrow and saturate even blue components */
         "vqshrun.s16 d5, q12, #6\n"       /* right shift, narrow and saturate odd blue components */
         "vzip.8      d0, d3\n"    /* join even and odd red components */
         "vzip.8      d1, d4\n"    /* join even and odd green components */
         "vzip.8      d2, d5\n"    /* join even and odd blue components */
         "vshll.u8     q7, d0, #8\n" //red
         "vshll.u8     q6, d1, #8\n" //greed
         "vsri.u16   q7, q6, #5\n"
         "vshll.u8     q6, d2, #8\n" //blue
         "vsri.u16   q7, q6, #11\n" //now there is rgb16 in q7
         ".if \\size == 16\n"
         "vst1.16 {d14, d15}, [%[dst]]!\n"
         //"vst3.8  {d0, d1, d2}, [%[dst]]!\n"
         "vshll.u8     q7, d3, #8\n" //red
         "vshll.u8     q6, d4, #8\n" //greed
         "vsri.u16   q7, q6, #5\n"
         "vshll.u8     q6, d5, #8\n" //blue
         "vsri.u16   q7, q6, #11\n" //now there is rgb16 in q7
         //"vst3.8  {d3, d4, d5}, [%[dst]]!\n"
         "vst1.16 {d14, d15}, [%[dst]]!\n"
         ".elseif \\size == 8\n"
         "vst1.16 {d14, d15}, [%[dst]]!\n"
         //"vst3.8  {d0, d1, d2}, [%[dst]]!\n"
         ".elseif \\size == 4\n"
         "vst1.8 {d14}, [%[dst]]!\n"
         ".elseif \\size == 2\n"
         "vst1.8 {d14[0]}, [%[dst]]!\n"
         "vst1.8 {d14[1]}, [%[dst]]!\n"
         ".else\n"
         ".error \"unsupported macroblock size\"\n"
         ".endif\n"
         ".endm\n"
         "vmov.u8     d8, #15\n"  /* add this to U/V to saturate upper boundary */
         "vmov.u8     d9, #20\n"   /* add this to Y to saturate upper boundary */
         "vmov.u8     d10, #31\n"  /* sub this from U/V to saturate lower boundary */
         "vmov.u8     d11, #36\n"  /* sub this from Y to saturate lower boundary */
         "vmov.u8     d26, #16\n"
         "vmov.u8     d27, #149\n"
         "vmov.u8     d28, #204\n"
         "vmov.u8     d29, #50\n"
         "vmov.u8     d30, #104\n"
         "vmov.u8     d31, #154\n"
         "subs        %[n], %[n], #16\n"
         "blt         2f\n"
         "1:\n"
         "convert_macroblock 16\n"
         "subs        %[n], %[n], #16\n"
         "bge         1b\n"
         "2:\n"
         "tst         %[n], #8\n"
         "beq         3f\n"
         "convert_macroblock 8\n"
         "3:\n"
         "tst         %[n], #4\n"
         "beq         4f\n"
         "convert_macroblock 4\n"
         "4:\n"
         "tst         %[n], #2\n"
         "beq         5f\n"
         "convert_macroblock 2\n"
         "5:\n"
         ".purgem convert_macroblock\n":[src] "+&r" (src),[dst] "+&r" (dst),
         [n] "+&r" (n)
         :[acc_r] "r" (&acc_r[0]),[acc_g] "r" (&acc_g[0]),[acc_b] "r" (&acc_b[0])
         :"cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
         "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
         "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31");
}
#endif

}

#endif
