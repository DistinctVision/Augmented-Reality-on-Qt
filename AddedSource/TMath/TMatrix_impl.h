#ifndef TMATH_TMATRIX_IMPL_H
#define TMATH_TMATRIX_IMPL_H

#include "TMatrix.h"
#include "TTools.h"

namespace TMath {

template<typename Type>
RefTransposedMatrix<Type> TMatrix<Type>::refTransposed() const
{
    return RefTransposedMatrix<Type>(this);
}

template<typename Type>
void TMatrix<Type>::fill(int beginRow, int beginCol, const RefTransposedMatrix<Type>& matrix)
{
    TMath_assert((m_rows >= (beginRow + matrix.rows())) && (m_cols >= (beginCol + matrix.cols())));
    TMath_assert(matrix.rows() > 0);
    int i, j;
    Type* dataRow = &m_data[beginRow * m_cols + beginCol];
    const Type* mData = matrix.data();
    const int mRows = matrix.rows();
    for (j=0; j<matrix.cols(); ++j)
        dataRow[j] = mData[j * mRows];
    for (i=1; i<matrix.rows(); ++i) {
        dataRow = nextRow(dataRow);
        ++mData;
        for (j=0; j<matrix.cols(); ++j)
            dataRow[j] = mData[j * mRows];
    }
}


template<typename Type>
inline TMatrix<Type> operator * (Type value, const TMatrix<Type>& matrix)
{
    return matrix * value;
}

template<typename Type>
inline TMatrix<Type> operator * (Type value, const RefTransposedMatrix<Type>& matrix)
{
    return matrix * value;
}

template<typename Type>
void TMatrix<Type>::operator = (const RefTransposedMatrix<Type>& matrix)
{
    TMath_assert(m_data != matrix.data());
    if (m_rows != matrix.rows() || m_cols != matrix.cols()) {
        TMath_assert((matrix.m_rows > 0) && (matrix.m_cols > 0));
        if (m_data != nullptr)
            delete[] m_data;
        m_rows = matrix.m_rows;
        m_cols = matrix.m_cols;
        m_data = new Type[m_rows * m_cols];
    }
    Type* dataRow = firstDataRow();
    const Type* mData = matrix.m_data;
    int i, j;
    for (j=0; j<m_cols; ++j)
        dataRow[j] = mData[m_rows * j];
    for (i=1; i<m_rows; ++i) {
        dataRow = nextRow(dataRow);
        ++mData;
        for (j=0; j<m_cols; ++j)
            dataRow[j] = mData[m_rows * j];
    }
}

template<typename Type>
TMatrix<Type>::TMatrix(const RefTransposedMatrix<Type>& matrix):
    m_rows(matrix.rows()), m_cols(matrix.cols())
{
    m_data = new Type[m_rows * m_cols];
    Type* dataRow = firstDataRow();
    const Type* mData = matrix.m_data;
    int i, j;
    for (j=0; j<m_cols; ++j)
        dataRow[j] = mData[m_rows * j];
    for (i=1; i<m_rows; ++i) {
        dataRow = nextRow(dataRow);
        ++mData;
        for (j=0; j<m_cols; ++j)
            dataRow[j] = mData[m_rows * j];
    }
}

template<typename Type>
void multiply(TMatrix<Type>& result, const TMatrix<Type>& a, const TMatrix<Type>& b)
{
    TMath_assert((result.rows() >= a.rows()) && (result.cols() >= b.cols()));
    TMath_assert(a.cols() == b.rows());
    Type* resultRow = result.firstDataRow();
    const Type* a_rowData = a.firstDataRow();
    int i, j, k;
    for (j=0; j<b.cols(); ++j) {
        resultRow[j] = a_rowData[0] * b(0, j);
        for (k=1; k<a.cols(); ++k)
            resultRow[j] += a_rowData[k] * b(k, j);
    }
    for (i=1; i<a.rows(); ++i) {
        resultRow = result.nextRow(resultRow);
        a_rowData = a.nextRow(a_rowData);
        for (j=0; j<b.cols(); ++j) {
            resultRow[j] = a_rowData[0] * b(0, j);
            for (k=1; k<a.cols(); ++k)
                resultRow[j] += a_rowData[k] * b(k, j);
        }
    }
}

template<typename Type>
inline TMatrix<Type> operator * (const TMatrix<Type>& a, const TMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), b.cols());
    multiply(result, a, b);
    return result;
}

