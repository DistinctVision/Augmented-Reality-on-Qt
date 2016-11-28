#ifndef TMATH_TSVD_H
#define TMATH_TSVD_H

#include "TTools.h"
#include <cmath>
#include <climits>
#include <utility>
#include <cstdlib>

namespace TMath {

template<typename Type = TMath_DefaultType>
class TSVD
{
public:
    TSVD():m_Ut(3, 3), m_diagonalW(3), m_Vt(3, 3) { m_transposed = false; m_tempBuf = new double[3]; m_maxSizeA = 3; m_maxSizeB = 3; }
    ~TSVD() { delete[] m_tempBuf; }
    inline bool transposed() const { return m_transposed; }

    inline const TMatrix<Type>& U_transposed() const { return m_Ut; }
    inline RefTransposedMatrix<Type> U() const { return m_Ut.refTransposed(); }
    inline TVector<Type>& diagonalW() { return m_diagonalW; }
    inline const TVector<Type>& diagonalW() const { return m_diagonalW; }
    inline const Type& diagonalW(int index) const { return m_diagonalW(index); }
    inline const TMatrix<Type>& V_transposed() const { return m_Vt; }
    inline RefTransposedMatrix<Type> V() const { return m_Vt.refTransposed(); }

    void compute(const TMatrix<Type>& matrix, bool full_decomposition = true)
    {
        if (m_transposed)
            std::swap(m_Vt.m_data, m_Ut.m_data);
        int sizeA, sizeB;
        if (matrix.rows() > matrix.cols()) {
            sizeA = matrix.rows();
            sizeB = matrix.cols();
            m_transposed = true;
        } else {
            sizeA = matrix.cols();
            sizeB = matrix.rows();
            m_transposed = false;
        }
        if (sizeA > m_maxSizeA) {
            delete[] m_Vt.m_data;
            m_maxSizeA = sizeA;
            m_Vt.m_data = new Type[m_maxSizeA * m_maxSizeA];
        }
        if (sizeB > m_maxSizeB) {
            delete[] m_Ut.m_data;
            delete[] m_diagonalW.m_data;
            delete[] m_tempBuf;
            m_maxSizeB = sizeB;
            m_Ut.m_data = new Type[m_maxSizeB * m_maxSizeB];
            m_diagonalW.m_data = new Type[m_maxSizeB];
            m_tempBuf = new double[m_maxSizeB];
        }
        if (m_transposed) {
            m_Vt.m_rows = matrix.cols();
            m_Vt.m_cols = matrix.rows();
            m_Vt = matrix.refTransposed();
        } else {
            m_Vt.m_rows = matrix.rows();
            m_Vt.m_cols = matrix.cols();
            m_Vt = matrix;
        }
        m_Ut.m_rows = m_Ut.m_cols = sizeB;
        m_diagonalW.m_size = sizeB;
        m_Vt.m_cols = sizeA;
        if (full_decomposition) {
            m_Vt.m_rows = sizeA;
            _JacobiSVDImpl(sizeA, sizeB, sizeB, sizeA, sizeA, std::numeric_limits<Type>::min(), std::numeric_limits<Type>::epsilon());
         } else {
            m_Vt.m_rows = sizeB;
            _JacobiSVDImpl(sizeA, sizeB, sizeB, sizeA, sizeB, std::numeric_limits<Type>::min(), std::numeric_limits<Type>::epsilon());
        }
        if (m_transposed)
            m_Vt.swap(m_Ut);
    }
    inline void compute(Type* dataMatrix, int rows, int cols, bool full_decomposition = true)
    {
        TMatrix<Type> matrix;
        matrix.m_data = dataMatrix;
        matrix.m_rows = rows;
        matrix.m_cols = cols;
        compute(matrix, full_decomposition);
        matrix.m_data = nullptr;
    }

