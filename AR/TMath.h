#ifndef TMATH_H
#define TMATH_H

#include <cfloat>
#include <cmath>
#include <cassert>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QGenericMatrix>
#include <QMatrix4x4>

//#define TMath_assert(condition) assert(condition);
#define TMath_assert(condition) static_cast<void>(condition);

namespace TMath {

template<typename Type>
class TSVD;

template<typename Type>
class Tools;

typedef float DefaultType;

template<typename Type = DefaultType>
class TVector
{
    friend class TSVD<Type>;
    friend class Tools<Type>;
public:
    typedef Type TypeElement;

public:
    TVector() { _size = 0; _data = nullptr; }
    TVector(int size) { TMath_assert(size > 0); _size = size; _data = new Type[_size]; }
    TVector(const TVector& vector) { _size = vector.size(); _data = new Type[_size];
        for (int i=0; i<_size; ++i) _data[i] = vector(i); }
    TVector(TVector&& moveVector) { _data = moveVector._data; _size = moveVector._size;
                                    moveVector._data = nullptr; moveVector._size = 0; }
    inline TVector(Type x1, Type x2) { _size = 2; _data = new Type[2]; _data[0] = x1; _data[1] = x2; }
    inline TVector(Type x1, Type x2, Type x3) { _size = 3; _data = new Type[3]; _data[0] = x1; _data[1] = x2; _data[2] = x3; }
    inline TVector(Type x1, Type x2, Type x3, Type x4) { _size = 4; _data = new Type[4]; _data[0] = x1; _data[1] = x2;
                                                                                         _data[2] = x3; _data[3] = x4; }
    inline TVector(const Type* dataFrom, int size) { _size = size; _data = new Type[size];
                                                     for (int i=0; i<_size; ++i) _data[i] = dataFrom[i]; }
    ~TVector() { delete[] _data; }
    void setZero() { for (int i=0; i<_size; ++i) _data[i] = (Type)0; }
    inline int size() const { return _size; }
    inline Type& operator() (int index) { TMath_assert((index >= 0) && (index < _size)); return _data[index]; }
    inline const Type& operator() (int index) const { TMath_assert((index >= 0) && (index < _size)); return _data[index]; }
    const Type* data() const { return _data; }
    Type* data() { return _data; }
    void swapData(TVector<Type>& vector) { TMath_assert(_size == vector.size()); std::swap(_data, vector._data); }
    void swap(TVector<Type>& vector) { std::swap(_size, vector._size); std::swap(_data, vector._data); }
    inline void recreate(int newSize) { delete[] _data; _size = newSize; _data = new Type[_size]; }
    TVector<Type>& operator = (const TVector& vector) { if (_size != vector.size()) recreate(vector.size());
                                              for (int i=0; i<_size; ++i) _data[i] = vector(i); return (*this); }
    TVector<Type>& operator = (TVector&& moveVector)
    {
        TMath_assert(this != &moveVector);
        delete[] _data;
        _data = moveVector._data;
        _size = moveVector._size;
        moveVector._data = nullptr;
        moveVector._size = 0;
        return (*this);
    }
    TVector<Type> operator - () const
    {
        TVector<Type> result(_size);
        for (int i=0; i<_size; ++i)
            result(i) = - _data[i];
        return result;
    }
    TVector<Type> operator + (const TVector& vector) const
    {
        TMath_assert(_size == vector.size());
        TVector<Type> result(_size);
        for (int i=0; i<_size; ++i)
            result(i) = _data[i] + vector(i);
        return result;
    }
    void operator += (const TVector& vector)
    {
        TMath_assert(_size == vector.size());
        for (int i=0; i<_size; ++i)
            _data[i] += vector(i);
    }
    TVector<Type> operator - (const TVector& vector) const
    {
        TMath_assert(_size == vector.size());
        TVector<Type> result(_size);
        for (int i=0; i<_size; ++i)
            result(i) = _data[i] - vector(i);
        return result;
    }
    void operator -= (const TVector& vector)
    {
        TMath_assert(_size == vector.size());
        for (int i=0; i<_size; ++i)
            _data[i] -= vector(i);
    }
    TVector<Type> operator * (float val) const
    {
        TVector<Type> result(_size);
        for (int i=0; i<_size; ++i)
            result(i) = _data[i] * val;
        return result;
    }
    TVector<Type> operator / (float val) const
    {
        TVector<Type> result(_size);
        for (int i=0; i<_size; ++i)
            result(i) = _data[i] / val;
        return result;
    }
    void operator *= (const TVector& vector)
    {
        TMath_assert(_size == vector.size());
        for (int i=0; i<_size; ++i)
            _data[i] *= vector(i);
    }
    void operator *= (float val)
    {
        for (int i=0; i<_size; ++i)
            _data[i] *= val;
    }
    void operator /= (const TVector& vector)
    {
        TMath_assert(_size == vector.size());
        for (int i=0; i<_size; ++i)
            _data[i] /= vector(i);
    }
    void operator /= (float val)
    {
        for (int i=0; i<_size; ++i)
            _data[i] /= val;
    }
    Type lengthSquare() const
    {
        Type result = _data[0] * _data[0];
        for (int i=1; i<_size; ++i)
            result += _data[i] * _data[i];
        return result;
    }
    Type length() const
    {
        return std::sqrt(lengthSquare());
    }
    Type normalize()
    {
        Type l = length();
        if (l < FLT_EPSILON) {
            setZero();
            return (Type)0;
        }
        for (int i=0; i<_size; ++i)
            _data[i] /= l;
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
        TMath_assert((size > 0) && (size < _size));
        result(0) = _data[0];
        for (int i=1; i<size; ++i)
            result(i) = _data[i];
    }
    inline TVector<Type> slice(int size) const { TVector<Type> result(size); slice(result, size); return result; }
    void slice(TVector<Type>& result, int beginIndex, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert((beginIndex >= 0) && ((beginIndex + size) <= _size));
        result(0) = _data[beginIndex];
        for (int i=1; i<size; ++i)
            result(i) = _data[beginIndex + i];
    }
    inline TVector<Type> slice(int beginIndex, int size) const { TVector<Type> result(size); slice(result, beginIndex, size);
                                                                 return result; }
    bool inverse()
    {
        bool success = true;
        for (int i=0; i<_size; ++i) {
            if (std::fabs(_data[i]) > FLT_EPSILON)
                _data[i] = Type(1) / _data[i];
            else {
                _data[i] = Type(0);
                success = false;
            }
        }
        return success;
    }
    TVector<Type> projectAffine() const
    {
        TVector<Type> result(_size - 1);
        TMath_assert(std::fabs(_data[result._size]) > FLT_EPSILON);
        for (int i=0; i<result._size; ++i)
            result._data[i] = _data[i] / _data[result._size];
        return result;
    }

protected:
    Type* _data;
    int _size;
};

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
    TVector<Type> result(3);
    result(0) = a(1) * b(2) - a(2) * b(1);
    result(1) = a(2) * b(0) - a(0) * b(2);
    result(2) = a(0) * b(1) - a(1) * b(0);
    return result;
}

