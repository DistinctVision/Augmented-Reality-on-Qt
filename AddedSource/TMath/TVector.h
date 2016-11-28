#ifndef TMATH_TVECTOR_H
#define TMATH_TVECTOR_H

#include <limits>
#include <climits>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstddef>

#include <cmath>
#ifdef QT_CORE_LIB
#include <qmath.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined (QT_DEBUG)
#define TMath_assert(condition) assert(condition);
#else
#define TMath_assert(condition) static_cast<void>(condition);
#endif

namespace TMath {

typedef float TMath_DefaultType;

template <typename Type>
class TSVD;

template<typename Type = TMath_DefaultType>
class TVector
{
public:
    typedef Type TypeElement;

    static TVector<Type> create(Type x, Type y)
    {
        TVector<Type> v(2);
        v(0) = x;
        v(1) = y;
        return v;
    }

    static TVector<Type> create(Type x, Type y, Type z)
    {
        TVector<Type> v(3);
        v(0) = x;
        v(1) = y;
        v(2) = z;
        return v;
    }

    static TVector<Type> create(Type x, Type y, Type z, Type w)
    {
        TVector<Type> v(4);
        v(0) = x;
        v(1) = y;
        v(2) = z;
        v(3) = w;
        return v;
    }

    static TVector<Type> create(Type x0, Type x1, Type x2, Type x3, Type x4)
    {
        TVector<Type> v(5);
        v(0) = x0;
        v(1) = x1;
        v(2) = x2;
        v(3) = x3;
        v(4) = x4;
        return v;
    }

    static TVector<Type> create(Type x0, Type x1, Type x2, Type x3, Type x4, Type x5)
    {
        TVector<Type> v(6);
        v(0) = x0;
        v(1) = x1;
        v(2) = x2;
        v(3) = x3;
        v(4) = x4;
        v(5) = x5;
        return v;
    }

    TVector()
    {
        m_size = 0;
        m_data = nullptr;
    }

    bool isValid()
    {
        return (m_size > 0);
    }

    TVector(int size)
    {
        TMath_assert(size > 0);
        m_size = size;
        m_data = new Type[m_size];
    }

    TVector(const TVector<Type>& vector)
    {
        m_size = vector.size();
        m_data = new Type[m_size];
        for (int i=0; i<m_size; ++i)
            m_data[i] = vector(i);
    }

    TVector(TVector<Type>&& moveVector)
    {
        m_data = moveVector.m_data;
        m_size = moveVector.m_size;
        moveVector.m_data = nullptr;
        moveVector.m_size = 0;
    }

    TVector(int size, const Type* dataFrom)
    {
        m_size = size;
        m_data = new Type[size];
        for (int i=0; i<m_size; ++i)
            m_data[i] = dataFrom[i];
    }

    ~TVector()
    {
        if (m_data != nullptr)
            delete[] m_data;
    }

    void setZero()
    {
        for (int i=0; i<m_size; ++i)
            m_data[i] = (Type)0;
    }

    inline int size() const
    {
        return m_size;
    }

    inline Type& operator() (int index)
    {
        TMath_assert((index >= 0) && (index < m_size));
        return m_data[index];
    }

    inline const Type& operator() (int index) const
    {
        TMath_assert((index >= 0) && (index < m_size));
        return m_data[index];
    }

    inline Type& operator[] (int index)
    {
        TMath_assert((index >= 0) && (index < m_size));
        return m_data[index];
    }

    inline const Type& operator[] (int index) const
    {
        TMath_assert((index >= 0) && (index < m_size));
        return m_data[index];
    }

    inline const Type* data() const
    {
        return m_data;
    }

    inline Type* data()
    {
        return m_data;
    }

    inline void swapData(TVector<Type>& vector)
    {
        TMath_assert(m_size == vector.size());
        std::swap(m_data, vector.m_data);
    }

    inline void swap(TVector<Type>& vector)
    {
        std::swap(m_size, vector.m_size);
        std::swap(m_data, vector.m_data);
    }

    TVector<Type>& operator = (const TVector<Type>& vector)
    {
        if (m_size != vector.size()) {
            if (m_data != nullptr)
                delete[] m_data;
            m_size = vector.size();
            m_data = new Type[m_size];
        }
        for (int i=0; i<m_size; ++i)
            m_data[i] = vector(i);
        return (*this);
    }

    TVector<Type>& operator = (TVector<Type>&& moveVector)
    {
        TMath_assert(this != &moveVector);
        delete[] m_data;
        m_data = moveVector.m_data;
        m_size = moveVector.m_size;
        moveVector.m_data = nullptr;
        moveVector.m_size = 0;
        return (*this);
    }

    TVector<Type> operator - () const
    {
        TVector<Type> result(m_size);
        for (int i=0; i<m_size; ++i)
            result(i) = - m_data[i];
        return result;
    }

    TVector<Type> operator + (const TVector<Type>& vector) const
    {
        TMath_assert(m_size == vector.size());
        TVector<Type> result(m_size);
        for (int i=0; i<m_size; ++i)
            result(i) = m_data[i] + vector(i);
        return result;
    }

    void operator += (const TVector<Type>& vector)
    {
        TMath_assert(m_size == vector.size());
        for (int i=0; i<m_size; ++i)
            m_data[i] += vector(i);
    }

    TVector<Type> operator - (const TVector<Type>& vector) const
    {
        TMath_assert(m_size == vector.size());
        TVector<Type> result(m_size);
        for (int i=0; i<m_size; ++i)
            result(i) = m_data[i] - vector(i);
        return result;
    }

