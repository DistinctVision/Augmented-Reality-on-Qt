#ifndef TWLS_H
#define TWLS_H

#include "AR/TMath.h"
#include "AR/TCholesky.h"

namespace TMath {

/// Performs Gauss-Newton weighted least squares computation.
/// @param Size The number of dimensions in the system
/// @param Precision The numerical precision used (TooN::DefaultPrecision, float etc)
/// @param Decomposition The class used to invert the inverse Covariance matrix (must have one integer size and one typename precision template arguments) this is Cholesky by default, but could also be SQSVD
/// @ingroup gEquations
template<typename Type = DefaultType, class Decomposition = TCholesky<Type>>
class TWLS
{
public:
    /// Default constructor or construct with the number of dimensions for the Dynamic case
    TWLS(int size) :
        _C_inv(size, size),
        _vector(size),
        _mu(size)
    {
        clear();
    }

    /// Clear all the measurements and apply a constant regularisation term.
    void clear()
    {
        _C_inv.setZero();
        _vector.setZero();
    }

    void resize(int size)
    {
        _C_inv.recreate(size, size);
        _vector.recreate(size);
        _mu.recreate(size);
    }

    /// Applies a constant regularisation term.
    /// Equates to a prior that says all the parameters are zero with \f$\sigma^2 = \frac{1}{\text{val}}\f$.
    /// @param val The strength of the prior
    void add_prior(Type val)
    {
        for(int i=0; i<_C_inv.rows(); ++i)
            _C_inv(i, i) += val;
    }

    /// Applies a regularisation term with a different strength for each parameter value.
    /// Equates to a prior that says all the parameters are zero with \f$\sigma_i^2 = \frac{1}{\text{v}_i}\f$.
    /// @param v The vector of priors
    void add_prior(const TVector<Type>& v)
    {
        TMath_assert(_C_inv.rows() == v.size());
        for(int i=0; i<_C_inv.rows(); ++i)
            _C_inv(i, i) += v(i);
    }

    /// Applies a whole-matrix regularisation term.
    /// This is the same as adding the \f$m\f$ to the inverse covariance matrix.
    /// @param m The inverse covariance matrix to add
    void add_prior(const TMatrix<Type>& matrix)
    {
        _C_inv += matrix;
    }

    /// Add a single measurement
    /// @param m The value of the measurement
    /// @param J The Jacobian for the measurement \f$\frac{\partial\text{m}}{\partial\text{param}_i}\f$
    /// @param weight The inverse variance of the measurement (default = 1)
    inline void add_mJ(Type m, const TVector<Type>& J, Type weight = (Type)1)
    {
        //Upper right triangle only, for speed
        for(int r=0; r < _C_inv.rows(); ++r) {
            Type Jw = weight * J(r);
            _vector(r) += m * Jw;
            for(int c=r; c<_C_inv.cols(); ++c)
                _C_inv(r, c) += Jw * J(c);
        }
    }

    /// Add multiple measurements at once (much more efficiently)
    /// @param m The measurements to add
    /// @param J The Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param invcov The inverse covariance of the measurement values
    inline void add_mJ(const TVector<Type>& m, const TMatrix<Type>& J, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp =  J * invcov;
        _C_inv += temp * J.refTransposed();
        _vector += temp * m;
    }

    /// Add multiple measurements at once (much more efficiently)
    /// @param m The measurements to add
    /// @param J The Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param invcov The inverse covariance of the measurement values
    inline void add_mJ_rows(const TVector<Type>& m, const TMatrix<Type>& J, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp =  J.refTransposed() * invcov;
        _C_inv += temp * J;
        _vector += temp * m;
    }

    /// Add a single measurement at once with a sparse Jacobian (much, much more efficiently)
    /// @param m The measurements to add
    /// @param J1 The first block of the Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param index1 starting index for the first block
    /// @param invcov The inverse covariance of the measurement values
    inline void add_sparse_mJ(const Type m, const TVector<Type>& J1, const int index1, const Type weight = (Type)1){
        //Upper right triangle only, for speed
        for(int r=0; r<J1.size(); ++r) {
            Type Jw = weight * J1(r);
            _vector(r+index1) += m * Jw;
            for(int c = r; c<J1.size(); ++c)
                _C_inv(r+index1, c+index1) += Jw * J1(c);
        }
    }