template<typename Type = DefaultType>
class RefTransposedMatrix;

template<typename Type = DefaultType>
class TMatrix
{
    friend class TSVD<Type>;
protected:
    int _rows;
    int _cols;
    Type* _data;

public:
    typedef Type TypeElement;

public:
    inline TMatrix() { _rows = 0; _cols = 0; _data = nullptr; }
    inline const Type* data() const { return _data; }
    inline Type* data() { return _data; }
    void swapData(TMatrix<Type>& matrix) {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols())); std::swap(_data, matrix._data); }
    void swap(TMatrix<Type>& matrix) { std::swap(_rows, matrix._rows);
                                       std::swap(_cols, matrix._cols); std::swap(_data, matrix._data); }
    inline int rows() const { return _rows; }
    inline int cols() const { return _cols; }
    inline Type* firstDataRow() { return _data; }
    inline const Type* firstDataRow() const { return _data; }
    inline Type* getDataRow(int row) { TMath_assert((row >= 0) && (row < _rows)); return &_data[row * _cols]; }
    inline const Type* getDataRow(int row) const { TMath_assert((row >= 0) && (row < _rows)); return &_data[row * _cols]; }
    inline Type* nextRow(Type* row) { TMath_assert(((unsigned int)(row) - (unsigned int)_data) / (_cols * sizeof(Type)) <
                                                         (unsigned int)(_rows - 1));
                                                  return &row[_cols]; }
    inline const Type* nextRow(const Type* row) const { TMath_assert(((unsigned int)(row) - (unsigned int)_data) /
                                                                     (_cols * sizeof(Type)) < (unsigned int)(_rows - 1));
                                                  return &row[_cols]; }
    inline Type* prevRow(Type* row) { TMath_assert(((unsigned int)(row) - (unsigned int)_data) /
                                                   (_cols * sizeof(Type)) > (unsigned int)(0));
                                                  return &row[-_cols]; }
    inline const Type* prevRow(const Type* row) const { TMath_assert(((unsigned int)(row) - (unsigned int)_data) /
                                                                     (_cols * sizeof(Type)) >
                                                         (unsigned int)(0));
                                                  return &row[-_cols]; }
    void setToIdentity()
    {
        TMath_assert((_rows > 0) && (_cols > 0));
        int i, j;
        Type* row = _data;
        row[0] = Type(1);
        for (j=1; j<_cols; ++j)
            row[j] = Type(0);
        if (_rows <= _cols) {
            for (i=1; i<_rows; ++i) {
                row = nextRow(row);
                for (j=0; j<i; ++j)
                    row[j] = Type(0);
                for (j=i+1; j<_cols; ++j)
                    row[j] = Type(0);
                row[i] = Type(1);
            }
        } else {
            for (i=1; i<_cols; ++i) {
                row = nextRow(row);
                for (j=0; j<i; ++j)
                    row[j] = Type(0);
                for (j=i+1; j<_cols; ++j)
                    row[j] = Type(0);
                row[i] = Type(1);
            }
            for (i=_cols; i<_rows; ++i) {
                row = nextRow(row);
                for (j=0; j<_cols; ++j)
                    row[j] = Type(0);
            }
        }
    }
    void setDiagonal(const TVector<Type>& diagonal)
    {
        TMath_assert(diagonal.size() == ((_rows > _cols) ? _cols : _rows));
        for (int i=0; i<diagonal.size(); ++i)
            _data[i * _cols + i] = diagonal(i);
    }
    void setZero() { const int size = _rows * _cols; for (int i=0; i<size; ++i) _data[i] = (Type)0; }
    inline TMatrix(int rows, int cols):_rows(rows), _cols(cols), _data(new Type[rows * cols])
                    { TMath_assert((_rows > 0) && (_cols > 0)); }
    TMatrix(const TMatrix& matrix)
    {
        _rows = matrix._rows;
        _cols = matrix._cols;
        const int size = _rows * _cols;
        _data = new Type[size];
        for (int i=0; i<size; ++i)
            _data[i] = matrix._data[i];
    }
    TMatrix(const RefTransposedMatrix<Type>& matrix);
    TMatrix(TMatrix&& moveMatrix)
    {
        _data = moveMatrix._data;
        _rows = moveMatrix._rows;
        _cols = moveMatrix._cols;
        moveMatrix._data = nullptr;
        moveMatrix._rows = 0;
        moveMatrix._cols = 0;
    }
    inline ~TMatrix() { delete[] _data; }
    inline const Type& operator ()(int row, int col) const { TMath_assert((row >= 0) && (row < _rows) && (col >= 0) && (col < _cols));
                                                             return _data[row * _cols + col]; }
    inline Type& operator ()(int row, int col) { TMath_assert((row >= 0) && (row < _rows) && (col >= 0) && (col < _cols));
                                                 return _data[row * _cols + col]; }
    inline void recreate(int rows, int cols) { delete[] _data; _rows = rows; _cols = cols; _data = new Type[_rows * _cols]; }
    void copyDataFrom(const Type* dataFrom)
    {
        const int size = _cols * _rows;
        for (int i=0; i<size; ++i)
            _data[i] = dataFrom[i];
    }
    TMatrix<Type>(int rows, int cols, const Type* data) { _rows = rows; _cols = cols; copyDataFrom(data); }
    TMatrix<Type>& operator = (const TMatrix<Type>& matrix)
    {
        if ((_rows != matrix.rows()) || (_cols != matrix.cols()))
            recreate(matrix.rows(), matrix.cols());
        copyDataFrom(matrix.data());
        return (*this);
    }
    TMatrix<Type>& operator = (TMatrix<Type>&& moveMatrix)
    {
        TMath_assert(this != &moveMatrix);
        delete[] _data;
        _data = moveMatrix._data;
        _rows = moveMatrix._rows;
        _cols = moveMatrix._cols;
        moveMatrix._data = nullptr;
        moveMatrix._rows = 0;
        moveMatrix._cols = 0;
        return (*this);
    }
    TMatrix<Type> operator - () const
    {
        TMatrix result(_rows, _cols);
        const int size = _cols * _rows;
        for (int i=0; i<size; ++i)
            result._data[i] = - _data[i];
        return result;
    }
    TMatrix<Type> operator * (Type val) const
    {
        const int size = _cols * _rows;
        TMatrix<Type> result(_rows, _cols);
        Type* data = result.data();
        for (int i=0; i<size; ++i)
            data[i] = _data[i] * val;
        return result;
    }
    void operator *= (Type val)
    {
        const int size = _cols * _rows;
        for (int i=0; i<size; ++i)
            _data[i] *= val;
    }
    TMatrix<Type> operator + (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        const int size = _rows * _cols;
        TMatrix<Type> result(_rows, _cols);
        for (int i=0; i<size; ++i)
            result._data[i] = _data[i] + matrix._data[i];
        return result;
    }
    TMatrix<Type> operator - (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        const int size = _rows * _cols;
        TMatrix<Type> result(_rows, _cols);
        for (int i=0; i<size; ++i)
            result._data[i] = _data[i] - matrix._data[i];
        return result;
    }
    void operator += (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        const int size = _rows * _cols;
        for (int i=0; i<size; ++i)
            _data[i] += matrix._data[i];
    }
    void operator -= (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        const int size = _rows * _cols;
        for (int i=0; i<size; ++i)
            _data[i] -= matrix._data[i];
    }

    void getRow(TVector<Type>& result, int row, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert(size <= _cols);
        const Type* dataRow = getDataRow(row);
        for (int i=0; i<size; ++i)
            result(i) = dataRow[i];
    }
    inline TVector<Type> getRow(int row, int size) const { TVector<Type> result(size); getRow(result, row, size); return result; }
    inline TVector<Type> getRow(int row) const { TVector<Type> result(_cols); getRow(result, row, _cols); return result; }
    void getColumn(TVector<Type>& result, int col, int size) const
    {
        TMath_assert(result.size() >= size);
        TMath_assert((col >= 0) && (col < _cols) && (size <= _rows));
        result(0) = _data[col];
        for (int i=1; i<size; ++i)
            result(i) = _data[_rows * i + col];
    }
    inline TVector<Type> getColumn(int col, int size) const { TVector<Type> result(size); getColumn(result, col, size);
                                                              return result; }
    inline TVector<Type> getColumn(int col) const { TVector<Type> result(_rows); getColumn(result, col, _rows); return result; }
    void setRow(int row, const  TVector<Type>& vector)
    {
        TMath_assert(_cols >= vector.size());
        Type* dataRow = getDataRow(row);
        for (int i=0; i<vector.size(); ++i)
            dataRow[i] = vector(i);
    }
    void setColumn(int col, const  TVector<Type>& vector)
    {
        TMath_assert(_rows >= vector.size());
        for (int i=0; i<vector.size(); ++i)
            _data[i * _rows + col] = vector(i);
    }
    void swapRows(int rowA, int rowB)
    {
        TMath_assert((rowA >= 0) && (rowA < _rows) && (rowB >= 0) && (rowB < _rows));
        Type tempVal;
        Type* strA = getDataRow(rowA);
        Type* strB = getDataRow(rowB);
        for (int i=0; i<_cols; ++i) {
            tempVal = strA[i];
            strA[i] = strB[i];
            strB[i] = tempVal;
        }
    }
    void slice(TMatrix<Type>& result, int beginRow, int beginCol, int rows, int cols) const
    {
        TMath_assert((result.rows() >= rows) && (result.cols() >= cols));
        TMath_assert((rows > 0) && ((beginRow + rows) <= _rows) && (cols > 0) && ((beginCol + cols) <= _cols));
        Type* dataRow = getDataRow(beginRow);
        Type* resultRow = result.firstDataRow();
        int i, j;
        for (j=0; j<cols; ++j)
            resultRow[j] = dataRow[beginCol + j];
        for (i=1; i<_rows; ++i) {
            dataRow = nextRow(dataRow);
            resultRow = result.nextRow(resultRow);
            for (j=0; j<cols; ++j)
                resultRow[j] = dataRow[beginCol + j];
        }
    }
    inline TMatrix<Type> slice(int beginRow, int beginCol, int rows, int cols) const { TMatrix<Type> result(rows, cols);
                                                                                       slice(result, beginRow, beginCol, rows, cols);
                                                                                       return result; }
    void slice(TMatrix<Type>& result, int rows, int cols) const
    {
        TMath_assert((result.rows() >= rows) && (result.cols() >= cols));
        TMath_assert((rows > 0) && (rows <= _rows) && (cols > 0) && (cols <= _cols));
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
    inline TMatrix<Type> slice(int rows, int cols) const { TMatrix<Type> result(rows, cols); slice(result, rows, cols);
                                                           return result; }
    void additionalMinor(TMatrix<Type>& result, int row, int col) const
    {
        TMath_assert((result.rows() >= _rows - 1) && (result.cols() >= _cols - 1));
        TMath_assert((_rows > 1) && (_cols > 1));
        TMath_assert((row >= 0) && (row < _rows) && (col >= 0) && (_cols < cols));
        const Type* dataRow;
        Type* resultRow;
        int i, j;
        if (row == 0) {
            dataRow = nextRow(firstDataRow());
            resultRow = result.nextRow(result.firstDataRow());
            for (j=0; j<col; ++j)
                resultRow[j] = dataRow[j];
            for (j=col+1; j<_cols; ++j)
                resultRow[j-1] = dataRow[j];
            for (i=row+2; i<_rows; ++i) {
                dataRow = nextRow(dataRow);
                resultRow = result.nextRow(resultRow);
                for (j=0; j<col; ++j)
                    resultRow[j] = dataRow[j];
                for (j=col+1; j<_cols; ++j)
                    resultRow[j-1] = dataRow[j];
            }
        } else {
            dataRow = firstDataRow();
            resultRow = result.firstDataRow();
            for (j=0; j<col; ++j)
                resultRow[j] = dataRow[j];
            for (j=col+1; j<_cols; ++j)
                resultRow[j-1] = dataRow[j];
            for (i=1; i<row; ++i) {
                dataRow = nextRow(dataRow);
                resultRow = result.nextRow(resultRow);
                for (j=0; j<col; ++j)
                    resultRow[j] = dataRow[j];
                for (j=col+1; j<_cols; ++j)
                    resultRow[j-1] = dataRow[j];
            }
            dataRow = nextRow(dataRow);
            for (i=row+1; i<_rows; ++i) {
                dataRow = nextRow(dataRow);
                resultRow = result.nextRow(resultRow);
                for (j=0; j<col; ++j)
                    resultRow[j] = dataRow[j];
                for (j=col+1; j<_cols; ++j)
                    resultRow[j-1] = dataRow[j];
            }
        }
    }
    inline TMatrix<Type> additionalMinor(int row, int col) const { TMatrix<Type> result(_rows - 1, _cols - 1);
                                                                   additionalMinor(result, row, col); return result; }

    inline bool isSquareMatrix() const { return (_rows == _cols); }
    void transpose()
    {
        Type temp;
        Type* dataRow = firstDataRow();
        int i, j;
        for (j=1; j<_cols; ++j) {
            const int index = _rows * j;
            temp = _data[index];
            _data[index] = dataRow[j];
            dataRow[j] = temp;
        }
        for (i=1; i<_rows; ++i) {
            dataRow = nextRow(dataRow);
            for (j=i+1; j<_cols; ++j) {
                const int index = _rows * j + i;
                temp = _data[index];
                _data[index] = dataRow[j];
                dataRow[j] = temp;
            }
        }
    }
    void transposed(TMatrix<Type>& result) const
    {
        TMath_assert((result.rows() >= _cols) && (result.cols() >= _rows));
        const Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            result(j, 0) = rowData[j];
        for (i=1; i<_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<_cols; ++j)
                result(j, i) = rowData[j];
        }
    }
    inline TMatrix<Type> transposed() const { TMatrix<Type> result(_cols, _rows); transposed(result); return result; }
    void operator = (const RefTransposedMatrix<Type>& matrix);
    inline RefTransposedMatrix<Type> refTransposed() const;

    void operator += (const RefTransposedMatrix<Type>& matrix)
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            rowData[j] += matrix(0, j);
        for (i=1; i<_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<_cols; ++j)
                rowData[j] += matrix(i, j);
        }
    }
    void operator -= (const RefTransposedMatrix<Type>& matrix)
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        Type* rowData = firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            rowData[j] -= matrix(0, j);
        for (i=1; i<_rows; ++i) {
            rowData = nextRow(rowData);
            for (j=0; j<_cols; ++j)
                rowData[j] -= matrix(i, j);
        }
    }
    TMatrix<Type> operator + (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_rows, _cols);
        const Type* rowData = firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            resultRowData[j] = rowData[j] + matrix(0, j);
        for (i=1; i<_rows; ++i) {
            rowData = nextRow(rowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<_cols; ++j)
                resultRowData[j] = rowData[j] + matrix(i, j);
        }
        return result;
    }
    TMatrix<Type> operator - (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_rows, _cols);
        const Type* rowData = firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            resultRowData[j] = rowData[j] - matrix(0, j);
        for (i=1; i<_rows; ++i) {
            rowData = nextRow(rowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<_cols; ++j)
                resultRowData[j] = rowData[j] - matrix(i, j);
        }
        return result;
    }

    static bool computeLU_decomposition(TMatrix<Type>& inoutMatrix);//result - inoutMatrix = U + L - E;
    static bool computeLUP_decomposition(TMatrix<Type>& inoutMatrix, TVector<int>& P);
    Type determinant() const;
    bool invert();
    TMatrix<Type> inverted() const
    {
        TMatrix<Type> inv = (*this);
        if (inv.invert())
            return inv;
        inv.recreate(0, 0);
        return inv;
    }
};

