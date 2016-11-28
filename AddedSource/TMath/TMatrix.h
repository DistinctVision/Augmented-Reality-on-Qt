#ifndef TMATH_TMATRIX_H
#define TMATH_TMATRIX_H

#include "TVector.h"

namespace TMath {

template<typename Type = TMath_DefaultType>
class RefTransposedMatrix;

template<typename Type = TMath_DefaultType>
class TMatrix
{
public:
    typedef Type TypeElement;

public:
    static TMatrix<Type> asColumn(const TVector<Type>& vector)
    {
        return TMatrix<Type>(vector.size(), 1, vector.data());
    }

    static TMatrix<Type> asRow(const TVector<Type>& vector)
    {
        return TMatrix<Type>(1, vector.size(), vector.data());
    }

    TMatrix()
    {
        m_rows = 0;
        m_cols = 0;
        m_data = nullptr;
    }

    TMatrix(int rows, int cols)
    {
        TMath_assert((rows > 0) && (cols > 0));
        m_rows = rows;
        m_cols = cols;
        m_data = new Type[m_rows * m_cols];
    }

    TMatrix(const TMatrix& matrix)
    {
        TMath_assert((matrix.m_rows > 0) && (matrix.m_cols > 0));
        m_rows = matrix.m_rows;
        m_cols = matrix.m_cols;
        const int size = m_rows * m_cols;
        m_data = new Type[size];
        for (int i=0; i<size; ++i)
            m_data[i] = matrix.m_data[i];
    }

    TMatrix(const RefTransposedMatrix<Type>& matrix);

    TMatrix(TMatrix&& moveMatrix)
    {
        TMath_assert((moveMatrix.m_rows > 0) && (moveMatrix.m_cols > 0));
        m_data = moveMatrix.m_data;
        m_rows = moveMatrix.m_rows;
        m_cols = moveMatrix.m_cols;
        moveMatrix.m_data = nullptr;
        moveMatrix.m_rows = 0;
        moveMatrix.m_cols = 0;
    }

    TMatrix<Type>(int rows, int cols, const Type* data)
    {
        TMath_assert((rows > 0) && (cols > 0));
        m_rows = rows;
        m_cols = cols;
        const int size = m_rows * m_cols;
        m_data = new Type[size];
        copyDataFrom(data);
    }

    ~TMatrix()
    {
        if (m_data != nullptr)
            delete[] m_data;
    }

    inline const Type* data() const
    {
        return m_data;
    }

    inline Type* data()
    {
        return m_data;
    }

    inline int rows() const
    {
        return m_rows;
    }

    inline int cols() const
    {
        return m_cols;
    }

    inline void swapData(TMatrix<Type>& matrix)
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        std::swap(m_data, matrix.m_data);
    }

    inline void swap(TMatrix<Type>& matrix)
    {
        std::swap(m_rows, matrix.m_rows);
        std::swap(m_cols, matrix.m_cols);
        std::swap(m_data, matrix.m_data);
    }

    bool isValid()
    {
        return ((m_rows > 0) && (m_cols > 0));
    }

    inline Type* firstDataRow()
    {
        return m_data;
    }

    inline const Type* firstDataRow() const
    {
        return m_data;
    }

    inline Type* getDataRow(int row)
    {
        TMath_assert((row >= 0) && (row < m_rows));
        return &m_data[row * m_cols];
    }

    inline const Type* getDataRow(int row) const
    {
        TMath_assert((row >= 0) && (row < m_rows));
        return &m_data[row * m_cols];
    }

    inline Type* nextRow(Type* row)
    {
        TMath_assert(((long)(row) - (long)m_data) / (long)(m_cols * sizeof(Type)) < (long)(m_rows));
        return &row[m_cols];
    }
    inline const Type* nextRow(const Type* row) const
    {
        TMath_assert(((long)(row) - (long)m_data) / (long)(m_cols * sizeof(Type)) < (long)(m_rows));
        return &row[m_cols];
    }

    inline Type* prevRow(Type* row)
    {
        TMath_assert(((long)(row) - (long)m_data) / (long)(m_cols * sizeof(Type)) > (long)(0));
        return &row[-m_cols];
    }

