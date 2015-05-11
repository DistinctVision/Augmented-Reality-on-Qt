#include "AR/FastFeature.h"

namespace AR {

float FastFeature::_getShiTomasiScore_10(const Image<unsigned char>& im, const IPoint& pos)
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
        float dx = p2[x+1] - p2[x-1]; float dy = p1[x] - p3[x];
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = p1[x+1] - p1[x-1]; dy = s[x] - p2[x];
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = s[x+1] - s[x-1]; dy = n1[x] - p1[x];
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = n1[x+1] - n1[x-1]; dy = n2[x] - s[x];
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;

        dx = n2[x+1] - n2[x-1]; dy = n3[x] - n1[x];
        Dxx += dx * dx; Dyy += dy * dy; Dxy += dx * dy;
    }

    const float sum_Dxx_Dyy = Dxx + Dyy;

    float r = 0.04f * 0.5f * (sum_Dxx_Dyy - sqrtf(sum_Dxx_Dyy * sum_Dxx_Dyy - 4.0f * (Dxx * Dyy - Dxy * Dxy)));
    return r;
}

#if (FASTFEATURE_USE_NONMAXSUP == 1)
void FastFeature::fastNonmaxSuppression(const std::vector<FastFeature::FastCorner>& corners,
                                             std::vector<FastFeature::FastCorner>& ret_nonmax)
{
    ret_nonmax.clear();
    int last_row;
    static std::vector<int> row_start;
    int i, j;
    const int sz = (int)corners.size();

    /*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
    int point_above = 0;
    int point_below = 0;

    if(corners.empty())
        return;

    /* Find where each row begins
       (the corners are output in raster scan order). A beginning of -1 signifies
       that there are no corners on that row. */
    last_row = corners[corners.size() - 1].pos.y;
    if ((int)row_start.size() < last_row + 1)
        row_start.resize(last_row + 1);

    for(i=0; i < last_row+1; i++)
        row_start[i] = -1;

    {
        int prev_row = -1;
        for(i=0; i< (int)corners.size(); i++)
            if(corners[i].pos.y != prev_row)
            {
                row_start[corners[i].pos.y] = i;
                prev_row = corners[i].pos.y;
            }
    }

    for(i=0; i < sz; i++)
    {
        int score = corners[i].score;
        IPoint pos = corners[i].pos;

        /*Check left */
        if(i > 0)
            if(corners[i-1].pos.x == pos.x-1 && corners[i-1].pos.y == pos.y && (corners[i-1].score >= score))
                continue;

        /*Check right*/
        if(i < (sz - 1))
            if(corners[i+1].pos.x == pos.x+1 && corners[i+1].pos.y == pos.y && (corners[i+1].score >= score))
                continue;

        /*Check above (if there is a valid row above)*/
        if(pos.y != 0 && row_start[pos.y - 1] != -1)
        {
            /*Make sure that current point_above is one
              row above.*/
            if(corners[point_above].pos.y < pos.y - 1)
                point_above = row_start[pos.y-1];

            /*Make point_above point to the first of the pixels above the current point,
              if it exists.*/
            for(; corners[point_above].pos.y < pos.y && corners[point_above].pos.x < pos.x - 1; point_above++)
            {}


            for(j=point_above; corners[j].pos.y < pos.y && corners[j].pos.x <= pos.x + 1; j++)
            {
                int x = corners[j].pos.x;
                if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && (corners[j].score >= score))
                    goto cont;
            }

        }

        /*Check below (if there is anything below)*/
        if(pos.y != last_row && row_start[pos.y + 1] != -1 && point_below < sz) /*Nothing below*/
        {
            if(corners[point_below].pos.y < pos.y + 1)
                point_below = row_start[pos.y+1];

            /* Make point below point to one of the pixels belowthe current point, if it
               exists.*/
            for(; point_below < sz && corners[point_below].pos.y == pos.y+1 && corners[point_below].pos.x < pos.x - 1; point_below++)
            {}

            for(j=point_below; j < sz && corners[j].pos.y == pos.y+1 && corners[j].pos.x <= pos.x + 1; j++)
            {
                int x = corners[j].pos.x;
                if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && (corners[j].score >= score))
                    goto cont;
            }
        }

        ret_nonmax.push_back(corners[i]);
        cont:
            ;
    }
    //END
}

#endif

}
