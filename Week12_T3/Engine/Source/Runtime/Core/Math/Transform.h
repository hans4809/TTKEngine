#pragma once
#include "MathSSE.h"
#include "Quat.h"
#include "Vector.h"
#include "Rotator.h"
#include <PxTransform.h>


class FTransform
{
public:
    FTransform() : Rotation(FQuat::Identity), Location(FVector::ZeroVector), Scale(FVector::OneVector) {}
    FTransform(FQuat InRotation, FVector InLocation, FVector InScale) : Rotation(InRotation), Location(InLocation), Scale(InScale) {}
    FTransform(const FMatrix& InMatrix);
    FTransform(const  physx::PxTransform& PxTransform)
    {
        Location = FVector::PToFVector(PxTransform.p);
        Rotation = FQuat::PToFQuat(PxTransform.q);
    };
    FQuat Rotation;
    FVector Location;
    FVector Scale;

    FVector GetLocation() const { return Location; }
    FQuat GetRotation() const { return Rotation; }
    FVector GetScale() const { return Scale; }

    void SetRotation(const FQuat& InRotation) { Rotation = InRotation; }
    void SetRotation(const FRotator& InRotation) { Rotation = InRotation.ToQuaternion(); }
    void SetLocation(const FVector& InLocation) { Location = InLocation; }
    void SetScale(const FVector& InScale) { Scale = InScale; }
    void SetFromMatrix(const FMatrix& InMatrix);


    FRotator Rotator() const
    {
        return FRotator(Rotation);
    }

    FTransform operator*(const FTransform& transform) const;
    void operator*=(const FTransform& Other);

    FVector InverseTransformPositionNoScale(FVector vector) const;
    FVector TransformPositionNoScale(FVector vector) const;
    FVector TransformVectorNoScale(FVector Vector) const;
    void ScaleTranslation(FVector InScale3D);

    FVector GetScaledAxis(EAxis::Type InAxis);
    void RemoveScaling();
    /** 이 트랜스폼의 역변환을 반환합니다. */
    FTransform Inverse() const;

    FString ToString() const;

    FTransform GetRelativeTransform(const FTransform& Base) const;

    const static FTransform Identity;

    static FTransform Blend(const FTransform& Atom1, const FTransform& Atom2, float Alpha);

    // 4x4 Matrix로 변환하는 함수
    FMatrix ToMatrixWithScale() const;
    FMatrix ToMatrixNoScale() const;

    inline friend FArchive& operator<<(FArchive& Ar, FTransform& Transform)
    {
        return Ar << Transform.Location
            << Transform.Rotation
            << Transform.Scale;
    }

    static void ConstructTransformFromMatrixWithDesiredScale(const FMatrix& matrix, const FMatrix& to_matrix_with_scale, VectorRegister4Float vector_multiply, FTransform& transform);
    static void MultiplyUsingMatrixWithScale(FTransform* transform, const FTransform* transform1, const FTransform* transform2);
    static void Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B);

    FVector TransformVector(const FVector InVector) const;

    FVector TransformPosition(const FVector V) const;

    physx::PxTransform ToPxTransform() const
    {
        physx::PxQuat pxRotation = Rotation.ToPxQuat();
        if (!pxRotation.isUnit())
        {
            pxRotation.normalize(); // PxQuat 정규화
        }
        return physx::PxTransform(Location.ToPxVec3(), pxRotation);
    }

    // 로컬 축을 월드 공간 단위 벡터로 반환
    FVector GetUnitAxis(EAxis::Type Axis) const;
    

private:
    static bool Private_AnyHasNegativeScale(FVector InScale3D, FVector InOtherScale3D);
};
inline const FTransform FTransform::Identity = FTransform();

inline FTransform::FTransform(const FMatrix& InMatrix)
{
    SetFromMatrix(InMatrix);
}