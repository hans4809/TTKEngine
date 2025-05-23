#pragma once

#include "ParticleModuleTypeDataBase.h"
#include "Math/RandomStream.h"
#include "Components/Mesh/StaticMesh.h" // UPROPERTY 전방선언 못함

class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
    DECLARE_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)
public:
    UPROPERTY(EditAnywhere, FString, MeshPath, = "Assets/apple_mid.obj")


    UPROPERTY(
        EditAnywhere,
        UStaticMesh*,
        Mesh,
        = nullptr
    )

public:
    UParticleModuleTypeDataMesh() = default;

    virtual FParticleEmitterInstance* CreateInstance(
        UParticleEmitter* InEmitterParent,
        UParticleSystemComponent* InComponent) override;

    virtual bool IsAMeshEmitter() const override { return true; }

    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, float Interp, FBaseParticle* Particle) override;

    virtual EModuleType GetType() const override;

    FRandomStream RandomStream;
};