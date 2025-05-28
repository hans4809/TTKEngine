#pragma once
#include "ShapeElem.h"

struct FBoundingBox;

struct FKSphereElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphereElem)
    
    UPROPERTY(FMatrix, TM, = FMatrix())

    UPROPERTY(EditAnywhere, FVector, Center, = FVector::ZeroVector)

    UPROPERTY(EditAnywhere, float, Radius, = 1.f)

    FKSphereElem();

    FKSphereElem(const float InRadius);

    friend bool operator==(const FKSphereElem& LHS, const FKSphereElem& RHS);

    // 현재 데이터를 기반으로 FTransform을 생성하는 유틸리티 함수
    FTransform GetTransform() const;;

    void SetTransform(const FTransform& InTransform);

    float GetVolume(const FVector& Scale) const;

    float GetScaledVolume(const FVector& Scale) const;

    FBoundingBox CalcAABB(const FTransform& InTransform, float Scale) const;

    void ScaleElem(FVector DeltaSize, float MinSize);

    static EAggCollisionShape::Type StaticShapeType;

    FKSphereElem GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const;

    /**  
     * 요소와 월드 좌표 간 최단 거리를 계산합니다. 입력과 출력은 월드 공간 기준입니다.  
     * @param WorldPosition    최단 거리를 계산할 월드 공간상의 점  
     * @param BodyToWorldTM    BodySetup을 월드 공간으로 변환하기 위한 트랜스폼  
     * @return                  WorldPosition과 도형 간의 거리. 0이면 점이 도형 내부에 있음을 의미합니다.  
     */
    float GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BodyToWorldTM) const;

    /**  
     * 월드 좌표상의 점이 주어졌을 때, 도형 위의 가장 가까운 점과 해당 표면 법선 벡터를 계산합니다. 입력과 출력은 월드 공간 기준입니다.  
     * @param WorldPosition          최단 거리를 계산할 월드 공간상의 점  
     * @param BodyToWorldTM          BodySetup을 월드 공간으로 변환하기 위한 트랜스폼  
     * @param ClosestWorldPosition   도형 위에서 WorldPosition에 가장 가까운 점 (월드 공간 기준)  
     * @param Normal                 ClosestWorldPosition에 대응하는 표면의 법선 벡터  
     * @return                        WorldPosition과 도형 간의 거리. 0이면 점이 도형 내부에 있음을 의미합니다.  
     */
    float GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BodyToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const;
};