template<typename Type>
inline void operator *= (TMatrix<Type>& a, const TMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), a.cols());
    multiply(result, a, b);
    a = result;
}

template<typename Type>
void multiply(TMatrix<Type>& result, const TMatrix<Type>& a, const RefTransposedMatrix<Type>& b)
{
    TMath_assert((result.rows() >= a.rows()) && (result.cols() >= b.cols()));
    TMath_assert(a.cols() == b.rows());
    Type* resultRow = result.firstDataRow();
    const Type* a_rowData = a.firstDataRow();
    const Type* b_columnData = b.firstDataColumn();
    int i, j, k;
    resultRow[0] = a_rowData[0] * b_columnData[0];
    for (k=1; k<a.cols(); ++k)
        resultRow[0] += a_rowData[k] * b_columnData[k];
    for (j=1; j<b.cols(); ++j) {
        b_columnData = b.nextColumn(b_columnData);
        resultRow[j] = a_rowData[0] * b_columnData[0];
        for (k=1; k<a.cols(); ++k)
            resultRow[j] += a_rowData[k] * b_columnData[k];
    }
    for (i=1; i<a.rows(); ++i) {
        resultRow = result.nextRow(resultRow);
        a_rowData = a.nextRow(a_rowData);
        b_columnData = b.firstDataColumn();
        resultRow[0] = a_rowData[0] * b_columnData[0];
        for (k=1; k<a.cols(); ++k)
            resultRow[0] += a_rowData[k] * b_columnData[k];
        for (j=1; j<b.cols(); ++j) {
            b_columnData = b.nextColumn(b_columnData);
            resultRow[j] = a_rowData[0] * b_columnData[0];
            for (k=1; k<a.cols(); ++k)
                resultRow[j] += a_rowData[k] * b_columnData[k];
        }
    }
}

template<typename Type>
inline TMatrix<Type> operator * (const TMatrix<Type>& a, const RefTransposedMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), b.cols());
    multiply(result, a, b);
    return result;
}

template<typename Type>
inline void operator *= (TMatrix<Type>& a, const RefTransposedMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), b.cols());
    multiply(result, a, b);
    a = result;
}

template<typename Type>
void multiply(TMatrix<Type>& result, const RefTransposedMatrix<Type>& a, const TMatrix<Type>& b)
{
    TMath_assert((result.rows() >= a.rows()) && (result.cols() >= b.cols()));
    TMath_assert(a.cols() == b.rows());
    Type* resultRow = result.firstDataRow();
    const Type* a_columnData = a.firstDataColumn();
    const Type* b_rowData = b.firstDataRow();
    int i, j, k;
    for (k=0; k<b.cols(); ++k)
        resultRow[k] = a_columnData[0] * b_rowData[k];
    for (j=1; j<a.rows(); ++j) {
        resultRow = result.nextRow(resultRow);
        for (k=0; k<b.cols(); ++k)
            resultRow[k] = a_columnData[j] * b_rowData[k];
    }
    for (i=1; i<a.cols(); ++i) {
        a_columnData = a.nextColumn(a_columnData);
        b_rowData = b.nextRow(b_rowData);
        resultRow = result.firstDataRow();
        for (k=0; k<b.cols(); ++k)
            resultRow[k] += a_columnData[0] * b_rowData[k];
        for (j=1; j<a.rows(); ++j) {
            resultRow = result.nextRow(resultRow);
            for (k=0; k<b.cols(); ++k)
                resultRow[k] += a_columnData[j] * b_rowData[k];
        }
    }
}

