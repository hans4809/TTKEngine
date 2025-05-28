#include "BodySetup.h"
#include "PhysicsEngine/PhysXSDKManager.h"
#include "PhysicsEngine/PhysicsMaterial.h"
UBodySetup::UBodySetup() :DefaultPhysicalMaterial(nullptr)
{
}

UBodySetup::~UBodySetup()
{
}

void UBodySetup::CopyBodyPropertiesFrom(const UBodySetup* FromSetup)
{
    AggGeom = FromSetup->AggGeom;

    // clear pointers copied from other BodySetup, as 
    for (int32 i = 0; i < AggGeom.ConvexElems.Num(); i++)
    {
        FKConvexElem& ConvexElem = AggGeom.ConvexElems[i];
        //ConvexElem.ResetChaosConvexMesh();
    }

    //DefaultInstance.CopyBodyInstancePropertiesFrom(&FromSetup->DefaultInstance);
    //PhysMaterial = FromSetup->PhysMaterial;
    bDoubleSidedGeometry = FromSetup->bDoubleSidedGeometry;
}

void UBodySetup::AddCollisionFrom(UBodySetup* FromSetup)
{
    AddCollisionFrom(FromSetup->AggGeom);
}

void UBodySetup::AddCollisionFrom(const FKAggregateGeom& FromAggGeom)
{
    // Add shapes from static mesh
    AggGeom.SphereElems.Append(FromAggGeom.SphereElems);
    AggGeom.BoxElems.Append(FromAggGeom.BoxElems);
    AggGeom.SphylElems.Append(FromAggGeom.SphylElems);

    // Remember how many convex we already have
    int32 FirstNewConvexIdx = AggGeom.ConvexElems.Num();
    // copy convex
    AggGeom.ConvexElems.Append(FromAggGeom.ConvexElems);
    // clear pointers on convex elements
    for (int32 i = FirstNewConvexIdx; i < AggGeom.ConvexElems.Num(); i++)
    {
        FKConvexElem& ConvexElem = AggGeom.ConvexElems[i];
        //ConvexElem.ResetChaosConvexMesh();
    }

}

template <typename TElem>
bool AddCollisionElemFrom_Helper(const TArray<TElem>& FromElems, const int32 ElemIndex, TArray<TElem>& ToElems)
{
    if (FromElems.IsValidIndex(ElemIndex))
    {
        ToElems.Add(FromElems[ElemIndex]);
        return true;
    }
    return false;
}

bool UBodySetup::AddCollisionElemFrom(const FKAggregateGeom& FromAggGeom, const EAggCollisionShape::Type ShapeType, const int32 ElemIndex)
{
    switch (ShapeType)
    {
    case EAggCollisionShape::Sphere:
        return AddCollisionElemFrom_Helper(FromAggGeom.SphereElems, ElemIndex, AggGeom.SphereElems);
    case EAggCollisionShape::Box:
        return AddCollisionElemFrom_Helper(FromAggGeom.BoxElems, ElemIndex, AggGeom.BoxElems);
    case EAggCollisionShape::Sphyl:
        return AddCollisionElemFrom_Helper(FromAggGeom.SphylElems, ElemIndex, AggGeom.SphylElems);
    case EAggCollisionShape::Convex:
        if (AddCollisionElemFrom_Helper(FromAggGeom.ConvexElems, ElemIndex, AggGeom.ConvexElems))
        {
            //AggGeom.ConvexElems.Last().ResetChaosConvexMesh();
            return true;
        }
        return false;
    }
    return false;
}

void UBodySetup::InvalidatePhysicsData()
{
    for (FKConvexElem& Convex : AggGeom.ConvexElems)
    {
        Convex.ResetCookedPhysXData();
    }

}

