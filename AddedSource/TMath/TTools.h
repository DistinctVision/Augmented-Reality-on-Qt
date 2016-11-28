#ifndef TMATH_TOOLS_H
#define TMATH_TOOLS_H

#include "TVector.h"
#include "TMatrix.h"

#include <cmath>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <climits>

namespace TMath {

namespace TTools
{
    template <typename CVectorType, int size, typename Type = TMath_DefaultType>
    inline static TVector<Type> convert(const CVectorType& vector)
    {
        TVector<Type> result(size);
        for (int i = 0; i < size; ++i)
            result(i) = vector[i];
        return result;
    }

    template <typename OutVectorClass, typename Type>
    inline static void convert(OutVectorClass& outVector, const TVector<Type>& inVector)
    {
        for (int i = 0; i < inVector.size(); ++i)
            outVector[i] = /*(decltype(outVector[0]))*/inVector(i);
    }

    template <typename OutVectorClass, typename Type>
    inline static OutVectorClass convert(const TVector<Type>& inVector)
    {
        OutVectorClass outVector;
        convert<OutVectorClass, Type>(outVector, inVector);
        return outVector;
    }

    template <typename CMatrixType, int rows, int cols, typename Type = TMath_DefaultType>
    inline static TMatrix<Type> convert(const CMatrixType& matrix)
    {
        TMatrix<Type> result(rows, cols);
        for (int row = 0; row < rows; ++row)
            for (int col = 0; col < cols; ++col)
                result(row, col) = matrix(row, col);
        return result;
    }

    template <typename OutMatrixClass, typename Type>
    inline static void convert(OutMatrixClass& outMatrix, const TMatrix<Type>& inMatrix)
    {
        //typedef std::remove_reference<decltype(outMatrix(0, 0))>::type OutMatrixElementType;
        for (int row = 0; row < inMatrix.rows(); ++row)
            for (int col = 0; col < inMatrix.cols(); ++col)
                outMatrix(row, col) = /*(OutMatrixElementType)*/(inMatrix(row, col));
    }

    template <typename OutMatrixClass, typename Type>
    inline static OutMatrixClass convert(const TMatrix<Type>& inMatrix)
    {
        OutMatrixClass outMatrix;
        convert<OutMatrixClass, Type>(outMatrix, inMatrix);
        return outMatrix;
    }

