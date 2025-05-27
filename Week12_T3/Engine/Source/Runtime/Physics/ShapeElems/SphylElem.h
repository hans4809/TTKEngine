#pragma once
#include "ShapeElem.h"

struct FBoundingBox;

struct FKSphylElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphylElem, FKShapeElem)
    
    UPROPERTY(EditAnywhere, FMatrix, TM, = FMatrix())
    UPROPERTY(EditAnywhere, FQuat, Orientation, = FQuat::Identity)

    /** 캡슐의 원점 위치 */
    UPROPERTY(EditAnywhere, FVector, Center, = FVector::ZeroVector)

    /** 캡슐의 회전 */
    UPROPERTY(EditAnywhere, FQuat, Rotation, = FQuat::Identity)

    /** 캡슐의 반지름 */
    UPROPERTY(EditAnywhere, float, Radius, = 1.0f)

    /** 선분 길이에 해당하며, 실제 전체 길이는 양 끝에 반지름을 더한 값입니다. */
    UPROPERTY(EditAnywhere, float, Length, = 1.0f)

    FKSphylElem();

    FKSphylElem(const float InRadius, const float InLength);

    ~FKSphylElem() override;

    friend bool operator==(const FKSphylElem& LHS, const FKSphylElem& RHS);;

    /** 현재 데이터를 기반으로 FTransform을 생성하는 유틸리티 함수 */
    FTransform GetTransform() const;

    void SetTransform(const FTransform& InTransform);

    float GetVolume(const FVector& Scale) const;

    float GetScaledVolume(const FVector& Scale3D) const;

    FBoundingBox CalcAABB(const FTransform& BoneTM, float Scale) const;

    void ScaleElem(FVector DeltaSize, float MinSize);

    FKSphylElem GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const;
    
    /** X/Y축 최대 스케일을 기반으로 계산되며, 전체 길이의 절반으로 클램핑된 스케일된 반지름을 반환합니다. */
    float GetScaledRadius(const FVector& Scale3D) const;
    /** 캡슐의 원통 부분에 대해 스케일된 길이를 반환합니다. */
    float GetScaledCylinderLength(const FVector& Scale3D) const;
    /** 상단 및 하단 캡을 포함한 전체 길이의 절반에 해당하는 값을 반환합니다. */
    float GetScaledHalfLength(const FVector& Scale3D) const;

    /**  
     * 요소와 월드 위치 간의 최단 거리를 계산합니다. 입력과 출력은 모두 월드 공간 기준입니다.  
     * @param WorldPosition 최단 거리를 구하려는 월드 공간상의 점  
     * @param BodyToWorldTM BodySetup을 월드 공간으로 변환하는 트랜스폼  
     * @return               WorldPosition과 도형 간의 거리. 0이면 WorldPosition이 도형 내부에 있음을 의미합니다.  
     */
    float GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BodyToWorldTM) const;

    /**  
     * 월드 공간상의 점에 대해 도형 위의 가장 가까운 점과 해당 법선 벡터를 계산합니다.  
     * 입력과 출력은 모두 월드 공간 기준입니다.  
     * @param WorldPosition         최단 거리를 구하려는 월드 공간상의 점  
     * @param BodyToWorldTM         BodySetup을 월드 공간으로 변환하는 트랜스폼  
     * @param ClosestWorldPosition  도형 위에서 WorldPosition에 가장 가까운 점 (월드 공간)  
     * @param Normal                ClosestWorldPosition에 대응하는 표면의 법선 벡터  
     * @return                      WorldPosition과 도형 간의 거리. 0이면 WorldPosition이 도형 내부에 있음을 의미합니다.  
     */
    float GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BodyToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const;

    static EAggCollisionShape::Type StaticShapeType;
};