    inline const Type* prevRow(const Type* row) const
    {
        TMath_assert(((long)(row) - (long)m_data) / (long)(m_cols * sizeof(Type)) > (long)(0));
        return &row[-m_cols];
    }

    void setToIdentity()
    {
        TMath_assert((m_rows > 0) && (m_cols > 0));
        int i, j;
        Type* row = m_data;
        row[0] = Type(1);
        for (j=1; j<m_cols; ++j)
            row[j] = Type(0);
        if (m_rows <= m_cols) {
            for (i=1; i<m_rows; ++i) {
                row = nextRow(row);
                for (j=0; j<i; ++j)
                    row[j] = Type(0);
                for (j=i+1; j<m_cols; ++j)
                    row[j] = Type(0);
                row[i] = Type(1);
            }
        } else {
            for (i=1; i<m_cols; ++i) {
                row = nextRow(row);
                for (j=0; j<i; ++j)
                    row[j] = Type(0);
                for (j=i+1; j<m_cols; ++j)
                    row[j] = Type(0);
                row[i] = Type(1);
            }
            for (i=m_cols; i<m_rows; ++i) {
                row = nextRow(row);
                for (j=0; j<m_cols; ++j)
                    row[j] = Type(0);
            }
        }
    }

    void setDiagonal(const TVector<Type>& diagonal)
    {
        TMath_assert(diagonal.size() == ((m_rows > m_cols) ? m_cols : m_rows));
        for (int i=0; i<diagonal.size(); ++i)
            m_data[i * m_cols + i] = diagonal(i);
    }

    void setZero()
    {
        const int size = m_rows * m_cols;
        for (int i=0; i<size; ++i)
            m_data[i] = (Type)0;
    }

    inline const Type& operator ()(int row, int col) const
    {
        TMath_assert((row >= 0) && (row < m_rows) && (col >= 0) && (col < m_cols));
        return m_data[row * m_cols + col];
    }

    inline Type& operator ()(int row, int col)
    {
        TMath_assert((row >= 0) && (row < m_rows) && (col >= 0) && (col < m_cols));
        return m_data[row * m_cols + col];
    }

    inline void copyDataFrom(const Type* dataFrom)
    {
        const int size = m_rows * m_cols;
        for (int i=0; i<size; ++i)
            m_data[i] = dataFrom[i];
    }

    TMatrix<Type>& operator = (const TMatrix<Type>& matrix)
    {
        if ((m_rows != matrix.rows()) || (m_cols != matrix.cols())) {
            TMath_assert((matrix.m_rows > 0) && (matrix.m_cols > 0));
            if (m_data != nullptr)
                delete[] m_data;
            m_rows = matrix.m_rows;
            m_cols = matrix.m_cols;
            const int size = m_rows * m_cols;
            m_data = new Type[size];
            for (int i=0; i<size; ++i)
                m_data[i] = matrix.m_data[i];
        }
        copyDataFrom(matrix.data());
        return (*this);
    }

    TMatrix<Type>& operator = (TMatrix<Type>&& moveMatrix)
    {
        TMath_assert(this != &moveMatrix);
        if (m_data != nullptr)
            delete[] m_data;
        m_data = moveMatrix.m_data;
        m_rows = moveMatrix.m_rows;
        m_cols = moveMatrix.m_cols;
        moveMatrix.m_data = nullptr;
        moveMatrix.m_rows = 0;
        moveMatrix.m_cols = 0;
        return (*this);
    }

    TMatrix<Type> operator - () const
    {
        TMatrix result(m_rows, m_cols);
        const int size = m_cols * m_rows;
        for (int i=0; i<size; ++i)
            result.m_data[i] = - m_data[i];
        return result;
    }

    TMatrix<Type> operator * (Type val) const
    {
        const int size = m_cols * m_rows;
        TMatrix<Type> result(m_rows, m_cols);
        Type* data = result.data();
        for (int i=0; i<size; ++i)
            data[i] = m_data[i] * val;
        return result;
    }

    void operator *= (Type val)
    {
        const int size = m_cols * m_rows;
        for (int i=0; i<size; ++i)
            m_data[i] *= val;
    }

