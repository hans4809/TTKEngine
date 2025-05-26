#include "SphereComp.h"

#include "Engine/FLoaderOBJ.h"
#include "UnrealEd/EditorViewportClient.h"


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
    FManagerOBJ::CreateStaticMesh("Assets/Sphere.obj");
    SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Sphere.obj"));
}

void USphereComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}