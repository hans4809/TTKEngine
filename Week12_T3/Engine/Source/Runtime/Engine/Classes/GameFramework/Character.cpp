#include "Character.h"

#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponents/Physics/UCapsuleShapeComponent.h"
#include "Animation/CustomAnimInstance/TestAnimInstance.h"
#include "Components/InputComponent.h"
#include "Components/GameFramework/ProjectileMovementComponent.h"
#include "Contents/GameManager.h"
#include "Engine/Asset/AssetManager.h"
#include "UObject/FunctionRegistry.h"

ACharacter::ACharacter()
{
    BodyMesh = AddComponent<USkeletalMeshComponent>(EComponentOrigin::Constructor);
    USkeletalMesh* SkeletalMesh = UAssetManager::Get().Get<USkeletalMesh>("NyeongFBX");
    BodyMesh->SetSkeletalMesh(SkeletalMesh);
    UTestAnimInstance* TestAnimInstance = FObjectFactory::ConstructObject<UTestAnimInstance>(BodyMesh);
    BodyMesh->SetAnimInstance(TestAnimInstance);
    RootComponent = BodyMesh;
    
    CollisionCapsule = AddComponent<UCapsuleShapeComponent>(EComponentOrigin::Constructor);
    CollisionCapsule->SetupAttachment(RootComponent);
    
    float XSize = BodyMesh->GetBoundingBox().Max.X - BodyMesh->GetBoundingBox().Min.X;
    float YSize = BodyMesh->GetBoundingBox().Max.Y - BodyMesh->GetBoundingBox().Min.Y;
    float ZSize = BodyMesh->GetBoundingBox().Max.Z - BodyMesh->GetBoundingBox().Min.Z;
    CollisionCapsule->SetRadius(std::min(XSize / 2, YSize / 2));
    CollisionCapsule->SetHalfHeight(ZSize / 2);
    CollisionCapsule->SetRelativeLocation(FVector(0, 0, ZSize/2));

    MovementComponent = AddComponent<UProjectileMovementComponent>(EComponentOrigin::Constructor);

    PlayAnimA.AddStatic(FGameManager::PlayAnimA);
    PlayAnimB.AddStatic(FGameManager::PlayAnimB);
    PlayAnimC.AddStatic(FGameManager::PlayAnimC);

    FGameManager::Get().GameOverEvent.Clear();
    FGameManager::Get().GameOverEvent.AddLambda([this]{ Cast<UTestAnimInstance>(BodyMesh->GetAnimInstance())->SetState(ETestState::Defeated); });
}

void ACharacter::Tick(float DeltaTime) // TODO: Character 상속받는 MyCharacter 만들어서 거기에다가 제작
{
    APawn::Tick(DeltaTime);
    MovementComponent->Velocity *= 0.9f;
    if (MovementComponent->Velocity.Magnitude() < 0.1f)
    {
        MovementComponent->Velocity = FVector::ZeroVector;
    }
}

void ACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) // TODO: Character 상속받는 MyCharacter 만들어서 거기에다가 제작
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // 카메라 조작용 축 바인딩
    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindAxis("MoveForward", [this](float V) { GetMovementComponent()->Velocity += FVector(V,0,0); });
        PlayerInputComponent->BindAxis("MoveForward", [this](float V) { GetMovementComponent()->Velocity += FVector(V,0,0); });
        PlayerInputComponent->BindAxis("MoveRight", [this](float V) { GetMovementComponent()->Velocity += FVector(0,V,0); });
        PlayerInputComponent->BindAxis("MoveRight", [this](float V) { GetMovementComponent()->Velocity += FVector(0,V,0); });
        PlayerInputComponent->BindAction("AnimA", [this] { Cast<UTestAnimInstance>(BodyMesh->GetAnimInstance())->SetState(ETestState::Pose); PlayAnimA.Broadcast(); });
        PlayerInputComponent->BindAction("AnimB", [this] { Cast<UTestAnimInstance>(BodyMesh->GetAnimInstance())->SetState(ETestState::Jump); PlayAnimB.Broadcast(); });
        PlayerInputComponent->BindAction("AnimC", [this] { Cast<UTestAnimInstance>(BodyMesh->GetAnimInstance())->SetState(ETestState::Dance); PlayAnimC.Broadcast(); });
    }
}


void ACharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
}

void ACharacter::UnPossessed()
{
    Super::UnPossessed();
}
