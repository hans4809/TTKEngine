#pragma once
#include "BodySetupEnums.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
public:
    UBodySetupCore();
    ~UBodySetupCore() override; 

    UPROPERTY(EditAnywhere, FName, BoneName, = TEXT("None"))

    UPROPERTY(EditAnywhere, EBodyCollisionResponse::Type, CollisionReponse, = EBodyCollisionResponse::Type::BodyCollision_Enabled)
};
