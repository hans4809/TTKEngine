#pragma once
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"

namespace EAggCollisionShape
{
    enum Type : int
    {
        Sphere,
        Box,
        Sphyl,
        Convex,
        TaperedCapsule,
        LevelSet,
        SkinnedLevelSet,

        Unknown
    };
}

struct FKShapeElem
{
    DECLARE_STRUCT(FKShapeElem)

    FKShapeElem();

    FKShapeElem(const EAggCollisionShape::Type InShapeType);

    FKShapeElem(const FKShapeElem& Copy);

    virtual ~FKShapeElem();

    const FKShapeElem& operator=(const FKShapeElem& Other);

    template <typename T>
    T* GetShapeCheck();

    //const FUserData* GetUserData() const { FUserData::Set<FKShapeElem>((void*)&UserData, const_cast<FKShapeElem*>(this));  return &UserData; }

    static EAggCollisionShape::Type StaticShapeType;

    // 이 쉐이프의 사용자 정의 이름을 가져옵니다.
    const FName& GetName() const;

    // 이 쉐이프의 사용자 정의 이름을 설정합니다.
    void SetName(const FName& InName);

    // 이 쉐이프의 타입을 가져옵니다.
    EAggCollisionShape::Type GetShapeType() const;

    // 이 쉐이프가 본의 질량에 기여하는지 여부를 가져옵니다.
    bool GetContributeToMass() const;

    // 이 쉐이프가 본의 질량에 기여할지 여부를 설정합니다.
    void SetContributeToMass(bool bInContributeToMass);

    // 이 쉐이프가 쿼리나 시뮬레이션 충돌에 포함될지 여부를 설정합니다.
    void SetCollisionEnabled(ECollisionEnabled::Type InCollisionEnabled);

    // 이 쉐이프가 쿼리나 시뮬레이션 충돌에 포함될지 여부를 가져옵니다.
    ECollisionEnabled::Type GetCollisionEnabled() const;

public:
    // 접촉점을 생성할 때 사용하는 오프셋입니다.
    // Minkowski 합을 반경 R만큼 부드럽게 만들어 불규칙한 표면 위에서도
    // 객체가 매끄럽게 미끄러지도록 돕습니다.
    float RestOffset = false;

    // 이 쉐이프가 엔진에서 생성되고 임포트된 것이 아닐 때 true입니다.
    bool bIsGenerated = false;

protected:
    // 이 쉐이프 요소의 인스턴스를 안전하게 복제하는 헬퍼 함수입니다.
    void CloneElem(const FKShapeElem& Other);

private:
    // 이 쉐이프의 사용자 정의 이름입니다.
    UPROPERTY(EditAnywhere, FName, Name, = TEXT("None"))

    EAggCollisionShape::Type ShapeType = EAggCollisionShape::Unknown;

    // 이 쉐이프가 속한 본의 전체 질량에 기여해야 할 경우 true입니다.
    // 이를 통해 실제 질량 속성에 영향을 주지 않는 추가 충돌 볼륨을 만들 수 있습니다.
    UPROPERTY(EditAnywhere, bool, bContributeToMass, = false)

    // 프리미티브별 충돌 필터링 설정입니다.
    // 시뮬레이션 및 쿼리 충돌에서 개별 프리미티브를
    // 필터링 세부정보 변경 없이 켜거나 끌 수 있습니다.
    ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::NoCollision;
    
    //FChaosUserData UserData;
};

template <typename T>
T* FKShapeElem::GetShapeCheck()
{
    if(T::StaticShapeType != ShapeType)
        return nullptr;
    else
        return static_cast<T*>(this);
}

void SetupNonUniformHelper(FVector InScale3D, double& OutMinScale, double& OutMinScaleAbs, FVector& OutScale3DAbs);

void SetupNonUniformHelper(FVector InScale3D, float& OutMinScale, float& OutMinScaleAbs, FVector& OutScale3DAbs);