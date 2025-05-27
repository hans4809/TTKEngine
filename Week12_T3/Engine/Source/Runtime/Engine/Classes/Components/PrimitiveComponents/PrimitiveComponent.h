#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Serialization/Archive.h"
#include "PhysicsEngine/BodyInstance.h"

struct HitResult;
class UPhysicalMaterial;
struct FPrimitiveComponentInfo : FSceneComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FPrimitiveComponentInfo);
    FVector ComponentVelocity;
    FName VBIBTopologyMappingName;

    FPrimitiveComponentInfo()
        : FSceneComponentInfo()
        , ComponentVelocity(FVector::ZeroVector)
        , VBIBTopologyMappingName(TEXT(""))
    {
        InfoType = TEXT("FPrimitiveComponentInfo");
        ComponentClass = TEXT("UPrimitiveComponent");
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FSceneComponentInfo::Serialize(ar);
        ar << ComponentVelocity << VBIBTopologyMappingName;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FSceneComponentInfo::Deserialize(ar);
        ar >> ComponentVelocity >> VBIBTopologyMappingName;
    }
};

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent();
    virtual ~UPrimitiveComponent() override;
    virtual void BeginPlay() override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void OnRegister() override;
    

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    );
    virtual void PostEditChangeProperty(const FProperty* PropertyThatChanged) override;
    virtual void PostDuplicate() override;

    // Physics - Collision
    bool IsGenerateOverlapEvents() const { return bGenerateOverlapEvents; }
    void SetGenerateOverlapEvents(bool bInGenerateOverlapEvents) { bGenerateOverlapEvents = bInGenerateOverlapEvents; }

    virtual void NotifyHit(HitResult Hit) const;
    virtual void NotifyBeginOverlap(const UPrimitiveComponent* OtherComponent) const;
    virtual void NotifyEndOverlap(const UPrimitiveComponent* OtherComponent) const;

    bool MoveComponent(const FVector& Delta) override;
    UPROPERTY(EditAnywhere, FVector, ComponentVelocity, = FVector::ZeroVector)
    virtual void RecreatePhysicsState();
    virtual void DestroyPhysicsState();
    virtual void OnCreatePhysicsState();
public:

    std::unique_ptr<FActorComponentInfo> GetComponentInfo() override;
    virtual void SaveComponentInfo(FActorComponentInfo& OutInfo) override;
    UPhysicalMaterial* GetPhysicalMaterial() const;
public:
    FName GetVBIBTopologyMappingName() const { return VBIBTopologyMappingName; }
protected:
    UPROPERTY(EditAnywhere, FName, VBIBTopologyMappingName, = TEXT("None"))
    FBodyInstance BodyInstance;
private:
    bool bGenerateOverlapEvents = true;
    UPhysicalMaterial* OverridePhysMaterial = nullptr;
};

