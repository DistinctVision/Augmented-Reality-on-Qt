#include <AR/FastFeature.h>

#include <vector>
#include <list>

namespace AR {

int FastFeature::_fast_pixel_ring[16];

void FastFeature::_make_fast_pixel_offset(int row_stride)
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

void FastFeature::fast_corner_detect_10(const Image<unsigned char>& im, const IPoint& begin, const IPoint& end,
                                        std::vector<FastCorner>& corners, int barrier, float minShiTomasiScore)
{
    corners.clear();
    FastCorner fastCorner;
    int x, y;
    int beginx = std::max(begin.x, 3);
    int beginy = std::max(begin.y, 3);
    int endx = std::min(im.width() - 3, end.x + 1);
    int endy = std::min(im.height() - 3, end.y + 1);

    int stride = im.width();

    _make_fast_pixel_offset(stride);

    for(y=beginy; y<endy; y++)
        for(x=beginx; x<endx; x++)
        {
            const unsigned char* p = im.data() + y*stride + x;

            int cb = *p + barrier;
            int c_b= *p - barrier;
        if(p[_fast_pixel_ring[0]] > cb)
         if(p[_fast_pixel_ring[1]] > cb)
          if(p[_fast_pixel_ring[2]] > cb)
           if(p[_fast_pixel_ring[3]] > cb)
            if(p[_fast_pixel_ring[4]] > cb)
             if(p[_fast_pixel_ring[5]] > cb)
              if(p[_fast_pixel_ring[6]] > cb)
               if(p[_fast_pixel_ring[7]] > cb)
                if(p[_fast_pixel_ring[8]] > cb)
                 if(p[_fast_pixel_ring[9]] > cb)
                  {}
                 else
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else if(p[_fast_pixel_ring[6]] < c_b)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[7]] < c_b)
                 if(p[_fast_pixel_ring[8]] < c_b)
                  if(p[_fast_pixel_ring[9]] < c_b)
                   if(p[_fast_pixel_ring[10]] < c_b)
                    if(p[_fast_pixel_ring[11]] < c_b)
                     if(p[_fast_pixel_ring[13]] < c_b)
                      if(p[_fast_pixel_ring[14]] < c_b)
                       if(p[_fast_pixel_ring[15]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else if(p[_fast_pixel_ring[5]] < c_b)
              if(p[_fast_pixel_ring[15]] > cb)
               if(p[_fast_pixel_ring[11]] > cb)
                if(p[_fast_pixel_ring[12]] > cb)
                 if(p[_fast_pixel_ring[13]] > cb)
                  if(p[_fast_pixel_ring[14]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[_fast_pixel_ring[11]] < c_b)
                if(p[_fast_pixel_ring[6]] < c_b)
                 if(p[_fast_pixel_ring[7]] < c_b)
                  if(p[_fast_pixel_ring[8]] < c_b)
                   if(p[_fast_pixel_ring[9]] < c_b)
                    if(p[_fast_pixel_ring[10]] < c_b)
                     if(p[_fast_pixel_ring[12]] < c_b)
                      if(p[_fast_pixel_ring[13]] < c_b)
                       if(p[_fast_pixel_ring[14]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               if(p[_fast_pixel_ring[6]] < c_b)
                if(p[_fast_pixel_ring[7]] < c_b)
                 if(p[_fast_pixel_ring[8]] < c_b)
                  if(p[_fast_pixel_ring[9]] < c_b)
                   if(p[_fast_pixel_ring[10]] < c_b)
                    if(p[_fast_pixel_ring[11]] < c_b)
                     if(p[_fast_pixel_ring[12]] < c_b)
                      if(p[_fast_pixel_ring[13]] < c_b)
                       if(p[_fast_pixel_ring[14]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[6]] < c_b)
                if(p[_fast_pixel_ring[7]] < c_b)
                 if(p[_fast_pixel_ring[8]] < c_b)
                  if(p[_fast_pixel_ring[9]] < c_b)
                   if(p[_fast_pixel_ring[10]] < c_b)
                    if(p[_fast_pixel_ring[12]] < c_b)
                     if(p[_fast_pixel_ring[13]] < c_b)
                      if(p[_fast_pixel_ring[14]] < c_b)
                       if(p[_fast_pixel_ring[15]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else if(p[_fast_pixel_ring[4]] < c_b)
             if(p[_fast_pixel_ring[14]] > cb)
              if(p[_fast_pixel_ring[10]] > cb)
               if(p[_fast_pixel_ring[11]] > cb)
                if(p[_fast_pixel_ring[12]] > cb)
                 if(p[_fast_pixel_ring[13]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       if(p[_fast_pixel_ring[9]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[_fast_pixel_ring[10]] < c_b)
               if(p[_fast_pixel_ring[5]] < c_b)
                if(p[_fast_pixel_ring[6]] < c_b)
                 if(p[_fast_pixel_ring[7]] < c_b)
                  if(p[_fast_pixel_ring[8]] < c_b)
                   if(p[_fast_pixel_ring[9]] < c_b)
                    if(p[_fast_pixel_ring[11]] < c_b)
                     if(p[_fast_pixel_ring[12]] < c_b)
                      if(p[_fast_pixel_ring[13]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[_fast_pixel_ring[14]] < c_b)
              if(p[_fast_pixel_ring[6]] < c_b)
               if(p[_fast_pixel_ring[7]] < c_b)
                if(p[_fast_pixel_ring[8]] < c_b)
                 if(p[_fast_pixel_ring[9]] < c_b)
                  if(p[_fast_pixel_ring[10]] < c_b)
                   if(p[_fast_pixel_ring[11]] < c_b)
                    if(p[_fast_pixel_ring[12]] < c_b)
                     if(p[_fast_pixel_ring[13]] < c_b)
                      if(p[_fast_pixel_ring[5]] < c_b)
                       {}
                      else
                       if(p[_fast_pixel_ring[15]] < c_b)
                        {}
                       else
                        continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              if(p[_fast_pixel_ring[5]] < c_b)
               if(p[_fast_pixel_ring[6]] < c_b)
                if(p[_fast_pixel_ring[7]] < c_b)
                 if(p[_fast_pixel_ring[8]] < c_b)
                  if(p[_fast_pixel_ring[9]] < c_b)
                   if(p[_fast_pixel_ring[10]] < c_b)
                    if(p[_fast_pixel_ring[11]] < c_b)
                     if(p[_fast_pixel_ring[12]] < c_b)
                      if(p[_fast_pixel_ring[13]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       if(p[_fast_pixel_ring[9]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[6]] < c_b)
               if(p[_fast_pixel_ring[7]] < c_b)
                if(p[_fast_pixel_ring[8]] < c_b)
                 if(p[_fast_pixel_ring[9]] < c_b)
                  if(p[_fast_pixel_ring[11]] < c_b)
                   if(p[_fast_pixel_ring[12]] < c_b)
                    if(p[_fast_pixel_ring[13]] < c_b)
                     if(p[_fast_pixel_ring[14]] < c_b)
                      if(p[_fast_pixel_ring[5]] < c_b)
                       {}
                      else
                       if(p[_fast_pixel_ring[15]] < c_b)
                        {}
                       else
                        continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
           else if(p[_fast_pixel_ring[3]] < c_b)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[6]] < c_b)
              if(p[_fast_pixel_ring[7]] < c_b)
               if(p[_fast_pixel_ring[8]] < c_b)
                if(p[_fast_pixel_ring[10]] < c_b)
                 if(p[_fast_pixel_ring[11]] < c_b)
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[4]] < c_b)
                     {}
                    else
                     if(p[_fast_pixel_ring[13]] < c_b)
                      if(p[_fast_pixel_ring[14]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[_fast_pixel_ring[13]] < c_b)
                     if(p[_fast_pixel_ring[14]] < c_b)
                      if(p[_fast_pixel_ring[15]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      if(p[_fast_pixel_ring[8]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[6]] < c_b)
              if(p[_fast_pixel_ring[7]] < c_b)
               if(p[_fast_pixel_ring[8]] < c_b)
                if(p[_fast_pixel_ring[10]] < c_b)
                 if(p[_fast_pixel_ring[11]] < c_b)
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[13]] < c_b)
                    if(p[_fast_pixel_ring[5]] < c_b)
                     if(p[_fast_pixel_ring[4]] < c_b)
                      {}
                     else
                      if(p[_fast_pixel_ring[14]] < c_b)
                       {}
                      else
                       continue;
                    else
                     if(p[_fast_pixel_ring[14]] < c_b)
                      if(p[_fast_pixel_ring[15]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
          else if(p[_fast_pixel_ring[2]] < c_b)
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[6]] < c_b)
             if(p[_fast_pixel_ring[7]] < c_b)
              if(p[_fast_pixel_ring[9]] < c_b)
               if(p[_fast_pixel_ring[10]] < c_b)
                if(p[_fast_pixel_ring[11]] < c_b)
                 if(p[_fast_pixel_ring[5]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[3]] < c_b)
                    {}
                   else
                    if(p[_fast_pixel_ring[12]] < c_b)
                     if(p[_fast_pixel_ring[13]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[12]] < c_b)
                    if(p[_fast_pixel_ring[13]] < c_b)
                     if(p[_fast_pixel_ring[14]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[13]] < c_b)
                    if(p[_fast_pixel_ring[14]] < c_b)
                     if(p[_fast_pixel_ring[15]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     if(p[_fast_pixel_ring[7]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[6]] < c_b)
             if(p[_fast_pixel_ring[7]] < c_b)
              if(p[_fast_pixel_ring[9]] < c_b)
               if(p[_fast_pixel_ring[10]] < c_b)
                if(p[_fast_pixel_ring[11]] < c_b)
                 if(p[_fast_pixel_ring[12]] < c_b)
                  if(p[_fast_pixel_ring[5]] < c_b)
                   if(p[_fast_pixel_ring[4]] < c_b)
                    if(p[_fast_pixel_ring[3]] < c_b)
                     {}
                    else
                     if(p[_fast_pixel_ring[13]] < c_b)
                      {}
                     else
                      continue;
                   else
                    if(p[_fast_pixel_ring[13]] < c_b)
                     if(p[_fast_pixel_ring[14]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[13]] < c_b)
                    if(p[_fast_pixel_ring[14]] < c_b)
                     if(p[_fast_pixel_ring[15]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
         else if(p[_fast_pixel_ring[1]] < c_b)
          if(p[_fast_pixel_ring[7]] > cb)
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[2]] > cb)
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[_fast_pixel_ring[7]] < c_b)
           if(p[_fast_pixel_ring[6]] < c_b)
            if(p[_fast_pixel_ring[8]] < c_b)
             if(p[_fast_pixel_ring[9]] < c_b)
              if(p[_fast_pixel_ring[10]] < c_b)
               if(p[_fast_pixel_ring[5]] < c_b)
                if(p[_fast_pixel_ring[4]] < c_b)
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[2]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[11]] < c_b)
                    if(p[_fast_pixel_ring[12]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[11]] < c_b)
                   if(p[_fast_pixel_ring[12]] < c_b)
                    if(p[_fast_pixel_ring[13]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[11]] < c_b)
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[13]] < c_b)
                    if(p[_fast_pixel_ring[14]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[11]] < c_b)
                 if(p[_fast_pixel_ring[12]] < c_b)
                  if(p[_fast_pixel_ring[13]] < c_b)
                   if(p[_fast_pixel_ring[14]] < c_b)
                    if(p[_fast_pixel_ring[15]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
         else
          if(p[_fast_pixel_ring[7]] > cb)
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[13]] > cb)
                 if(p[_fast_pixel_ring[14]] > cb)
                  if(p[_fast_pixel_ring[15]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[2]] > cb)
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[6]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[_fast_pixel_ring[7]] < c_b)
           if(p[_fast_pixel_ring[6]] < c_b)
            if(p[_fast_pixel_ring[8]] < c_b)
             if(p[_fast_pixel_ring[9]] < c_b)
              if(p[_fast_pixel_ring[10]] < c_b)
               if(p[_fast_pixel_ring[11]] < c_b)
                if(p[_fast_pixel_ring[5]] < c_b)
                 if(p[_fast_pixel_ring[4]] < c_b)
                  if(p[_fast_pixel_ring[3]] < c_b)
                   if(p[_fast_pixel_ring[2]] < c_b)
                    {}
                   else
                    if(p[_fast_pixel_ring[12]] < c_b)
                     {}
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[12]] < c_b)
                    if(p[_fast_pixel_ring[13]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[13]] < c_b)
                    if(p[_fast_pixel_ring[14]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[12]] < c_b)
                  if(p[_fast_pixel_ring[13]] < c_b)
                   if(p[_fast_pixel_ring[14]] < c_b)
                    if(p[_fast_pixel_ring[15]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
        else if(p[_fast_pixel_ring[0]] < c_b)
         if(p[_fast_pixel_ring[1]] > cb)
          if(p[_fast_pixel_ring[7]] > cb)
           if(p[_fast_pixel_ring[6]] > cb)
            if(p[_fast_pixel_ring[8]] > cb)
             if(p[_fast_pixel_ring[9]] > cb)
              if(p[_fast_pixel_ring[10]] > cb)
               if(p[_fast_pixel_ring[5]] > cb)
                if(p[_fast_pixel_ring[4]] > cb)
                 if(p[_fast_pixel_ring[3]] > cb)
                  if(p[_fast_pixel_ring[2]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[11]] > cb)
                    if(p[_fast_pixel_ring[12]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[11]] > cb)
                   if(p[_fast_pixel_ring[12]] > cb)
                    if(p[_fast_pixel_ring[13]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[11]] > cb)
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[13]] > cb)
                    if(p[_fast_pixel_ring[14]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[11]] > cb)
                 if(p[_fast_pixel_ring[12]] > cb)
                  if(p[_fast_pixel_ring[13]] > cb)
                   if(p[_fast_pixel_ring[14]] > cb)
                    if(p[_fast_pixel_ring[15]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[_fast_pixel_ring[7]] < c_b)
           if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[2]] < c_b)
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
         else if(p[_fast_pixel_ring[1]] < c_b)
          if(p[_fast_pixel_ring[2]] > cb)
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[6]] > cb)
             if(p[_fast_pixel_ring[7]] > cb)
              if(p[_fast_pixel_ring[9]] > cb)
               if(p[_fast_pixel_ring[10]] > cb)
                if(p[_fast_pixel_ring[11]] > cb)
                 if(p[_fast_pixel_ring[5]] > cb)
                  if(p[_fast_pixel_ring[4]] > cb)
                   if(p[_fast_pixel_ring[3]] > cb)
                    {}
                   else
                    if(p[_fast_pixel_ring[12]] > cb)
                     if(p[_fast_pixel_ring[13]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[12]] > cb)
                    if(p[_fast_pixel_ring[13]] > cb)
                     if(p[_fast_pixel_ring[14]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[13]] > cb)
                    if(p[_fast_pixel_ring[14]] > cb)
                     if(p[_fast_pixel_ring[15]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[_fast_pixel_ring[2]] < c_b)
           if(p[_fast_pixel_ring[3]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[6]] > cb)
              if(p[_fast_pixel_ring[7]] > cb)
               if(p[_fast_pixel_ring[8]] > cb)
                if(p[_fast_pixel_ring[10]] > cb)
                 if(p[_fast_pixel_ring[11]] > cb)
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[5]] > cb)
                    if(p[_fast_pixel_ring[4]] > cb)
                     {}
                    else
                     if(p[_fast_pixel_ring[13]] > cb)
                      if(p[_fast_pixel_ring[14]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[_fast_pixel_ring[13]] > cb)
                     if(p[_fast_pixel_ring[14]] > cb)
                      if(p[_fast_pixel_ring[15]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[_fast_pixel_ring[3]] < c_b)
            if(p[_fast_pixel_ring[4]] > cb)
             if(p[_fast_pixel_ring[14]] > cb)
              if(p[_fast_pixel_ring[6]] > cb)
               if(p[_fast_pixel_ring[7]] > cb)
                if(p[_fast_pixel_ring[8]] > cb)
                 if(p[_fast_pixel_ring[9]] > cb)
                  if(p[_fast_pixel_ring[10]] > cb)
                   if(p[_fast_pixel_ring[11]] > cb)
                    if(p[_fast_pixel_ring[12]] > cb)
                     if(p[_fast_pixel_ring[13]] > cb)
                      if(p[_fast_pixel_ring[5]] > cb)
                       {}
                      else
                       if(p[_fast_pixel_ring[15]] > cb)
                        {}
                       else
                        continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[_fast_pixel_ring[14]] < c_b)
              if(p[_fast_pixel_ring[10]] > cb)
               if(p[_fast_pixel_ring[5]] > cb)
                if(p[_fast_pixel_ring[6]] > cb)
                 if(p[_fast_pixel_ring[7]] > cb)
                  if(p[_fast_pixel_ring[8]] > cb)
                   if(p[_fast_pixel_ring[9]] > cb)
                    if(p[_fast_pixel_ring[11]] > cb)
                     if(p[_fast_pixel_ring[12]] > cb)
                      if(p[_fast_pixel_ring[13]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[_fast_pixel_ring[10]] < c_b)
               if(p[_fast_pixel_ring[11]] < c_b)
                if(p[_fast_pixel_ring[12]] < c_b)
                 if(p[_fast_pixel_ring[13]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       if(p[_fast_pixel_ring[9]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              if(p[_fast_pixel_ring[5]] > cb)
               if(p[_fast_pixel_ring[6]] > cb)
                if(p[_fast_pixel_ring[7]] > cb)
                 if(p[_fast_pixel_ring[8]] > cb)
                  if(p[_fast_pixel_ring[9]] > cb)
                   if(p[_fast_pixel_ring[10]] > cb)
                    if(p[_fast_pixel_ring[11]] > cb)
                     if(p[_fast_pixel_ring[12]] > cb)
                      if(p[_fast_pixel_ring[13]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else if(p[_fast_pixel_ring[4]] < c_b)
             if(p[_fast_pixel_ring[5]] > cb)
              if(p[_fast_pixel_ring[15]] < c_b)
               if(p[_fast_pixel_ring[11]] > cb)
                if(p[_fast_pixel_ring[6]] > cb)
                 if(p[_fast_pixel_ring[7]] > cb)
                  if(p[_fast_pixel_ring[8]] > cb)
                   if(p[_fast_pixel_ring[9]] > cb)
                    if(p[_fast_pixel_ring[10]] > cb)
                     if(p[_fast_pixel_ring[12]] > cb)
                      if(p[_fast_pixel_ring[13]] > cb)
                       if(p[_fast_pixel_ring[14]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[_fast_pixel_ring[11]] < c_b)
                if(p[_fast_pixel_ring[12]] < c_b)
                 if(p[_fast_pixel_ring[13]] < c_b)
                  if(p[_fast_pixel_ring[14]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               if(p[_fast_pixel_ring[6]] > cb)
                if(p[_fast_pixel_ring[7]] > cb)
                 if(p[_fast_pixel_ring[8]] > cb)
                  if(p[_fast_pixel_ring[9]] > cb)
                   if(p[_fast_pixel_ring[10]] > cb)
                    if(p[_fast_pixel_ring[11]] > cb)
                     if(p[_fast_pixel_ring[12]] > cb)
                      if(p[_fast_pixel_ring[13]] > cb)
                       if(p[_fast_pixel_ring[14]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else if(p[_fast_pixel_ring[5]] < c_b)
              if(p[_fast_pixel_ring[6]] > cb)
               if(p[_fast_pixel_ring[12]] > cb)
                if(p[_fast_pixel_ring[7]] > cb)
                 if(p[_fast_pixel_ring[8]] > cb)
                  if(p[_fast_pixel_ring[9]] > cb)
                   if(p[_fast_pixel_ring[10]] > cb)
                    if(p[_fast_pixel_ring[11]] > cb)
                     if(p[_fast_pixel_ring[13]] > cb)
                      if(p[_fast_pixel_ring[14]] > cb)
                       if(p[_fast_pixel_ring[15]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[_fast_pixel_ring[6]] < c_b)
               if(p[_fast_pixel_ring[7]] < c_b)
                if(p[_fast_pixel_ring[8]] < c_b)
                 if(p[_fast_pixel_ring[9]] < c_b)
                  {}
                 else
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[_fast_pixel_ring[11]] > cb)
               if(p[_fast_pixel_ring[6]] > cb)
                if(p[_fast_pixel_ring[7]] > cb)
                 if(p[_fast_pixel_ring[8]] > cb)
                  if(p[_fast_pixel_ring[9]] > cb)
                   if(p[_fast_pixel_ring[10]] > cb)
                    if(p[_fast_pixel_ring[12]] > cb)
                     if(p[_fast_pixel_ring[13]] > cb)
                      if(p[_fast_pixel_ring[14]] > cb)
                       if(p[_fast_pixel_ring[15]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[6]] > cb)
               if(p[_fast_pixel_ring[7]] > cb)
                if(p[_fast_pixel_ring[8]] > cb)
                 if(p[_fast_pixel_ring[9]] > cb)
                  if(p[_fast_pixel_ring[11]] > cb)
                   if(p[_fast_pixel_ring[12]] > cb)
                    if(p[_fast_pixel_ring[13]] > cb)
                     if(p[_fast_pixel_ring[14]] > cb)
                      if(p[_fast_pixel_ring[5]] > cb)
                       {}
                      else
                       if(p[_fast_pixel_ring[15]] > cb)
                        {}
                       else
                        continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       if(p[_fast_pixel_ring[9]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
           else
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[6]] > cb)
              if(p[_fast_pixel_ring[7]] > cb)
               if(p[_fast_pixel_ring[8]] > cb)
                if(p[_fast_pixel_ring[10]] > cb)
                 if(p[_fast_pixel_ring[11]] > cb)
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[13]] > cb)
                    if(p[_fast_pixel_ring[5]] > cb)
                     if(p[_fast_pixel_ring[4]] > cb)
                      {}
                     else
                      if(p[_fast_pixel_ring[14]] > cb)
                       {}
                      else
                       continue;
                    else
                     if(p[_fast_pixel_ring[14]] > cb)
                      if(p[_fast_pixel_ring[15]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      if(p[_fast_pixel_ring[8]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
          else
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[6]] > cb)
             if(p[_fast_pixel_ring[7]] > cb)
              if(p[_fast_pixel_ring[9]] > cb)
               if(p[_fast_pixel_ring[10]] > cb)
                if(p[_fast_pixel_ring[11]] > cb)
                 if(p[_fast_pixel_ring[12]] > cb)
                  if(p[_fast_pixel_ring[5]] > cb)
                   if(p[_fast_pixel_ring[4]] > cb)
                    if(p[_fast_pixel_ring[3]] > cb)
                     {}
                    else
                     if(p[_fast_pixel_ring[13]] > cb)
                      {}
                     else
                      continue;
                   else
                    if(p[_fast_pixel_ring[13]] > cb)
                     if(p[_fast_pixel_ring[14]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[13]] > cb)
                    if(p[_fast_pixel_ring[14]] > cb)
                     if(p[_fast_pixel_ring[15]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     if(p[_fast_pixel_ring[7]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
         else
          if(p[_fast_pixel_ring[7]] > cb)
           if(p[_fast_pixel_ring[6]] > cb)
            if(p[_fast_pixel_ring[8]] > cb)
             if(p[_fast_pixel_ring[9]] > cb)
              if(p[_fast_pixel_ring[10]] > cb)
               if(p[_fast_pixel_ring[11]] > cb)
                if(p[_fast_pixel_ring[5]] > cb)
                 if(p[_fast_pixel_ring[4]] > cb)
                  if(p[_fast_pixel_ring[3]] > cb)
                   if(p[_fast_pixel_ring[2]] > cb)
                    {}
                   else
                    if(p[_fast_pixel_ring[12]] > cb)
                     {}
                    else
                     continue;
                  else
                   if(p[_fast_pixel_ring[12]] > cb)
                    if(p[_fast_pixel_ring[13]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[13]] > cb)
                    if(p[_fast_pixel_ring[14]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[12]] > cb)
                  if(p[_fast_pixel_ring[13]] > cb)
                   if(p[_fast_pixel_ring[14]] > cb)
                    if(p[_fast_pixel_ring[15]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[_fast_pixel_ring[7]] < c_b)
           if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[11]] < c_b)
               if(p[_fast_pixel_ring[12]] < c_b)
                if(p[_fast_pixel_ring[13]] < c_b)
                 if(p[_fast_pixel_ring[14]] < c_b)
                  if(p[_fast_pixel_ring[15]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[2]] < c_b)
                 if(p[_fast_pixel_ring[3]] < c_b)
                  if(p[_fast_pixel_ring[4]] < c_b)
                   if(p[_fast_pixel_ring[5]] < c_b)
                    if(p[_fast_pixel_ring[6]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
        else
         if(p[_fast_pixel_ring[6]] > cb)
          if(p[_fast_pixel_ring[7]] > cb)
           if(p[_fast_pixel_ring[8]] > cb)
            if(p[_fast_pixel_ring[9]] > cb)
             if(p[_fast_pixel_ring[10]] > cb)
              if(p[_fast_pixel_ring[5]] > cb)
               if(p[_fast_pixel_ring[4]] > cb)
                if(p[_fast_pixel_ring[3]] > cb)
                 if(p[_fast_pixel_ring[2]] > cb)
                  if(p[_fast_pixel_ring[1]] > cb)
                   {}
                  else
                   if(p[_fast_pixel_ring[11]] > cb)
                    {}
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[11]] > cb)
                   if(p[_fast_pixel_ring[12]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[11]] > cb)
                  if(p[_fast_pixel_ring[12]] > cb)
                   if(p[_fast_pixel_ring[13]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[11]] > cb)
                 if(p[_fast_pixel_ring[12]] > cb)
                  if(p[_fast_pixel_ring[13]] > cb)
                   if(p[_fast_pixel_ring[14]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[_fast_pixel_ring[11]] > cb)
                if(p[_fast_pixel_ring[12]] > cb)
                 if(p[_fast_pixel_ring[13]] > cb)
                  if(p[_fast_pixel_ring[14]] > cb)
                   if(p[_fast_pixel_ring[15]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
         else if(p[_fast_pixel_ring[6]] < c_b)
          if(p[_fast_pixel_ring[7]] < c_b)
           if(p[_fast_pixel_ring[8]] < c_b)
            if(p[_fast_pixel_ring[9]] < c_b)
             if(p[_fast_pixel_ring[10]] < c_b)
              if(p[_fast_pixel_ring[5]] < c_b)
               if(p[_fast_pixel_ring[4]] < c_b)
                if(p[_fast_pixel_ring[3]] < c_b)
                 if(p[_fast_pixel_ring[2]] < c_b)
                  if(p[_fast_pixel_ring[1]] < c_b)
                   {}
                  else
                   if(p[_fast_pixel_ring[11]] < c_b)
                    {}
                   else
                    continue;
                 else
                  if(p[_fast_pixel_ring[11]] < c_b)
                   if(p[_fast_pixel_ring[12]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[_fast_pixel_ring[11]] < c_b)
                  if(p[_fast_pixel_ring[12]] < c_b)
                   if(p[_fast_pixel_ring[13]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[_fast_pixel_ring[11]] < c_b)
                 if(p[_fast_pixel_ring[12]] < c_b)
                  if(p[_fast_pixel_ring[13]] < c_b)
                   if(p[_fast_pixel_ring[14]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[_fast_pixel_ring[11]] < c_b)
                if(p[_fast_pixel_ring[12]] < c_b)
                 if(p[_fast_pixel_ring[13]] < c_b)
                  if(p[_fast_pixel_ring[14]] < c_b)
                   if(p[_fast_pixel_ring[15]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           continue;
         else
          continue;
            fastCorner.pos.set(x, y);
            fastCorner.scoreShiTomasi = _getShiTomasiScore_10(im, fastCorner.pos);
            if (fastCorner.scoreShiTomasi > minShiTomasiScore) {
#if (FASTFEATURE_USE_NONMAXSUP == 1)
                fastCorner.score = _fast_corner_score_10(p, barrier);
#endif
                corners.push_back(fastCorner);
            }
        }
}

int FastFeature::_fast_corner_score_10(const unsigned char* p, int barrier)
{
    int bmin = barrier;
    int bmax = 255;
    int b = (bmax + bmin) / 2;
    /*Compute the score using binary search*/
    for(;;)
    {
        int cb = *p + b;
        int c_b= *p - b;


        if( p[_fast_pixel_ring[0]] > cb)
         if( p[_fast_pixel_ring[1]] > cb)
          if( p[_fast_pixel_ring[2]] > cb)
           if( p[_fast_pixel_ring[3]] > cb)
            if( p[_fast_pixel_ring[4]] > cb)
             if( p[_fast_pixel_ring[5]] > cb)
              if( p[_fast_pixel_ring[6]] > cb)
               if( p[_fast_pixel_ring[7]] > cb)
                if( p[_fast_pixel_ring[8]] > cb)
                 if( p[_fast_pixel_ring[9]] > cb)
                  goto is_a_corner;
                 else
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else if( p[_fast_pixel_ring[6]] < c_b)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[7]] < c_b)
                 if( p[_fast_pixel_ring[8]] < c_b)
                  if( p[_fast_pixel_ring[9]] < c_b)
                   if( p[_fast_pixel_ring[10]] < c_b)
                    if( p[_fast_pixel_ring[11]] < c_b)
                     if( p[_fast_pixel_ring[13]] < c_b)
                      if( p[_fast_pixel_ring[14]] < c_b)
                       if( p[_fast_pixel_ring[15]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else if( p[_fast_pixel_ring[5]] < c_b)
              if( p[_fast_pixel_ring[15]] > cb)
               if( p[_fast_pixel_ring[11]] > cb)
                if( p[_fast_pixel_ring[12]] > cb)
                 if( p[_fast_pixel_ring[13]] > cb)
                  if( p[_fast_pixel_ring[14]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else if( p[_fast_pixel_ring[11]] < c_b)
                if( p[_fast_pixel_ring[6]] < c_b)
                 if( p[_fast_pixel_ring[7]] < c_b)
                  if( p[_fast_pixel_ring[8]] < c_b)
                   if( p[_fast_pixel_ring[9]] < c_b)
                    if( p[_fast_pixel_ring[10]] < c_b)
                     if( p[_fast_pixel_ring[12]] < c_b)
                      if( p[_fast_pixel_ring[13]] < c_b)
                       if( p[_fast_pixel_ring[14]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[6]] < c_b)
                if( p[_fast_pixel_ring[7]] < c_b)
                 if( p[_fast_pixel_ring[8]] < c_b)
                  if( p[_fast_pixel_ring[9]] < c_b)
                   if( p[_fast_pixel_ring[10]] < c_b)
                    if( p[_fast_pixel_ring[11]] < c_b)
                     if( p[_fast_pixel_ring[12]] < c_b)
                      if( p[_fast_pixel_ring[13]] < c_b)
                       if( p[_fast_pixel_ring[14]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[6]] < c_b)
                if( p[_fast_pixel_ring[7]] < c_b)
                 if( p[_fast_pixel_ring[8]] < c_b)
                  if( p[_fast_pixel_ring[9]] < c_b)
                   if( p[_fast_pixel_ring[10]] < c_b)
                    if( p[_fast_pixel_ring[12]] < c_b)
                     if( p[_fast_pixel_ring[13]] < c_b)
                      if( p[_fast_pixel_ring[14]] < c_b)
                       if( p[_fast_pixel_ring[15]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
            else if( p[_fast_pixel_ring[4]] < c_b)
             if( p[_fast_pixel_ring[14]] > cb)
              if( p[_fast_pixel_ring[10]] > cb)
               if( p[_fast_pixel_ring[11]] > cb)
                if( p[_fast_pixel_ring[12]] > cb)
                 if( p[_fast_pixel_ring[13]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       if( p[_fast_pixel_ring[9]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else if( p[_fast_pixel_ring[10]] < c_b)
               if( p[_fast_pixel_ring[5]] < c_b)
                if( p[_fast_pixel_ring[6]] < c_b)
                 if( p[_fast_pixel_ring[7]] < c_b)
                  if( p[_fast_pixel_ring[8]] < c_b)
                   if( p[_fast_pixel_ring[9]] < c_b)
                    if( p[_fast_pixel_ring[11]] < c_b)
                     if( p[_fast_pixel_ring[12]] < c_b)
                      if( p[_fast_pixel_ring[13]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else if( p[_fast_pixel_ring[14]] < c_b)
              if( p[_fast_pixel_ring[6]] < c_b)
               if( p[_fast_pixel_ring[7]] < c_b)
                if( p[_fast_pixel_ring[8]] < c_b)
                 if( p[_fast_pixel_ring[9]] < c_b)
                  if( p[_fast_pixel_ring[10]] < c_b)
                   if( p[_fast_pixel_ring[11]] < c_b)
                    if( p[_fast_pixel_ring[12]] < c_b)
                     if( p[_fast_pixel_ring[13]] < c_b)
                      if( p[_fast_pixel_ring[5]] < c_b)
                       goto is_a_corner;
                      else
                       if( p[_fast_pixel_ring[15]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              if( p[_fast_pixel_ring[5]] < c_b)
               if( p[_fast_pixel_ring[6]] < c_b)
                if( p[_fast_pixel_ring[7]] < c_b)
                 if( p[_fast_pixel_ring[8]] < c_b)
                  if( p[_fast_pixel_ring[9]] < c_b)
                   if( p[_fast_pixel_ring[10]] < c_b)
                    if( p[_fast_pixel_ring[11]] < c_b)
                     if( p[_fast_pixel_ring[12]] < c_b)
                      if( p[_fast_pixel_ring[13]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
            else
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       if( p[_fast_pixel_ring[9]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[6]] < c_b)
               if( p[_fast_pixel_ring[7]] < c_b)
                if( p[_fast_pixel_ring[8]] < c_b)
                 if( p[_fast_pixel_ring[9]] < c_b)
                  if( p[_fast_pixel_ring[11]] < c_b)
                   if( p[_fast_pixel_ring[12]] < c_b)
                    if( p[_fast_pixel_ring[13]] < c_b)
                     if( p[_fast_pixel_ring[14]] < c_b)
                      if( p[_fast_pixel_ring[5]] < c_b)
                       goto is_a_corner;
                      else
                       if( p[_fast_pixel_ring[15]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
           else if( p[_fast_pixel_ring[3]] < c_b)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[6]] < c_b)
              if( p[_fast_pixel_ring[7]] < c_b)
               if( p[_fast_pixel_ring[8]] < c_b)
                if( p[_fast_pixel_ring[10]] < c_b)
                 if( p[_fast_pixel_ring[11]] < c_b)
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[4]] < c_b)
                     goto is_a_corner;
                    else
                     if( p[_fast_pixel_ring[13]] < c_b)
                      if( p[_fast_pixel_ring[14]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    if( p[_fast_pixel_ring[13]] < c_b)
                     if( p[_fast_pixel_ring[14]] < c_b)
                      if( p[_fast_pixel_ring[15]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      if( p[_fast_pixel_ring[8]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[6]] < c_b)
              if( p[_fast_pixel_ring[7]] < c_b)
               if( p[_fast_pixel_ring[8]] < c_b)
                if( p[_fast_pixel_ring[10]] < c_b)
                 if( p[_fast_pixel_ring[11]] < c_b)
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[13]] < c_b)
                    if( p[_fast_pixel_ring[5]] < c_b)
                     if( p[_fast_pixel_ring[4]] < c_b)
                      goto is_a_corner;
                     else
                      if( p[_fast_pixel_ring[14]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                    else
                     if( p[_fast_pixel_ring[14]] < c_b)
                      if( p[_fast_pixel_ring[15]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
          else if( p[_fast_pixel_ring[2]] < c_b)
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[6]] < c_b)
             if( p[_fast_pixel_ring[7]] < c_b)
              if( p[_fast_pixel_ring[9]] < c_b)
               if( p[_fast_pixel_ring[10]] < c_b)
                if( p[_fast_pixel_ring[11]] < c_b)
                 if( p[_fast_pixel_ring[5]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[3]] < c_b)
                    goto is_a_corner;
                   else
                    if( p[_fast_pixel_ring[12]] < c_b)
                     if( p[_fast_pixel_ring[13]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[12]] < c_b)
                    if( p[_fast_pixel_ring[13]] < c_b)
                     if( p[_fast_pixel_ring[14]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[13]] < c_b)
                    if( p[_fast_pixel_ring[14]] < c_b)
                     if( p[_fast_pixel_ring[15]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     if( p[_fast_pixel_ring[7]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[6]] < c_b)
             if( p[_fast_pixel_ring[7]] < c_b)
              if( p[_fast_pixel_ring[9]] < c_b)
               if( p[_fast_pixel_ring[10]] < c_b)
                if( p[_fast_pixel_ring[11]] < c_b)
                 if( p[_fast_pixel_ring[12]] < c_b)
                  if( p[_fast_pixel_ring[5]] < c_b)
                   if( p[_fast_pixel_ring[4]] < c_b)
                    if( p[_fast_pixel_ring[3]] < c_b)
                     goto is_a_corner;
                    else
                     if( p[_fast_pixel_ring[13]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    if( p[_fast_pixel_ring[13]] < c_b)
                     if( p[_fast_pixel_ring[14]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[13]] < c_b)
                    if( p[_fast_pixel_ring[14]] < c_b)
                     if( p[_fast_pixel_ring[15]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
         else if( p[_fast_pixel_ring[1]] < c_b)
          if( p[_fast_pixel_ring[7]] > cb)
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[2]] > cb)
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else if( p[_fast_pixel_ring[7]] < c_b)
           if( p[_fast_pixel_ring[6]] < c_b)
            if( p[_fast_pixel_ring[8]] < c_b)
             if( p[_fast_pixel_ring[9]] < c_b)
              if( p[_fast_pixel_ring[10]] < c_b)
               if( p[_fast_pixel_ring[5]] < c_b)
                if( p[_fast_pixel_ring[4]] < c_b)
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[2]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[11]] < c_b)
                    if( p[_fast_pixel_ring[12]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[11]] < c_b)
                   if( p[_fast_pixel_ring[12]] < c_b)
                    if( p[_fast_pixel_ring[13]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[11]] < c_b)
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[13]] < c_b)
                    if( p[_fast_pixel_ring[14]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[11]] < c_b)
                 if( p[_fast_pixel_ring[12]] < c_b)
                  if( p[_fast_pixel_ring[13]] < c_b)
                   if( p[_fast_pixel_ring[14]] < c_b)
                    if( p[_fast_pixel_ring[15]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
         else
          if( p[_fast_pixel_ring[7]] > cb)
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[13]] > cb)
                 if( p[_fast_pixel_ring[14]] > cb)
                  if( p[_fast_pixel_ring[15]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[2]] > cb)
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[6]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else if( p[_fast_pixel_ring[7]] < c_b)
           if( p[_fast_pixel_ring[6]] < c_b)
            if( p[_fast_pixel_ring[8]] < c_b)
             if( p[_fast_pixel_ring[9]] < c_b)
              if( p[_fast_pixel_ring[10]] < c_b)
               if( p[_fast_pixel_ring[11]] < c_b)
                if( p[_fast_pixel_ring[5]] < c_b)
                 if( p[_fast_pixel_ring[4]] < c_b)
                  if( p[_fast_pixel_ring[3]] < c_b)
                   if( p[_fast_pixel_ring[2]] < c_b)
                    goto is_a_corner;
                   else
                    if( p[_fast_pixel_ring[12]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[12]] < c_b)
                    if( p[_fast_pixel_ring[13]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[13]] < c_b)
                    if( p[_fast_pixel_ring[14]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[12]] < c_b)
                  if( p[_fast_pixel_ring[13]] < c_b)
                   if( p[_fast_pixel_ring[14]] < c_b)
                    if( p[_fast_pixel_ring[15]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
        else if( p[_fast_pixel_ring[0]] < c_b)
         if( p[_fast_pixel_ring[1]] > cb)
          if( p[_fast_pixel_ring[7]] > cb)
           if( p[_fast_pixel_ring[6]] > cb)
            if( p[_fast_pixel_ring[8]] > cb)
             if( p[_fast_pixel_ring[9]] > cb)
              if( p[_fast_pixel_ring[10]] > cb)
               if( p[_fast_pixel_ring[5]] > cb)
                if( p[_fast_pixel_ring[4]] > cb)
                 if( p[_fast_pixel_ring[3]] > cb)
                  if( p[_fast_pixel_ring[2]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[11]] > cb)
                    if( p[_fast_pixel_ring[12]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[11]] > cb)
                   if( p[_fast_pixel_ring[12]] > cb)
                    if( p[_fast_pixel_ring[13]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[11]] > cb)
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[13]] > cb)
                    if( p[_fast_pixel_ring[14]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[11]] > cb)
                 if( p[_fast_pixel_ring[12]] > cb)
                  if( p[_fast_pixel_ring[13]] > cb)
                   if( p[_fast_pixel_ring[14]] > cb)
                    if( p[_fast_pixel_ring[15]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else if( p[_fast_pixel_ring[7]] < c_b)
           if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[2]] < c_b)
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
         else if( p[_fast_pixel_ring[1]] < c_b)
          if( p[_fast_pixel_ring[2]] > cb)
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[6]] > cb)
             if( p[_fast_pixel_ring[7]] > cb)
              if( p[_fast_pixel_ring[9]] > cb)
               if( p[_fast_pixel_ring[10]] > cb)
                if( p[_fast_pixel_ring[11]] > cb)
                 if( p[_fast_pixel_ring[5]] > cb)
                  if( p[_fast_pixel_ring[4]] > cb)
                   if( p[_fast_pixel_ring[3]] > cb)
                    goto is_a_corner;
                   else
                    if( p[_fast_pixel_ring[12]] > cb)
                     if( p[_fast_pixel_ring[13]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[12]] > cb)
                    if( p[_fast_pixel_ring[13]] > cb)
                     if( p[_fast_pixel_ring[14]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[13]] > cb)
                    if( p[_fast_pixel_ring[14]] > cb)
                     if( p[_fast_pixel_ring[15]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else if( p[_fast_pixel_ring[2]] < c_b)
           if( p[_fast_pixel_ring[3]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[6]] > cb)
              if( p[_fast_pixel_ring[7]] > cb)
               if( p[_fast_pixel_ring[8]] > cb)
                if( p[_fast_pixel_ring[10]] > cb)
                 if( p[_fast_pixel_ring[11]] > cb)
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[5]] > cb)
                    if( p[_fast_pixel_ring[4]] > cb)
                     goto is_a_corner;
                    else
                     if( p[_fast_pixel_ring[13]] > cb)
                      if( p[_fast_pixel_ring[14]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    if( p[_fast_pixel_ring[13]] > cb)
                     if( p[_fast_pixel_ring[14]] > cb)
                      if( p[_fast_pixel_ring[15]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else if( p[_fast_pixel_ring[3]] < c_b)
            if( p[_fast_pixel_ring[4]] > cb)
             if( p[_fast_pixel_ring[14]] > cb)
              if( p[_fast_pixel_ring[6]] > cb)
               if( p[_fast_pixel_ring[7]] > cb)
                if( p[_fast_pixel_ring[8]] > cb)
                 if( p[_fast_pixel_ring[9]] > cb)
                  if( p[_fast_pixel_ring[10]] > cb)
                   if( p[_fast_pixel_ring[11]] > cb)
                    if( p[_fast_pixel_ring[12]] > cb)
                     if( p[_fast_pixel_ring[13]] > cb)
                      if( p[_fast_pixel_ring[5]] > cb)
                       goto is_a_corner;
                      else
                       if( p[_fast_pixel_ring[15]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else if( p[_fast_pixel_ring[14]] < c_b)
              if( p[_fast_pixel_ring[10]] > cb)
               if( p[_fast_pixel_ring[5]] > cb)
                if( p[_fast_pixel_ring[6]] > cb)
                 if( p[_fast_pixel_ring[7]] > cb)
                  if( p[_fast_pixel_ring[8]] > cb)
                   if( p[_fast_pixel_ring[9]] > cb)
                    if( p[_fast_pixel_ring[11]] > cb)
                     if( p[_fast_pixel_ring[12]] > cb)
                      if( p[_fast_pixel_ring[13]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else if( p[_fast_pixel_ring[10]] < c_b)
               if( p[_fast_pixel_ring[11]] < c_b)
                if( p[_fast_pixel_ring[12]] < c_b)
                 if( p[_fast_pixel_ring[13]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       if( p[_fast_pixel_ring[9]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              if( p[_fast_pixel_ring[5]] > cb)
               if( p[_fast_pixel_ring[6]] > cb)
                if( p[_fast_pixel_ring[7]] > cb)
                 if( p[_fast_pixel_ring[8]] > cb)
                  if( p[_fast_pixel_ring[9]] > cb)
                   if( p[_fast_pixel_ring[10]] > cb)
                    if( p[_fast_pixel_ring[11]] > cb)
                     if( p[_fast_pixel_ring[12]] > cb)
                      if( p[_fast_pixel_ring[13]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
            else if( p[_fast_pixel_ring[4]] < c_b)
             if( p[_fast_pixel_ring[5]] > cb)
              if( p[_fast_pixel_ring[15]] < c_b)
               if( p[_fast_pixel_ring[11]] > cb)
                if( p[_fast_pixel_ring[6]] > cb)
                 if( p[_fast_pixel_ring[7]] > cb)
                  if( p[_fast_pixel_ring[8]] > cb)
                   if( p[_fast_pixel_ring[9]] > cb)
                    if( p[_fast_pixel_ring[10]] > cb)
                     if( p[_fast_pixel_ring[12]] > cb)
                      if( p[_fast_pixel_ring[13]] > cb)
                       if( p[_fast_pixel_ring[14]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else if( p[_fast_pixel_ring[11]] < c_b)
                if( p[_fast_pixel_ring[12]] < c_b)
                 if( p[_fast_pixel_ring[13]] < c_b)
                  if( p[_fast_pixel_ring[14]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[6]] > cb)
                if( p[_fast_pixel_ring[7]] > cb)
                 if( p[_fast_pixel_ring[8]] > cb)
                  if( p[_fast_pixel_ring[9]] > cb)
                   if( p[_fast_pixel_ring[10]] > cb)
                    if( p[_fast_pixel_ring[11]] > cb)
                     if( p[_fast_pixel_ring[12]] > cb)
                      if( p[_fast_pixel_ring[13]] > cb)
                       if( p[_fast_pixel_ring[14]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else if( p[_fast_pixel_ring[5]] < c_b)
              if( p[_fast_pixel_ring[6]] > cb)
               if( p[_fast_pixel_ring[12]] > cb)
                if( p[_fast_pixel_ring[7]] > cb)
                 if( p[_fast_pixel_ring[8]] > cb)
                  if( p[_fast_pixel_ring[9]] > cb)
                   if( p[_fast_pixel_ring[10]] > cb)
                    if( p[_fast_pixel_ring[11]] > cb)
                     if( p[_fast_pixel_ring[13]] > cb)
                      if( p[_fast_pixel_ring[14]] > cb)
                       if( p[_fast_pixel_ring[15]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else if( p[_fast_pixel_ring[6]] < c_b)
               if( p[_fast_pixel_ring[7]] < c_b)
                if( p[_fast_pixel_ring[8]] < c_b)
                 if( p[_fast_pixel_ring[9]] < c_b)
                  goto is_a_corner;
                 else
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else
              if( p[_fast_pixel_ring[11]] > cb)
               if( p[_fast_pixel_ring[6]] > cb)
                if( p[_fast_pixel_ring[7]] > cb)
                 if( p[_fast_pixel_ring[8]] > cb)
                  if( p[_fast_pixel_ring[9]] > cb)
                   if( p[_fast_pixel_ring[10]] > cb)
                    if( p[_fast_pixel_ring[12]] > cb)
                     if( p[_fast_pixel_ring[13]] > cb)
                      if( p[_fast_pixel_ring[14]] > cb)
                       if( p[_fast_pixel_ring[15]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
            else
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[6]] > cb)
               if( p[_fast_pixel_ring[7]] > cb)
                if( p[_fast_pixel_ring[8]] > cb)
                 if( p[_fast_pixel_ring[9]] > cb)
                  if( p[_fast_pixel_ring[11]] > cb)
                   if( p[_fast_pixel_ring[12]] > cb)
                    if( p[_fast_pixel_ring[13]] > cb)
                     if( p[_fast_pixel_ring[14]] > cb)
                      if( p[_fast_pixel_ring[5]] > cb)
                       goto is_a_corner;
                      else
                       if( p[_fast_pixel_ring[15]] > cb)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       if( p[_fast_pixel_ring[9]] < c_b)
                        goto is_a_corner;
                       else
                        goto is_not_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
           else
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[6]] > cb)
              if( p[_fast_pixel_ring[7]] > cb)
               if( p[_fast_pixel_ring[8]] > cb)
                if( p[_fast_pixel_ring[10]] > cb)
                 if( p[_fast_pixel_ring[11]] > cb)
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[13]] > cb)
                    if( p[_fast_pixel_ring[5]] > cb)
                     if( p[_fast_pixel_ring[4]] > cb)
                      goto is_a_corner;
                     else
                      if( p[_fast_pixel_ring[14]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                    else
                     if( p[_fast_pixel_ring[14]] > cb)
                      if( p[_fast_pixel_ring[15]] > cb)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      if( p[_fast_pixel_ring[8]] < c_b)
                       goto is_a_corner;
                      else
                       goto is_not_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
          else
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[6]] > cb)
             if( p[_fast_pixel_ring[7]] > cb)
              if( p[_fast_pixel_ring[9]] > cb)
               if( p[_fast_pixel_ring[10]] > cb)
                if( p[_fast_pixel_ring[11]] > cb)
                 if( p[_fast_pixel_ring[12]] > cb)
                  if( p[_fast_pixel_ring[5]] > cb)
                   if( p[_fast_pixel_ring[4]] > cb)
                    if( p[_fast_pixel_ring[3]] > cb)
                     goto is_a_corner;
                    else
                     if( p[_fast_pixel_ring[13]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                   else
                    if( p[_fast_pixel_ring[13]] > cb)
                     if( p[_fast_pixel_ring[14]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[13]] > cb)
                    if( p[_fast_pixel_ring[14]] > cb)
                     if( p[_fast_pixel_ring[15]] > cb)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     if( p[_fast_pixel_ring[7]] < c_b)
                      goto is_a_corner;
                     else
                      goto is_not_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
         else
          if( p[_fast_pixel_ring[7]] > cb)
           if( p[_fast_pixel_ring[6]] > cb)
            if( p[_fast_pixel_ring[8]] > cb)
             if( p[_fast_pixel_ring[9]] > cb)
              if( p[_fast_pixel_ring[10]] > cb)
               if( p[_fast_pixel_ring[11]] > cb)
                if( p[_fast_pixel_ring[5]] > cb)
                 if( p[_fast_pixel_ring[4]] > cb)
                  if( p[_fast_pixel_ring[3]] > cb)
                   if( p[_fast_pixel_ring[2]] > cb)
                    goto is_a_corner;
                   else
                    if( p[_fast_pixel_ring[12]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                  else
                   if( p[_fast_pixel_ring[12]] > cb)
                    if( p[_fast_pixel_ring[13]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[13]] > cb)
                    if( p[_fast_pixel_ring[14]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[12]] > cb)
                  if( p[_fast_pixel_ring[13]] > cb)
                   if( p[_fast_pixel_ring[14]] > cb)
                    if( p[_fast_pixel_ring[15]] > cb)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else if( p[_fast_pixel_ring[7]] < c_b)
           if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[11]] < c_b)
               if( p[_fast_pixel_ring[12]] < c_b)
                if( p[_fast_pixel_ring[13]] < c_b)
                 if( p[_fast_pixel_ring[14]] < c_b)
                  if( p[_fast_pixel_ring[15]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[2]] < c_b)
                 if( p[_fast_pixel_ring[3]] < c_b)
                  if( p[_fast_pixel_ring[4]] < c_b)
                   if( p[_fast_pixel_ring[5]] < c_b)
                    if( p[_fast_pixel_ring[6]] < c_b)
                     goto is_a_corner;
                    else
                     goto is_not_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
        else
         if( p[_fast_pixel_ring[6]] > cb)
          if( p[_fast_pixel_ring[7]] > cb)
           if( p[_fast_pixel_ring[8]] > cb)
            if( p[_fast_pixel_ring[9]] > cb)
             if( p[_fast_pixel_ring[10]] > cb)
              if( p[_fast_pixel_ring[5]] > cb)
               if( p[_fast_pixel_ring[4]] > cb)
                if( p[_fast_pixel_ring[3]] > cb)
                 if( p[_fast_pixel_ring[2]] > cb)
                  if( p[_fast_pixel_ring[1]] > cb)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[11]] > cb)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[11]] > cb)
                   if( p[_fast_pixel_ring[12]] > cb)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[11]] > cb)
                  if( p[_fast_pixel_ring[12]] > cb)
                   if( p[_fast_pixel_ring[13]] > cb)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[11]] > cb)
                 if( p[_fast_pixel_ring[12]] > cb)
                  if( p[_fast_pixel_ring[13]] > cb)
                   if( p[_fast_pixel_ring[14]] > cb)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[11]] > cb)
                if( p[_fast_pixel_ring[12]] > cb)
                 if( p[_fast_pixel_ring[13]] > cb)
                  if( p[_fast_pixel_ring[14]] > cb)
                   if( p[_fast_pixel_ring[15]] > cb)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
         else if( p[_fast_pixel_ring[6]] < c_b)
          if( p[_fast_pixel_ring[7]] < c_b)
           if( p[_fast_pixel_ring[8]] < c_b)
            if( p[_fast_pixel_ring[9]] < c_b)
             if( p[_fast_pixel_ring[10]] < c_b)
              if( p[_fast_pixel_ring[5]] < c_b)
               if( p[_fast_pixel_ring[4]] < c_b)
                if( p[_fast_pixel_ring[3]] < c_b)
                 if( p[_fast_pixel_ring[2]] < c_b)
                  if( p[_fast_pixel_ring[1]] < c_b)
                   goto is_a_corner;
                  else
                   if( p[_fast_pixel_ring[11]] < c_b)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                 else
                  if( p[_fast_pixel_ring[11]] < c_b)
                   if( p[_fast_pixel_ring[12]] < c_b)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                else
                 if( p[_fast_pixel_ring[11]] < c_b)
                  if( p[_fast_pixel_ring[12]] < c_b)
                   if( p[_fast_pixel_ring[13]] < c_b)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
               else
                if( p[_fast_pixel_ring[11]] < c_b)
                 if( p[_fast_pixel_ring[12]] < c_b)
                  if( p[_fast_pixel_ring[13]] < c_b)
                   if( p[_fast_pixel_ring[14]] < c_b)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
              else
               if( p[_fast_pixel_ring[11]] < c_b)
                if( p[_fast_pixel_ring[12]] < c_b)
                 if( p[_fast_pixel_ring[13]] < c_b)
                  if( p[_fast_pixel_ring[14]] < c_b)
                   if( p[_fast_pixel_ring[15]] < c_b)
                    goto is_a_corner;
                   else
                    goto is_not_a_corner;
                  else
                   goto is_not_a_corner;
                 else
                  goto is_not_a_corner;
                else
                 goto is_not_a_corner;
               else
                goto is_not_a_corner;
             else
              goto is_not_a_corner;
            else
             goto is_not_a_corner;
           else
            goto is_not_a_corner;
          else
           goto is_not_a_corner;
         else
          goto is_not_a_corner;

        is_a_corner:
            bmin=b;
            goto end_if;

        is_not_a_corner:
            bmax=b;
            goto end_if;

        end_if:

        if(bmin == bmax - 1 || bmin == bmax)
            return bmin;
        b = (bmin + bmax) / 2;
    }

    return 0;
}

}
