#include "FastCorner.h"

#include <vector>
#include <list>
#include <algorithm>

namespace AR {

void FastCorner::fast_corner_detect_10(const ImageRef<unsigned char>& im,
                                        const Point2i& begin, const Point2i& end,
                                        std::vector<Corner>& corners, int barrier)
{
    int beginx = std::max(begin.x, 3);
    int beginy = std::max(begin.y, 3);
    int endx = std::min(im.width() - 3, end.x + 1);
    int endy = std::min(im.height() - 3, end.y + 1);

    int stride = im.width();

    _make_fast_pixel_offset(stride);
    Corner corner;

    const unsigned char* data = im.data();
    //const bool* mdata = mask.data();

    const unsigned char* p;
    //const bool* mp;

    for(corner.pos.y=beginy; corner.pos.y<endy; ++corner.pos.y) {
        p = &data[stride * corner.pos.y];
        //mp = &mdata[stride * corner.pos.y];
        for(corner.pos.x=beginx; corner.pos.x<endx; ++corner.pos.x, ++p/*, ++mp*/)
        {
            //if (mp)
            //    continue;

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
            corners.push_back(corner);
        }
    }
}

int FastCorner::fast_corner_score_10(const unsigned char* p, int barrier)
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
