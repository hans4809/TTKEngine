#pragma once
#include "GameFramework/Actor.h"

struct PostProcessSettings
{
    DECLARE_STRUCT(PostProcessSettings);

    // Depth of field
    UPROPERTY(EPropertyFlags::EditAnywhere, float, FocusDistance, = 1000.0f)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, SensorWidth, = 36.0f)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, Aperture, = 1.2f)
    UPROPERTY(EPropertyFlags::EditAnywhere, float, MaxCoCRadius, = 20.0f)
    UPROPERTY(EPropertyFlags::EditAnywhere, uint32, SampleCount, = 12)
};


class APostProcessVolume : public AActor
{
    DECLARE_CLASS(APostProcessVolume, AActor)
    
public:
    APostProcessVolume();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EPropertyFlags::EditAnywhere, bool, bEnabled, = true)
    UPROPERTY(EPropertyFlags::EditAnywhere, bool, bUnbound, = true)
    
    UPROPERTY(EPropertyFlags::EditAnywhere, PostProcessSettings, PostProcess, = {})
};
