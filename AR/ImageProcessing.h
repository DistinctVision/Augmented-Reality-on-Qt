#ifndef IMAGESOBEL_H
#define IMAGESOBEL_H

#include "AR/Image.h"

namespace AR {

class ImageProcessing
{
public:
    static void sobel(Image<uchar>& in, Image<bool>& out, Image<int>& outIntegral, const uchar threshold);
    static void erode(Image<int>& integral, Image<bool>& out, int size, float k);
};

}

#endif // IMAGESOBEL_H
