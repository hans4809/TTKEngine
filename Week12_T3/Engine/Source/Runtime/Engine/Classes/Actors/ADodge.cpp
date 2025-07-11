#include "ADodge.h"
#include "Components/PrimitiveComponents/MeshComponents/StaticMeshComponents/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/World.h"
#include "Delegates/Delegate.impl.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"

ADodge::ADodge()
    : Super()
{
    // SetActorLabel(TEXT("OBJ_DODGE"));
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(TEXT("Dodge"));
    //FManagerOBJ::CreateStaticMesh("Assets/Dodge/Dodge.obj");
    MeshComp->SetStaticMesh(StaticMesh);
    FunctionRegistry()->RegisterMemberFunction<ThisClass>("TestRotate", &ADodge::TestRotate);
    FunctionRegistry()->RegisterMemberFunction<ThisClass>("TestTranslate", &ADodge::TestTranslate);
}


void ADodge::BeginPlay()
{
    Super::BeginPlay();
    for (auto& function : FunctionRegistry()->GetRegisteredFunctions())
    {
        std::cout << function.Key.ToString();
    }
    // TestDelegate.AddUObject(this, &ADodge::test);
    // TestDelegate.AddLambda([this]
    // {
    //     SetActorLocation(GetActorLocation() + FVector(0.1,0,0));
    // });
}

void ADodge::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TestDelegate.Broadcast();
}

void ADodge::Destroyed()
{
    Super::Destroyed();
}

void ADodge::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

bool ADodge::Destroy()
{
    return Super::Destroy();
}

void ADodge::TestTranslate()
{
    SetActorLocation(GetActorLocation() + FVector(0.01,0,0));
}

void ADodge::TestRotate()
{
    SetActorRotation(GetActorRotation() + FRotator(0.01,0,0));
}

void ADodge::PostDuplicate()
{
    Super::PostDuplicate();
    // TODO: PIE world 받아오는 다른 방법 생각해보기 지금은 하드코딩
    // 아직 Duplicate 중이라 GetWorld가 Editor World를 뱉음
    // TestDelegate = TestDelegate.Duplicate(GetWorld()->GetPIEWorld()->GetLevel()->GetDuplicatedObjects());
    TestDelegate = TestDelegate.Duplicate(GetWorld()->GetLevel()->GetDuplicatedObjects());
}

void ADodge::LoadAndConstruct(const TArray<std::unique_ptr<FActorComponentInfo>>& InfoArray)
{
    AActor::LoadAndConstruct(InfoArray);
}

FActorInfo ADodge::GetActorInfo()
{
    return AActor::GetActorInfo();
}
