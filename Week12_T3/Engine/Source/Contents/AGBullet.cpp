#include "AGBullet.h"
#include "Components/PrimitiveComponents/MeshComponents/StaticMeshComponents/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponents/Physics/USphereShapeComponent.h"
#include "Components/PrimitiveComponents/Physics/UBoxShapeComponent.h"
#include "Contents/AGPlayer.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"

AGBullet::AGBullet()
    : Super()
    , Velocity(FVector::ZeroVector)
    , Acceleration(FVector(0, 0, -20))
    , DragCoefficient(0.02f)
{
    UStaticMeshComponent* MeshComp = GetStaticMeshComponent();
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(TEXT("apple_mid"));
    //FManagerOBJ::CreateStaticMesh("Assets/apple_mid.obj");
    MeshComp->SetStaticMesh(StaticMesh);
    AddComponent<USphereShapeComponent>(EComponentOrigin::Constructor);

}

AGBullet::~AGBullet()
{}


void AGBullet::BeginPlay()
{
    Super::BeginPlay();

    LifeElapsed = 0.0f;

    AddBeginOverlapUObject(this, &AGBullet::OnHit);
}

void AGBullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bFired)
    {
        return;
    }
    
    float Dt = DeltaTime * 0.001f;
    LifeElapsed += Dt;
    if (LifeElapsed >= LifeTime)
    {
        ReturnToPool();
        return;
    }

    Velocity += Acceleration * Dt;

    if (!Velocity.IsNearlyZero())
    {
        FVector Drag = Velocity * -DragCoefficient * Velocity.Magnitude();
        float DragZ = Velocity.Z * -0.5f * FMath::Abs(Velocity.Z);
        Drag.Z += DragZ * Dt;
        Velocity += Drag * Dt;
    }

    Position = GetActorLocation() + Velocity * Dt;
    SetActorLocation(Position);
}

void AGBullet::Destroyed()
{
    Super::Destroyed();
}

void AGBullet::Fire(FVector FirePosition, FVector FireDirection, float Power)
{
    LifeElapsed = 0.0f;

    Position = FirePosition;
    SetActorLocation(Position);
    FVector V = FireDirection.Normalize();
    V *= 10;
    V.Z += 5;
    Velocity = V.Normalize() * Power;

    bFired = true;
}

void AGBullet::ReturnToPool()
{
    Destroy();
}

void AGBullet::OnHit(const UPrimitiveComponent* Other)
{
    if (Other->GetOwner()->IsA<AGPlayer>()) return;
    if (Other->IsA<UBoxShapeComponent>())
    {
        if (Other->GetAttachParent()->GetRelativeScale().Z > 20)
            Velocity *= -1;
        else
            Velocity.Z *= -1;
    }
    else
        ReturnToPool();
}

void AGBullet::PostDuplicate()
{
    Super::PostDuplicate();
}