    template <typename Type>
    inline static Type matrix2x2Determinant(const TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 2) && (matrix.cols() == 2));
        const Type* data = matrix.data();
        return (data[0] * data[3] - data[1] * data[2]);
    }

    template <typename Type>
    inline static Type matrix3x3Determinant(const TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 3) &&  (matrix.cols() == 3));
        const Type* data = matrix.data();
        return ((data[0] * data[4] * data[8] + data[1] * data[5] * data[6] + data[2] * data[3] * data[7]) -
                (data[0] * data[5] * data[7] + data[1] * data[3] * data[8] + data[2] * data[4] * data[6]));
    }

    template <typename Type>
    inline static bool matrix2x2Invert(TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 2) && (matrix.cols() == 2));
        Type* mData = matrix.data();
        Type det = mData[0] * mData[3] - mData[1] * mData[2];
        if (std::fabs(det) < std::numeric_limits<Type>::epsilon())
            return false;
        Type data[4];
        data[0] = mData[0];
        data[1] = mData[1];
        data[2] = mData[2];
        data[3] = mData[3];
        mData[3] = data[0] / det;
        mData[1] = - data[1] / det;
        mData[2] = - data[2] / det;
        mData[0] = data[3] / det;
        return true;
    }

    template <typename Type>
    inline static TMatrix<Type> matrix2x2Inverted(const TMatrix<Type>& matrix)
    {
        TMatrix<Type> m = matrix;
        matrix2x2Invert(m);
        return m;
    }

    template <typename Type>
    inline static bool matrix3x3Invert(TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 3) && (matrix.cols() == 3));
        Type* mData = matrix.data();
        Type det = ((mData[0] * mData[4] * mData[8] + mData[1] * mData[5] * mData[6] + mData[2] * mData[3] * mData[7]) -
                    (mData[0] * mData[5] * mData[7] + mData[1] * mData[3] * mData[8] + mData[2] * mData[4] * mData[6]));
        if (std::fabs(det) < std::numeric_limits<Type>::epsilon())
            return false;
        Type data[9];
        data[0] = mData[0];
        data[1] = mData[1];
        data[2] = mData[2];
        data[3] = mData[3];
        data[4] = mData[4];
        data[5] = mData[5];
        data[6] = mData[6];
        data[7] = mData[7];
        data[8] = mData[8];
        mData[0] = (data[4] * data[8] - data[5] * data[7]) / det;
        mData[3] = - (data[3] * data[8] - data[5] * data[6]) / det;
        mData[6] = (data[3] * data[7] - data[6] * data[4]) / det;
        mData[1] = - (data[1] * data[8] - data[2] * data[7]) / det;
        mData[4] = (data[0] * data[8] - data[2] * data[6]) / det;
        mData[7] = - (data[0] * data[7] - data[1] * data[6]) / det;
        mData[2] = (data[1] * data[5] - data[2] * data[4]) / det;
        mData[5] = - (data[0] * data[5] - data[2] * data[3]) / det;
        mData[8] = (data[0] * data[4] - data[1] * data[3]) / det;
        return true;
    }

    template <typename Type>
    inline static TMatrix<Type> matrix3x3Inverted(const TMatrix<Type>& matrix)
    {
        TMatrix<Type> m = matrix;
        matrix3x3Invert(m);
        return m;
    }

    template <typename Type>
    inline static Type matrixDeterminant(Type* dataMatrix, int rows, int cols)
    {
        TMatrix<Type> matrix;
        matrix.m_data = dataMatrix;
        matrix.m_rows = rows;
        matrix.m_cols = cols;
        Type det = matrix.determinant();
        matrix.m_data = nullptr;
        return det;
    }

    template <typename Type>
    inline static bool matrixInvert(Type* inoutDataMatrix, int rows, int cols)
    {
        TMatrix<Type> matrix;
        matrix.m_data = inoutDataMatrix;
        matrix.m_rows = rows;
        matrix.m_cols = cols;
        bool success = matrix.invert();
        matrix.m_data = nullptr;
        return success;
    }

    template <typename Type>
    inline static bool computeLU_decomposition(TMatrix<Type>& inoutMatrix);

    template <typename Type>
    inline static bool computeLUP_decomposition(TMatrix<Type>& inoutMatrix, TVector<int>& P);

    //Compute a rotation exponential using the Rodrigues Formula.
    template <typename Type>
    inline static void exp_rodrigues(TMatrix<Type>& outRotationMatrix,
                                     const TVector<Type>& w, const Type A, const Type B)
    {
        TMath_assert((outRotationMatrix.rows() >= 3) && (outRotationMatrix.cols() >= 3));
        TMath_assert(w.size() >= 3);
        {
            const Type wx2 = w(0) * w(0);
            const Type wy2 = w(1) * w(1);
            const Type wz2 = w(2) * w(2);

            outRotationMatrix(0, 0) = Type(1) - B * (wy2 + wz2);
            outRotationMatrix(1, 1) = Type(1) - B * (wx2 + wz2);
            outRotationMatrix(2, 2) = Type(1) - B * (wx2 + wy2);
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

    template <typename Type>
    inline static TMatrix<Type> exp_rodrigues(const TVector<Type>& w, const Type A, const Type B)
    {
        TMatrix<Type> result(3, 3);
        exp_rodrigues(result, w, A, B);
        return result;
    }

    template <typename Type>
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
                B = Type(0.5) - (Type)(0.25 * one_6th * theta_sq);
                A = Type(1.0) - (Type)(theta_sq * one_6th * (1.0 - one_20th * theta_sq));
            } else {
                const Type inv_theta = Type(1) / theta;
                A = std::sin(theta) * inv_theta;
                B = (Type(1) - std::cos(theta)) * (inv_theta * inv_theta);
            }
        }
        exp_rodrigues(outRotationMatrix, w, A, B);
    }

    template <typename Type>
    inline static TMatrix<Type> exp_rotationMatrix(const TVector<Type>& w)
    {
        TMatrix<Type> result(3, 3);
        exp_rotationMatrix(result, w);
        return result;
    }

    template <typename Type>
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
                C = one_6th * (Type(1) - one_20th * theta_square);
                A = Type(1.0) - theta_square * C;
                B = Type(0.5) - (Type)(0.25 * one_6th * theta_square);
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

    template <typename Type>
    inline static void exp_transform(TMatrix<Type>& outRotationMatrix, TVector<Type>& outTranslation, const TVector<Type>& mu)
    {
        TMath_assert((outRotationMatrix.rows() >= 3) && (outRotationMatrix.cols() >= 3));
        TMath_assert(outTranslation.size() >= 3);
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
            outTranslation = mu_3 + Type(0.5) * crossVector;
        } else {
            Type C;
            if (theta_square < 1e-6) {
                C = one_6th * (Type(1) - one_20th * theta_square);
                A = Type(1.0) - theta_square * C;
                B = Type(0.5) - (Type)(0.25 * one_6th * theta_square);
            } else {
                const Type inv_theta = Type(1) / theta;
                A = std::sin(theta) * inv_theta;
                B = (Type(1) - std::cos(theta)) * (inv_theta * inv_theta);
                C = (Type(1) - A) * (inv_theta * inv_theta);
            }
            outTranslation = mu_3 + B * crossVector + C * cross3(w,  crossVector);
        }
        exp_rodrigues(outRotationMatrix, w, A, B);
    }

    template <typename Type>
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

    template <typename Type>
    inline static TVector<Type> ln_rotationMatrix(const TMatrix<Type>& rotationMatrix)
    {
        static const Type m_sqrt1_2 = (Type)(0.707106781186547524401);

        TVector<Type> result(3);

        const Type cos_angle = (rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2) - Type(1)) * Type(0.5);
        result(0) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) * Type(0.5);
        result(1) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) * Type(0.5);
        result(2) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) * Type(0.5);

        Type sin_angle_abs = std::sqrt(result(0) * result(0) + result(1) * result(1) + result(2) * result(2));
        if (cos_angle > m_sqrt1_2) {            // [0 - Pi/4[ use asin
            if (sin_angle_abs > Type(0)) {
                result *= std::asin(sin_angle_abs) / sin_angle_abs;
            }
        } else if (cos_angle > -m_sqrt1_2) {    // [Pi/4 - 3Pi/4[ use acos, but antisymmetric part
            const Type angle = std::acos(cos_angle);
            result *= angle / sin_angle_abs;
        } else {  // rest use symmetric part
            // antisymmetric part vanishes, but still large rotation, need information from symmetric part
            const Type angle = Type(M_PI) - std::asin(sin_angle_abs);
            const Type d0 = rotationMatrix(0, 0) - cos_angle,
                d1 = rotationMatrix(1, 1) - cos_angle,
                d2 = rotationMatrix(2, 2) - cos_angle;
            TVector<Type> r2(3);
            if (((d0 * d0) > (d1 * d1)) && ((d0 * d0) > (d2 * d2))) { // first is largest, fill with first column
                r2(0) = d0;
                r2(1) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * Type(0.5);
                r2(2) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * Type(0.5);
            } else if ((d1 * d1) > (d2 * d2)) { 			    // second is largest, fill with second column
                r2(0) = (rotationMatrix(1, 0) + rotationMatrix(0, 1)) * Type(0.5);
                r2(1) = d1;
                r2(2) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * Type(0.5);
            } else {							    // third is largest, fill with third column
                r2(0) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) * Type(0.5);
                r2(1) = (rotationMatrix(2, 1) + rotationMatrix(1, 2)) * Type(0.5);
                r2(2) = d2;
            }
            // flip, if we point in the wrong direction!
            if (dot(r2, result) < Type(0))
                r2 *= Type(-1);
            r2.normalize();
            result = r2 * angle;
        }
        return result;
    }

    template <typename Type>
    static TVector<Type> ln_transform(const TMatrix<Type>& rotationMatrix, const TVector<Type>& translation)
    {
        TMath_assert((rotationMatrix.rows() >= 3) && (rotationMatrix.cols() >= 3));
        TMath_assert(translation.size() >= 3);
        TVector<Type> rot = ln_rotationMatrix(rotationMatrix);
        const Type square_theta = rot(0) * rot(0) + rot(1) * rot(1) + rot(2) * rot(2);
    	const Type theta = std::sqrt(square_theta);

    	Type shtot = Type(0.5);
    	if (theta > Type(0.00001)) {
    		shtot = std::sin(theta * Type(0.5)) / theta;
    	}

    	// now do the rotation
    	const TMatrix<Type> halfrotator = exp_rotationMatrix(rot * Type(-0.5));
    	TVector<Type> rottrans = halfrotator * translation;

    	if (theta > Type(0.001)){
            rottrans -= (rot * ((dot(translation, rot) * (Type(1) - Type(2) * shtot) / (square_theta))));
    	} else {
            rottrans -= (rot * ((dot(translation, rot) / Type(24))));
    	}

    	rottrans /= (Type(2) * shtot);

    	TVector<Type> result(6);
    	result.fill(0, 3, rottrans);
    	result.fill(3, 3, rot);
    	return result;
    }

    template <typename Type>
    static TVector<Type> ln_transformMatrix(const TMatrix<Type>& transformMatrix)
    {
        TMath_assert((transformMatrix.rows() >= 3) && (transformMatrix.cols() >= 4));
        return ln_transform(transformMatrix.slice(3, 3), transformMatrix.getColumn(3, 3));
    }

    template <typename Type>
    static bool collision2DLines(TVector<Type>& result, float& tLineA, float& tLineB,
                                 const TVector<Type>& pointA, const TVector<Type>& dirA,
                                 const TVector<Type>& pointB, const TVector<Type>& dirB)
    {
        TMath_assert((result.size() == 2) && (pointA.size() == 2) && (pointB.size() == 2));
        TMath_assert((dirA.size() == 2) && (dirB.size() == 2));
        if (std::fabs(dirB(0)) <= std::numeric_limits<Type>::epsilon()) {
            if (std::fabs(dirA(0)) <= std::numeric_limits<Type>::epsilon())
                return false;
            result(1) = dirA(1) / dirA(0);
            tLineA = dirB(1) - dirB(0) * result(1);
            if (std::fabs(tLineA) <= std::numeric_limits<Type>::epsilon())
                return false;
            result(0) = pointB(0) - pointA(0);
            tLineB = (result(0) * result(1) + pointA(1) - pointB(1)) / tLineA;
            tLineA = (result(0) + dirB(0) * tLineB) / dirA(0);
            result = pointA + (dirA * tLineA);
            return true;
        }
        //if (std::fabs(dirA(0)) <= std::numeric_limits<Type>::epsilon())
        //    return false;
        result(1) = dirB(1) / dirB(0);
        tLineB = dirA(1) - dirA(0) * result(1);
        if (std::fabs(tLineB) <= std::numeric_limits<Type>::epsilon())
            return false;
        result(0) = pointA(0) - pointB(0);
        tLineA = (result(0) * result(1) + pointB(1) - pointA(1)) / tLineB;
        tLineB = (result(0) + dirA(0) * tLineA) / dirB(0);
        result = pointB + (dirB * tLineB);
        return true;
    }

    template <typename Type>
    static TMatrix<Type> invertedWorldMatrix(const TMatrix<Type>& matrix)
    {
        TMath_assert((matrix.rows() == 4) && (matrix.cols() == 4));
        TMatrix<Type> result(4, 4);
        result(0, 0) = matrix(0, 0);
        result(0, 1) = matrix(1, 0);
        result(0, 2) = matrix(2, 0);
        result(1, 0) = matrix(0, 1);
        result(1, 1) = matrix(1, 1);
        result(1, 2) = matrix(2, 1);
        result(2, 0) = matrix(0, 2);
        result(2, 1) = matrix(1, 2);
        result(2, 2) = matrix(2, 2);
        result(0, 3) = - (matrix(0, 0) * matrix(0, 3) + matrix(1, 0) * matrix(1, 3) + matrix(2, 0) * matrix(2, 3));
        result(1, 3) = - (matrix(0, 1) * matrix(0, 3) + matrix(1, 1) * matrix(1, 3) + matrix(2, 1) * matrix(2, 3));
        result(2, 3) = - (matrix(0, 2) * matrix(0, 3) + matrix(1, 2) * matrix(1, 3) + matrix(2, 2) * matrix(2, 3));
        result(3, 0) = 0.0f;
        result(3, 1) = 0.0f;
        result(3, 2) = 0.0f;
        result(3, 3) = 1.0f;
        return result;
    }

    template <typename Type>
    static TMatrix<Type> ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        if (left == right || bottom == top || nearPlane == farPlane)
            return TMatrix<Type>();

        // Construct the projection.
        Type width = right - left;
        Type invheight = top - bottom;
        Type clip = farPlane - nearPlane;
        TMatrix<Type> m(4, 4);
        m(0, 0) = Type(2) / width;
        m(0, 1) = Type(0);
        m(0, 2) = Type(0);
        m(0, 3) = -(left + right) / width;
        m(1, 0) = Type(0);
        m(1, 1) = Type(2) / invheight;
        m(1, 2) = Type(0);
        m(1, 3) = - (top + bottom) / invheight;
        m(2, 0) = Type(0);
        m(2, 1) = Type(0);
        m(2, 2) = - Type(2) / clip;
        m(2, 3) = - (nearPlane + farPlane) / clip;
        m(3, 0) = Type(0);
        m(3, 1) = Type(0);
        m(3, 2) = Type(0);
        m(3, 3) = Type(1);

        return m;
    }

    template <typename Type>
    static TMatrix<Type> perspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
    {
        // Bail out if the projection volume is zero-sized.
        if (nearPlane == farPlane || aspectRatio == 0.0f)
            return TMatrix<Type>();

        // Construct the projection.
        TMatrix<Type> m(4, 4);
        float radians = (verticalAngle / Type(2)) * Type(M_PI / 180.0);
        float sine = std::sin(radians);
        if (sine == Type(0))
            return TMatrix<Type>();
        float cotan = cosf(radians) / sine;
        float clip = farPlane - nearPlane;
        m(0, 0) = cotan / aspectRatio;
        m(0, 1) = Type(0);
        m(0, 2) = Type(0);
        m(0, 3) = Type(0);
        m(1, 0) = Type(0);
        m(1, 1) = cotan;
        m(1, 2) = Type(0);
        m(1, 3) = Type(0);
        m(2, 0) = Type(0);
        m(2, 1) = Type(0);
        m(2, 2) = -(nearPlane + farPlane) / clip;
        m(2, 3) = -(Type(2) * nearPlane * farPlane) / clip;
        m(3, 0) = Type(0);
        m(3, 1) = Type(0);
        m(3, 2) = -Type(1);
        m(3, 3) = Type(0);

        return m;
    }

    template <typename Type>
    inline static TVector<Type> generator_field(int i, const TVector<Type>& pos)
    {
        TMath_assert(pos.size() == 4);
        TVector<Type> result(4);
        if (i < 3) {
            result.setZero();
            result(i) = pos(3);
            return result;
        }
        result(i % 3) = (Type)0;
        result((i + 1) % 3) = - pos((i + 2) % 3);
        result((i + 2) % 3) = pos((i + 1) % 3);
        result(3) = (Type)0;
        return result;
    }

    template <typename Type>
    inline static TVector<Type> triangulatePoint(const TMatrix<Type>& deltaRotation,
                                                 const TVector<Type>& deltaTranslation,
                                                 const TVector<Type>& camDirA,
                                                 const TVector<Type>& camDirB)
    {
        TMath_assert((deltaRotation.rows() == 3) && (deltaRotation.cols() == 3));
        TMath_assert(deltaTranslation.size() == 3);
        TMath_assert((camDirA.size() == 3) && (camDirB.size() == 3));
        TVector<Type> camDirB_2 = deltaRotation * camDirB;
        Type b_0 = deltaTranslation(0) * camDirA(0) +
                   deltaTranslation(1) * camDirA(1) +
                   deltaTranslation(2) * camDirA(2);
        Type b_1 = deltaTranslation(0) * camDirB_2(0) +
                   deltaTranslation(1) * camDirB_2(1) +
                   deltaTranslation(2) * camDirB_2(2);
        Type A_00 = camDirA(0) * camDirA(0) +
                    camDirA(1) * camDirA(1) +
                    camDirA(2) * camDirA(2);
        Type A_10 = camDirA(0) * camDirB_2(0) +
                    camDirA(1) * camDirB_2(1) +
                    camDirA(2) * camDirB_2(2);
        Type A_11 = - (camDirB_2(0) * camDirB_2(0) +
                       camDirB_2(1) * camDirB_2(1) +
                       camDirB_2(2) * camDirB_2(2));
        Type det = A_00 * A_11 + A_10 * A_10;
        if (std::fabs(det) < std::numeric_limits<Type>::epsilon())
            return TVector<Type>();
        Type lambda_0 = (A_11 * b_0 + A_10 * b_1) / det;
        Type lambda_1 = (A_00 * b_1 - A_10 * b_0) / det;

        return (camDirA * lambda_0 + (deltaTranslation + camDirB_2 * lambda_1)) * Type(0.5);
    }

    template <typename Type>
    inline static bool depthFromTriangulation(Type& depth,
                                              const TMatrix<Type>& deltaRotation,
                                              const TVector<Type>& deltaTranslation,
                                              const TVector<Type>& camDirA,
                                              const TVector<Type>& camDirB)
    {
        TMath_assert((deltaRotation.rows() == 3) && (deltaRotation.cols() == 3));
        TMath_assert(deltaTranslation.size() == 3);
        TMath_assert((camDirA.size() == 3) && (camDirB.size() == 3));
        TVector<Type> camDirA_2 = deltaRotation * camDirA;
        TMatrixd AtA(2, 2);
        AtA(0, 0) = camDirA_2(0) * camDirA_2(0) + camDirA_2(1) * camDirA_2(1) + camDirA_2(2) * camDirA_2(2);
        AtA(0, 1) = camDirA_2(0) * camDirB(0) + camDirA_2(1) * camDirB(1) + camDirA_2(2) * camDirB(2);
        AtA(1, 0) = AtA(0, 1);
        AtA(1, 1) = camDirB(0) * camDirB(0) + camDirB(1) * camDirB(1) + camDirB(2) * camDirB(2);
        if (!TTools::matrix2x2Invert(AtA))
            return false;
        //depth = - (AtA^-1 * A^t * deltaTranslation);
        depth = - ((AtA(0, 0) * camDirA_2(0) + AtA(0, 1) * camDirB(0)) * deltaTranslation(0) +
                   (AtA(0, 0) * camDirA_2(1) + AtA(0, 1) * camDirB(1)) * deltaTranslation(1) +
                   (AtA(0, 0) * camDirA_2(2) + AtA(0, 1) * camDirB(2)) * deltaTranslation(2));
        if (depth < std::numeric_limits<Type>::epsilon())
            return false;
        return true;
    }

    //!RADIANS!
    template <typename Type>
    inline static TVector<Type> rotationMatrixToEulerAngles_3d(const TMatrix<Type>& rotationMatrix)
    {
        TMath_assert((rotationMatrix.rows() >= 3) && (rotationMatrix.cols() >= 3));

        Type Y = - std::asin(rotationMatrix(0, 2));
        Type C = std::cos(Y);
        Type rotx, roty, X, Z;

        //Y *= (Type)(180.0 / M_PI);

        if (std::fabs(C) > std::numeric_limits<Type>::epsilon()) {
            C       = (Type)(1.0 / C);
            rotx    = rotationMatrix(2, 2) * C;
            roty    = rotationMatrix(1, 2) * C;
            X       = (Type)(std::atan2(roty, rotx));
            rotx    = rotationMatrix(0, 0) * C;
            roty    = rotationMatrix(0, 1) * C;
            Z       = (Type)(std::atan2(roty, rotx));
        } else {
            X       = (Type)0;
            rotx    =   rotationMatrix(1, 1);
            roty    = - rotationMatrix(1, 0);
            Z       = std::atan2(roty, rotx);
        }

        /*if (X < (Type)0)
            X += (Type)360;
        if (Y < (Type)0)
            Y += (Type)360;
        if (Z < (Type)0)
            Z += (Type)360;*/

        if (X < (Type)0)
            X += (Type)(2.0 * M_PI);
        if (Y < (Type)0)
            Y += (Type)(2.0 * M_PI);
        if (Z < (Type)0)
            Z += (Type)(2.0 * M_PI);

        return TVector<Type>::create(X, Y, Z);
    }

    //!RADIANS!
    template <typename Type>
    inline static TMatrix<Type> eulerAnglesToRotationMatrix_3d(const TVector<Type>& angles)
    {
        TMath_assert(angles.size() == 3);

        Type cx = std::cos(angles(0));
        Type sx = std::sin(angles(0));
        Type cy = std::cos(angles(1));
        Type sy = std::sin(angles(1));
        Type cz = std::cos(angles(2));
        Type sz = std::sin(angles(2));

        Type sxy = sx * sy;
        Type cxy = cx * sy;

        TMatrix<Type> rotationMatrix(3, 3);
        Type* m_data = rotationMatrix.data();

        m_data[0] = (Type)(cy * cz);
        m_data[1] = (Type)(cy * sz);
        m_data[2] = (Type)(- sy);

        m_data[3] = (Type)(sxy * cz - cx * sz);
        m_data[4] = (Type)(sxy * sz + cx * cz);
        m_data[5] = (Type)(sx * cy);

        m_data[6] = (Type)(cxy * cz + sx * sz);
        m_data[7] = (Type)(cxy * sz - sx * cz);
        m_data[8] = (Type)(cx * cy);

        return rotationMatrix;
    }

    template <typename Type>
    inline static TVector<Type> rotationMatrixToQuaternion(const TMatrix<Type>& rotationMatrix)
    {
        TMath_assert((rotationMatrix.rows() >= 3) && (rotationMatrix.cols() >= 3));

        TVector<Type> q(4);
        Type trace = rotationMatrix(0, 0) + rotationMatrix(1, 1) + rotationMatrix(2, 2) + (Type)1;
        if (trace > (Type)0) {
            const Type s = (Type)(2) * std::sqrt(trace);
            q(0) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) / s;
            q(1) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) / s;
            q(2) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) / s;
            q(3) = (Type)(0.25) * s;
        } else {
            if ((rotationMatrix(0, 0) > rotationMatrix(1, 1)) && (rotationMatrix(0, 0) > rotationMatrix(2, 2))) {
                const Type s = (Type)(2) * std::sqrt(1 + rotationMatrix(0, 0) - rotationMatrix(1, 1) - rotationMatrix(2, 2));
                q(0) = (Type)(0.25) * s;
                q(1) = (rotationMatrix(0, 1) + rotationMatrix(1, 0)) / s;
                q(2) = (rotationMatrix(2, 0) + rotationMatrix(0, 2)) / s;
                q(3) = (rotationMatrix(2, 1) - rotationMatrix(1, 2)) / s;
            } else if (rotationMatrix(1, 1) > rotationMatrix(2, 2)){
                const Type s = (Type)(2) * std::sqrt((Type)(1) + rotationMatrix(1, 1) - rotationMatrix(0, 0) - rotationMatrix(2, 2));
                q(0) = (rotationMatrix(0, 1) + rotationMatrix(1, 0)) / s;
                q(1) = (Type)(0.25) * s;
                q(2) = (rotationMatrix(1, 2) + rotationMatrix(2, 1)) / s;
                q(3) = (rotationMatrix(0, 2) - rotationMatrix(2, 0)) / s;
            } else {
                const Type s = (Type)(2) * std::sqrt((Type)(1) + rotationMatrix(2, 2) - rotationMatrix(0, 0) - rotationMatrix(1, 1));
                q(0) = (rotationMatrix(0, 2) + rotationMatrix(2, 0)) / s;
                q(1) = (rotationMatrix(1, 2) + rotationMatrix(2, 1)) / s;
                q(2) = (Type)(0.25) * s;
                q(3) = (rotationMatrix(1, 0) - rotationMatrix(0, 1)) / s;
            }
        }
        q.normalize();
        return q;
    }

    template <typename Type>
    inline static TMatrix<Type> quaterionToRotationMatrix(const TVector<Type>& quaternion)
    {
        TMath_assert(quaternion.size() >= 4);

        TMatrix<Type> rotationMatrix(3, 3);
        Type* data = rotationMatrix.data();

        Type xx = quaternion(0) * quaternion(0);
        Type xy = quaternion(0) * quaternion(1);
        Type xz = quaternion(0) * quaternion(2);
        Type xw = quaternion(0) * quaternion(3);
        Type yy = quaternion(1) * quaternion(1);
        Type yz = quaternion(1) * quaternion(2);
        Type yw = quaternion(1) * quaternion(3);
        Type zz = quaternion(2) * quaternion(2);
        Type zw = quaternion(2) * quaternion(3);

        data[0] = (Type)(1) - (Type)(2) * (yy + zz);
        data[3] =             (Type)(2) * (xy + zw);
        data[6] =             (Type)(2) * (xz - yw);

        data[1] =             (Type)(2) * (xy - zw);
        data[4] = (Type)(1) - (Type)(2) * (xx + zz);
        data[7] =             (Type)(2) * (yz + xw);

        data[2] =             (Type)(2) * (xz + yw);
        data[5] =             (Type)(2) * (yz - xw);
        data[8] = (Type)(1) - (Type)(2) * (xx + yy);

        return rotationMatrix;
    }

    template <typename Type>
    inline static TVector<Type> multiplyQuaternions(const TVector<Type>& a, const TVector<Type>& b)
    {
        return TVector<Type>::create(a(3) * b(0) + a(0) * b(3) + a(1) * b(2) - a(2) * b(1),
                                     a(3) * b(1) - a(0) * b(2) + a(1) * b(3) + a(2) * b(0),
                                     a(3) * b(2) + a(0) * b(1) - a(1) * b(0) + a(2) * b(3),
                                     a(3) * b(3) - a(0) * b(0) - a(1) * b(1) - a(2) * b(2));
    }
};

} // namespace TMath

#include "TTools_impl.h"
#endif // TMATH_H
