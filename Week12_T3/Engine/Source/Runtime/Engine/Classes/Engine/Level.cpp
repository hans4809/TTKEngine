#include "Level.h"

#include "GameFramework/Actor.h"
#include "Delegates/Delegate.impl.h"
#include "UserInterface/Console.h"

ULevel::ULevel()
{
}

ULevel::~ULevel()
{
}

void ULevel::PostLoad()
{
    // 1. 모든 SceneComponent를 빠르게 찾기 위한 맵 생성 (로드 중에 미리 만들어 둘 수도 있음)
    TMap<FGuid, USceneComponent*> ComponentMap;
    for (AActor* Actor : GetActors()) // 실제 액터 목록 가져오는 방식 사용
    {
        for (UActorComponent* Comp : Actor->GetComponents()) // 실제 컴포넌트 목록 가져오는 방식 사용
        {
            if (USceneComponent* SceneComp = Cast<USceneComponent>(Comp))
            {
                ComponentMap.Add(SceneComp->GetComponentID(), SceneComp);
            }
        }
    }

    // 2. 모든 SceneComponent를 순회하며 부모 찾아 연결
    for (auto const& [ID, SceneComp] : ComponentMap)
    {
        FGuid ParentID = SceneComp->GetPendingAttachParentID();
        if (ParentID.IsValid()) // 유효한 부모 ID가 있는지 확인
        {
            USceneComponent** FoundParentPtr = ComponentMap.Find(ParentID);
            if (FoundParentPtr)
            {
                // 부모를 찾았으면 Attach!
                SceneComp->AttachToComponent(*FoundParentPtr);
            }
            else
            {
                // 오류: 저장된 부모 ID에 해당하는 컴포넌트를 찾을 수 없음
                UE_LOG(LogLevel::Error,TEXT("Component '%s' (ID: %s) could not find parent with ID: %s during linkup."),
                       *SceneComp->GetName());
            }
        }
    }
}

void ULevel::PostDuplicate()
{
    UObject::PostDuplicate();
    for (AActor* Actor : Actors)
    {
        PendingBeginPlayActors.Add(Actor);
    }
}

