#include "PhysicsAsset.h"

#include "Animation/Skeleton.h"
#include "BodySetup/BodySetup.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"

int32 UPhysicsAsset::FindBodyIndex(const FName BodyName) const
{
    const int32* IdxData = BodySetupIndexMap.Find(BodyName);
    if (IdxData)
    {
        return *IdxData;
    }

    return INDEX_NONE;
}

void UPhysicsAsset::BodyFindConstraints(int32 BodyIndex, TArray<int32>& Constraints)
{
    Constraints.Empty();
    FName BodyName = BodySetups[BodyIndex]->BoneName;

    // for(int32 ConIdx=0; ConIdx < ConstraintSetup.Num(); ConIdx++)
    // {
    //     if( ConstraintSetup[ConIdx]->DefaultInstance.ConstraintBone1 == BodyName || ConstraintSetup[ConIdx]->DefaultInstance.ConstraintBone2 == BodyName )
    //     {
    //         Constraints.Add(ConIdx);
    //     }
    // }
}

void UPhysicsAsset::SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty)
{
    PreviewSkeletalMesh = PreviewMesh;
}

USkeletalMesh* UPhysicsAsset::GetPreviewMesh() const
{
    return PreviewSkeletalMesh;
}

UPhysicsAsset::UPhysicsAsset()
{
}

UPhysicsAsset::~UPhysicsAsset()
{
}

int32 UPhysicsAsset::FindControllingBodyIndex(const class USkeletalMesh* skelMesh, int32 StartBoneIndex) const
{
    int32 BoneIndex = StartBoneIndex;
    while(BoneIndex != INDEX_NONE)
    {
        FName BoneName = skelMesh->GetSkeleton()->GetRefSkeletal().GetBoneName(BoneIndex);
        int32 BodyIndex = FindBodyIndex(BoneName);
    
        if(BodyIndex != INDEX_NONE)
            return BodyIndex;
    
        int32 ParentBoneIndex = skelMesh->GetSkeleton()->GetRefSkeletal().GetParentIndex(BoneIndex);
    
        if(ParentBoneIndex == BoneIndex)
            return INDEX_NONE;
    
        BoneIndex = ParentBoneIndex;
    }

    return INDEX_NONE; // Shouldn't reach here.
}

int32 UPhysicsAsset::FindParentBodyIndex(const class USkeletalMesh* skelMesh, int32 StartBoneIndex) const
{
    if (skelMesh)
    {
        return FindParentBodyIndex(skelMesh->GetSkeleton()->GetRefSkeletal(), StartBoneIndex);
    }

    return INDEX_NONE;
}

int32 UPhysicsAsset::FindParentBodyIndex(FRefSkeletal& RefSkeleton, const int32 StartBoneIndex) const
{
    int32 BoneIndex = StartBoneIndex;
    while ((BoneIndex = RefSkeleton.GetParentIndex(BoneIndex)) != INDEX_NONE)
    {
        const FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
        const int32 BodyIndex = FindBodyIndex(BoneName);
    
        if (StartBoneIndex == BoneIndex)
            return INDEX_NONE;
    
        if (BodyIndex != INDEX_NONE)
            return BodyIndex;
    }
    
    return INDEX_NONE;
}

int32 UPhysicsAsset::FindConstraintIndex(FName ConstraintName) const
{
    // for(int32 i=0; i < ConstraintSetup.Num(); i++)
    // {
    //     if( ConstraintSetup[i]->DefaultInstance.JointName == ConstraintName )
    //     {
    //         return i;
    //     }
    // }
    //
    return INDEX_NONE;
}

int32 UPhysicsAsset::FindConstraintIndex(FName Bone1Name, FName Bone2Name) const
{
    // for (int32 i = 0; i < ConstraintSetup.Num(); i++)
    // {
    //     if (ConstraintSetup[i]->DefaultInstance.ConstraintBone1 == Bone1Name &&
    //         ConstraintSetup[i]->DefaultInstance.ConstraintBone2 == Bone2Name)
    //     {
    //         return i;
    //     }
    // }
    //
    return INDEX_NONE;
}