    /*/// Add multiple measurements at once with a sparse Jacobian (much, much more efficiently)
    /// @param m The measurements to add
    /// @param J1 The first block of the Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param index1 starting index for the first block
    /// @param invcov The inverse covariance of the measurement values
    inline void add_sparse_mJ_rows(const TVector<Type>& m, const TMatrix<Type>& J1, const int index1, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp1 = J1.refTransposed() * invcov;
        const int size1 = J1.cols();
        _C_inv.slice(index1, index1, size1, size1) += temp1 * J1;
        _vector.slice(index1, size1) += temp1 * m;
    }*/

    /*/// Add multiple measurements at once with a sparse Jacobian (much, much more efficiently)
    /// @param m The measurements to add
    /// @param J1 The first block of the Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param index1 starting index for the first block
    /// @param J2 The second block of the Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param index2 starting index for the second block
    /// @param invcov The inverse covariance of the measurement values
    inline void add_sparse_mJ_rows(const TVector<Type>& m, const TMatrix<Type>& J1, const int index1,
                                   const TMatrix<Type>& J2, const int index2, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp1 = J1.refTransposed() * invcov;
        const TMatrix<Type> temp2 = J2.refTransposed() * invcov;
        const TMatrix<Type> mixed = temp1 * J2;
        const int size1 = J1.cols();
        const int size2 = J2.cols();
        _C_inv.slice(index1, index1, size1, size1) += temp1 * J1;
        _C_inv.slice(index2, index2, size2, size2) += temp2 * J2;
        _C_inv.slice(index1, index2, size1, size2) += mixed;
        _C_inv.slice(index2, index1, size2, size1) += mixed.refTransposed();
        _vector.slice(index1, size1) += temp1 * m;
        _vector.slice(index2, size2) += temp2 * m;
    }*/

    /// Process all the measurements and compute the weighted least squares set of parameter values
    /// stores the result internally which can then be accessed by calling get_mu()
    void compute()
    {
        //Copy the upper right triangle to the empty lower-left.
        int r, c;
        for(r=1; r<_C_inv.rows(); ++r)
            for(c=0; c<r; ++c)
                _C_inv(r, c) = _C_inv(c, r);

        _decomposition.compute(_C_inv);
        _mu = _decomposition.backsub(_vector);
    }

    /// Combine measurements from two WLS systems
    /// @param meas The measurements to combine with
    void operator += (const TWLS& meas)
    {
        _vector += meas._vector;
        _C_inv += meas._C_inv;
    }

    /// Returns the inverse covariance matrix
    TMatrix<Type>& C_inv() { return _C_inv; }
    /// Returns the inverse covariance matrix
    const TMatrix<Type>& C_inv() const { return _C_inv; }
    TVector<Type>& mu(){ return _mu; }  ///<Returns the update. With no prior, this is the result of \f$J^\dagger e\f$.
    const TVector<Type>& mu() const { return _mu; } ///<Returns the update. With no prior, this is the result of \f$J^\dagger e\f$.
    TVector<Type>& vector() { return _vector; } ///<Returns the  vector \f$J^{\mathsf T} e\f$
    const TVector<Type>& vector() const {return _vector;} ///<Returns the  vector \f$J^{\mathsf T} e\f$
    Decomposition& decomposition() { return _decomposition; } ///< Return the decomposition object used to compute \f$(J^{\mathsf T}  J + P)^{-1}\f$
    const Decomposition& decomposition() const { return _decomposition; } ///< Return the decomposition object used to compute \f$(J^{\mathsf T}  J + P)^{-1}\f$

private:
    TMatrix<Type> _C_inv;
    TVector<Type> _vector;
    Decomposition _decomposition;
    TVector<Type> _mu;

    // comment out to allow bitwise copying
    // WLS( WLS& copyof );
    // int operator = ( WLS& copyof );
};

} // TMath

#endif // TWLS_H
