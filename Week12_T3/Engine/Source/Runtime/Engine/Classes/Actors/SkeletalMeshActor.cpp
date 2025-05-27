#include "SkeletalMeshActor.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
#include "Animation/CustomAnimInstance/TestAnimInstance.h"
ASkeletalMeshActor::ASkeletalMeshActor()
{
    SkeletalMeshComp = AddComponent<USkeletalMeshComponent>(EComponentOrigin::Constructor);
    SkeletalMeshComp->SetData("Contents/FBX/Rumba_Dancing.fbx");
    // NOTICE : PhysicsPreview 기준 애니메이션 초기화 필요없어서 주석처리
    //UTestAnimInstance* TestAnimInstance = FObjectFactory::ConstructObject<UTestAnimInstance>(SkeletalMeshComp);
    //SkeletalMeshComp->SetAnimInstance(TestAnimInstance);
    RootComponent = SkeletalMeshComp;
}