#pragma once
#include "Define.h"
#include "Engine/Asset/Asset.h"

class UMaterial : public UAsset
{
    DECLARE_CLASS(UMaterial, UAsset)

public:
    UMaterial() {}
    UMaterial(FObjMaterialInfo InMaterialInfo) : MaterialInfo(std::move(InMaterialInfo)) {}
    ~UMaterial() {}
    FObjMaterialInfo& GetMaterialInfo() { return MaterialInfo; }
    void SetMaterialInfo(FObjMaterialInfo value) { MaterialInfo = value; }

    // 색상 및 재질 속성 설정자
    void SetDiffuse(const FVector& DiffuseIn) { MaterialInfo.Diffuse = DiffuseIn; }
    void SetSpecular(const FVector& SpecularIn) { MaterialInfo.Specular = SpecularIn; }
    void SetAmbient(const FVector& AmbientIn) { MaterialInfo.Ambient = AmbientIn; }
    void SetEmissive(const FVector& EmissiveIn) { MaterialInfo.Emissive = EmissiveIn; }

    // 스칼라 속성 설정자
    void SetSpecularPower(float SpecularPowerIn) { MaterialInfo.SpecularScalar = SpecularPowerIn; }
    void SetOpticalDensity(float DensityIn) { MaterialInfo.DensityScalar = DensityIn; }
    void SetTransparency(float TransparencyIn) {
        MaterialInfo.TransparencyScalar = TransparencyIn;
        MaterialInfo.bTransparent = (TransparencyIn < 1.0f);
    }

    virtual void Serialize(FArchive& Ar) const
    {
        Ar << MaterialInfo;
    }

    virtual void Deserialize(FArchive& Ar)
    {
        Ar >> MaterialInfo;
    }

    bool LoadFromFile(const FString& filepath) override;
    bool SerializeToFile(std::ostream& Out) override;
    bool DeserializeFromFile(std::istream& In) override;
    void PostLoad() override;

private:
    UPROPERTY(EditAnywhere, FObjMaterialInfo, MaterialInfo, = {})
};
