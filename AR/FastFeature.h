#ifndef FASTFEATURE_H
#define FASTFEATURE_H

#include "AR/Image.h"
#include <vector>

#define FASTFEATURE_USE_NONMAXSUP 0

namespace AR {

class FastFeature
{
public:
    typedef struct FastCorner
    {
        IPoint pos;
#if (FASTFEATURE_USE_NONMAXSUP == 1)
        int score;
#endif
        float scoreShiTomasi;
    } FastCorner;

    /// Perform tree based 10 point FAST feature detection
    /// If you use this, please cite the paper given in @ref fast_corner_detect
    ///
    /// @param im 		The input image
    /// @param corners	The resulting container of corner locations
    /// @param barrier	Corner detection threshold
    /// @ingroup	gVision
    static void fast_corner_detect_10(const Image<unsigned char>& im, const IPoint& begin, const IPoint& end,
                                      std::vector<FastCorner>& corners, int barrier, float minShiTomasiScore);

#if (FASTFEATURE_USE_NONMAXSUP == 1)
    static void fastNonmaxSuppression(const std::vector<FastCorner>& corners, std::vector<FastCorner>& ret_nonmax);
#endif

    static bool comp(const FastFeature::FastCorner& a, const FastFeature::FastCorner& b)
                    { return a.scoreShiTomasi > b.scoreShiTomasi; }

protected:
    /// The 16 offsets from the centre pixel used in FAST feature detection.
    ///
    /// @ingroup gVision
    static int _fast_pixel_ring[16];

    static int _fast_corner_score_10(const unsigned char* p, int barrier);
    static void _make_fast_pixel_offset(int row_stride);
    static float _getShiTomasiScore_10(const Image<unsigned char>& im, const IPoint& pos);
};

}

#endif // FASTFEATURE_H
