#pragma once

#include <DirectXMath.h>

#include "MathUtility.h"
#include "Container/String.h"
#include "Serialization/Archive.h"

namespace physx 
{
    struct PxVec3;
}


struct FVector2D
{
    float X, Y;
    explicit FVector2D(float _x = 0, float _y = 0) : X(_x), Y(_y) {}

    static const FVector2D ZeroVector;
    static const FVector2D OneVector;
    
    FVector2D operator+(const FVector2D& rhs) const
    {
        return FVector2D(X + rhs.X, Y + rhs.Y);
    }
    FVector2D operator-(const FVector2D& rhs) const
    {
        return FVector2D(X - rhs.X, Y - rhs.Y);
    }
    FVector2D operator*(float rhs) const
    {
        return FVector2D(X * rhs, Y * rhs);
    }
    FVector2D operator/(float rhs) const
    {
        return FVector2D(X / rhs, Y / rhs);
    }
    FVector2D& operator+=(const FVector2D& rhs)
    {
        X += rhs.X;
        Y += rhs.Y;
        return *this;
    }

    void Serialize(FArchive& Ar) const
    {
        Ar << X << Y;
    }
    void Deserialize(FArchive& Ar)
    {
        Ar >> X >> Y;
    }
};

// 3D 벡터
struct FVector
{
    float X, Y, Z;

    FVector() : X(0), Y(0), Z(0) {}
    FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
    explicit FVector(float Scalar) : X(Scalar), Y(Scalar), Z(Scalar) {}

    FVector operator+(const FVector& Other) const;
    FVector& operator+=(const FVector& Other);

    FVector operator-(const FVector& Other) const;
    FVector& operator-=(const FVector& Other);

    FVector operator*(const FVector& Other) const;
    FVector operator*(float Scalar) const;
    FVector& operator*=(float Scalar);
    FVector& operator *= (const FVector& Other);
    FVector operator/(const FVector& Other) const;
    FVector operator/(float Scalar) const;
    FVector& operator/=(float Scalar);

    FVector operator-() const;
    // 인덱스 접근 연산자 추가
    float& operator[](int Index)
    {
        switch (Index)
        {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        default: throw std::out_of_range("FVector index out of range");
        }
    }

    float operator[](int Index) const
    {
        switch (Index)
        {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        default: throw std::out_of_range("FVector index out of range");
        }
    }

    // 벡터 내적
    float Dot(const FVector& other) const {
        return X * other.X + Y * other.Y + Z * other.Z;
    }

    // 벡터 크기
    float Magnitude() const {
        return std::sqrt(X * X + Y * Y + Z * Z);
    }

    float MagnitudeSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }

    // 벡터 정규화
    FVector Normalize() const {
        float mag = Magnitude();
        return (mag > 0) ? FVector(X / mag, Y / mag, Z / mag) : FVector(0, 0, 0);
    }

    FVector GetSafeNormal(float Tolerance = KINDA_SMALL_NUMBER) const
    {
        const float SquareSum = X * X + Y * Y + Z * Z;

        // Not sure if it's safe to add tolerance in there. Might introduce too many errors
        if (SquareSum == 1.f)
        {
            return *this;
        }
        else if (SquareSum < Tolerance)
        {
            return ZeroVector;
        }
        const float Scale = FMath::InvSqrt(SquareSum);
        return { X * Scale, Y * Scale, Z * Scale };
    }

    FVector Cross(const FVector& Other) const
    {
        return FVector{
            Y * Other.Z - Z * Other.Y,
            Z * Other.X - X * Other.Z,
            X * Other.Y - Y * Other.X
        };
    }
    FVector Min(const FVector& Other) const
    {
        FVector result = *this;
        if (X > Other.X)
            result.X = Other.X;
        if (Y > Other.Y)
            result.Y = Other.Y;
        if (Z > Other.Z)
            result.Z = Other.Z;

        return result;
    }

    float GetMin() const
    {
        return FMath::Min3(X,Y,Z);
    }
    
    float GetAbsMin() const
    {
        return FMath::Min3(FMath::Abs(X), FMath::Abs(Y), FMath::Abs(Z));
    }

    FVector Max(const FVector& Other) const
    {
        FVector result = *this;
        if (X < Other.X)
            result.X = Other.X;
        if (Y < Other.Y)
            result.Y = Other.Y;
        if (Z < Other.Z)
            result.Z = Other.Z;

        return result;
    }

    float MaxValue() const
    {
        float result = X >= Y ? X : Y;
        result = result >= Z ? result : Z;
        return result;
    }

    bool operator==(const FVector& other) const {
        return (X == other.X && Y == other.Y && Z == other.Z);
    }

    float Distance(const FVector& other) const {
        // 두 벡터의 차 벡터의 크기를 계산
        return ((*this - other).Magnitude());
    }

    FString ToString() const;
    bool IsUniform(float Tolerance = KINDA_SMALL_NUMBER) const;

    static float Distance(const FVector& V1, const FVector& V2)
    {
        return FMath::Sqrt(
            FMath::Square(V2.X - V1.X)
            + FMath::Square(V2.Y - V1.Y)
            + FMath::Square(V2.Z - V1.Z)
        );
    }

    DirectX::XMFLOAT3 ToXMFLOAT3() const
    {
        return DirectX::XMFLOAT3(X, Y, Z);
    }

    physx::PxVec3 ToPxVec3() const;

    FVector ClampMaxSize(float MaxSize) const
    {
        if (MaxSize < 1.e-4f)
        {
            return ZeroVector;
        }

        const float VSq = MagnitudeSquared();
        if (VSq > MaxSize * MaxSize)
        {
            const float Scale = MaxSize * FMath::InvSqrt(VSq);
            return { X * Scale, Y * Scale, Z * Scale };
        }
        else
        {
            return *this;
        }
    }
    void Serialize(FArchive& Ar) const
    {
        Ar << X << Y << Z;
    }

    void Deserialize(FArchive& Ar)
    {
        Ar >> X >> Y >> Z;
    }
    bool IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const
    {
        return FMath::Abs(X) <= Tolerance &&
            FMath::Abs(Y) <= Tolerance &&
            FMath::Abs(Z) <= Tolerance;
    }

    FVector GetAbs() const;
    bool ContainsNAN() const;
    bool IsFinitie() const;

    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
    static const FVector PToFVector(physx::PxVec3 InPxVec3);
    // Unit X Axis Vector (1, 0, 0)
    static const FVector XAxisVector;
    // Unit Y Axis Vector (0, 1, 0)
    static const FVector YAxisVector;
    // Unit Z Axis Vector (0, 0, 1)
    static const FVector ZAxisVector;
};
inline FVector FVector::operator+(const FVector& Other) const
{
    return { X + Other.X, Y + Other.Y, Z + Other.Z };
}

