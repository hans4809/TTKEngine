#pragma once
#include "SceneComponent.h"

class USphereShapeComponent;

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
    USpringArmComponent();
    virtual ~USpringArmComponent() override;

    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent(float DeltaTime) override;
    void DestroyComponent() override;

    void PostDuplicate() override;

private:


public:
#pragma region Getter & Setter
    USceneComponent* GetTargetComponent() const { return TargetComponent; }
    void SetTargetComponent(USceneComponent* InTargetComponent);

    // 1. Offset
    float GetTargetArmLength() const { return TargetArmLength; }
    void SetTargetArmLength(float InTargetArmLength) { ChangeTargetArmLength = InTargetArmLength; }
    FVector GetSocketOffset() const { return SocketOffset; }
    void SetSocketOffset(const FVector& InSocketOffset) { SocketOffset = InSocketOffset; }
    FVector GetTargetOffset() const { return TargetOffset; }
    void SetTargetOffset(const FVector& InTargetOffset) { TargetOffset = InTargetOffset; }

    // 2. Camera Collision
    bool IsUsingCollisionTest() const { return bUseCollisionTest; }
    void SetUseCollisionTest(bool InUseCollisionTest) { bUseCollisionTest = InUseCollisionTest; }
    float GetProbeSize() const { return ProbeSize; }
    void SetProbeSize(float InProbeSize) 
    { 
        ProbeSize = FMath::Clamp<float>(InProbeSize, 0, 50); 
    }

    // 3. Lag
    bool isActiveCameraLag() const { return bActiveCameraLag; }
    void SetActiveCameraLag(bool InActiveCameraLag) { bActiveCameraLag = InActiveCameraLag; }
    float GetCameraLagSpeed() const { return CameraLagSpeed; }
    void SetCameraLagSpeed(float InCameraLagSpeed) { CameraLagSpeed = InCameraLagSpeed; }
    float GetCameraLagMaxDistance() const { return CameraLagMaxDistance; }
    void SetCameraLagMaxDistance(float InCameraLagMaxDistance) 
    { 
        CameraLagMaxDistance = FMath::Clamp<float>(InCameraLagMaxDistance, 0, 100); 
    }

    bool isActiveCameraRotationLag() const { return bActiveCameraRotationLag; }
    void SetActiveCameraRotationLag(bool InActiveCameraRotationLag) { bActiveCameraRotationLag = InActiveCameraRotationLag; }
    float GetCameraRotationLagSpeed() const { return CameraRotationLagSpeed; }
    void SetCameraRotationLagSpeed(float InCameraRotationLagSpeed) { CameraRotationLagSpeed = InCameraRotationLagSpeed; }
    float GetCameraRotationLagMaxAngle() const { return CameraRotationLagMaxAngle; }
    void SetCameraRotationLagMaxAngle(float InCameraRotationLagMaxAngle) 
    { 
        CameraRotationLagMaxAngle = FMath::Clamp<float>(InCameraRotationLagMaxAngle, 0, 100); 
    }

    // 4. Camera Setting
    bool IsUsingPawnControlRotation() const { return bUsePawnControlRotation; }
    void SetUsePawnControlRotation(bool InUsePawnControlRotation) { bUsePawnControlRotation = InUsePawnControlRotation; }
    bool IsInheritingPitch() const { return bInheritPitch; }
    void SetInheritPitch(bool InInheritPitch) { bInheritPitch = InInheritPitch; }
    bool IsInheritingYaw() const { return bInheritYaw; }
    void SetInheritYaw(bool InInheritYaw) { bInheritYaw = InInheritYaw; }
    bool IsInheritingRoll() const { return bInheritRoll; }
    void SetInheritRoll(bool InInheritRoll) { bInheritRoll = InInheritRoll; }
#pragma endregion

private:
    USphereShapeComponent* Probe = nullptr;
    UPROPERTY(EditAnywhere, USceneComponent*, TargetComponent, = nullptr)

    // 1. Offset
    UPROPERTY(EditAnywhere, float, ChangeTargetArmLength, = 0.f)
    UPROPERTY(EditAnywhere, float, TargetArmLength, = 0.f)

    UPROPERTY(EditAnywhere, FVector, SocketOffset, = FVector::ZeroVector)
    UPROPERTY(EditAnywhere, FVector, TargetOffset, = FVector::ZeroVector)

    // 2. Camera Collision
    UPROPERTY(EditAnywhere, bool, bUseCollisionTest, = true)
    UPROPERTY(EditAnywhere, float, ProbeSize, = 0.5f)
    //float ProbeChannel = 0.5f;

    // 3. Lag
    UPROPERTY(EditAnywhere, bool, bActiveCameraLag, = false)
    UPROPERTY(EditAnywhere, float, CameraLagSpeed, = 0.5f)
    UPROPERTY(EditAnywhere, float, CameraLagMaxDistance, = 50.f)

    UPROPERTY(EditAnywhere, bool, bActiveCameraRotationLag, = false)
    UPROPERTY(EditAnywhere, float, CameraRotationLagSpeed, = 0.5f)
    UPROPERTY(EditAnywhere, float, CameraRotationLagMaxAngle, = 20.f)

    // 4. Camera Setting
    UPROPERTY(EditAnywhere, bool, bUsePawnControlRotation, = false)
    UPROPERTY(EditAnywhere, bool, bInheritPitch, = true)
    UPROPERTY(EditAnywhere, bool, bInheritYaw, = true)
    UPROPERTY(EditAnywhere, bool, bInheritRoll, = true)
};

