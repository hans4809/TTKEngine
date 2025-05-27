#pragma once
#include "ShapeElem.h"
#include "Define.h"
#include "Math/BoundingBox.h"
#include "Math/Transform.h"

#include <PxPhysicsAPI.h>

struct FKBoxElem;

struct FKConvexElem : public FKShapeElem
{
    DECLARE_STRUCT(FKConvexElem, FKShapeElem)

    /** Array of indices that make up the convex hull. */
    UPROPERTY(EditAnywhere, TArray<FVector>, VertexData, = {})

    UPROPERTY(EditAnywhere, TArray<int32>, IndexData, = {})

    UPROPERTY(EditAnywhere, FBoundingBox, ElemBox, = {})
private:
    UPROPERTY(EditAnywhere, FTransform, Transform, = FTransform::Identity)

    //Chaos::FConvexPtr ChaosConvex;

public:
    enum class EConvexDataUpdateMethod
    {
        /** Update convex index and vertex data from the chaos convex object only if they are missing */
        UpdateConvexDataOnlyIfMissing = 0,

        /** Always convex recompute index and vertex data from the set chaos convex object */
        AlwaysUpdateConvexData
    };
    physx::PxConvexMesh* CookedPxConvexMesh = nullptr;

    FKConvexElem();
    FKConvexElem(const FKConvexElem& Other);
    ~FKConvexElem() override;

    const FKConvexElem& operator=(const FKConvexElem& Other);

    void AddCachedSolidConvexGeom(TArray<FVertexSimple>& VertexBuffer, TArray<uint32>& IndexBuffer, const FColor VertexColor) const;

    /** Reset the hull to empty all arrays */
    void Reset();

    /** Updates internal ElemBox based on current value of VertexData */
    void UpdateElemBox();

    /** Calculate a bounding box for this convex element with the specified transform and scale */
    FBoundingBox CalcAABB(const FTransform& BoneTM, const FVector& Scale3D) const;

    /** Get set of planes that define this convex hull */
    void GetPlanes(TArray<FPlane>& Planes) const;

    /** Utility for creating a convex hull from a set of planes. Will reset current state of this elem. */
    bool HullFromPlanes(const TArray<FPlane>& InPlanes, const TArray<FVector>& SnapVerts, float InSnapDistance = SMALL_NUMBER);

    /** Utility for setting this convex element to match a supplied box element. Also copies transform. */
    void ConvexFromBoxElem(const FKBoxElem& InBox);

    /** Returns the volume of this element */
    float GetVolume(const FVector& Scale) const;

    /** Returns the volume of this element */
    float GetScaledVolume(const FVector& Scale3D) const;

    /** Get current transform applied to convex mesh vertices */
    FTransform GetTransform() const
    {
        return Transform;
    };

    /** 
     * Modify the transform to apply to convex mesh vertices 
     * NOTE: When doing this, BodySetup convex meshes need to be recooked - usually by calling InvalidatePhysicsData() and CreatePhysicsMeshes()
     */
    void SetTransform(const FTransform& InTransform)
    {
        Transform = InTransform;
    }

    void ScaleElem(FVector DeltaSize, float MinSize);
    
    /**
     * Finds the closest point on the shape given a world position. Input and output are given in world space
     * @param	WorldPosition			The point we are trying to get close to
     * @param	BodyToWorldTM			The transform to convert BodySetup into world space
     * @param	ClosestWorldPosition	The closest point on the shape in world space
     * @param	Normal					The normal of the feature associated with ClosestWorldPosition.
     * @return							The distance between WorldPosition and the shape. 0 indicates WorldPosition is inside the shape.
     */
    float GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& BodyToWorldTM, FVector& ClosestWorldPosition, FVector& Normal) const;

    /**	
     * Finds the shortest distance between the element and a world position. Input and output are given in world space
     * @param	WorldPosition	The point we are trying to get close to
     * @param	BodyToWorldTM	The transform to convert BodySetup into world space
     * @return					The distance between WorldPosition and the shape. 0 indicates WorldPosition is inside one of the shapes.
     */
    float GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& BodyToWorldTM) const;
	
    void ResetCookedPhysXData(); 
    
    static EAggCollisionShape::Type StaticShapeType;

private:
    /** Helper function to safely copy instances of this shape*/
    void CloneElem(const FKConvexElem& Other);
};
