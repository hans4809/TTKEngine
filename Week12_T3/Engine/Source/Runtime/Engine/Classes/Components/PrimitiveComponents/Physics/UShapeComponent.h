#pragma once
#include "Components/PrimitiveComponents/PrimitiveComponent.h"
#include "Math/Color.h"

enum class EShapeType
{
    Box, Sphere, Capsule, None
};

struct FShapeInfo
{
    DECLARE_STRUCT(FShapeInfo)
    
    FShapeInfo() : Type(EShapeType::None), Center(FVector::ZeroVector), WorldMatrix(FMatrix::Identity) {}
    FShapeInfo(EShapeType InType, const FVector& InCenter, const FMatrix& InWorldMatrix)
        : Type(InType), Center(InCenter), WorldMatrix(InWorldMatrix) {}

    virtual ~FShapeInfo() = default;

    UPROPERTY(EditAnywhere, EShapeType, Type, = EShapeType::None)
    UPROPERTY(EditAnywhere, FVector, Center, = FVector::ZeroVector)
    UPROPERTY(EditAnywhere, FMatrix, WorldMatrix, = FMatrix::Identity)
};

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent);

public:
    UShapeComponent();
    ~UShapeComponent() override;

    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent(float DeltaTime) override;
    void DestroyComponent() override;
    virtual void PostDuplicate() override;

    // Getter, Setter
    FLinearColor GetShapeColor() const { return ShapeColor; }
    void SetShapeColor(const FLinearColor& InColor) { ShapeColor = InColor; }
    FBoundingBox GetBroadAABB() const { return BroadAABB; }
    bool IsDrawOnlyIfSelected() const { return bDrawOnlyIfSelected; }
    void SetbDrawOnlyIfSelected(bool bInDraw) { bDrawOnlyIfSelected = bInDraw; }

    FVector GetPrevLocation() const { return PrevLocation; }
    FRotator GetPrevRotation() const { return PrevRotation; }
    FVector GetPrevScale() const { return PrevScale; }

    virtual const FShapeInfo* GetShapeInfo() const { return &ShapeInfo; }

    virtual bool TestOverlaps(const UShapeComponent* OtherShape) const;
    virtual bool BroadPhaseCollisionCheck(const UShapeComponent* OtherShape) const;
    virtual bool NarrowPhaseCollisionCheck(const UShapeComponent* OtherShape) const { return false; }

protected:
    virtual void UpdateBroadAABB() {}

protected:
    mutable FShapeInfo ShapeInfo;

    FBoundingBox BroadAABB;
    
    FVector PrevLocation;
    FRotator PrevRotation;
    FVector PrevScale;

private:
    UPROPERTY(EditAnywhere, FLinearColor, ShapeColor, = FLinearColor::Green)
    UPROPERTY(EditAnywhere, bool, bDrawOnlyIfSelected, = true)
};

