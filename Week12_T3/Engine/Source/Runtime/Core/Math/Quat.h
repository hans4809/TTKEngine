#pragma once
#include "Matrix.h"
#include "Container/String.h"
#include "Serialization/Archive.h"

#include <PxQuat.h>

// 쿼터니언
struct FQuat
{
    float X, Y, Z, W;

    // 기본 생성자
    explicit FQuat()
        : W(1.0f), X(0.0f), Y(0.0f), Z(0.0f)
    {}

    // FQuat 생성자 추가: 회전 축과 각도를 받아서 FQuat 생성
    explicit FQuat(const FVector& Axis, float Angle);

    // W, X, Y, Z 값으로 초기화
    explicit FQuat(float InW, float InX, float InY, float InZ)
        : W(InW), X(InX), Y(InY), Z(InZ)
    {}

    explicit FQuat(const FMatrix& InMatrix);

    const static FQuat Identity;

    void Serialize(FArchive& Ar) const
    {
        Ar << W << X << Y << Z;
    }
    void Deserialize(FArchive& Ar)
    {
        Ar >> W >> X >> Y >> Z;
    }

    // 쿼터니언의 곱셈 연산 (회전 결합)
    FQuat operator*(const FQuat& Other) const;

    // (쿼터니언) 벡터 회전
    FVector RotateVector(const FVector& Vec) const;

    // 단위 쿼터니언 여부 확인
    bool IsNormalized() const;

    // 쿼터니언 정규화 (단위 쿼터니언으로 만듬)
    void Normalize(float Tolerance = SMALL_NUMBER);

    FQuat GetUnsafeNormal() const;
    FQuat GetSafeNormal(float Tolerance = SMALL_NUMBER) const;

    // 회전 각도와 축으로부터 쿼터니언 생성 (axis-angle 방식)
    static FQuat FromAxisAngle(const FVector& Axis, float Angle);

    static FQuat CreateRotation(float roll, float pitch, float yaw);

    // 쿼터니언을 회전 행렬로 변환
    FMatrix ToMatrix() const;

    bool Equals(const FQuat& Q, float Tolerance = KINDA_SMALL_NUMBER) const;

    FRotator Rotator() const;

    float AngularDistance(const FQuat& Q) const;
    
    static FQuat Slerp(const FQuat& Quat1, const FQuat& Quat2, float Slerp);
    
    static FQuat Slerp_NotNormalized(const FQuat& Quat1, const FQuat& Quat2, float Slerp);

    static FQuat PToFQuat(physx::PxQuat InPxQuat);

    physx::PxQuat ToPxQuat() const
    {
        return physx::PxQuat(X, Y, Z, W);
    }

    FString ToString() const;

    bool operator==(const FQuat& Other) const;

    FQuat Inverse() const;
};

inline const FQuat FQuat::Identity = FQuat(1.0f, 0.0f, 0.0f, 0.0f);
