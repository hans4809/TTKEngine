#pragma once
#include "Animation/CustomAnimInstance/TestAnimInstance.h"
#include "GameFramework/Actor.h"

class USkeletalMeshComponent;

class ASkeletalMeshActor : public AActor
{
    DECLARE_CLASS(ASkeletalMeshActor, AActor)
    
public:
    ASkeletalMeshActor();

    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComp; }

private:
    UPROPERTY(EditAnywhere, USkeletalMeshComponent*, SkeletalMeshComp, = nullptr)
};
