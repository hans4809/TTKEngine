#pragma once
#include "GameFramework/Actor.h"

class UHeightFogComponent;

class AExponentialHeightFogActor : public AActor
{
    DECLARE_CLASS(AExponentialHeightFogActor, AActor)
public:
    AExponentialHeightFogActor();
    virtual ~AExponentialHeightFogActor() override;
    void BeginPlay() override; 
    void Tick(float DeltaTime) override;
    void Destroyed() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    bool Destroy() override;
    void PostDuplicate() override;
    void LoadAndConstruct(const TArray<std::unique_ptr<FActorComponentInfo>>& InfoArray) override;
    FActorInfo GetActorInfo() override;

protected:
    UHeightFogComponent* FogComponent = nullptr;
};