    /// result = (M^+ * vector)
    inline TVector<Type> backsub(const TVector<Type>& vector) const
    {
        TMath_assert(vector.size() == m_Ut.cols());
        TVector<Type> temp(m_diagonalW.size());
        int i, j;
        const Type* data = m_Ut.firstDataRow();
        for (i=0; i<temp.size(); ++i) {
            if (std::fabs(m_diagonalW(i)) > std::numeric_limits<Type>::epsilon()) {
                temp(i) = data[0] * vector(0);
                for (j=1; j<vector.size(); ++j)
                    temp(i) += data[j] * vector(j);
                temp(i) /= m_diagonalW(i);
            } else {
                temp(i) = (Type)0;
            }
            data = m_Ut.nextRow(data);
        }
        TVector<Type> result(m_Vt.cols());
        data = m_Vt.data();
        for (j=0; j<m_Vt.cols(); ++j)
            result(j) = data[j] * temp(0);
        for (i=1; i<temp.size(); ++i) {
            data = m_Vt.nextRow(data);
            for (j=0; j<m_Vt.cols(); ++j)
                result(j) += data[j] * temp(i);
        }
        return result;
    }

    /// result = M^+
    TMatrix<Type> pseudoinverse() const
    {
        TMatrix<Type> W_Ut(m_Vt.rows(), m_Ut.cols());
        Type* resultDataRow = W_Ut.firstDataRow();
        const Type* dataRow = m_Ut.firstDataRow();
        int i, j;
        Type inv_v;
        if (std::fabs(m_diagonalW(0)) > std::numeric_limits<Type>::epsilon()) {
            inv_v = (Type)1 / m_diagonalW(0);
            for (j=0; j<m_Ut.cols(); ++j)
                resultDataRow[j] = dataRow[j] * inv_v;
        } else {
            for (j=0; j<m_Ut.cols(); ++j)
                resultDataRow[j] = (Type)0;
        }
        for (i=1; i<m_diagonalW.size(); ++i) {
            resultDataRow = W_Ut.nextRow(resultDataRow);
            dataRow = m_Ut.nextRow(dataRow);
            if (std::fabs(m_diagonalW(i)) > std::numeric_limits<Type>::epsilon()) {
                inv_v = (Type)1 / m_diagonalW(i);
                for (j=0; j<m_Ut.cols(); ++j)
                    resultDataRow[j] = dataRow[j] * inv_v;
            } else {
                for (j=0; j<m_Ut.cols(); ++j)
                    resultDataRow[j] = (Type)0;
            }
        }
        for (; i<W_Ut.rows(); ++i) {
            resultDataRow = W_Ut.nextRow(resultDataRow);
            for (j=0; j<m_Ut.cols(); ++j)
                resultDataRow[j] = (Type)0;
        }
        return m_Vt.refTransposed() * W_Ut;
    }


private:
    TMatrix<Type> m_Ut;
    TVector<Type> m_diagonalW;
    TMatrix<Type> m_Vt;
    bool m_transposed;
    int m_maxSizeA;
    int m_maxSizeB;
    double* m_tempBuf;

