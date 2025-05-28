#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "Engine/World.h"

#include "UnrealEd/EditorViewportClient.h"

#include "Engine/FLoaderOBJ.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"

UCubeComp::UCubeComp()
{
    AABB.Max = { 1,1,1 };
    AABB.Min = { -1,-1,-1 };

}

UCubeComp::~UCubeComp()
{
}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();

    //FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(TEXT("helloBlender"));

    SetStaticMesh(StaticMesh);
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}