#ifndef TMATH_TTOOLS_IMPL_H
#define TMATH_TTOOLS_IMPL_H

#include "TTools.h"
#include "TSVD.h"

namespace TMath {

template<typename Type>
bool TTools::computeLU_decomposition(TMatrix<Type>& inoutMatrix)
{
    TMath_assert(inoutMatrix.isSquareMatrix());
    int i, j, k, rank = inoutMatrix.rows();
    Type mult;
    Type* str_i;
    Type* str_k = inoutMatrix.firstDataRow();
    if (std::fabs(str_k[0]) < std::numeric_limits<Type>::epsilon())
        return false;
    {
        str_i = inoutMatrix.getDataRow(1);
        {
            mult = str_i[0] / str_k[0];
            str_i[0] = mult;
            for (j=1; j<rank; ++j)
                str_i[j] -= mult * str_k[j];
        }
        for (i=2; i<rank; ++i) {
            str_i = inoutMatrix.nextRow(str_i);
            mult = str_i[0] / str_k[0];
            str_i[0] = mult;
            for (j=1; j<rank; ++j)
                str_i[j] -= mult * str_k[j];
        }
    }
    for (k=1; k<rank-1; ++k) {
        str_k = inoutMatrix.nextRow(str_k);
        if (std::fabs(str_k[k]) < std::numeric_limits<Type>::epsilon())
            return false;
        str_i = inoutMatrix.getDataRow(k+1);
        {
            mult = str_i[k] / str_k[k];
            str_i[k] = mult;
            for (j=k+1; j<rank; ++j)
                str_i[j] -= mult * str_k[j];
        }
        for (i=k+2; i<rank; ++i) {
            str_i = inoutMatrix.nextRow(str_i);
            mult = str_i[k] / str_k[k];
            str_i[k] = mult;
            for (j=k+1; j<rank; ++j)
                str_i[j] -= mult * str_k[j];
        }
    }
    return true;
}

template <typename Type>
bool TTools::computeLUP_decomposition(TMatrix<Type>& inoutMatrix, TVector<int>& P)
{
    TMath_assert(inoutMatrix.isSquareMatrix());
    TMath_assert(P.size() == inoutMatrix.rows());
    int i, j, rank = inoutMatrix.rows(), k = 0;
    Type* str_i = inoutMatrix.data();
    Type max = std::fabs(*str_i);
    for (j=1; j<rank; ++j) {
        str_i = &str_i[rank];
        const Type val = std::fabs(*str_i);
        if (val > max) {
            max = val;
            k = j;
        }
    }
    if (k != 0) {
        P(0) = k;
        for (j=1; j<k; ++j)
            P(j) = j;
        P(k) = 0;
        for (j=k+1; j<rank; ++j)
            P(j) = j;
        inoutMatrix.swapRows(0, k);
    } else {
        for (j=0; j<rank; ++j)
            P(j) = j;
    }
    for (i=1; i<rank; ++i) {
        k = i;
        str_i = &inoutMatrix.getDataRow(i)[i];
        max = std::fabs(*str_i);
        for (j=i+1; j<rank; ++j) {
            str_i = &str_i[rank];
            const Type val = std::fabs(*str_i);
            if (val > max) {
                max = val;
                k = j;
            }
        }
        if (i != k) {
            std::swap(P(i), P(k));
            inoutMatrix.swapRows(i, k);
        }
    }
    return computeLU_decomposition(inoutMatrix);
}

}

#endif // TMATH_TTOOLS_IMPL_H
