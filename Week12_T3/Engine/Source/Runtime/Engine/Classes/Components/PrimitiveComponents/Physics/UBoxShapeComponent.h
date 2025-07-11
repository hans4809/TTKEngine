#pragma once
#include "UShapeComponent.h"

class USphereShapeComponent;
class UCapsuleShapeComponent;

struct FBoxShapeInfo : public FShapeInfo
{
    DECLARE_STRUCT(FBoxShapeInfo, FShapeInfo)
    FBoxShapeInfo()
        : FShapeInfo(EShapeType::Box, FVector::ZeroVector, FMatrix::Identity)
        , Extent(FVector::ZeroVector)
        , RotationMatrix(FMatrix::Identity) {
    }

    FBoxShapeInfo(const FVector& InCenter, const FMatrix& InWorldMatrix, const FVector& InExtent)
        : FShapeInfo(EShapeType::Box, InCenter, InWorldMatrix), Extent(InExtent) {
    }

    UPROPERTY(EditAnywhere, FVector, Extent, = FVector::ZeroVector)
    UPROPERTY(EditAnywhere, FMatrix, RotationMatrix, = FMatrix::Identity)
};

class UBoxShapeComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxShapeComponent, UShapeComponent);

public:
    UBoxShapeComponent();
    virtual ~UBoxShapeComponent() override;

    void InitializeComponent() override;
    void TickComponent(float DeltaTime) override;

    virtual void PostDuplicate() override;

    void SetBoxExtent(const FVector& InExtent) { BoxExtent = InExtent; }
    FVector GetBoxExtent() const { return BoxExtent; }

    virtual const FShapeInfo* GetShapeInfo() const override;
    virtual void UpdateBroadAABB() override;

    virtual bool TestOverlaps(const UShapeComponent* OtherShape) const override;
    virtual bool NarrowPhaseCollisionCheck(const UShapeComponent* OtherShape) const override;

private:
    bool CollisionCheckWithBox(const UBoxShapeComponent* OtherBox) const;
    bool CollisionCheckWithSphere(const USphereShapeComponent* OtherSphere) const;
    bool CollisionCheckWithCapsule(const UCapsuleShapeComponent* OtherCapsule) const;

protected:
    mutable FBoxShapeInfo ShapeInfo;

private:
    FVector PrevExtent;

    UPROPERTY(EditAnywhere, FVector, BoxExtent, = FVector::OneVector * 0.5f)
};