void UBodySetup::CreatePhysicsMeshes()
{
    FPhysXSDKManager::GetInstance().Initalize();
    physx::PxCooking* Cooking = FPhysXSDKManager::GetInstance().GetCooking();
    physx::PxPhysics* PhysicsSDK = FPhysXSDKManager::GetInstance().GetPhysicsSDK();

    if (!Cooking || !PhysicsSDK)
    {
        UE_LOG(LogLevel::Error, TEXT("UBodySetup::CreatePhysicsMeshes: Cooking API or Physics SDK is not available."));
        return;
    }

    for (int32 i = 0; i < AggGeom.ConvexElems.Num(); ++i) // 인덱스 기반 루프
    {
        FKConvexElem& ConvexElemInstance = AggGeom.ConvexElems[i]; // 명시적으로 참조 가져오기

        // 이미 쿠킹된 데이터가 있거나, 원본 버텍스 데이터가 없으면 스킵
        if (ConvexElemInstance.CookedPxConvexMesh || ConvexElemInstance.VertexData.IsEmpty())
        {
            continue;
        }

        TArray<physx::PxVec3> PxVerts;
        PxVerts.Reserve(ConvexElemInstance.VertexData.Num());
        for (const FVector& Vert : ConvexElemInstance.VertexData)
        {
            PxVerts.Add(physx::PxVec3(Vert.X, Vert.Y, Vert.Z));
        }

        physx::PxConvexMeshDesc ConvexDesc;
        ConvexDesc.points.count = PxVerts.Num();
        ConvexDesc.points.stride = sizeof(physx::PxVec3);
        ConvexDesc.points.data = PxVerts.GetData();
        ConvexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

        physx::PxDefaultMemoryOutputStream WriteBuffer;
        physx::PxConvexMeshCookingResult::Enum CookResult;
        bool bCookSuccess = Cooking->cookConvexMesh(ConvexDesc, WriteBuffer, &CookResult);

        if (bCookSuccess && CookResult == physx::PxConvexMeshCookingResult::eSUCCESS)
        {
            physx::PxDefaultMemoryInputData ReadBuffer(WriteBuffer.getData(), WriteBuffer.getSize());
            physx::PxConvexMesh* TempCookedMesh = PhysicsSDK->createConvexMesh(ReadBuffer);

            if (TempCookedMesh)
            {
                // 이 할당이 실제 AggGeom.ConvexElems[i]에 영향을 주는지 확인
                ConvexElemInstance.CookedPxConvexMesh = TempCookedMesh;
                UE_LOG(LogLevel::Display, TEXT("Successfully cooked AND created PxConvexMesh for element %d. Pointer: %p"), i, TempCookedMesh);
            }
            else
            {
                UE_LOG(LogLevel::Error, TEXT("Failed to create PxConvexMesh from cooked data for element %d (createConvexMesh returned null)."), i);
                ConvexElemInstance.CookedPxConvexMesh = nullptr;
            }
        }
        else
        {
            UE_LOG(LogLevel::Error, TEXT("Failed to cook convex mesh for element %d. CookResult: %d"), i, static_cast<int>(CookResult)); // CookResult 값도 로그에 추가
        }
    }
}
float UBodySetup::GetVolume(const FVector& Scale) const
{
    return GetScaledVolume(Scale);
}

float UBodySetup::GetScaledVolume(const FVector& Scale) const
{
    return AggGeom.GetScaledVolume(Scale);
}

float UBodySetup::CalculateMass(const UPrimitiveComponent* Component) const
{
    return 1.0f;
}

void UBodySetup::RemoveSimpleCollision()
{
    //InvalidatePhysicsData();
    AggGeom.EmptyElements();
}

