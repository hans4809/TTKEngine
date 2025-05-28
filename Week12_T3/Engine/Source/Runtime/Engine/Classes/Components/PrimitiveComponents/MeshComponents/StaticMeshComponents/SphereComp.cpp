#include "SphereComp.h"

#include "Engine/FLoaderOBJ.h"
#include "Engine/Asset/AssetManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/Classes/Components/Mesh/StaticMesh.h"


USphereComp::USphereComp()
{
    AABB.Max = {1, 1, 1};
    AABB.Min = {-1, -1, -1};
}


USphereComp::~USphereComp()
{
}

void USphereComp::InitializeComponent()
{
    Super::InitializeComponent();
    //FManagerOBJ::CreateStaticMesh("Assets/Sphere.obj");
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(TEXT("Sphere"));

    SetStaticMesh(StaticMesh);
}

void USphereComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}