template<typename Type>
inline TMatrix<Type> operator * (Type value, const TMatrix<Type>& matrix)
{
    return matrix * value;
}

template<typename Type>
class RefTransposedMatrix
{
    friend class TSVD<Type>;
public:
    typedef Type TypeElement;

public:
    RefTransposedMatrix(const TMatrix<Type>* matrix):_rows(matrix->cols()), _cols(matrix->rows())
                { _matrix = matrix; _data = matrix->data(); }

    inline const TMatrix<Type>* sourceMatrix() const { return _matrix; }
    inline int rows() const { return _rows; }
    inline int cols() const { return _cols; }
    inline const Type* data() const { return _data; }
    inline const Type* firstDataColumn() const { return _data; }
    inline const Type* getDataColumn(int col) const { TMath_assert(col < _cols); return &_data[col * _rows]; }
    inline const Type* nextColumn(const Type* column) const { TMath_assert(((unsigned int)(column) - (unsigned int)_data) /
                                                                     (_rows * sizeof(Type)) < (unsigned int)(_cols - 1));
                                                  return &column[_rows]; }
    inline const Type* prevColumn(const Type* column) const { TMath_assert(((unsigned int)(column) - (unsigned int)_data) /
                                                                           (_rows * sizeof(Type)) > (unsigned int)(0));
                                                  return &column[-_rows]; }
    inline const Type& operator ()(int row, int col) const { TMath_assert((row >= 0) && (row < _rows) && (col >= 0) && (col <= _cols));
                                                             return _data[col * _rows + row]; }
    TMatrix<Type> operator * (Type val) const
    {
        const int size = _cols * _rows;
        TMatrix<Type> result(_cols, _rows);
        const Type* columnData = firstDataColumn();
        int i, j;
        for (j=0; j<_rows; ++j)
            result(j, 0) = columnData[j] * val;
        for (i=1; i<size; ++i) {
            columnData = nextColumn(columnData);
            for (j=0; j<_rows; ++j)
                result(j, i) = columnData[j] * val;
        }
        return result;
    }
    TMatrix<Type> operator + (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_rows, _cols);
        int i, j;
        const Type* columnData = firstDataColumn();
        const Type* matrixColumnData = matrix.firstDataColumn();
        for (j=0; j<_rows; ++j)
            result(j, 0) = columnData[j] + matrixColumnData[j];
        for (i=1; i<_cols; ++i) {
            columnData = nextColumn(columnData);
            matrixColumnData = matrix.nextColumn(matrixColumnData);
            for (j=0; j<_cols; ++j)
                result(j, i) = columnData[j] + matrixColumnData[j];
        }
        return result;
    }
    TMatrix<Type> operator - (const RefTransposedMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_rows, _cols);
        int i, j;
        const Type* columnData = firstDataColumn();
        const Type* matrixColumnData = matrix.firstDataColumn();
        for (j=0; j<_rows; ++j)
            result(j, 0) = columnData[j] - matrixColumnData[j];
        for (i=1; i<_cols; ++i) {
            columnData = nextColumn(columnData);
            matrixColumnData = matrix.nextColumn(matrixColumnData);
            for (j=0; j<_cols; ++j)
                result(j, i) = columnData[j] - matrixColumnData[j];
        }
        return result;
    }

    TMatrix<Type> operator + (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_rows, _cols);
        const Type* matrixRowData = matrix.firstDataRow();
        Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<_cols; ++j)
            resultRowData[j] = _data[j * _rows] + matrixRowData[j];
        for (i=1; i<_rows; ++i) {
            matrixRowData = matrix.nextRow(matrixRowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<_cols; ++j)
                resultRowData[j] = _data[j * _rows + i] + matrixRowData[j];
        }
        return result;
    }
    TMatrix<Type> operator - (const TMatrix<Type>& matrix) const
    {
        TMath_assert((_rows == matrix.rows()) && (_cols == matrix.cols()));
        TMatrix<Type> result(_cols, _rows);
        const Type* matrixRowData = matrix.firstDataRow();
        const Type* resultRowData = result.firstDataRow();
        int i, j;
        for (j=0; j<_rows; ++j)
            resultRowData[j] = _data[j * _cols] - matrixRowData[j];
        for (i=1; i<_cols; ++i) {
            matrixRowData = matrix.nextRow(matrixRowData);
            resultRowData = result.nextRow(resultRowData);
            for (j=0; j<_rows; ++j)
                resultRowData[j] = _data[j * _cols + i] - matrixRowData[j];
        }
        return result;
    }

    inline void getColumn(TVector<Type>& result, int col, int size) const { _matrix->getRow(result, col, size); }
    inline TVector<Type> getColumn(int col, int size) const { return _matrix->getRow(col, size); }
    inline TVector<Type> getColumn(int col) const { return _matrix->getRow(col, _rows); }
    inline void getRow(TVector<Type>& result, int row, int size) const { _matrix->getColumn(result, row, size); }
    inline TVector<Type> getRow(int row, int size) const { return _matrix->getColumn(row, size); }
    inline TVector<Type> getRow(int row) const { return _matrix->getColumn(row, _cols); }
    inline bool isSquareMatrix() const { return (_rows == _cols); }

    inline Type determinant() const { return _matrix->determinant(); }

