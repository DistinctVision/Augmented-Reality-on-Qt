#ifndef AR_ZMSSD_H
#define AR_ZMSSD_H

#include "TMath/TVector.h"
#include "Point2.h"
#include "Image.h"

namespace AR {

/// Zero Mean Sum of Squared Differences Cost
class ZMSSD
{
public:
    template <typename Type>
    static Type compare(const ImageRef<Type>& imageA, const Point2i& pointA,
                        const ImageRef<Type>& imageB, const Point2i& pointB,
                        const Point2i& size)
    {
        TMath_assert((size.x > 0) && (size.y > 0));
        TMath_assert((pointA.x >= 0) && (pointA.y >= 0));
        TMath_assert(((pointA.x + size.x) <= imageA.width()) && ((pointA.y + size.y) <= imageA.height()));
        TMath_assert((pointB.x >= 0) && (pointB.y >= 0));
        TMath_assert(((pointB.x + size.x) <= imageB.width()) && ((pointB.y + size.y) <= imageB.height()));
        const Type* ptrA = imageA.pointer(pointA);
        const Type* ptrB = imageB.pointer(pointB);
        int dif, sum = 0;
        Point2i p(0, 0);
        for (; p.y < size.y; ++p.y) {
            for ( ; p.x < size.x; ++p.x) {
                dif = ptrA[p.x] - ptrB[p.x];
                sum += dif * dif;
            }
            ptrA = &ptrA[imageA.width()];
            ptrB = &ptrB[imageB.width()];
        }
        return sum;
    }
};

}

#endif // AR_ZMSSD_H
