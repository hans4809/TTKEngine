#pragma once
#include "SphereComp.h"

class UFireBallComponent: public USphereComp
{
    DECLARE_CLASS(UFireBallComponent, USphereComp)
public:
    UFireBallComponent() = default;
    virtual ~UFireBallComponent() override;
    void InitializeComponent() override;
    void TickComponent(float DeltaTime) override;

    virtual void PostDuplicate() override;
};