FName UPhysicsAsset::FindConstraintBoneName(int32 ConstraintIndex) const
{
    // if ( (ConstraintIndex < 0) || (ConstraintIndex >= ConstraintSetup.Num()) )
    // {
    //     return NAME_None;
    // }
    //
    // return ConstraintSetup[ConstraintIndex]->DefaultInstance.GetChildBoneName();
    return NAME_None;
}

void UPhysicsAsset::GetBodyIndicesBelow(TArray<int32>& OutBodyIndices, FName InBoneName, const USkeletalMesh* InSkelMesh, bool bIncludeParent) const
{
    if (InSkelMesh)
    {
        GetBodyIndicesBelow(OutBodyIndices, InBoneName, InSkelMesh->GetSkeleton()->GetRefSkeletal(), bIncludeParent);
    }
}

void UPhysicsAsset::GetBodyIndicesBelow(TArray<int32>& OutBodyIndices, const FName InBoneName, FRefSkeletal& RefSkeleton,
    const bool bIncludeParent) const
{
    const int32 BaseIndex = RefSkeleton.FindBoneIndex(InBoneName);
    
    if (BaseIndex != INDEX_NONE)
    {
        // Iterate over all other bodies, looking for 'children' of this one
        for (int32 i = 0; i < BodySetups.Num(); i++)
        {
            const UBodySetup* BS = BodySetups[i];

            if (BS == nullptr)
            {
                continue;
            }
            
            FName TestName = BS->BoneName;
            int32 TestIndex = RefSkeleton.FindBoneIndex(TestName);
    
            if ((bIncludeParent && TestIndex == BaseIndex) || RefSkeleton.BoneIsChildOf(TestIndex, BaseIndex))
            {
                OutBodyIndices.Add(i);
            }
        }
    }
}

void UPhysicsAsset::GetNearestBodyIndicesBelow(TArray<int32>& OutBodyIndices, const FName InBoneName, const USkeletalMesh* InSkelMesh) const
{
    TArray<int32> AllBodiesBelow;
    GetBodyIndicesBelow(AllBodiesBelow, InBoneName, InSkelMesh, false);

    //we need to filter all bodies below to first in the chain
    TArray<bool> Nearest;
    Nearest.AddUninitialized(BodySetups.Num());
    for (int32 i = 0; i < Nearest.Num(); ++i)
    {
        Nearest[i] = true;
    }

    for (int32 i = 0; i < AllBodiesBelow.Num(); i++)
    {
        int32 BodyIndex = AllBodiesBelow[i];
        if (Nearest[BodyIndex] == false) continue;

        const UBodySetup * Body = BodySetups[BodyIndex];
        if (Body == nullptr)
        {
            continue;
        }
        TArray<int32> BodiesBelowMe;
        GetBodyIndicesBelow(BodiesBelowMe, Body->BoneName, InSkelMesh, false);
		
        for (int j = 0; j < BodiesBelowMe.Num(); ++j)
        {
            Nearest[BodiesBelowMe[j]] = false;	
        }
    }

    for (int32 i = 0; i < AllBodiesBelow.Num(); i++)
    {
        int32 BodyIndex = AllBodiesBelow[i];
        if (Nearest[BodyIndex])
        {
            OutBodyIndices.Add(BodyIndex);
        }
    }
}