    TMatrix<Type> operator + (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        const int size = m_rows * m_cols;
        TMatrix<Type> result(m_rows, m_cols);
        for (int i=0; i<size; ++i)
            result.m_data[i] = m_data[i] + matrix.m_data[i];
        return result;
    }

    TMatrix<Type> operator - (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        const int size = m_rows * m_cols;
        TMatrix<Type> result(m_rows, m_cols);
        for (int i=0; i<size; ++i)
            result.m_data[i] = m_data[i] - matrix.m_data[i];
        return result;
    }

    void operator += (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        const int size = m_rows * m_cols;
        for (int i=0; i<size; ++i)
            m_data[i] += matrix.m_data[i];
    }

    void operator -= (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        const int size = m_rows * m_cols;
        for (int i=0; i<size; ++i)
            m_data[i] -= matrix.m_data[i];
    }

    inline void getRow(TVector<Type>& result, int row, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert(size <= m_cols);
        const Type* dataRow = getDataRow(row);
        for (int i=0; i<size; ++i)
            result(i) = dataRow[i];
    }

    inline TVector<Type> getRow(int row, int size) const
    {
        TVector<Type> result(size);
        getRow(result, row, size);
        return result;
    }

    inline TVector<Type> getRow(int row) const
    {
        TVector<Type> result(m_cols);
        getRow(result, row, m_cols);
        return result;
    }

    void getColumn(TVector<Type>& result, int col, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert((col >= 0) && (col < m_cols) && (size <= m_rows));
        result(0) = m_data[col];
        for (int i=1; i<size; ++i)
            result(i) = m_data[m_cols * i + col];
    }

    inline TVector<Type> getColumn(int col, int size) const {
        TVector<Type> result(size);
        getColumn(result, col, size);
        return result;
    }

    inline TVector<Type> getColumn(int col) const {
        TVector<Type> result(m_rows);
        getColumn(result, col, m_rows);
        return result;
    }

    void setRow(int row, const  TVector<Type>& vector)
    {
        TMath_assert(m_cols >= vector.size());
        Type* dataRow = getDataRow(row);
        for (int i=0; i<vector.size(); ++i)
            dataRow[i] = vector(i);
    }

    void setColumn(int col, const  TVector<Type>& vector)
    {
        TMath_assert(m_rows >= vector.size());
        for (int i=0; i<vector.size(); ++i)
            m_data[i * m_cols + col] = vector(i);
    }

    void swapRows(int rowA, int rowB)
    {
        TMath_assert((rowA >= 0) && (rowA < m_rows) && (rowB >= 0) && (rowB < m_rows));
        Type tempVal;
        Type* strA = getDataRow(rowA);
        Type* strB = getDataRow(rowB);
        for (int i=0; i<m_cols; ++i) {
            tempVal = strA[i];
            strA[i] = strB[i];
            strB[i] = tempVal;
        }
    }

    void slice(TMatrix<Type>& result, int beginRow, int beginCol, int rows, int cols) const
    {
        TMath_assert((result.rows() >= rows) && (result.cols() >= cols));
        TMath_assert((rows > 0) && ((beginRow + rows) <= m_rows) && (cols > 0) && ((beginCol + cols) <= m_cols));
        const Type* dataRow = &m_data[beginRow * m_cols + beginCol];
        Type* resultRow = result.firstDataRow();
        int i, j;
        for (j=0; j<cols; ++j)
            resultRow[j] = dataRow[j];
        for (i=1; i<rows; ++i) {
            dataRow = nextRow(dataRow);
            resultRow = result.nextRow(resultRow);
            for (j=0; j<cols; ++j)
                resultRow[j] = dataRow[j];
        }
    }

    inline TMatrix<Type> slice(int beginRow, int beginCol, int rows, int cols) const
    {
        TMatrix<Type> result(rows, cols);
        slice(result, beginRow, beginCol, rows, cols);
        return result;
    }

