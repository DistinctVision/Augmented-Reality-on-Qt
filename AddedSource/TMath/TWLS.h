#ifndef TMATH_TWLS_H
#define TMATH_TWLS_H

#include "TTools.h"
#include "TCholesky.h"

namespace TMath {

/// Performs Gauss-Newton weighted least squares computation.
template<typename Type = TMath_DefaultType, class Decomposition = TCholesky<Type>>
class TWLS
{
public:
    TWLS(int size) :
        m_invA(size, size),
        m_B(size),
        m_X(size)
    {
        clear();
    }

    /// Clear all the measurements and apply a constant regularisation term.
    inline void clear()
    {
        m_invA.setZero();
        m_B.setZero();
    }

    inline void resize(int size)
    {
        m_invA = TMatrix<Type>(size, size);
        m_B = TVector<Type>(size);
        m_X = TVector<Type>(size);
    }

    /// Applies a constant regularisation term.
    /// Equates to a prior that says all the parameters are zero with \f$\sigma^2 = \frac{1}{\text{val}}\f$.
    /// @param val The strength of the prior
    inline void addPrior(Type val)
    {
        for(int i=0; i<m_invA.rows(); ++i)
            m_invA(i, i) += val;
    }

    /// Applies a regularisation term with a different strength for each parameter value.
    /// Equates to a prior that says all the parameters are zero with \f$\sigma_i^2 = \frac{1}{\text{v}_i}\f$.
    /// @param v The vector of priors
    inline void addPrior(const TVector<Type>& v)
    {
        TMath_assert(m_invA.rows() == v.size());
        for(int i=0; i<m_invA.rows(); ++i)
            m_invA(i, i) += v(i);
    }

    /// Applies a whole-matrix regularisation term.
    /// This is the same as adding the \f$m\f$ to the inverse covariance matrix.
    /// @param m The inverse covariance matrix to add
    inline void addPrior(const TMatrix<Type>& matrix)
    {
        m_invA += matrix;
    }

    /// Add a single measurement
    /// @param m The value of the measurement
    /// @param J The Jacobian for the measurement \f$\frac{\partial\text{m}}{\partial\text{param}_i}\f$
    /// @param weight The inverse variance of the measurement
    inline void addMeasurement(Type m, const TVector<Type>& J, Type weight)
    {
        //Upper right triangle only, for speed
        for(int r=0; r < m_invA.rows(); ++r) {
            Type Jw = weight * J(r);
            m_B(r) += m * Jw;
            for(int c=r; c<m_invA.cols(); ++c)
                m_invA(r, c) += Jw * J(c);
        }
    }

    /// Add a single measurement
    /// @param m The value of the measurement
    /// @param J The Jacobian for the measurement \f$\frac{\partial\text{m}}{\partial\text{param}_i}\f$
    inline void addMeasurement(Type m, const TVector<Type>& J)
    {
        //Upper right triangle only, for speed
        for(int r=0; r < m_invA.rows(); ++r) {
            m_B(r) += m * J(r);
            for(int c=r; c<m_invA.cols(); ++c)
                m_invA(r, c) += J(r) * J(c);
        }
    }

    /// Add multiple measurements at once (much more efficiently)
    /// @param m The measurements to add
    /// @param J The Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param invcov The inverse covariance of the measurement values
    inline void addMeasurement(const TVector<Type>& m, const TMatrix<Type>& J, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp =  J * invcov;
        m_invA += temp * J.refTransposed();
        m_B += temp * m;
    }

    /*/// Add multiple measurements at once (much more efficiently)
    /// @param m The measurements to add
    /// @param J The Jacobian matrix \f$\frac{\partial\text{m}_i}{\partial\text{param}_j}\f$
    /// @param invcov The inverse covariance of the measurement values
    inline void add_mJ_rows(const TVector<Type>& m, const TMatrix<Type>& J, const TMatrix<Type>& invcov)
    {
        const TMatrix<Type> temp =  J.refTransposed() * invcov;
        m_A_inv += temp * J;
        m_b += temp * m;
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
            m_b(r+index1) += m * Jw;
            for(int c = r; c<J1.size(); ++c)
                m_A_inv(r+index1, c+index1) += Jw * J1(c);
        }
    }*/

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
    inline void compute()
    {
        //Copy the upper right triangle to the empty lower-left.
        int r, c;
        for(r=1; r<m_invA.rows(); ++r)
            for(c=0; c<r; ++c)
                m_invA(r, c) = m_invA(c, r);

        m_decomposition.compute(m_invA);
        m_X = m_decomposition.backsub(m_B);
    }

    /// Combine measurements from two WLS systems
    /// @param meas The measurements to combine with
    inline void operator += (const TWLS& meas)
    {
        m_B += meas.m_B;
        m_invA += meas.m_invA;
    }

    /// Returns the inverse covariance matrix
    TMatrix<Type>& invA() { return m_invA; }

    /// Returns the inverse covariance matrix
    const TMatrix<Type>& invA() const { return m_invA; }

    ///<Returns the update. With no prior, this is the result of \f$J^\dagger e\f$.
    TVector<Type>& X(){ return m_X; }

    ///<Returns the update. With no prior, this is the result of \f$J^\dagger e\f$.
    const TVector<Type>& X() const { return m_X; }

    ///<Returns the vector b \f$J^{\mathsf T} e\f$
    TVector<Type>& B() { return m_B; }

    ///<Returns the  vector b \f$J^{\mathsf T} e\f$
    const TVector<Type>& B() const {return m_B;}

    ///< Return the decomposition object used to compute \f$(J^{\mathsf T}  J + P)^{-1}\f$
    Decomposition& decomposition() { return m_decomposition; }

    ///< Return the decomposition object used to compute \f$(J^{\mathsf T}  J + P)^{-1}\f$
    const Decomposition& decomposition() const { return m_decomposition; }

private:
    TMatrix<Type> m_invA;
    TVector<Type> m_B;
    Decomposition m_decomposition;
    TVector<Type> m_X;

    // comment out to allow bitwise copying
    // WLS( WLS& copyof );
    // int operator = ( WLS& copyof );
};

} // TMath

#endif // TMATH_TWLS_H