template<typename Type>
inline TMatrix<Type> operator * (const RefTransposedMatrix<Type>& a, const TMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), b.cols());
    multiply(result, a, b);
    return result;
}

template<typename Type>
void multiply(TMatrix<Type>& result, const RefTransposedMatrix<Type>& a, const RefTransposedMatrix<Type>& b)
{
    TMath_assert((result.rows() >= a.rows()) && (result.cols() >= b.cols()));
    TMath_assert(a.cols() == b.rows());
    int i, j, k;
    Type* resultRowData = result.firstDataRow();
    const Type* b_columnData = b.firstDataColumn();
    resultRowData[0] = a(0, 0) * b_columnData[0];
    for (k=1; k<b.rows(); ++k)
        resultRowData[0] += a(0, k) * b_columnData[k];
    for (j=1; j<b.cols(); ++j) {
        b_columnData = b.nextColumn(b_columnData);
        resultRowData[j] = a(0, 0) * b_columnData[0];
        for (k=1; k<b.rows(); ++k)
            resultRowData[j] += a(0, k) * b_columnData[k];
    }
    for (i=1; i<a.rows(); ++i) {
        resultRowData = result.nextRow(resultRowData);
        b_columnData = b.firstDataColumn();
        resultRowData[0] = a(i, 0) * b_columnData[0];
        for (k=1; k<b.rows(); ++k)
            resultRowData[0] += a(i, k) * b_columnData[k];
        for (j=1; j<b.cols(); ++j) {
            b_columnData = b.nextColumn(b_columnData);
            resultRowData[j] = a(i, 0) * b_columnData[0];
            for (k=1; k<b.rows(); ++k)
                resultRowData[j] += a(i, k) * b_columnData[k];
        }
    }
}

template<typename Type>
inline TMatrix<Type> operator * (const RefTransposedMatrix<Type>& a, const RefTransposedMatrix<Type>& b)
{
    TMatrix<Type> result(a.rows(), b.cols());
    multiply(result, a, b);
    return result;
}

template<typename Type>
void multiply(TVector<Type>& result, const TMatrix<Type>& matrix, const TVector<Type>& vector)
{
    TMath_assert(result.size() >= matrix.rows());
    TMath_assert(vector.size() == matrix.cols());
    int i, j;
    const Type* dataRow = matrix.firstDataRow();
    result(0) = dataRow[0] * vector(0);
    for (j=1; j<matrix.cols(); ++j)
        result(0) += dataRow[j] * vector(j);
    for (i=1; i<matrix.rows(); ++i) {
        dataRow = matrix.nextRow(dataRow);
        result(i) = dataRow[0] * vector(0);
        for (j=1; j<matrix.cols(); ++j)
            result(i) += dataRow[j] * vector(j);
    }
}

template<typename Type>
inline TVector<Type> operator * (const TMatrix<Type>& matrix, const TVector<Type>& vector)
{
    TVector<Type> result(matrix.rows());
    multiply(result, matrix, vector);
    return result;
}

template<typename Type>
void multiply(TVector<Type>& result, const RefTransposedMatrix<Type>& matrix, const TVector<Type>& vector)
{
    TMath_assert(result.size() >= matrix.rows());
    TMath_assert(vector.size() == matrix.cols());
    int i, j;
    const Type* columnData = matrix.firstDataColumn();
    for (j=0; j<matrix.rows(); ++j)
        result(j) = columnData[j] * vector(0);
    for (i=1; i<matrix.cols(); ++i) {
        columnData = matrix.nextColumn(columnData);
        for (j=0; j<matrix.rows(); ++j)
            result(j) += columnData[j] * vector(i);
    }
}

template<typename Type>
inline TVector<Type> operator * (const RefTransposedMatrix<Type>& matrix, const TVector<Type>& vector)
{
    TVector<Type> result(matrix.rows());
    multiply(result, matrix, vector);
    return result;
}