    void slice(TMatrix<Type>& result, int rows, int cols) const
    {
        TMath_assert((result.rows() >= rows) && (result.cols() >= cols));
        TMath_assert((rows > 0) && (rows <= m_rows) && (cols > 0) && (cols <= m_cols));
        const Type* dataRow = firstDataRow();
        Type* resultRow = result.firstDataRow();
        int i, j;
        for (j=0; j<cols; ++j)
            resultRow[j] = dataRow[j];
        for (i=1; i<rows; ++i) {
            dataRow = nextRow(dataRow);
            resultRow = result.nextRow(resultRow);
            for (j=0; j<cols; ++j)
                resultRow[j] = dataRow[j];
        }
    }
    inline TMatrix<Type> slice(int rows, int cols) const
    {
        TMatrix<Type> result(rows, cols);
        slice(result, rows, cols);
        return result;
    }

    void fill(Type value)
    {
        const int size = m_rows * m_cols;
        for (int i=0; i<size; ++i)
            m_data[i] = value;
    }

    void fill(int beginRow, int beginCol, const TMatrix<Type>& matrix)
    {
        TMath_assert((m_rows >= (beginRow + matrix.rows())) && (m_cols >= (beginCol + matrix.cols())));
        TMath_assert(matrix.rows() > 0);
        int i, j;
        Type* dataRow = &m_data[beginRow * m_cols + beginCol];
        const Type* mDataRow = matrix.firstDataRow();
        for (j=0; j<matrix.cols(); ++j)
            dataRow[j] = mDataRow[j];
        for (i=1; i<matrix.rows(); ++i) {
            dataRow = nextRow(dataRow);
            mDataRow = matrix.nextRow(mDataRow);
            for (j=0; j<matrix.cols(); ++j)
                dataRow[j] = mDataRow[j];
        }
    }

    void fill(int beginRow, int beginCol, const RefTransposedMatrix<Type>& matrix);

    inline bool isSquareMatrix() const
    {
        return (m_rows == m_cols);
    }

    void transpose()
    {
        Type temp;
        Type* dataRow = firstDataRow();
        int i, j;
        for (j=1; j<m_cols; ++j) {
            const int index = m_rows * j;
            temp = m_data[index];
            m_data[index] = dataRow[j];
            dataRow[j] = temp;
        }
        for (i=1; i<m_rows; ++i) {
            dataRow = nextRow(dataRow);
            for (j=i+1; j<m_cols; ++j) {
                const int index = m_rows * j + i;
                temp = m_data[index];
                m_data[index] = dataRow[j];
                dataRow[j] = temp;
            }
        }
    }

    void transposed(TMatrix<Type>& result) const
    {
        TMath_assert((result.rows() >= m_cols) && (result.cols() >= m_rows));
        const Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            result(j, 0) = rowData[j];
        for (i=1; i<m_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<m_cols; ++j)
                result(j, i) = rowData[j];
        }
    }

    inline TMatrix<Type> transposed() const
    {
        TMatrix<Type> result(m_cols, m_rows);
        transposed(result);
        return result;
    }

    void operator = (const RefTransposedMatrix<Type>& matrix);

    inline RefTransposedMatrix<Type> refTransposed() const;

    void operator += (const RefTransposedMatrix<Type>& matrix)
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            rowData[j] += matrix(0, j);
        for (i=1; i<m_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<m_cols; ++j)
                rowData[j] += matrix(i, j);
        }
    }
    void operator -= (const RefTransposedMatrix<Type>& matrix)
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            rowData[j] -= matrix(0, j);
        for (i=1; i<m_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<m_cols; ++j)
                rowData[j] -= matrix(i, j);
        }
    }
    TMatrix<Type> operator + (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_rows, m_cols);
        const Type* rowData = firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            resultRowData[j] = rowData[j] + matrix(0, j);
        for (i=1; i<m_rows; ++i) {
            rowData = nextRow(rowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<m_cols; ++j)
                resultRowData[j] = rowData[j] + matrix(i, j);
        }
        return result;
    }

    TMatrix<Type> operator - (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_rows, m_cols);
        const Type* rowData = firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            resultRowData[j] = rowData[j] - matrix(0, j);
        for (i=1; i<m_rows; ++i) {
            rowData = nextRow(rowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<m_cols; ++j)
                resultRowData[j] = rowData[j] - matrix(i, j);
        }
        return result;
    }

    Type determinant() const;

    bool invert();

    TMatrix<Type> inverted() const
    {
        TMatrix<Type> inv = (*this);
        if (!inv.invert())
            inv.setZero();
        return inv;
    }

    bool operator == (const TMatrix<Type>& b) const
    {
        if ((m_rows != b.rows()) || (m_cols != b.cols()))
            return false;
        int size = m_rows * m_cols;
        const Type* bData = b.data();
        for (int i = 0; i < size; ++i) {
            if (m_data[i] != bData[i])
                return false;
        }
        return true;
    }

    bool operator != (const TMatrix<Type>& b) const
    {
        return !(operator == (b));
    }

    bool operator == (const RefTransposedMatrix<Type>& b) const;

    bool operator != (const RefTransposedMatrix<Type>& b) const
    {
        return !(operator == (b));
    }

    template <typename CastType>
    inline TMatrix<CastType> cast() const
    {
        TMatrix<CastType> result(m_rows, m_cols);
        const int size = m_rows * m_cols;
        CastType* data = result.data();
        for (int i = 0; i < size; ++i)
            data[i] = (CastType)m_data[i];
        return result;
    }

    static TMatrix<Type> Identity(int size)
    {
        TMatrix<Type> I(size, size);
        I.setToIdentity();
        return I;
    }

