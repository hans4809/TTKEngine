#pragma once
#include "Engine/StaticMeshActor.h"

class APhysicsRoadActor : public AStaticMeshActor
{
    DECLARE_CLASS(APhysicsRoadActor, AStaticMeshActor)
public:
    APhysicsRoadActor();
    ~APhysicsRoadActor() override;

    void Init();

    physx::PxRigidStatic* gGroundPlane = nullptr;
};
