#include "FastCorner.h"
#include <cmath>

namespace AR {

int FastCorner::_fast_pixel_ring[16];

void FastCorner::_make_fast_pixel_offset(int row_stride)
{
    _fast_pixel_ring[0] = 0 + row_stride * 3;
    _fast_pixel_ring[1] = 1 + row_stride * 3;
    _fast_pixel_ring[2] = 2 + row_stride * 2;
    _fast_pixel_ring[3] = 3 + row_stride * 1;
    _fast_pixel_ring[4] = 3 + row_stride * 0;
    _fast_pixel_ring[5] = 3 + row_stride * -1;
    _fast_pixel_ring[6] = 2 + row_stride * -2;
    _fast_pixel_ring[7] = 1 + row_stride * -3;
    _fast_pixel_ring[8] = 0 + row_stride * -3;
    _fast_pixel_ring[9] = -1 + row_stride * -3;
    _fast_pixel_ring[10] = -2 + row_stride * -2;
    _fast_pixel_ring[11] = -3 + row_stride * -1;
    _fast_pixel_ring[12] = -3 + row_stride * 0;
    _fast_pixel_ring[13] = -3 + row_stride * 1;
    _fast_pixel_ring[14] = -2 + row_stride * 2;
    _fast_pixel_ring[15] = -1 + row_stride * 3;
}

float FastCorner::shiTomasiScore_10(const ImageRef<unsigned char>& im, const Point2i& pos)
{
    const unsigned char* p3 = &(im.data())[(pos.y - 3) * im.width()];
    const unsigned char* p2 = &(im.data())[(pos.y - 2) * im.width()];
    const unsigned char* p1 = &(im.data())[(pos.y - 1) * im.width()];
    const unsigned char* s  = &(im.data())[pos.y * im.width()];
    const unsigned char* n1 = &(im.data())[(pos.y + 1) * im.width()];
    const unsigned char* n2 = &(im.data())[(pos.y + 2) * im.width()];
    const unsigned char* n3 = &(im.data())[(pos.y + 3) * im.width()];
    float Dxx = 0.0f;
    float Dxy = 0.0f;
    float Dyy = 0.0f;
    const int endx = pos.x + 3;
    for (int x = pos.x - 3; x<=endx; ++x) {
        float dx = (float)(p2[x+1] - p2[x-1]); float dy = (float)(p1[x] - p3[x]);
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = (float)(p1[x+1] - p1[x-1]); dy = (float)(s[x] - p2[x]);
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = (float)(s[x+1] - s[x-1]); dy = (float)(n1[x] - p1[x]);
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = (float)(n1[x+1] - n1[x-1]); dy = (float)(n2[x] - s[x]);
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = (float)(n2[x+1] - n2[x-1]); dy = (float)(n3[x] - n1[x]);
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;
    }

    const float sum_Dxx_Dyy = Dxx + Dyy;

    float r = 0.04f * 0.5f * (sum_Dxx_Dyy - std::sqrt(sum_Dxx_Dyy * sum_Dxx_Dyy - 4.0f * (Dxx * Dyy - Dxy * Dxy)));
    return r;
}

void FastCorner::fastNonmaxSuppression(const std::vector<FastCorner::Corner>& corners,
                                        std::vector<FastCorner::Corner>& ret_nonmax)
{
    int last_row;
    int* row_start;
    int i, j;
    const int sz = (int)(corners.size());

    /*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
    int point_above = 0;
    int point_below = 0;


    if (sz < 1) {
        return;
    }

    ret_nonmax.reserve((std::size_t)(sz));

    /* Find where each row begins
       (the corners are output in raster scan order). A beginning of -1 signifies
       that there are no corners on that row. */
    last_row = corners[sz - 1].pos.y;
    row_start = new int[last_row + 1];

    for (i = 0; i < last_row + 1; ++i)
        row_start[i] = -1;

    {
        int prev_row = -1;
        for (i = 0; i < sz; ++i) {
            if (corners[i].pos.y != prev_row) {
                row_start[corners[i].pos.y] = i;
                prev_row = corners[i].pos.y;
            }
        }
    }



    for (i = 0; i < sz; ++i) {
        const Corner& c = corners[i];

        /*Check left */
        if (i > 0)
            if(corners[i-1].pos.x == c.pos.x-1 && corners[i-1].pos.y == c.pos.y && (corners[i-1].score >= c.score))
                continue;

        /*Check right*/
        if (i < (sz - 1))
            if(corners[i+1].pos.x == c.pos.x+1 && corners[i+1].pos.y == c.pos.y && (corners[i+1].score >= c.score))
                continue;

        /*Check above (if there is a valid row above)*/
        if (c.pos.y != 0 && row_start[c.pos.y - 1] != -1)
        {
            /*Make sure that current point_above is one
              row above.*/
            if (corners[point_above].pos.y < c.pos.y - 1)
                point_above = row_start[c.pos.y-1];

            /*Make point_above point to the first of the pixels above the current point,
              if it exists.*/
            for (; corners[point_above].pos.y < c.pos.y && corners[point_above].pos.x < c.pos.x - 1; ++point_above)
            {}


            for (j=point_above; corners[j].pos.y < c.pos.y && corners[j].pos.x <= c.pos.x + 1; j++)
            {
                int x = corners[j].pos.x;
                if( (x == c.pos.x - 1 || x == c.pos.x || x == c.pos.x+1) && (corners[j].score >= c.score))
                    goto cont;
            }

        }

        /*Check below (if there is anything below)*/
        if (c.pos.y != last_row && row_start[c.pos.y + 1] != -1 && point_below < sz) /*Nothing below*/
        {
            if (corners[point_below].pos.y < c.pos.y + 1)
                point_below = row_start[c.pos.y+1];

            /* Make point below point to one of the pixels belowthe current point, if it
               exists.*/
            for (; point_below < sz && corners[point_below].pos.y == c.pos.y+1 &&
                corners[point_below].pos.x < c.pos.x - 1; point_below++)
            {}

            for (j=point_below; j < sz && corners[j].pos.y == c.pos.y+1 && corners[j].pos.x <= c.pos.x + 1; j++)
            {
                int x = corners[j].pos.x;
                if ((x == c.pos.x - 1 || x == c.pos.x || x == c.pos.x+1) && (corners[j].score >= c.score))
                    goto cont;
            }
        }

        ret_nonmax.push_back(corners[i]);
        cont:
            ;
    }

    delete[] row_start;
}

}
