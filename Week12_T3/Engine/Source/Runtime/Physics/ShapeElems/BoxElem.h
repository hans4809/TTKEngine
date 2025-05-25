#pragma once
#include "ShapeElem.h"
#include "Math/BoundingBox.h"

struct FKBoxElem : public FKShapeElem
{
    DECLARE_STRUCT(FKBoxElem, FKShapeElem)

    UPROPERTY(EditAnywhere, FMatrix, TM, = FMatrix())

    UPROPERTY(EditAnywhere, FQuat, Orientation, = FQuat::Identity)

    UPROPERTY(EditAnywhere, FVector, Center, = FVector::ZeroVector)

    UPROPERTY(EditAnywhere, FQuat, Rotation, = FQuat::Identity)

    /** 박스의 x축 방향 절반 길이 */
    UPROPERTY(EditAnywhere, float, X, = 0.0f)

    /** 박스의 y축 방향 크기 */
    UPROPERTY(EditAnywhere, float, Y, = 0.0f)
    
    /** 박스의 z축 방향 크기 */
    UPROPERTY(EditAnywhere, float, Z, = 0.0f)

    FKBoxElem();

    FKBoxElem(const float InScale);

    FKBoxElem(const float InX, const float InY, const float InZ );

    ~FKBoxElem() override;

    friend bool operator==( const FKBoxElem& LHS, const FKBoxElem& RHS );;

    // Utility function that builds an FTransform from the current data
    FTransform GetTransform() const;;

    void SetTransform( const FTransform& InTransform );

    float GetVolume(const FVector& Scale3D) const;

    float GetScaledVolume(const FVector& Scale3D) const;

    FBoundingBox CalcAABB(const FTransform& BoneTM, float Scale) const;

    void ScaleElem(FVector DeltaSize, float MinSize);

    FKBoxElem GetFinalScaled(const FVector& Scale3D, const FTransform& RelativeTM) const;

    static EAggCollisionShape::Type StaticShapeType;

    /**	
    * 요소와 월드 위치 간의 최단 거리를 계산합니다. 입력과 출력은 월드 공간 기준입니다.
    * @param	WorldPosition	최단 거리를 구하려는 월드 공간상의 점
    * @param	BodyToWorldTM	BodySetup을 월드 공간으로 변환하는 트랜스폼
    * @return					WorldPosition과 도형 간의 거리. 0이면 도형 내부에 있음을 의미합니다.
    */
    float GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BodyToWorldTM) const;

	
    /**	
     * 월드 공간상의 점이 주어졌을 때, 도형 위의 가장 가까운 점을 찾습니다. 입력과 출력은 월드 공간 기준입니다.
     * @param	WorldPosition			최단 거리를 구하려는 월드 공간상의 점
     * @param	BodyToWorldTM			BodySetup을 월드 공간으로 변환하는 트랜스폼
     * @param	ClosestWorldPosition	도형 위에서 WorldPosition에 가장 가까운 점 (월드 공간)
     * @param	Normal					ClosestWorldPosition에 대응하는 표면의 법선 벡터
     * @return							WorldPosition과 도형 간의 거리. 0이면 도형 내부에 있음을 의미합니다.
     */
    float GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BodyToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const;
};
