#ifndef TMATH_TCHOLESKY_H
#define TMATH_TCHOLESKY_H

#include <climits>
#include <limits>
#include "TTools.h"

namespace TMath {

template <typename Type = TMath_DefaultType>
class TCholesky
{
public:
    TCholesky() {}

    /// Construct the Cholesky decomposition of a matrix. This initialises the class, and
    /// performs the decomposition immediately.
    TCholesky(const TMatrix<Type>& matrix): m_choleskyMatrix(matrix)
    {
        TMath_assert(matrix.isSquareMatrix());
        _do_compute();
    }

    /// Constructor
    TCholesky(int size) : m_choleskyMatrix(size, size) { TMath_assert(size > 0); }


    /// Compute the LDL^T decomposition of another matrix.
    /// Run time is O(N^3)
    void compute(const TMatrix<Type>& matrix)
    {
        TMath_assert(matrix.isSquareMatrix());
        m_choleskyMatrix = matrix;
        _do_compute();
    }


    /// Compute x = A^-1*v
    /// Run time is O(N^2)
    TVector<Type> backsub(const TVector<Type>& vector) const
    {
        int size = m_choleskyMatrix.rows();
        TMath_assert(vector.size() == size);
        int i, j;

        // first backsub through L
        TVector<Type> y(size);
        for(i=0; i<size; ++i){
            Type val = vector(i);
            for(int j=0; j<i; ++j)
                val -= m_choleskyMatrix(i, j) * y(j);
            y(i) = val;
        }

        // backsub through diagonal
        for(i=0; i<size; ++i)
            y(i) /= m_choleskyMatrix(i, i);

        // backsub through L_transpose
        TVector<Type> result(size);
        for(i=size-1; i>=0; --i) {
            Type val = y(i);
            for(j=i+1; j<size; ++j)
                val -= m_choleskyMatrix(j, i) * result(j);
            result(i) = val;
        }

        return result;
    }

    ///Compute the determinant.
    Type determinant()
    {
        Type answer = m_choleskyMatrix(0, 0);
        for(int i=1; i<m_choleskyMatrix.rows(); ++i)
            answer *= m_choleskyMatrix(i, i);
        return answer;
    }

    Type mahalanobis(const TVector<Type>& v) const
    {
        return v * backsub(v);
    }

    TMatrix<Type> get_unscaled_L() const
    {
        TMatrix<Type> matrix(m_choleskyMatrix.rows(), m_choleskyMatrix.rows());
        matrix.setToIdentity();
        int i, j;
        for (i=1; i<m_choleskyMatrix.rows(); ++i)
            for (j=0; j<i; ++j)
                matrix(i, j) = m_choleskyMatrix(i, j);
        return matrix;
    }

    TMatrix<Type> get_D() const
    {
        TMatrix<Type> matrix(m_choleskyMatrix.rows(), m_choleskyMatrix.rows());
        matrix.setZero();
        for (int i=0; i<m_choleskyMatrix.rows(); ++i)
            matrix(i, i) = m_choleskyMatrix(i, i);
        return matrix;
    }

    TMatrix<Type> get_L() const
    {
        TMatrix<Type> matrix(m_choleskyMatrix.rows(), m_choleskyMatrix.rows());
        matrix.setZero();
        int i, j;
        for (j=0; j<m_choleskyMatrix.cols(); ++j) {
            Type sqrtd = std::sqrt(m_choleskyMatrix(j, j));
            matrix(j, j) = sqrtd;
            for (i=j+1; i<m_choleskyMatrix.rows(); ++i)
                matrix(i, j) = m_choleskyMatrix(i, j) * sqrtd;
        }
        return matrix;
    }

    inline int rank() const { return m_rank; }

private:
    TMatrix<Type> m_choleskyMatrix;
    int m_rank;

    void _do_compute() {
        int size = m_choleskyMatrix.rows();
        int col, col2, row;
        for (col=0; col<size; ++col) {
            Type inv_diag = (Type)1;
            for (row=col; row<size; ++row){
                // correct for the parts of cholesky already computed
                Type val = m_choleskyMatrix(row, col);
                for (col2=0; col2<col; ++col2){
                    // val -= _cholesky(col, col2) * _cholesky(row, col2)* _cholesky(col2, col2);
                    val -= m_choleskyMatrix(col2, col) * m_choleskyMatrix(row, col2);
                }
                if (row == col){
                    // this is the diagonal element so don't divide
                    m_choleskyMatrix(row, col) = val;
                    if (std::fabs(val) < std::numeric_limits<Type>::epsilon()){
                        m_rank = row;
                        return;
                    }
                    inv_diag = 1 / val;
                } else {
                    // cache the value without division in the upper half
                    m_choleskyMatrix(col, row) = val;
                    // divide my the diagonal element for all others
                    m_choleskyMatrix(row, col) = val * inv_diag;
                }
            }
        }
        m_rank = size;
    }
};

} // TMath

#endif // TMATH_TCHOLESKY_H
