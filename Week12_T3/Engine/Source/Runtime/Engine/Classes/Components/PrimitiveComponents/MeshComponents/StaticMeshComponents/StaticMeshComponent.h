#pragma once
#include "Components/PrimitiveComponents/MeshComponents/MeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"

enum class EPhysBodyShapeType
{
    Box,
    Sphere,
    Sphyl,
    Convex
};

class UStaticMesh;

struct FStaticMeshComponentInfo : public FPrimitiveComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FStaticMeshComponentInfo);
    FString StaticMeshPath;

    FStaticMeshComponentInfo()
        : FPrimitiveComponentInfo()
        , StaticMeshPath("")
    {
        InfoType = TEXT("FStaticMeshComponentInfo");
        ComponentClass = TEXT("UStaticMeshComponent");
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FPrimitiveComponentInfo::Serialize(ar);
        ar << StaticMeshPath;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FPrimitiveComponentInfo::Deserialize(ar);
        ar >> StaticMeshPath;
    }
};

class UStaticMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)

public:
    UStaticMeshComponent() = default;
    
    virtual void PostDuplicate() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual uint32 GetNumMaterials() const override;
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    virtual TArray<FName> GetMaterialSlotNames() const override;
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    
    UStaticMesh* GetStaticMesh() const { return staticMesh; }
    void SetStaticMesh(UStaticMesh* value);

    virtual void DestroyPhysicsState() override;
    virtual void OnCreatePhysicsState() override;

    std::unique_ptr<FActorComponentInfo> GetComponentInfo() override;
    virtual void SaveComponentInfo(FActorComponentInfo& OutInfo) override;
    virtual void LoadAndConstruct(const FActorComponentInfo& Info) override;
    
    UPROPERTY(EditAnywhere, EPhysBodyType, BodyType, = EPhysBodyType::Dynamic)
    UPROPERTY(EditAnywhere, EPhysBodyShapeType, ShapeType, = EPhysBodyShapeType::Box)
   
    int SelectedSubMeshIndex = 0;

protected:
    UPROPERTY(EditAnywhere | DuplicateTransient, UStaticMesh*, staticMesh, = nullptr;)
private:
    float Timer = 0.0f;
};