template<typename Type>
void multiply(TVector<Type>& result, const TVector<Type>& vector, const TMatrix<Type>& matrix)
{
    TMath_assert(result.size() >= matrix.cols());
    TMath_assert(vector.size() == matrix.rows());
    int i, j;
    const Type* rowData = matrix.firstDataRow();
    for (j=0; j<matrix.cols(); ++j)
        result(j) = rowData[j] * vector(0);
    for (i=1; i<matrix.rows(); ++i) {
        rowData = matrix.nextRow(rowData);
        for (j=0; j<matrix.cols(); ++j)
            result(j) += rowData[j] * vector(i);
    }
}

template<typename Type>
inline TVector<Type> operator * (const TVector<Type>& vector, const TMatrix<Type>& matrix)
{
    TVector<Type> result(matrix.cols());
    multiply(result, vector, matrix);
    return result;
}

template<typename Type>
inline void operator *= (TVector<Type>& vector, const TMatrix<Type>& matrix)
{
    TVector<Type> result(matrix.cols());
    result = vector * matrix;
    vector.swapData(result);
}

template<typename Type>
void multiply(TVector<Type>& result, const TVector<Type>& vector, const RefTransposedMatrix<Type>& matrix)
{
    TMath_assert(result.size() >= matrix.cols());
    TMath_assert(vector.size() == matrix.rows());
    int i, j;
    const Type* columnData = matrix.firstDataColumn();
    result(0) = columnData[0] * vector(0);
    for (j=1; j<matrix.rows(); ++j)
        result(0) += columnData[j] * vector(j);
    for (i=1; i<matrix.cols(); ++i) {
        columnData = matrix.nextColumn(columnData);
        result(i) = columnData[0] * vector(0);
        for (j=1; j<matrix.rows(); ++j)
            result(i) += columnData[j] * vector(j);
    }
}

template<typename Type>
inline TVector<Type> operator * (const TVector<Type>& vector, const RefTransposedMatrix<Type>& matrix)
{
    TVector<Type> result(matrix.cols());
    multiply(result, vector, matrix);
    return result;
}

template<typename Type>
inline void operator *= (TVector<Type>& vector, const RefTransposedMatrix<Type>& matrix)
{
    TVector<Type> result(matrix.cols());
    result = vector * matrix;
    vector.swapData(result);
}

template<typename Type>
Type TMatrix<Type>::determinant() const
{
    TMath_assert(isSquareMatrix());
    int rank = m_rows;
    Type sign = (Type)1;
    const int size = rank * rank;
    Type* dataG = new Type[size];
    int i, j, k, index;
    for (i=0; i<size; ++i)
        dataG[i] = m_data[i];
    Type temp, maxVal;
    Type* strG;
    Type* strG_a;
    int* indexRows = new int[rank];
    for (i=0; i<rank; ++i)
        indexRows[i] = i;
    for (i=0; i<rank; ++i) {
        index = i;
        maxVal = std::fabs(dataG[indexRows[i] * rank + i]);
        for (j=i+1; j<rank; ++j) {
            temp = std::fabs(dataG[indexRows[j] * rank + i]);
            if (temp > maxVal) {
                index = j;
                maxVal = temp;
            }
        }
        if (maxVal < std::numeric_limits<Type>::epsilon()) {
            delete[] indexRows;
            delete[] dataG;
            return (Type)0;
        }
        if (index != i) {
            j = index;
            index = indexRows[j];
            indexRows[j] = indexRows[i];
            indexRows[i] = index;
            sign = - sign;
        } else {
            index = indexRows[i];
        }
        strG = &dataG[index * rank];
        temp = - (Type)(1) / strG[i];
        for (j=i+1; j<rank; ++j) {
            strG_a = &dataG[indexRows[j] * rank];
            Type kF = strG_a[i] * temp;
            for (k=i+1; k<rank; ++k)
                strG_a[k] += strG[k] * kF;
        }
    }
    Type determinant = dataG[indexRows[0] * rank];
    for (i=1; i<rank; ++i) {
        j = indexRows[i];
        maxVal = dataG[indexRows[i] * rank + i];
        determinant *= dataG[indexRows[i] * rank + i];
    }
    determinant *= sign;
    delete[] indexRows;
    delete[] dataG;
    return determinant;
}

