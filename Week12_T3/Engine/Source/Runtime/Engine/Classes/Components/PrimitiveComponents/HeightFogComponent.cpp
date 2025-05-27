#include "HeightFogComponent.h"

UHeightFogComponent::UHeightFogComponent()
    : Super()
    , FogColor(FVector(0.7f, 0.2f, 0.2f))
    , FogDensity(0.7f)
    , FogStart(30.0f)
    , FogEnd(100.0f)
    , FogBaseHeight(5.0f)
    , HeightFallOff(0.02f)
    , bIsHeightFog(true)
    , MaxOpacity(1.0f)
    , LightShaftDensity(1.0f)
{
}

void UHeightFogComponent::SetFogColor(const FVector& InColor) { FogColor = InColor; }
void UHeightFogComponent::SetFogDensity(float InDensity) { FogDensity = InDensity; }
void UHeightFogComponent::SetFogStart(float InStart) { FogStart = InStart; }
void UHeightFogComponent::SetFogEnd(float InEnd) { FogEnd = InEnd; }
void UHeightFogComponent::SetFogBaseHeight(float InHeight) { FogBaseHeight = InHeight; }
void UHeightFogComponent::SetHeightFallOff(float InFalloff) { HeightFallOff = InFalloff; }
void UHeightFogComponent::SetHeightFog(bool bEnabled) { bIsHeightFog = bEnabled; }
void UHeightFogComponent::SetMaxOpacity(float InMaxOpacity) { MaxOpacity = InMaxOpacity; }
void UHeightFogComponent::SetLightShaftDensity(float InDensity) { LightShaftDensity = InDensity; }