private:
    const TMatrix<Type>* _matrix;
    const int _rows;
    const int _cols;
    const Type* _data;
};

template<typename Type>
RefTransposedMatrix<Type> TMatrix<Type>::refTransposed() const
{
    return RefTransposedMatrix<Type>(this);
}

template<typename Type>
inline TMatrix<Type> operator * (Type value, const RefTransposedMatrix<Type>& matrix)
{
    return matrix * value;
}

template<typename Type>
void TMatrix<Type>::operator = (const RefTransposedMatrix<Type>& matrix)
{
    TMath_assert(_data != matrix.data());
    if (_rows != matrix.rows() || _cols != matrix.cols())
        recreate(matrix.rows(), matrix.cols());
    Type* dataRow = firstDataRow();
    int i, j;
    for (j=0; j<_cols; ++j)
        dataRow[j] = matrix(0, j);
    for (i=1; i<_rows; ++i) {
        dataRow = nextRow(dataRow);
        for (j=0; j<_cols; ++j)
            dataRow[j] = matrix(i, j);
    }
}

template<typename Type>
TMatrix<Type>::TMatrix(const RefTransposedMatrix<Type>& matrix):
    _rows(matrix.rows()), _cols(matrix.cols()), _data(new Type[_rows * _cols])
{
    Type* dataRow = firstDataRow();
    int i, j;
    for (j=0; j<_cols; ++j)
        dataRow[j] = matrix(0, j);
    for (i=1; i<_rows; ++i) {
        dataRow = nextRow(dataRow);
        for (j=0; j<_cols; ++j)
            dataRow[j] = matrix(i, j);
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
bool TMatrix<Type>::computeLU_decomposition(TMatrix<Type>& inoutMatrix)
{
    TMath_assert(inoutMatrix.isSquareMatrix());
    int i, j, k, rank = inoutMatrix.rows();
    Type mult;
    Type* str_i;
    Type* str_k = inoutMatrix.firstDataRow();
    if (std::fabs(str_k[0]) < FLT_EPSILON)
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
        if (std::fabs(str_k[k]) < FLT_EPSILON)
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

template<typename Type>
bool TMatrix<Type>::computeLUP_decomposition(TMatrix<Type>& inoutMatrix, TVector<int>& P)
{
    TMath_assert(inoutMatrix.isSquareMatrix());
    TMath_assert(P.size() == inoutMatrix.rows());
    int i, j, rank = inoutMatrix.rows(), k = 0;
    Type* str_i = inoutMatrix.data();
    Type max = std::fabs(*str_i);
    for (j=1; j<rank; ++j) {
        str_i = &str_i[rank];
        const float val = std::fabs(*str_i);
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
            const float val = std::fabs(*str_i);
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

template<typename Type>
Type TMatrix<Type>::determinant() const
{
    TMath_assert(isSquareMatrix());
    int rank = _rows;
    Type sign = (Type)1;
    const int size = rank * rank;
    Type* dataG = new Type[size];
    int i, j, k, index;
    for (i=0; i<size; ++i)
        dataG[i] = _data[i];
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
        if (maxVal < FLT_EPSILON) {
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
    TVector<int> P(_rows);
    if (!computeLUP_decomposition(LU, P))
        return false;
    TMatrix<Type> temp(_rows, _rows);
    int i, j, k;
    Type* strA = temp.firstDataRow();
    Type* strB = LU.firstDataRow();
    Type* strC;
    for (i=1; i<_rows; ++i) {
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
    const int lastIndex = _rows - 1;
    const int preLastIndex = lastIndex - 1;
    strA = temp.getDataRow(lastIndex);
    strB = LU.getDataRow(lastIndex);
    strA[lastIndex] = (Type(1)) / strB[lastIndex];
    for (i=preLastIndex; i>=0; --i) {
        strA = temp.prevRow(strA);
        strB = LU.prevRow(strB);
        strA[i] = (Type(1)) / strB[i];
        for (j=i+1; j<_rows; ++j)
            strB[j] *= strA[i];
        strC = temp.getDataRow(lastIndex);
        strA[lastIndex] = - strC[lastIndex] * strB[lastIndex];
        for (j=preLastIndex; j>i; --j) {
            strC = temp.prevRow(strC);
            strA[j] = - strC[j] * strB[j];
            for (k=j+1; k<_rows; ++k)
                strA[k] -= strC[k] * strB[j];
        }
    }
    strA = firstDataRow();
    strB = temp.firstDataRow();
    for (j=0; j<_rows; ++j) {
        strA[P(j)] = strB[j];
        for (k=j+1; k<_rows; ++k)
            strA[P(j)] += strB[k] * temp(k, j);
    }
    for (i=1; i<_rows; ++i) {
        strA = nextRow(strA);
        strB = temp.nextRow(strB);
        for (j=0; j<i; ++j) {
            strA[P(j)] = strB[i] * temp(i, j);
            for (k=i+1; k<_rows; ++k)
                strA[P(j)] += strB[k] * temp(k, j);
        }
        for (; j<_rows; ++j) {
            strA[P(j)] = strB[j];
            for (k=j+1; k<_rows; ++k)
                strA[P(j)] += strB[k] * temp(k, j);
        }
    }
    return true;
}

template<typename Type = DefaultType>
class Tools
{
public:
    template <int N, int M>
    inline static TMatrix<Type> convert(const QGenericMatrix<M, N, Type>& matrix)
    {
        TMatrix<Type> result(M, N);
        for (int row = 0; row < M; ++row)
            for (int col = 0; col < N; ++col)
                result(row, col) = matrix(row, col);
        return result;
    }
    inline static TMatrix<Type> convert(const QMatrix4x4& matrix)
    {
        TMatrix<Type> result(4, 4);
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                result(row, col) = matrix(row, col);
        return result;
    }
    template <class OutMatrixClass>
    inline static void convert(OutMatrixClass& outMatrix, const TMath::TMatrix<Type>& inMatrix)
    {
        for (int row = 0; row < inMatrix.rows(); ++row)
            for (int col = 0; col < inMatrix.cols(); ++col)
                outMatrix(row, col) = inMatrix(row, col);
    }
    inline static Type matrix2x2Determinant(const TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 2) && (matrix.cols() == 2));
        const float* data = matrix.data();
        return (data[0] * data[3] - data[1] * data[2]);
    }
    inline static Type matrix3x3Determinant(const TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 3) &&  (matrix.cols() == 3));
        const float* data = matrix.data();
        return ((data[0] * data[4] * data[8] + data[1] * data[5] * data[6] + data[2] * data[3] * data[7]) -
                (data[0] * data[5] * data[7] + data[1] * data[3] * data[8] + data[2] * data[4] * data[6]));
    }
    inline static Type matrixDeterminant(Type* dataMatrix, int rows, int cols)
    {
        TMatrix<Type> matrix;
        matrix._data = dataMatrix;
        matrix._rows = rows;
        matrix._cols = cols;
        Type det = matrix.determinant();
        matrix._data = nullptr;
        return det;
    }
    inline static bool matrixInvert(Type* inoutDataMatrix, int rows, int cols)
    {
        TMatrix<Type> matrix;
        matrix._data = inoutDataMatrix;
        matrix._rows = rows;
        matrix._cols = cols;
        bool success = matrix.invert();
        matrix._data = nullptr;
        return success;
    }
    //Compute a rotation exponential using the Rodrigues Formula.
    inline static void exp_rodrigues(TMatrix<Type>& outRotationMatrix, const TVector<Type>& w, const Type A, const Type B)
    {
        TMath_assert((outRotationMatrix.rows() >= 3) && (outRotationMatrix.cols() >= 3));
        TMath_assert(w.size() >= 3);
        {
            const Type wx2 = w(0) * w(0);
            const Type wy2 = w(1) * w(1);
            const Type wz2 = w(2) * w(2);

            outRotationMatrix(0, 0) = 1.0 - B * (wy2 + wz2);
            outRotationMatrix(1, 1) = 1.0 - B * (wx2 + wz2);
            outRotationMatrix(2, 2) = 1.0 - B * (wx2 + wy2);
        }
        {
            const Type a = A * w(2);
            const Type b = B * (w(0) * w(1));
            outRotationMatrix(0, 1) = b - a;
            outRotationMatrix(1, 0) = b + a;
        }
        {
            const Type a = A * w(1);
            const Type b = B * (w(0) * w(2));
            outRotationMatrix(0, 2) = b + a;
            outRotationMatrix(2, 0) = b - a;
        }
        {
            const Type a = A * w(0);
            const Type b = B * (w(1) * w(2));
            outRotationMatrix(1, 2) = b - a;
            outRotationMatrix(2, 1) = b + a;
        }
    }
    inline static TMatrix<Type> exp_rodrigues(const TVector<Type>& w, const Type A, const Type B)
    {
        TMatrix<Type> result(3, 3);
        exp_rodrigues(result, w, A, B);
        return result;
    }
    inline static void exp_rotationMatrix(TMatrix<Type>& outRotationMatrix, const TVector<Type>& w)
    {
        TMath_assert(w.size() >= 3);
        static const Type one_6th = Type(1.0 / 6.0);
        static const Type one_20th = Type(1.0 / 20.0);

        const Type theta_sq = dot(w, w);
        const Type theta = std::sqrt(theta_sq);
        Type A, B;
        //Use a Taylor series expansion near zero. This is required for
        //accuracy, since sin t / t and (1-cos t)/t^2 are both 0/0.
        if (theta_sq < 1e-8) {
            A = Type(1.0) - one_6th * theta_sq;
            B = Type(0.5);
        } else {
            if (theta_sq < 1e-6) {
                B = Type(0.5) - static_cast<Type>(0.25 * one_6th * theta_sq);
                A = Type(1.0) - static_cast<Type>(theta_sq * one_6th * (1.0 - one_20th * theta_sq));
            } else {
                const Type inv_theta = Type(1) / theta;
                A = std::sin(theta) * inv_theta;
                B = (Type(1) - std::cos(theta)) * (inv_theta * inv_theta);
            }
        }
        exp_rodrigues(outRotationMatrix, w, A, B);
    }
    inline static TMatrix<Type> exp_rotationMatrix(const TVector<Type>& w)
    {
        TMatrix<Type> result(3, 3);
        exp_rotationMatrix(result, w);
        return result;
    }
    inline static void exp_transform(TMatrix<Type>& outMatrixTransform, const TVector<Type>& mu)
    {
        TMath_assert((outMatrixTransform.rows() >= 3) && (outMatrixTransform.cols() >= 4));
        TMath_assert(mu.size() >= 6);
        static const Type one_6th = Type(1.0 / 6.0);
        static const Type one_20th = Type(1.0 / 20.0);
        const TVector<Type> w = mu.slice(3, 3);
        const TVector<Type> mu_3 = mu.slice(3);
        const Type theta_square = dot(w, w);
        const Type theta = std::sqrt(theta_square);
        Type A, B;

        const TVector<Type> crossVector = cross3(w,  mu.slice(3));
        if (theta_square < 1e-8) {
            A = Type(1.0) - one_6th * theta_square;
            B = Type(0.5);
            outMatrixTransform.setColumn(3, mu_3 + Type(0.5) * crossVector);
        } else {
            Type C;
            if (theta_square < 1e-6) {
                C = one_6th * (1.0 - one_20th * theta_square);
                A = Type(1.0) - theta_square * C;
                B = Type(0.5) - static_cast<Type>(0.25 * one_6th * theta_square);
            } else {
                const Type inv_theta = Type(1) / theta;
                A = std::sin(theta) * inv_theta;
                B = (Type(1) - std::cos(theta)) * (inv_theta * inv_theta);
                C = (Type(1) - A) * (inv_theta * inv_theta);
            }
            outMatrixTransform.setColumn(3, mu_3 + B * crossVector + C * cross3(w,  crossVector));
        }
        exp_rodrigues(outMatrixTransform, w, A, B);
    }
    inline static TMatrix<Type> exp_transformMatrix(const TVector<Type>& mu)
    {
        TMatrix<Type> result(4, 4);
        exp_transform(result, mu);
        result(3, 0) = Type(0);
        result(3, 1) = Type(0);
        result(3, 2) = Type(0);
        result(3, 3) = Type(1);
        return result;
    }
};

} // namespace TMath

#endif // TMATH_H