private:
    friend class TSVD<Type>;

    int m_rows;
    int m_cols;
    Type* m_data;
};


template<typename Type>
class RefTransposedMatrix
{
public:
    typedef Type TypeElement;

public:
    RefTransposedMatrix(const TMatrix<Type>* matrix):
        m_rows(matrix->cols()), m_cols(matrix->rows())
    {
        m_matrix = matrix;
        m_data = matrix->data();
    }

    inline const TMatrix<Type>* sourceMatrix() const
    {
        return m_matrix;
    }

    inline int rows() const
    {
        return m_rows;
    }

    inline int cols() const
    {
        return m_cols;
    }

    inline const Type* data() const
    {
        return m_data;
    }

    inline const Type* firstDataColumn() const
    {
        return m_data;
    }

    inline const Type* getDataColumn(int col) const
    {
        TMath_assert(col < m_cols);
        return &m_data[col * m_rows];
    }

    inline const Type* nextColumn(const Type* column) const
    {
        TMath_assert(((long)(column) - (long)m_data) / (m_rows * sizeof(Type)) < (unsigned int)(m_cols - 1));
        return &column[m_rows];
    }

    inline const Type* prevColumn(const Type* column) const
    {
        TMath_assert(((long)(column) - (long)m_data) / (m_rows * sizeof(Type)) > (unsigned int)(0));
        return &column[-m_rows];
    }

    inline const Type& operator ()(int row, int col) const
    {
        TMath_assert((row >= 0) && (row < m_rows) && (col >= 0) && (col <= m_cols));
        return m_data[col * m_rows + row];
    }

    TMatrix<Type> operator * (Type val) const
    {
        const int size = m_cols * m_rows;
        TMatrix<Type> result(m_cols, m_rows);
        const Type* columnData = firstDataColumn();
        int i, j;
        for (j=0; j<m_rows; ++j)
            result(j, 0) = columnData[j] * val;
        for (i=1; i<size; ++i) {
            columnData = nextColumn(columnData);
            for (j=0; j<m_rows; ++j)
                result(j, i) = columnData[j] * val;
        }
        return result;
    }

    TMatrix<Type> operator - () const
    {
        TMatrix<Type> result(m_rows, m_cols);
        int i, j;
        const Type* columnData = firstDataColumn();
        for (j=0; j<m_rows; ++j)
            result(j, 0) = - columnData[j];
        for (i=1; i<m_cols; ++i) {
            columnData = nextColumn(columnData);
            for (j=0; j<m_rows; ++j)
                result(j, i) = - columnData[j];
        }
        return result;
    }