inline FVector& FVector::operator+=(const FVector& Other)
{
    X += Other.X; Y += Other.Y; Z += Other.Z;
    return *this;
}

inline FVector FVector::operator-(const FVector& Other) const
{
    return { X - Other.X, Y - Other.Y, Z - Other.Z };
}

inline FVector& FVector::operator-=(const FVector& Other)
{
    X -= Other.X; Y -= Other.Y; Z -= Other.Z;
    return *this;
}

inline FVector FVector::operator*(const FVector& Other) const
{
    return { X * Other.X, Y * Other.Y, Z * Other.Z };
}

inline FVector FVector::operator*(float Scalar) const
{
    return { X * Scalar, Y * Scalar, Z * Scalar };
}

inline FVector& FVector::operator*=(float Scalar)
{
    X *= Scalar; Y *= Scalar; Z *= Scalar;
    return *this;
}

inline FVector& FVector::operator*=(const FVector& Other)
{
    X *= Other.X;
    Y *= Other.Y;
    Z *= Other.Z;
    return *this;
}

inline FVector FVector::operator/(const FVector& Other) const
{
    return { X / Other.X, Y / Other.Y, Z / Other.Z };
}

inline FVector FVector::operator/(float Scalar) const
{
    return { X / Scalar, Y / Scalar, Z / Scalar };
}

inline FVector& FVector::operator/=(float Scalar)
{
    X /= Scalar; Y /= Scalar; Z /= Scalar;
    return *this;
}

inline FVector FVector::operator-() const
{
    return { -X, -Y, -Z };
}

inline FString FVector::ToString() const
{
    // FString::Printf를 사용하여 포맷팅된 문자열 생성
    // TEXT() 매크로는 리터럴 문자열을 TCHAR 타입으로 만들어줍니다.
    return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);

    // 필요에 따라 소수점 정밀도 지정 가능: 예) "X=%.2f Y=%.2f Z=%.2f"
    // return FString::Printf(TEXT("X=%.2f Y=%.2f Z=%.2f"), x, y, z);
}

inline bool FVector::IsUniform(const float Tolerance) const
{
    return FMath::Abs(X - Y) <= Tolerance && FMath::Abs(X - Z) <= Tolerance && FMath::Abs(Y - Z) <= Tolerance;
}

inline FVector FVector::GetAbs() const
{
    return FVector(FMath::Abs(X), FMath::Abs(Y), FMath::Abs(Z));
}

inline bool FVector::ContainsNAN() const
{
    return isnan(X) || isnan(Y) || isnan(Z);
}

inline bool FVector::IsFinitie() const
{
    return isfinite(X) && isfinite(Y) && isfinite(Z);
}
