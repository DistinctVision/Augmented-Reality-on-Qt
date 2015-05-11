#ifndef TSVD_H
#define TSVD_H

#include "AR/TMath.h"

namespace TMath {

template<typename Type = DefaultType>
class TSVD
{
public:
    TSVD():_Ut(3, 3), _W(3), _Vt(3, 3) { _transposed = false; _tempBuf = new double[3]; _maxSizeA = 3; _maxSizeB = 3; }
    ~TSVD() { delete[] _tempBuf; }
    inline bool transposed() const { return _transposed; }

    inline const TMatrix<Type>& U_transposed() const { return _Ut; }
    inline RefTransposedMatrix<Type> U() const { return _Ut.refTransposed(); }
    inline TVector<Type>& W() { return _W; }
    inline const TVector<Type>& W() const { return _W; }
    inline Type& W(int index) { return _W(index); }
    inline const Type& W(int index) const { return _W(index); }
    inline const TMatrix<Type>& V_transposed() const { return _Vt; }
    inline RefTransposedMatrix<Type> V() const { return _Vt.refTransposed(); }

    void compute(const TMatrix<Type>& matrix, bool full_V = false)
    {
        if (_transposed)
            std::swap(_Vt._data, _Ut._data);
        int sizeA, sizeB;
        if (matrix.rows() > matrix.cols()) {
            sizeA = matrix.rows();
            sizeB = matrix.cols();
            _transposed = true;
        } else {
            sizeA = matrix.cols();
            sizeB = matrix.rows();
            _transposed = false;
        }
        if (sizeA > _maxSizeA) {
            delete[] _Vt._data;
            _maxSizeA = sizeA;
            _Vt._data = new Type[_maxSizeA * _maxSizeA];
        }
        if (sizeB > _maxSizeB) {
            delete[] _Ut._data;
            delete[] _W._data;
            delete[] _tempBuf;
            _maxSizeB = sizeB;
            _Ut._data = new Type[_maxSizeB * _maxSizeB];
            _W._data = new Type[_maxSizeB];
            _tempBuf = new double[_maxSizeB];
        }
        if (_transposed) {
            _Vt._rows = matrix.cols();
            _Vt._cols = matrix.rows();
            _Vt = matrix.refTransposed();
        } else {
            _Vt._rows = matrix.rows();
            _Vt._cols = matrix.cols();
            _Vt = matrix;
        }
        _Ut._rows = _Ut._cols = sizeB;
        _W._size = sizeB;
        _Vt._cols = sizeA;
        if (full_V) {
            _Vt._rows = sizeA;
            _JacobiSVDImpl(sizeA, sizeB, sizeB, sizeA, sizeA, FLT_MIN, FLT_EPSILON);
         } else {
            _Vt._rows = sizeB;
            _JacobiSVDImpl(sizeA, sizeB, sizeB, sizeA, sizeB, FLT_MIN, FLT_EPSILON);
        }
        if (_transposed)
            _Vt.swap(_Ut);
    }
    inline void compute(Type* dataMatrix, int rows, int cols, bool full_V = false)
    {
        TMatrix<Type> matrix;
        matrix._data = dataMatrix;
        matrix._rows = rows;
        matrix._cols = cols;
        compute(matrix, full_V);
        matrix._data = nullptr;
    }

private:
    TMatrix<Type> _Ut;
    TVector<Type> _W;
    TMatrix<Type> _Vt;
    bool _transposed;
    int _maxSizeA;
    int _maxSizeB;
    double* _tempBuf;

    void _JacobiSVDImpl(int vtstep, int ustep, int n, int m, int n1, Type minval, Type eps)
    {
        TMath_assert(m >= n);
        TMath_assert(n1 <= m);
        TMath_assert(_maxSizeA >= m);
        TMath_assert(_maxSizeB >= n);
        int i, j, k, iter, max_iter = std::max(m, 30);
        Type c, s;
        double sd;

        Type* _vt = _Vt._data;
        Type* _w = _W._data;
        Type* _ut = _Ut._data;

        for( i = 0; i < n; i++ )
        {
            for( k = 0, sd = 0; k < m; k++ )
            {
                Type t = _vt[i*vtstep + k];
                sd += (double)t*t;
            }
            _tempBuf[i] = sd;

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
                    double a = _tempBuf[i], p = 0, b = _tempBuf[j];

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
                    _tempBuf[i] = a; _tempBuf[j] = b;

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
            _tempBuf[i] = std::sqrt(sd);
        }

        for( i = 0; i < n-1; i++ )
        {
            j = i;
            for( k = i+1; k < n; k++ )
            {
                if( _tempBuf[j] < _tempBuf[k] )
                    j = k;
            }
            if( i != j )
            {
                std::swap(_tempBuf[i], _tempBuf[j]);
                for( k = 0; k < m; k++ )
                    std::swap(_vt[i*vtstep + k], _vt[j*vtstep + k]);

                for( k = 0; k < n; k++ )
                    std::swap(_ut[i*ustep + k], _ut[j*ustep + k]);
            }
        }

        for( i = 0; i < n; i++ )
            _w[i] = (Type)_tempBuf[i];

        srand(0x12345678);
        for( i = 0; i < n1; i++ )
        {
            sd = (i < n) ? _tempBuf[i] : 0.0;

            while( sd <= minval )
            {
                // if we got a zero singular value, then in order to get the corresponding left singular vector
                // we generate a random vector, project it to the previously computed left singular vectors,
                // subtract the projection and normalize the difference.
                const Type val0 = (Type)(1.0/m);
                for( k = 0; k < m; k++ )
                {
                    Type val = ((rand() % 256) != 0) ? val0 : -val0;
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

            s = (Type)(1/sd);
            for( k = 0; k < m; k++ )
                _vt[i*vtstep + k] *= s;
        }
    }
};

}// TMath

#endif // TSVD_H