FBoundingBox UPhysicsAsset::CalcAABB(const USkeletalMeshComponent* MeshComponent, const FTransform& LocalToWorld) const
{
    FBoundingBox Box = FBoundingBox();

	if (!MeshComponent)
	{
		return Box;
	}

	FVector Scale3D = LocalToWorld.GetScale();
	if(Scale3D.IsUniform())
	{
		const TArray<int32>* BodyIndexRefs = NULL;
		TArray<int32> AllBodies;
		// If we want to consider all bodies, make array with all body indices in
		if(MeshComponent->bConsiderAllBodiesForBounds)
		{
			AllBodies.AddUninitialized(BodySetups.Num());
			for(int32 i=0; i < BodySetups.Num();i ++)
			{
				AllBodies[i] = i;
			}
			BodyIndexRefs = &AllBodies;
		}
		// Otherwise, use the cached shortlist of bodies to consider
		else
		{
			BodyIndexRefs = &BoundsBodies;
		}

		// Then iterate over bodies we want to consider, calculating bounding box for each
		const int32 BodySetupNum = (*BodyIndexRefs).Num();

		for(int32 i=0; i < BodySetupNum; i++)
		{
			const int32 BodyIndex = (*BodyIndexRefs)[i];
			UBodySetup* bs = BodySetups[BodyIndex];

			// Check if setup should be considered for bounds, or if all bodies should be considered anyhow
			if (bs->bConsiderForBounds || MeshComponent->bConsiderAllBodiesForBounds)
			{
				if (i + 1 < BodySetupNum)
				{
					int32 NextIndex = (*BodyIndexRefs)[i+1];
					FPlatformMemory::Prefetch(BodySetups[NextIndex]);
					FPlatformMemory::Prefetch(BodySetups[NextIndex], PLATFORM_CACHE_LINE_SIZE);
				}

				int32 BoneIndex = MeshComponent->GetBoneIndex(bs->BoneName);
				if(BoneIndex != INDEX_NONE)
				{
					const FTransform WorldBoneTransform = MeshComponent->GetBoneTransform(BoneIndex);
					FBoundingBox BodySetupBounds = bs->AggGeom.CalcAABB(WorldBoneTransform);

					// When the transform contains a negative scale CalcAABB could return a invalid FBox that has Min and Max reversed
					// @TODO: Maybe CalcAABB should handle that inside and never return a reversed FBox
					if (BodySetupBounds.Min.X > BodySetupBounds.Max.X)
					{
						Swap(BodySetupBounds.Min.X, BodySetupBounds.Max.X);
					}

					if (BodySetupBounds.Min.Y > BodySetupBounds.Max.Y)
					{
						Swap(BodySetupBounds.Min.Y, BodySetupBounds.Max.Y);
					}

					if (BodySetupBounds.Min.Z > BodySetupBounds.Max.Z)
					{
						Swap(BodySetupBounds.Min.Z, BodySetupBounds.Max.Z);
					}

					Box += BodySetupBounds;
				}
			}
		}
	}
    
	const float MinBoundSize = 1.f;
	const FVector BoxSize = Box.GetSize();

	if(BoxSize.GetMin() < MinBoundSize)
	{
		const FVector ExpandByDelta ( FMath::Max(0.0f, MinBoundSize - BoxSize.X), FMath::Max(0.0f, MinBoundSize - BoxSize.Y), FMath::Max(0.0f, MinBoundSize - BoxSize.Z) );
		Box = Box.ExpandBy(ExpandByDelta * 0.5f);	//expand by applies to both directions with GetSize applies to total size so divide by 2
	}

	return Box;
}

FTransform UPhysicsAsset::GetSkelBoneTransform(int32 BoneIndex, const TArray<FTransform>& SpaceBases, const FTransform& LocalToWorld)
{
    if(BoneIndex != INDEX_NONE && BoneIndex < SpaceBases.Num())
    {
        return SpaceBases[BoneIndex] * LocalToWorld;
    }
    else
    {
        return FTransform::Identity;
    }
}

void UPhysicsAsset::GetCollisionMesh(const int32 ViewIndex, const FRefSkeletal& RefSkeleton)
{
    for( int32 i=0; i < BodySetups.Num(); i++)
    {
        int32 BoneIndex = RefSkeleton.FindBoneIndex(BodySetups[i]->BoneName);

        const FColor* BoneColor = reinterpret_cast<FColor*>(&BodySetups[i]);

        FTransform BoneTransform = RefSkeleton.RawBones[BoneIndex].GlobalTransform;
        // SkelBoneTransform should have the appropriate scale baked in from Component and Import Transform.
        // BoneTransform.SetScale3D(Scale3D);
        if (BodySetups[i]->bCreatedPhysicsMeshes)
        {
            BodySetups[i]->AggGeom.GetAggGeom(BoneTransform, *BoneColor, nullptr, false, false, true, ViewIndex);
        }
    }
}

