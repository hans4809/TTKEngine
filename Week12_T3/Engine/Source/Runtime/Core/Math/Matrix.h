#pragma once
#include "MathUtility.h"

class FArchive;
struct FVector4;
struct FVector;
struct FRotator;
struct FQuat;

namespace EAxis
{
    enum Type
    {
        None,
        X,
        Y,
        Z,
    };
}


// Extended axis enum for more specialized usage
namespace EAxisList
{
    enum Type
    {
        None = 0,
        X = 1,
        Y = 2,
        Z = 4,

        Screen = 8,
        XY = X | Y,
        XZ = X | Z,
        YZ = Y | Z,
        XYZ = X | Y | Z,
        All = XYZ | Screen,

        //alias over Axis YZ since it isn't used when the z-rotation widget is being used
        ZRotation = YZ,

        // alias over Screen since it isn't used when the 2d translate rotate widget is being used
        Rotate2D = Screen,
    };
}


// 4x4 행렬 연산
struct alignas(16) FMatrix
{
public:
    alignas(16) float M[4][4];

public:
    static const FMatrix Identity;

public:
    // 기본 연산자 오버로딩
    FMatrix operator+(const FMatrix& Other) const;
    FMatrix operator-(const FMatrix& Other) const;
    FMatrix operator*(const FMatrix& Other) const;
    FMatrix operator*(float Scalar) const;
    FMatrix operator/(float Scalar) const;
    float* operator[](int row);
    const float* operator[](int row) const;
	
    FVector ExtractScaling(float Tolerance = SMALL_NUMBER);
    FVector GetOrigin() const;
    float Determinant() const;

    void SetAxis(int32 i, const FVector& Axis);
    FVector GetScaledAxis(EAxis::Type InAxis) const;

    // 유틸리티 함수
    static FMatrix Transpose(const FMatrix& Mat);
    static FMatrix Inverse(const FMatrix& Mat);
    static FMatrix CreateRotationMatrix(float roll, float pitch, float yaw);
    static FMatrix CreateRotationMatrix(const FRotator& rot);
    static FMatrix CreateRotationMatrix(const FQuat& quat);
    static FMatrix CreateScaleMatrix(float scaleX, float scaleY, float scaleZ);
    static FMatrix CreateScaleMatrix(const FVector& scale);
    static FVector TransformVector(const FVector& v, const FMatrix& m);
    static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
    static FMatrix CreateTranslationMatrix(const FVector& position);

    FVector4 TransformFVector4(const FVector4& vector) const;
    FVector TransformPosition(const FVector& vector) const;
    
    static FMatrix GetScaleMatrix(const FVector& InScale);
    static FMatrix GetTranslationMatrix(const FVector& InPosition);
    static FMatrix GetRotationMatrix(const FRotator& InRotation);
    static FMatrix GetRotationMatrix(const FQuat& InRotation);

    FQuat ToQuat() const;

    FVector GetScaleVector(float Tolerance = SMALL_NUMBER) const;

    FVector GetTranslationVector() const;

    FMatrix GetMatrixWithoutScale(float Tolerance = SMALL_NUMBER) const;

    void RemoveScaling(float Tolerance = SMALL_NUMBER);

    void Serialize(FArchive& Ar) const;
    void Deserialize(FArchive& Ar);
};