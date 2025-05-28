#pragma once
#include "Define.h"
#include "CoreUObject/UObject/Object.h"
#include "CoreUObject/UObject/ObjectMacros.h"

class UMaterial : public UObject
{
    DECLARE_CLASS(UMaterial, UObject)

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

private:
    UPROPERTY(EditAnywhere, FObjMaterialInfo, MaterialInfo, = {})
};
