#include "PixelsReader.h"
#include <cstring>
#include <utility>
#include <memory>
#include <QTime>
#include <QDebug>

PixelsReader::PixelsReader(const AR::Point2i & size)
{
    m_gl = new QOpenGLFunctions();
    m_gl->initializeOpenGLFunctions();
    m_image = AR::Image<AR::Rgba>(size);
}

PixelsReader::~PixelsReader()
{
    delete m_gl;
}

AR::Point2i PixelsReader::size() const
{
    return m_image.size();
}

void PixelsReader::resize(const AR::Point2i& size)
{
    m_image = AR::Image<AR::Rgba>(size);
}

void PixelsReader::resize(int width, int height)
{
    resize(AR::Point2i(width, height));
}

AR::ConstImage<AR::Rgba> PixelsReader::image() const
{
    return m_image;
}

void PixelsReader::read()
{
    m_gl->glReadPixels(0, 0, m_image.width(), m_image.height(), GL_RGBA, GL_UNSIGNED_BYTE, m_image.data());
}
