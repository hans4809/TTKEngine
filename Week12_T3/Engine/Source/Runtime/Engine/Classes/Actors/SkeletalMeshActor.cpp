#include "SkeletalMeshActor.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
#include "Animation/CustomAnimInstance/TestAnimInstance.h"
#include "Engine/Asset/AssetManager.h"

ASkeletalMeshActor::ASkeletalMeshActor()
{
    SkeletalMeshComp = AddComponent<USkeletalMeshComponent>(EComponentOrigin::Constructor);
    USkeletalMesh* SkeletalMesh = UAssetManager::Get().Get<USkeletalMesh>("Rumba_Dancing");
    SkeletalMeshComp->SetSkeletalMesh(SkeletalMesh);
    // NOTICE : PhysicsPreview 기준 애니메이션 초기화 필요없어서 주석처리
    //UTestAnimInstance* TestAnimInstance = FObjectFactory::ConstructObject<UTestAnimInstance>(SkeletalMeshComp);
    //SkeletalMeshComp->SetAnimInstance(TestAnimInstance);
    RootComponent = SkeletalMeshComp;
}
