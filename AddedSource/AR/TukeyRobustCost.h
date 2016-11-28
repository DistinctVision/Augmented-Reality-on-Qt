#ifndef TMATH_TUKEYROBUSTCOST_H
#define TMATH_TUKEYROBUSTCOST_H

#include <vector>
#include <algorithm>
#include <cmath>
#include "TMath/TTools.h"

namespace TMath {

class TukeyRobustCost
{
public:
    template <typename Type>
    inline static Type findSquareSigma(std::vector<Type>& squareErrors)
    {
        assert(squareErrors.size() > 0);
        std::size_t median_offset = squareErrors.size() / 2;
        std::nth_element(squareErrors.begin(), squareErrors.begin() + median_offset, squareErrors.end());
        Type squareMedian = squareErrors[median_offset];
        Type sigma = (Type)(4.6851 * 1.4826 * std::sqrt(squareMedian));
        //Type sigma = (Type)(4.6851 * 1.4826 * (1.0 + 5.0 / (squareErrors.size() * 2.0 - 6.0)) * std::sqrt(squareMedian));
        Type squareSigma = sigma * sigma;
        return squareSigma;
    }

    template <typename Type>
    inline static Type squareRootWeight(const Type& squareError, const Type& squareSigma)
    {
        if (squareError > squareSigma)
            return (Type)0;
        return (Type)1.0 - (squareError / squareSigma);
    }

    template <typename Type>
    inline static Type weight(const Type& squareError, const Type& squareSigma)
    {
        Type dSqrt = squareRootWeight(squareError, squareSigma);
        return dSqrt * dSqrt;
    }

    template <typename Type>
    inline static Type objectiveScore(const Type& squareError, const Type& squareSigma)
    {
        // NB All returned are scaled because
        // I'm not multiplying by sigmasquared/6.0
        if (squareError > squareSigma)
            return (Type)1.0;
        Type d = (Type)1.0 - squareError / squareSigma;
        return ((Type)1.0 - d * d * d);
    }
};

} // TMath

#endif // TNATH_TUKEYROBUSTCOST_H
