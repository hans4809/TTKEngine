#pragma once
#include "BodySetupCore.h"

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
public:
    UBodySetup();
    ~UBodySetup() override;
};
