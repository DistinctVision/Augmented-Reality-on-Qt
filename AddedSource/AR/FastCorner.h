#ifndef AR_FASTCORNER_H
#define AR_FASTCORNER_H

#include "Image.h"
#include <vector>

namespace AR {

class FastCorner
{
public:
    struct Corner
    {
        Point2i pos;
        int score;
        int level;
    };

    /// Perform tree based 10 point FAST feature detection
    static void fast_corner_detect_10(const ImageRef<unsigned char>& im,
                                      const Point2i& begin, const Point2i& end,
                                      std::vector<Corner>& corners, int barrier);

    static void fastNonmaxSuppression(const std::vector<Corner>& corners, std::vector<Corner>& ret_nonmax);

    static bool comp(const FastCorner::Corner& a, const FastCorner::Corner& b)
    {
        return a.score > b.score;
    }

    static int fast_corner_score_10(const unsigned char* p, int barrier);
    static float shiTomasiScore_10(const ImageRef<unsigned char>& im, const Point2i& pos);

protected:
    static int _fast_pixel_ring[16];

    static void _make_fast_pixel_offset(int row_stride);
};

}

#endif // AR_FASTCORNER_H
