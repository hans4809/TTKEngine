#pragma once
#include "PrimitiveComponent.h"
#include "Classes/Engine/Texture.h"

class UTexture;

struct FBillboardComponentInfo : public FPrimitiveComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FBillboardComponentInfo);
    FString TexturePath;
    bool bOnlyForEditor;

    FBillboardComponentInfo()
        : FPrimitiveComponentInfo()
        , TexturePath("")
    {
        InfoType = TEXT("FBillboardComponentInfo");
        ComponentClass = TEXT("UBillboardComponent");
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FPrimitiveComponentInfo::Serialize(ar);
        ar << TexturePath << bOnlyForEditor;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FPrimitiveComponentInfo::Deserialize(ar);
        ar >> TexturePath >> bOnlyForEditor;
    }
};

class UBillboardComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UBillboardComponent, UPrimitiveComponent)
    
public:
    UBillboardComponent();
    virtual ~UBillboardComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;

    void SetTexture(FString _fileName);
    //void SetUUIDParent(USceneComponent* _parent);
    FMatrix CreateBillboardMatrix();
    virtual void PostDuplicate() override;

    UPROPERTY(EditAnywhere, float, finalIndexU, = 0.0f)
    UPROPERTY(EditAnywhere, float, finalIndexV, = 0.0f)

    UPROPERTY(EditAnywhere | DuplicateTransient, UTexture*, Texture, = nullptr)

    UPROPERTY(EditAnywhere, bool, bOnlyForEditor, = true)

public:
    std::unique_ptr<FActorComponentInfo> GetComponentInfo() override;
    virtual void SaveComponentInfo(FActorComponentInfo& OutInfo) override;
    void LoadAndConstruct(const FActorComponentInfo& Info) override;

protected:
    //USceneComponent* m_parent = nullptr;

    bool CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance);

private:
    void CreateQuadTextureVertexBuffer();
};