void UBodySetup::UpdateTriMeshVertices(const TArray<FVector>& NewPositions)
{



}
template <bool bPositionAndNormal, bool bUseConvexShapes>
float GetClosestPointAndNormalImpl(const UBodySetup* BodySetup, const FVector& WorldPosition, const FTransform& LocalToWorld, FVector* ClosestWorldPosition, FVector* FeatureNormal)
{
    float ClosestDist = FLT_MAX;
    FVector TmpPosition, TmpNormal;
    int32 NumShapeTested = 0;

    //Note that this function is optimized for BodySetup with few elements. This is more common. If we want to optimize the case with many elements we should really return the element during the distance check to avoid pointless iteration
    for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
    {
        NumShapeTested++;
        if constexpr (bPositionAndNormal)
        {
            const float Dist = SphereElem.GetClosestPointAndNormal(WorldPosition, LocalToWorld, TmpPosition, TmpNormal);

            if (Dist < ClosestDist)
            {
                *ClosestWorldPosition = TmpPosition;
                *FeatureNormal = TmpNormal;
                ClosestDist = Dist;
            }
        }
        else
        {
            const float Dist = SphereElem.GetShortestDistanceToPoint(WorldPosition, LocalToWorld);
            ClosestDist = Dist < ClosestDist ? Dist : ClosestDist;
        }
    }

    for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
    {
        NumShapeTested++;
        if constexpr (bPositionAndNormal)
        {
            const float Dist = SphylElem.GetClosestPointAndNormal(WorldPosition, LocalToWorld, TmpPosition, TmpNormal);

            if (Dist < ClosestDist)
            {
                *ClosestWorldPosition = TmpPosition;
                *FeatureNormal = TmpNormal;
                ClosestDist = Dist;
            }
        }
        else
        {
            const float Dist = SphylElem.GetShortestDistanceToPoint(WorldPosition, LocalToWorld);
            ClosestDist = Dist < ClosestDist ? Dist : ClosestDist;
        }
    }

    for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
    {
        NumShapeTested++;
        if constexpr (bPositionAndNormal)
        {
            const float Dist = BoxElem.GetClosestPointAndNormal(WorldPosition, LocalToWorld, TmpPosition, TmpNormal);

            if (Dist < ClosestDist)
            {
                *ClosestWorldPosition = TmpPosition;
                *FeatureNormal = TmpNormal;
                ClosestDist = Dist;
            }
        }
        else
        {
            const float Dist = BoxElem.GetShortestDistanceToPoint(WorldPosition, LocalToWorld);
            ClosestDist = Dist < ClosestDist ? Dist : ClosestDist;
        }
    }

    if constexpr (bUseConvexShapes)
    {
        NumShapeTested++;
        for (const FKConvexElem& ConvexElem : BodySetup->AggGeom.ConvexElems)
        {
            if constexpr (bPositionAndNormal)
            {
                const float Dist = ConvexElem.GetClosestPointAndNormal(WorldPosition, LocalToWorld, TmpPosition, TmpNormal);

                if (Dist < ClosestDist)
                {
                    *ClosestWorldPosition = TmpPosition;
                    *FeatureNormal = TmpNormal;
                    ClosestDist = Dist;
                }
            }
            else
            {
                const float Dist = ConvexElem.GetShortestDistanceToPoint(WorldPosition, LocalToWorld);
                ClosestDist = Dist < ClosestDist ? Dist : ClosestDist;
            }
        }
    }

    if (NumShapeTested > 0 && ClosestDist == FLT_MAX)
    {
        UE_LOG(LogLevel::Warning, TEXT("GetClosestPointAndNormalImpl ClosestDist for BodySetup %s is coming back as FLT_MAX. WorldPosition = %s, LocalToWorld = %s"), *BodySetup->GetName(), *WorldPosition.ToString(), *LocalToWorld.ToString());
    }

    return ClosestDist;
}

float UBodySetup::GetShortestDistanceToPoint(const FVector& WorldPosition, const FTransform& LocalToWorld, const bool bUseConvexShapes) const
{
    if (bUseConvexShapes)
    {
        return GetClosestPointAndNormalImpl<false, true>(this, WorldPosition, LocalToWorld, nullptr, nullptr);
    }
    else
    {
        return GetClosestPointAndNormalImpl<false, false>(this, WorldPosition, LocalToWorld, nullptr, nullptr);
    }
}

float UBodySetup::GetClosestPointAndNormal(const FVector& WorldPosition, const FTransform& LocalToWorld, FVector& ClosestWorldPosition,
    FVector& FeatureNormal, const bool bUseConvexShapes) const
{
    if (bUseConvexShapes)
    {
        return GetClosestPointAndNormalImpl<true, true>(this, WorldPosition, LocalToWorld, &ClosestWorldPosition, &FeatureNormal);
    }
    else
    {
        return GetClosestPointAndNormalImpl<true, false>(this, WorldPosition, LocalToWorld, &ClosestWorldPosition, &FeatureNormal);
    }
}

void UBodySetup::CopyBodySetupProperty(const UBodySetup* Other)
{
    BoneName = Other->BoneName;
    //PhysicsType = Other->PhysicsType;
    bConsiderForBounds = Other->bConsiderForBounds;
    bMeshCollideAll = Other->bMeshCollideAll;
    bDoubleSidedGeometry = Other->bDoubleSidedGeometry;
    bGenerateNonMirroredCollision = Other->bGenerateNonMirroredCollision;
    bSharedCookedData = Other->bSharedCookedData;
    bGenerateMirroredCollision = Other->bGenerateMirroredCollision;
    //PhysMaterial = Other->PhysMaterial;
    CollisionReponse = Other->CollisionReponse;
    //CollisionTraceFlag = Other->CollisionTraceFlag;
    //DefaultInstance = Other->DefaultInstance
    BuildScale3D = Other->BuildScale3D;
}

UPhysicalMaterial* UBodySetup::GetDefaultMaterial()
{
    if (DefaultPhysicalMaterial == nullptr)
    {
        DefaultPhysicalMaterial = FObjectFactory::ConstructObject<UPhysicalMaterial>(nullptr);
        DefaultPhysicalMaterial->Initialize(FPhysXSDKManager::GetInstance().GetPhysicsSDK(), 0.5, 0.5, 1.f);
    }
    return DefaultPhysicalMaterial;
}


