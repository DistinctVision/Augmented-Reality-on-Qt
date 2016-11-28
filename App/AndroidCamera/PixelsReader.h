#ifndef PIXELSREADER_H
#define PIXELSREADER_H

#include "AR/Image.h"
#include <qgl.h>
#include <QOpenGLFunctions>

class PixelsReader
{
public:
    PixelsReader(const AR::Point2i & size);
    ~PixelsReader();

    AR::Point2i size() const;
    void resize(int width, int height);
    void resize(const AR::Point2i& size);

    AR::ConstImage<AR::Rgba> image() const;

    void read();

private:
    QOpenGLFunctions* m_gl;
    AR::Image<AR::Rgba> m_image;
};

#endif // PIXELSREADER_H
