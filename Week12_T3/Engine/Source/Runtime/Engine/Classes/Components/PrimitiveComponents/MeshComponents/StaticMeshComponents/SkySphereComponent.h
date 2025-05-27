#pragma once
#include "StaticMeshComponent.h"
#include "Engine/Texture.h"


class USkySphereComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(USkySphereComponent, UStaticMeshComponent)

public:
    USkySphereComponent();
    virtual ~USkySphereComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    UPROPERTY(EditAnywhere, float, UOffset, = 0)
    UPROPERTY(EditAnywhere, float, VOffset, = 0)
};