    void operator -= (const TVector<Type>& vector)
    {
        TMath_assert(m_size == vector.size());
        for (int i=0; i<m_size; ++i)
            m_data[i] -= vector(i);
    }

    TVector<Type> operator * (Type val) const
    {
        TVector<Type> result(m_size);
        for (int i=0; i<m_size; ++i)
            result(i) = m_data[i] * val;
        return result;
    }

    TVector<Type> operator / (Type val) const
    {
        TVector<Type> result(m_size);
        for (int i=0; i<m_size; ++i)
            result(i) = m_data[i] / val;
        return result;
    }

    void operator *= (const TVector& vector)
    {
        TMath_assert(m_size == vector.size());
        for (int i=0; i<m_size; ++i)
            m_data[i] *= vector(i);
    }

    void operator *= (Type val)
    {
        for (int i=0; i<m_size; ++i)
            m_data[i] *= val;
    }

    void operator /= (const TVector<Type>& vector)
    {
        TMath_assert(m_size == vector.size());
        for (int i=0; i<m_size; ++i)
            m_data[i] /= vector(i);
    }

    void operator /= (Type val)
    {
        for (int i=0; i<m_size; ++i)
            m_data[i] /= val;
    }

    Type lengthSquared() const
    {
        Type result = m_data[0] * m_data[0];
        for (int i=1; i<m_size; ++i)
            result += m_data[i] * m_data[i];
        return result;
    }

    Type length() const
    {
        return std::sqrt(lengthSquared());
    }

    Type normalize()
    {
        Type l = length();
        if (l < std::numeric_limits<Type>::epsilon()) {
            setZero();
            return Type(0);
        }
        for (int i=0; i<m_size; ++i)
            m_data[i] /= l;
        return l;
    }

    TVector<Type> normalized() const
    {
        TMath::TVector<Type> result = *this;
        result.normalize();
        return result;
    }

    void slice(TVector<Type>& result, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert((size > 0) && (size <= m_size));
        for (int i=0; i<size; ++i)
            result(i) = m_data[i];
    }

    inline TVector<Type> slice(int size) const
    {
        TVector<Type> result(size);
        slice(result, size);
        return result;
    }

    void slice(TVector<Type>& result, int beginIndex, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert((beginIndex >= 0) && ((beginIndex + size) <= m_size));
        for (int i=0; i<size; ++i)
            result(i) = m_data[beginIndex + i];
    }

    inline TVector<Type> slice(int beginIndex, int size) const
    {
        TVector<Type> result(size);
        slice(result, beginIndex, size);
        return result;
    }

    void fill(Type value)
    {
        for (int i=0; i<m_size; ++i)
            m_data[i] = value;
    }

    void fill(int beginIndex, TVector<Type> v)
    {
        TMath_assert((beginIndex >= 0) && ((beginIndex + v.size()) <= m_size));
        for (int i=0; i<v.size(); ++i)
            m_data[beginIndex + i] = v(i);
    }

    void fill(int beginIndex, int size, TVector<Type> v)
    {
        TMath_assert((beginIndex >= 0) && ((beginIndex + size) <= m_size));
        for (int i=0; i<size; ++i)
            m_data[beginIndex + i] = v(i);
    }

    bool invert()
    {
        bool success = true;
        for (int i=0; i<m_size; ++i) {
            if (std::fabs(m_data[i]) > std::numeric_limits<Type>::epsilon()) {
                m_data[i] = Type(1) / m_data[i];
            } else {
                m_data[i] = Type(0);
                success = false;
            }
        }
        return success;
    }

    TVector<Type> inverted() const
    {
        TVector<Type> result(m_size, m_data);
        result.invert();
        return result;
    }

    template<typename CastType>
    TVector<CastType> cast() const
    {
        TVector<CastType> result(m_size);
        for (int i = 0; i < m_size; ++i) {
            result(i) = (CastType)m_data[i];
        }
        return result;
    }

    bool operator == (const TVector<Type>& b) const
    {
        if (m_size != b.size())
            return false;
        for (int i = 0; i < m_size; ++i) {
            if (std::fabs(m_data[i] - b(i)) > std::numeric_limits<Type>::epsilon())
                return false;
        }
        return true;
    }
    bool operator != (const TVector<Type>& b) const
    {
        return !(operator == (b));
    }

private:
    friend class TSVD<Type>;
    friend class Tools;

    Type* m_data;
    int m_size;
};

typedef TVector<float> TVectorf;
typedef TVector<double> TVectord;

template<typename Type>
inline TVector<Type> operator * (Type value, const TVector<Type>& vector)
{
    return vector * value;
}

template<typename Type>
Type dot(const TVector<Type>& a, const TVector<Type>& b)
{
    TMath_assert((a.size() == b.size()) && (a.size() > 0));
    Type result = a(0) * b(0);
    for (int i=1; i<a.size(); ++i)
        result += a(i) * b(i);
    return result;
}

template<typename Type>
TVector<Type> cross3(const TVector<Type>& a, const TVector<Type>& b)
{
    TMath_assert((a.size() == 3) && (b.size() == 3));
    return TVector<Type>::create(a(1) * b(2) - a(2) * b(1),
                                 a(2) * b(0) - a(0) * b(2),
                                 a(0) * b(1) - a(1) * b(0));
}

} //namespace TMath

#endif // TMATH_TVECTOR_H
