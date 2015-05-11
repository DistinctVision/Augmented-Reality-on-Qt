#ifndef OTHERESTIMATORS_H
#define OTHERESTIMATORS_H

#include <vector>
#include <algorithm>
#include "AR/TMath.h"

namespace TMath {

template<typename Type = DefaultType>
struct Tukey
{
    inline static Type findSigmaSquared(std::vector<Type>& vdErrorSquared)
    {
        assert(vdErrorSquared.size() > 0);
        std::sort(vdErrorSquared.begin(), vdErrorSquared.end());
        Type dMedianSquared = vdErrorSquared[vdErrorSquared.size() / 2];
        Type dSigma = 1.4826 * (1.0 + 5.0 / (vdErrorSquared.size() * 2.0 - 6.0)) * sqrtf(dMedianSquared);
        dSigma = 4.6851 * dSigma;
        Type dSigmaSquared = dSigma * dSigma;
        return dSigmaSquared;
    }

    inline static Type squareRootWeight(Type dErrorSquared, Type dSigmaSquared)
    {
        if (dErrorSquared > dSigmaSquared)
            return (Type)0;
        return (Type)1.0 - (dErrorSquared / dSigmaSquared);
    }

    inline static Type weight(Type dErrorSquared, Type dSigmaSquared)
    {
        Type dSqrt = squareRootWeight(dErrorSquared, dSigmaSquared);
        return dSqrt * dSqrt;
    }

    inline static Type objectiveScore(Type dErrorSquared, Type dSigmaSquared)
    {
        // NB All returned are scaled because
        // I'm not multiplying by sigmasquared/6.0
        if(dErrorSquared > dSigmaSquared)
            return (Type)1;
        Type d = (Type)1 - dErrorSquared / dSigmaSquared;
        return ((Type)1.0 - d * d * d);
    }
};

} // TMath

#endif // OTHERESTIMATORS_H