    TMatrix<Type> operator + (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_rows, m_cols);
        int i, j;
        const Type* columnData = firstDataColumn();
        const Type* matrixColumnData = matrix.firstDataColumn();
        for (j=0; j<m_rows; ++j)
            result(j, 0) = columnData[j] + matrixColumnData[j];
        for (i=1; i<m_cols; ++i) {
            columnData = nextColumn(columnData);
            matrixColumnData = matrix.nextColumn(matrixColumnData);
            for (j=0; j<m_rows; ++j)
                result(j, i) = columnData[j] + matrixColumnData[j];
        }
        return result;
    }

    TMatrix<Type> operator - (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_rows, m_cols);
        int i, j;
        const Type* columnData = firstDataColumn();
        const Type* matrixColumnData = matrix.firstDataColumn();
        for (j=0; j<m_rows; ++j)
            result(j, 0) = columnData[j] - matrixColumnData[j];
        for (i=1; i<m_cols; ++i) {
            columnData = nextColumn(columnData);
            matrixColumnData = matrix.nextColumn(matrixColumnData);
            for (j=0; j<m_rows; ++j)
                result(j, i) = columnData[j] - matrixColumnData[j];
        }
        return result;
    }

    TMatrix<Type> operator + (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_rows, m_cols);
        const Type* matrixRowData = matrix.firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<m_cols; ++j)
            resultRowData[j] = m_data[j * m_rows] + matrixRowData[j];
        for (i=1; i<m_rows; ++i) {
            matrixRowData = matrix.nextRow(matrixRowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<m_cols; ++j)
                resultRowData[j] = m_data[j * m_rows + i] + matrixRowData[j];
        }
        return result;
    }

    TMatrix<Type> operator - (const TMatrix<Type>& matrix) const
    {
        TMath_assert((m_rows == matrix.rows()) && (m_cols == matrix.cols()));
        TMatrix<Type> result(m_cols, m_rows);
        const Type* matrixRowData = matrix.firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<m_rows; ++j)
            resultRowData[j] = m_data[j * m_cols] - matrixRowData[j];
        for (i=1; i<m_cols; ++i) {
            matrixRowData = matrix.nextRow(matrixRowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<m_rows; ++j)
                resultRowData[j] = m_data[j * m_cols + i] - matrixRowData[j];
        }
        return result;
    }

    inline void getColumn(TVector<Type>& result, int col, int size) const
    {
        m_matrix->getRow(result, col, size);
    }

    inline TVector<Type> getColumn(int col, int size) const
    {
        return m_matrix->getRow(col, size);
    }

    inline TVector<Type> getColumn(int col) const
    {
        return m_matrix->getRow(col, m_rows);
    }

    inline void getRow(TVector<Type>& result, int row, int size) const
    {
        m_matrix->getColumn(result, row, size);
    }

    inline TVector<Type> getRow(int row, int size) const
    {
        return m_matrix->getColumn(row, size);
    }

    inline TVector<Type> getRow(int row) const
    {
        return m_matrix->getColumn(row, m_cols);
    }

    inline bool isSquareMatrix() const
    {
        return (m_rows == m_cols);
    }

    inline Type determinant() const
    {
        return m_matrix->determinant();
    }

    template <typename CastType>
    inline TMatrix<CastType> cast() const
    {
        TMatrix<CastType> result(m_rows, m_cols);
        Type* dataRow = result.data();
        const Type* mData = m_data;
        int i, j;
        for (j=0; j<m_cols; ++j)
            dataRow[j] = mData[j * m_rows];
        for (i=1; i<m_rows; ++i) {
            dataRow = result.nextRow(dataRow);
            ++mData;
            for (j=0; j<m_cols; ++j)
                dataRow[j] = mData[j * m_rows];
        }
        return result;
    }

    bool operator == (const TMatrix<Type>& b) const
    {
        return (b == (*this));
    }

    bool operator != (const TMatrix<Type>& b) const
    {
        return (b != (*this));
    }

    bool operator == (const RefTransposedMatrix<Type>& b) const
    {
        if ((m_rows != b.rows()) || (m_cols != b.cols()))
            return false;
        int size = m_rows * m_cols;
        const Type* bData = b.data();
        for (int i = 0; i < size; ++i) {
            if (m_data[i] != bData[i])
                return false;
        }
        return true;
    }

    bool operator != (const RefTransposedMatrix<Type>& b) const
    {
        return !(operator == (b));
    }

private:
    friend class TMatrix<Type>;
    friend class TSVD<Type>;

    const TMatrix<Type>* m_matrix;
    const int m_rows;
    const int m_cols;
    const Type* m_data;
};

typedef TMatrix<float> TMatrixf;
typedef TMatrix<double> TMatrixd;

} // namespace TMath

#endif // TMATH_TMATRIX_H
