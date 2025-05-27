#pragma once
#include "PrimitiveComponent.h"

class UHeightFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
public:
    UHeightFogComponent();

    FVector GetFogColor() const { return FogColor; }
    float GetFogDensity() const { return FogDensity; }
    float GetFogStart() const { return FogStart; }
    float GetFogEnd() const { return FogEnd; }
    float GetFogZPosition() const { return GetWorldLocation().Z; }
    float GetFogBaseHeight() const { return FogBaseHeight; }
    float GetHeightFallOff() const { return HeightFallOff; }
    bool IsHeightFog() const { return bIsHeightFog; }
    float GetMaxOpacity() const { return MaxOpacity; }
    float GetLightShaftDensity() const { return LightShaftDensity; }

    void SetFogColor(const FVector& InColor);
    void SetFogDensity(float InDensity);
    void SetFogStart(float InStart);
    void SetFogEnd(float InEnd);
    void SetFogBaseHeight(float InHeight);
    void SetHeightFallOff(float InFalloff);
    void SetHeightFog(bool bEnabled);
    void SetMaxOpacity(float InMaxOpacity);
    void SetLightShaftDensity(float InDensity);

private:
    UPROPERTY(EditAnywhere, FVector, FogColor, = FVector(0.7f, 0.2f, 0.2f))
    UPROPERTY(EditAnywhere, float, FogDensity, = 0.7f)
    UPROPERTY(EditAnywhere, float, FogStart, = 30.0f)
    UPROPERTY(EditAnywhere, float, FogEnd, = 100.0f)
    UPROPERTY(EditAnywhere, float, FogBaseHeight, = 5.0f)
    UPROPERTY(EditAnywhere, float, HeightFallOff, = 0.02f)
    UPROPERTY(EditAnywhere, bool, bIsHeightFog, = true)
    UPROPERTY(EditAnywhere, float, MaxOpacity, = 1.0f)
    UPROPERTY(EditAnywhere, float, LightShaftDensity, = 1.0f)
};