    void _JacobiSVDImpl(int vtstep, int ustep, int n, int m, int n1, Type minval, Type eps)
    {
        TMath_assert(m >= n);
        TMath_assert(n1 <= m);
        TMath_assert(m_maxSizeA >= m);
        TMath_assert(m_maxSizeB >= n);
        int i, j, k, iter, max_iter = (m < 30) ? m : 30;
        Type c, s;
        double sd;

        Type* _vt = m_Vt.m_data;
        Type* _w = m_diagonalW.m_data;
        Type* _ut = m_Ut.m_data;

        for( i = 0; i < n; i++ )
        {
            for( k = 0, sd = 0; k < m; k++ )
            {
                Type t = _vt[i*vtstep + k];
                sd += (double)t*t;
            }
            m_tempBuf[i] = sd;

            for( k = 0; k < i; k++ )
                _ut[i*ustep + k] = 0;
            for( ++k; k < n; k++ )
                _ut[i*ustep + k] = 0;
            _ut[i*ustep + i] = 1;
        }

        for( iter = 0; iter < max_iter; iter++ )
        {
            bool changed = false;

            for( i = 0; i < n-1; i++ )
                for( j = i+1; j < n; j++ )
                {
                    Type *Vi = _vt + i*vtstep, *Vj = _vt + j*vtstep;
                    double a = m_tempBuf[i], p = 0, b = m_tempBuf[j];

                    for( k = 0; k < m; k++ )
                        p += (double)Vi[k]*Vj[k];

                    if( std::abs(p) <= eps*std::sqrt((double)a*b) )
                        continue;

                    p *= 2;
                    double beta = a - b, gamma = hypot((double)p, beta);
                    if( beta < 0 )
                    {
                        double delta = (gamma - beta)*0.5;
                        s = (Type)std::sqrt(delta/gamma);
                        c = (Type)(p/(gamma*s*2));
                    }
                    else
                    {
                        c = (Type)std::sqrt((gamma + beta)/(gamma*2));
                        s = (Type)(p/(gamma*c*2));
                    }

                    a = b = 0;
                    for( k = 0; k < m; k++ )
                    {
                        Type t0 = c*Vi[k] + s*Vj[k];
                        Type t1 = -s*Vi[k] + c*Vj[k];
                        Vi[k] = t0; Vj[k] = t1;

                        a += (double)t0*t0; b += (double)t1*t1;
                    }
                    m_tempBuf[i] = a; m_tempBuf[j] = b;

                    changed = true;

                    Type *Uti = _ut + i*ustep, *Utj = _ut + j*ustep;

                    for(k = 0; k < n; k++)
                    {
                        Type t0 = c*Uti[k] + s*Utj[k];
                        Type t1 = -s*Uti[k] + c*Utj[k];
                        Uti[k] = t0; Utj[k] = t1;
                    }
                }
            if( !changed )
                break;
        }

        for( i = 0; i < n; i++ )
        {
            for( k = 0, sd = 0; k < m; k++ )
            {
                Type t = _vt[i*vtstep + k];
                sd += (double)t*t;
            }
            m_tempBuf[i] = std::sqrt(sd);
        }

        for( i = 0; i < n-1; i++ )
        {
            j = i;
            for( k = i+1; k < n; k++ )
            {
                if( m_tempBuf[j] < m_tempBuf[k] )
                    j = k;
            }
            if( i != j )
            {
                std::swap(m_tempBuf[i], m_tempBuf[j]);
                for( k = 0; k < m; k++ )
                    std::swap(_vt[i*vtstep + k], _vt[j*vtstep + k]);

                for( k = 0; k < n; k++ )
                    std::swap(_ut[i*ustep + k], _ut[j*ustep + k]);
            }
        }

        for( i = 0; i < n; i++ )
            _w[i] = (Type)m_tempBuf[i];

        std::srand(0x12345678);
        for( i = 0; i < n1; i++ )
        {
            sd = (i < n) ? m_tempBuf[i] : 0.0;

            while( sd <= minval )
            {
                // if we got a zero singular value, then in order to get the corresponding left singular vector
                // we generate a random vector, project it to the previously computed left singular vectors,
                // subtract the projection and normalize the difference.
                const Type val0 = (Type)(1.0/m);
                for( k = 0; k < m; k++ )
                {
                    Type val = ((std::rand() % 256) != 0) ? val0 : -val0;
                    _vt[i*vtstep + k] = val;
                }
                for( iter = 0; iter < 2; iter++ )
                {
                    for( j = 0; j < i; j++ )
                    {
                        sd = 0;
                        for( k = 0; k < m; k++ )
                            sd += _vt[i*vtstep + k]*_vt[j*vtstep + k];
                        Type asum = 0;
                        for( k = 0; k < m; k++ )
                        {
                            Type t = (Type)(_vt[i*vtstep + k] - sd*_vt[j*vtstep + k]);
                            _vt[i*vtstep + k] = t;
                            asum += std::abs(t);
                        }
                        asum = asum ? 1/asum : 0;
                        for( k = 0; k < m; k++ )
                            _vt[i*vtstep + k] *= asum;
                    }
                }
                sd = 0;
                for( k = 0; k < m; k++ )
                {
                    Type t = _vt[i*vtstep + k];
                    sd += (double)t*t;
                }
                sd = std::sqrt(sd);
            }

            s = (Type)(1 / sd);
            for( k = 0; k < m; k++ )
                _vt[i*vtstep + k] *= s;
        }
    }
};

}// TMath

#endif // TMATH_TSVD_H