template<typename Type>
bool TMatrix<Type>::invert()
{
    //used LU decomposition
    TMath_assert(isSquareMatrix());
    TMatrix<Type> LU = (*this);
    TVector<int> P(m_rows);
    if (!TTools::computeLUP_decomposition(LU, P))
        return false;
    TMatrix<Type> temp(m_rows, m_rows);
    int i, j, k;
    Type* strA = temp.firstDataRow();
    Type* strB = LU.firstDataRow();
    Type* strC;
    for (i=1; i<m_rows; ++i) {
        strA = temp.nextRow(strA);
        strB = LU.nextRow(strB);
        strA[0] = - strB[0];
        strC = temp.firstDataRow();
        for (j=1; j<i; ++j) {
            strC = temp.nextRow(strC);
            for (k=0; k<j; ++k)
                strA[k] -= strC[k] * strB[j];
            strA[j] = - strB[j];
        }
    }
    const int lastIndex = m_rows - 1;
    const int preLastIndex = lastIndex - 1;
    strA = temp.getDataRow(lastIndex);
    strB = LU.getDataRow(lastIndex);
    strA[lastIndex] = (Type(1)) / strB[lastIndex];
    for (i=preLastIndex; i>=0; --i) {
        strA = temp.prevRow(strA);
        strB = LU.prevRow(strB);
        strA[i] = (Type(1)) / strB[i];
        for (j=i+1; j<m_rows; ++j)
            strB[j] *= strA[i];
        strC = temp.getDataRow(lastIndex);
        strA[lastIndex] = - strC[lastIndex] * strB[lastIndex];
        for (j=preLastIndex; j>i; --j) {
            strC = temp.prevRow(strC);
            strA[j] = - strC[j] * strB[j];
            for (k=j+1; k<m_rows; ++k)
                strA[k] -= strC[k] * strB[j];
        }
    }
    strA = firstDataRow();
    strB = temp.firstDataRow();
    for (j=0; j<m_rows; ++j) {
        strA[P(j)] = strB[j];
        for (k=j+1; k<m_rows; ++k)
            strA[P(j)] += strB[k] * temp(k, j);
    }
    for (i=1; i<m_rows; ++i) {
        strA = nextRow(strA);
        strB = temp.nextRow(strB);
        for (j=0; j<i; ++j) {
            strA[P(j)] = strB[i] * temp(i, j);
            for (k=i+1; k<m_rows; ++k)
                strA[P(j)] += strB[k] * temp(k, j);
        }
        for (; j<m_rows; ++j) {
            strA[P(j)] = strB[j];
            for (k=j+1; k<m_rows; ++k)
                strA[P(j)] += strB[k] * temp(k, j);
        }
    }
    return true;
}

template <typename Type>
bool TMatrix<Type>::operator == (const RefTransposedMatrix<Type>& b) const
{
    if ((m_rows != b.rows()) || (m_cols != b.cols()))
        return false;

    const Type* dataRow = firstDataRow();
    const Type* mData = b.m_data;
    int i, j;
    for (j=0; j<m_cols; ++j) {
        if (dataRow[j] != mData[m_rows * j])
            return false;
    }
    for (i=1; i<m_rows; ++i) {
        dataRow = nextRow(dataRow);
        ++mData;
        for (j=0; j<m_cols; ++j) {
            if (dataRow[j] != mData[m_rows * j])
                return false;
        }
    }
    return true;
}

}

#endif // TMATH_TMATRIX_IMPL_H
