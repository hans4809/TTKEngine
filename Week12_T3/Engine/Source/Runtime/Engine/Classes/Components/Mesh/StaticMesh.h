#pragma once
#include "Engine/Asset/Asset.h"
#include "Define.h"
#include "Components/Material/Material.h"

class UBodySetup;

class UStaticMesh : public UAsset
{
    DECLARE_CLASS(UStaticMesh, UAsset)

public:
    UStaticMesh();
    virtual ~UStaticMesh() override;
    const TArray<FMaterialSlot>& GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& Out) const;
    FStaticMeshRenderData GetRenderData() const { return staticMeshRenderData; }

    void SetData(FStaticMeshRenderData renderData);
    void SetBodySetup(UBodySetup* bodySetup);
    UBodySetup* GetBodySetup();
    
    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;
    void PostLoad() override;

private:
    UPROPERTY(EditAnywhere, FStaticMeshRenderData, staticMeshRenderData, = {})
    UPROPERTY(EditAnywhere, TArray<FMaterialSlot>, materials, {})
    UPROPERTY(EditAnywhere, UBodySetup*, BodySetup, = nullptr)
};