void UPhysicsAsset::DisableCollision(const int32 BodyIndexA, const int32 BodyIndexB)
{
    if(BodyIndexA == BodyIndexB)
    {
        return;
    }

    const FRigidBodyIndexPair Key(BodyIndexA, BodyIndexB);

    // If its already in the disable table - do nothing
    if(CollisionDisableTable.Find(Key))
    {
        return;
    }

    CollisionDisableTable.Add(Key, false);
}

void UPhysicsAsset::EnableCollision(const int32 BodyIndexA, const int32 BodyIndexB)
{
    if(BodyIndexA == BodyIndexB)
    {
        return;
    }

    const FRigidBodyIndexPair Key(BodyIndexA, BodyIndexB);

    // If its not in table - do nothing
    if( !CollisionDisableTable.Find(Key) )
    {
        return;
    }

    CollisionDisableTable.Remove(Key);
}

bool UPhysicsAsset::IsCollisionEnabled(const int32 BodyIndexA, const int32 BodyIndexB) const
{
    if(BodyIndexA == BodyIndexB)
    {
        return false;
    }

    if(CollisionDisableTable.Find(FRigidBodyIndexPair(BodyIndexA, BodyIndexB)))
    {
        return false;
    }

    return true;
}

void UPhysicsAsset::SetPrimitiveCollision(const int32 BodyIndex, const EAggCollisionShape::Type PrimitiveType, const int32 PrimitiveIndex,
    ECollisionEnabled::Type CollisionEnabled)
{
    FKAggregateGeom* AggGeom = &BodySetups[BodyIndex]->AggGeom;
    AggGeom->GetElement(PrimitiveType, PrimitiveIndex)->SetCollisionEnabled(CollisionEnabled);
}

ECollisionEnabled::Type UPhysicsAsset::GetPrimitiveCollision(const int32 BodyIndex, const EAggCollisionShape::Type PrimitiveType, const int32 PrimitiveIndex) const
{
    FKAggregateGeom* AggGeom = &BodySetups[BodyIndex]->AggGeom;
    return AggGeom->GetElement(PrimitiveType, PrimitiveIndex)->GetCollisionEnabled();
}

void UPhysicsAsset::SetPrimitiveContributeToMass(const int32 BodyIndex, const EAggCollisionShape::Type PrimitiveType, const int32 PrimitiveIndex,
    const bool bContributesToMass)
{
    FKAggregateGeom* AggGeom = &BodySetups[BodyIndex]->AggGeom;
    return AggGeom->GetElement(PrimitiveType, PrimitiveIndex)->SetContributeToMass(bContributesToMass);
}

bool UPhysicsAsset::GetPrimitiveContributeToMass(const int32 BodyIndex, const EAggCollisionShape::Type PrimitiveType, const int32 PrimitiveIndex) const
{
    FKAggregateGeom* AggGeom = &BodySetups[BodyIndex]->AggGeom;
    return AggGeom->GetElement(PrimitiveType, PrimitiveIndex)->GetContributeToMass();
}

void UPhysicsAsset::UpdateBoundsBodiesArray()
{
    BoundsBodies.Empty();

    for(int32 i=0; i < BodySetups.Num(); i++)
    {
        if(BodySetups[i] && BodySetups[i]->bConsiderForBounds)
        {
            BoundsBodies.Add(i);
        }
    }
}

void UPhysicsAsset::UpdateBodySetupIndexMap()
{
    // update BodySetupIndexMap
    BodySetupIndexMap.Empty();

    for(int32 i=0; i<BodySetups.Num(); i++)
    {
        if (BodySetups[i])
        {
            BodySetupIndexMap.Add(BodySetups[i]->BoneName, i);
        }
    }
}


