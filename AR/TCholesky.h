#ifndef TCHOLESKY_H
#define TCHOLESKY_H

#include "AR/TMath.h"

namespace TMath {

template <typename Type = DefaultType>
class TCholesky
{
public:
    TCholesky(){}

    /// Construct the Cholesky decomposition of a matrix. This initialises the class, and
    /// performs the decomposition immediately.
    TCholesky(const TMatrix<Type>& matrix): _cholesky(matrix)
    {
        TMath_assert(matrix.isSquareMatrix());
        _do_compute();
    }

    /// Constructor for Size=Dynamic
    TCholesky(int size) : _cholesky(size, size) { TMath_assert(size > 0); }


    /// Compute the LDL^T decomposition of another matrix.
    /// Run time is O(N^3)
    void compute(const TMatrix<Type>& matrix)
    {
        TMath_assert(matrix.isSquareMatrix());
        _cholesky = matrix;
        _do_compute();
    }


    /// Compute x = A^-1*v
    /// Run time is O(N^2)
    TVector<Type> backsub(const TVector<Type>& vector) const
    {
        int size = _cholesky.rows();
        TMath_assert(vector.size() == size);
        int i, j;

        // first backsub through L
        TVector<Type> y(size);
        for(i=0; i<size; ++i){
            Type val = vector(i);
            for(int j=0; j<i; ++j)
                val -= _cholesky(i, j) * y(j);
            y(i) = val;
        }

        // backsub through diagonal
        for(i=0; i<size; ++i)
            y(i) /= _cholesky(i, i);

        // backsub through L_transpose
        TVector<Type> result(size);
        for(i=size-1; i>=0; --i) {
            Type val = y(i);
            for(j=i+1; j<size; ++j)
                val -= _cholesky(j, i) * result(j);
            result(i) = val;
        }

        return result;
    }

    ///Compute the determinant.
    Type determinant()
    {
        Type answer = _cholesky(0, 0);
        for(int i=1; i<_cholesky.rows(); ++i)
            answer *= _cholesky(i, i);
        return answer;
    }

    Type mahalanobis(const TVector<Type>& v) const
    {
        return v * backsub(v);
    }

    TMatrix<Type> get_unscaled_L() const
    {
        TMatrix<Type> matrix(_cholesky.rows(), _cholesky.rows());
        matrix.setToIdentity();
        int i, j;
        for (i=1; i<_cholesky.rows(); ++i)
            for (j=0; j<i; ++j)
                matrix(i, j) = _cholesky(i, j);
        return matrix;
    }

    TMatrix<Type> get_D() const
    {
        TMatrix<Type> matrix(_cholesky.rows(), _cholesky.rows());
        matrix.setZero();
        for (int i=0; i<_cholesky.rows(); ++i)
            matrix(i, i) = _cholesky(i, i);
        return matrix;
    }

    TMatrix<Type> get_L() const
    {
        TMatrix<Type> matrix(_cholesky.rows(), _cholesky.rows());
        matrix.setZero();
        int i, j;
        for (j=0; j<_cholesky.cols(); ++j) {
            Type sqrtd = std::sqrt(_cholesky(j, j));
            matrix(j, j) = sqrtd;
            for (i=j+1; i<_cholesky.rows(); ++i)
                matrix(i, j) = _cholesky(i, j) * sqrtd;
        }
        return matrix;
    }

    inline int rank() const { return _rank; }

private:
    TMatrix<Type> _cholesky;
    int _rank;

    void _do_compute() {
        int size = _cholesky.rows();
        int col, col2, row;
        for(col=0; col<size; ++col) {
            Type inv_diag = (Type)1;
            for(row=col; row<size; ++row){
                // correct for the parts of cholesky already computed
                Type val = _cholesky(row,col);
                for(col2=0; col2<col; ++col2){
                    // val -= _cholesky(col, col2) * _cholesky(row, col2)* _cholesky(col2, col2);
                    val -= _cholesky(col2,col) * _cholesky(row,col2);
                }
                if(row == col){
                    // this is the diagonal element so don't divide
                    _cholesky(row, col) = val;
                    if(std::fabs(val) < FLT_EPSILON){
                        _rank = row;
                        return;
                    }
                    inv_diag = 1 / val;
                } else {
                    // cache the value without division in the upper half
                    _cholesky(col, row) = val;
                    // divide my the diagonal element for all others
                    _cholesky(row, col) = val*inv_diag;
                }
            }
        }
        _rank = size;
    }
};

} // TMath

#endif // TCHOLESKY_H
