#ifndef PAINTER_H
#define PAINTER_H

#include <cassert>
#include "Point2.h"
#include "Image.h"

namespace AR {

class Painter
{
public:
    template <typename T>
    static void drawLine(Image<T>& image, float x1, float y1, float x2, float y2, const T& c)
    {
        float dx = x2-x1;
        float dy = y2-y1;
        int w = image.width();
        int h = image.height();
        float len = std::fabs(dx)+std::fabs(dy);
        for(int t=0; t<=len; ++t) {
            int x = (int)(x1 + t/(len)*dx+0.5f);
            int y = (int)(y1 + t/(len)*dy+0.5f);
            if (x>=0 && x<w && y>=0 && y<h)
                image(x, y) = c;
        }
    }
    template <typename T>
    inline static void drawLine(Image<T>& image, const Point2i& p1, const Point2i& p2, const T& c) {
                    drawLine<T>(image, static_cast<float>(p1.x), static_cast<float>(p1.y),
                                    static_cast<float>(p2.x), static_cast<float>(p2.y), c); }
    template <typename T>
    inline static void drawLine(Image<T>& image, const Point2f& p1, const Point2f& p2, const T& c) {
                    drawLine<T>(image, p1.x, p1.y, p2.x, p2.y, c); }

    template <typename T>
    static void drawBox(Image<T>& image, float upperleftX, float upperleftY, float lowerrightX, float lowerrightY,
                        const T& c)
    {
        drawLine<T>(image, upperleftX, upperleftY, upperleftX, lowerrightY, c);
        drawLine<T>(image, upperleftX, upperleftY, lowerrightX, upperleftY, c);
        drawLine<T>(image, upperleftX, lowerrightY, lowerrightX, lowerrightY, c);
        drawLine<T>(image, lowerrightX, upperleftY, lowerrightX, lowerrightY, c);
    }
    template <typename T>
    inline static void drawBox(Image<T>& image, const Point2i& upperleft, const Point2i& lowerright, const T& c) {
                    drawBox<T>(image, static_cast<float>(upperleft.x), static_cast<float>(upperleft.y),
                                   static_cast<float>(lowerright.x), static_cast<float>(lowerright.y), c); }
    template <typename T>
    inline static void drawBox(Image<T>& image, const Point2f& upperleft, const Point2f& lowerright, const T& c) {
                    drawBox<T>(image, upperleft.x, upperleft.y, lowerright.x, lowerright.y, c); }

    template <typename T>
    inline static void drawFillBox(Image<T>& image, const Point2i& center, const Point2i& halfSize, const T& c) {
        drawFillBox1<T>(image, Point2i(std::max(center.x - halfSize.x, 0),
                                     std::max(center.y - halfSize.y, 0)),
                       Point2i(std::min(center.x + halfSize.x, image.width() - 1),
                              std::min(center.y + halfSize.y, image.height() - 1)), c);
    }

    template <typename T>
    inline static void drawFillBox1(Image<T>& image, const Point2i& upperleft, const Point2i& lowerright, const T& c) {
        T* str = image.pointer(upperleft);
        Point2i p;
        for (p.y=upperleft.y; p.y<=lowerright.y; ++p.y) {
            for (p.x=upperleft.x; p.x<=lowerright.x; ++p.x)
                str[p.x] = c;
            str = &str[image.width()];
        }
    }

    template <typename T>
    static void drawCross(Image<T>& image, float x, float y, float len, const T& c)
    {
        drawLine<T>(image, x-len, y, x+len, y, c);
        drawLine<T>(image, x, y-len, x, y+len, c);
    }
    template <typename T>
    inline static void drawCross(Image<T>& image, const Point2i& p, float len, const T& c) {
                    drawCross<T>(image, static_cast<float>(p.x), static_cast<float>(p.y), len, c); }
    template <typename T>
    inline static void drawCross(Image<T>& image, const Point2f& p, float len, const T& c) {
                    drawCross<T>(image, p.x, p.y, len, c); }
    template <typename T>
    static void drawImage(Image<T>& canvasImage, const ImageRef<T>& image, const Point2f& pos, const Point2f& size)
    {
        assert((image.width() > 0) && (image.height() > 0));
        assert((size.x > 0.0005f) && (size.y > 0.0005f));
        Point2i cStart(std::max((int)std::floor(pos.x), 0), std::max((int)std::floor(pos.y), 0));
        Point2i cEnd(std::min(cStart.x + (int)std::ceil(size.x), canvasImage.width()),
                     std::min(cStart.y + (int)std::ceil(size.y), canvasImage.height()));
        Point2f scale(image.width() / size.x, image.height() / size.y);
        Point2i p;
        for (p.y=cStart.y; p.y<cEnd.y; ++p.y) {
            T* canvasStr = &canvasImage.data()[canvasImage.width() * p.y];
            float subY = (p.y - pos.y) * scale.y;
            int y = (int)std::floor(subY);
            subY = subY - y;
            const T* strA = &image.data()[image.width() * std::max(y, 0)];
            const T* strB = &image.data()[image.width() * std::min(y + 1, image.height() - 1)];
            for (p.x=cStart.x; p.x<cEnd.x; ++p.x) {
                float subX = (p.x - pos.x) * scale.x;
                int x = (int)std::floor(subX);
                subX = subX - x;
                int nextX = std::min(x + 1, image.width() - 1);
                x = std::max(0, x);
                x = std::min(nextX, image.width() - 1);
                canvasStr[p.x] = (T)(strA[x] * ((1.0f - subX) * (1.0f - subY)) + strA[nextX] * (subX * (1.0f - subY)) +
                                     strB[x] * ((1.0f - subX) * subY) + strB[nextX] * (subX * subY));
            }
        }
    }
};

}

#endif // PAINTER_H
