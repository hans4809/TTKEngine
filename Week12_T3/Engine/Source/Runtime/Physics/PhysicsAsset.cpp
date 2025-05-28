#include "PhysicsAsset.h"

#include "Animation/Skeleton.h"
#include "BodySetup/BodySetup.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
#include "PhysicsConstraintTemplate.h"
#include "Serialization/Serializer.h"

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
    if (!BodySetups.IsValidIndex(BodyIndex))
    {
        UE_LOG(LogLevel::Warning, "[UPhysicsAsset::BodyFindConstraints()] Invalid BodyIndex ");
    }

    FName BodyName = BodySetups[BodyIndex]->BoneName;

    for (int32 ConIdx = 0; ConIdx < ConstraintSetup.Num(); ConIdx++)
    {
        const UPhysicsConstraintTemplate* Constraint = ConstraintSetup[ConIdx];
        {
            if (Constraint->ConstraintBone1 == BodyName || Constraint->ConstraintBone2 == BodyName)
            {
                Constraints.Add(ConIdx);
            }
        }
    }
}

void UPhysicsAsset::SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty)
{
    PreviewSkeletalMesh = PreviewMesh;
}

USkeletalMesh* UPhysicsAsset::GetPreviewMesh() const
{
    return PreviewSkeletalMesh;
}

void UPhysicsAsset::AutoGenerateBodies()
{
    if (!PreviewSkeletalMesh || !PreviewSkeletalMesh->GetSkeleton())
    {
        return;
    }

    const FRefSkeletal& RefSkeletal = PreviewSkeletalMesh->GetSkeleton()->GetRefSkeletal();
    const TArray<FBone>& AllBones = RefSkeletal.RawBones;
    for (int32 i = 0; i < RefSkeletal.RawBones.Num(); ++i)
    {
        const FBone& bone = RefSkeletal.RawBones[i];
        FString parentBoneName = TEXT("None");
        if (bone.ParentIndex != INDEX_NONE && bone.ParentIndex >= 0 && bone.ParentIndex < RefSkeletal.RawBones.Num())
        {
            parentBoneName = RefSkeletal.RawBones[bone.ParentIndex].BoneName;
        }
        UE_LOG(LogLevel::Warning, TEXT("Bone: %s (Idx: %d), Parent: %s (ParentIdx: %d)"),
            *bone.BoneName, i, *parentBoneName, bone.ParentIndex);

    }

    BodySetups.Empty(); // 한 번만 호출해도 됩니다.

    const float DefaultCapsuleRadiusFactor = 0.1f; // 본 길이에 대한 반지름 비율
    const float MinCapsuleRadiusFallback = 0.1f;  // 최소 반지름 (0이 되는 것 방지)
  
    const FVector AssumedLeafBoneDirection = FVector::XAxisVector; // 말단 본의 기본 방향

    // Pass 1: 각 본의 반지름을 미리 계산하여 저장 (겹침 계산 시 필요)
    TArray<float> BoneRadii;
    BoneRadii.SetNum(AllBones.Num());

    for (int32 BoneIndex = 0; BoneIndex < AllBones.Num(); ++BoneIndex)
    {
        const FBoneNode& CurrentBoneTreeNode = RefSkeletal.BoneTree[BoneIndex];
        float CurrentBoneLength;

        if (CurrentBoneTreeNode.ChildIndices.Num() > 0)
        {
            int32 FirstChildBoneIndex = CurrentBoneTreeNode.ChildIndices[0];
            const FBone& FirstChildBoneData = AllBones[FirstChildBoneIndex];
            CurrentBoneLength = FirstChildBoneData.LocalTransform.GetOrigin().Magnitude();
        }
        else // 말단 본
        {
            // 말단 본의 길이를 반지름에 기반하여 설정하거나, 다른 휴리스틱 사용 가능
            // 여기서는 임시로 작은 값 또는 이전처럼 DefaultCapsuleRadiusFactor를 사용
            CurrentBoneLength = MinCapsuleRadiusFallback * 4.0f;
        }

        if (CurrentBoneLength < KINDA_SMALL_NUMBER)
        {
            CurrentBoneLength = KINDA_SMALL_NUMBER;
        }
        BoneRadii[BoneIndex] = FMath::Max(CurrentBoneLength * DefaultCapsuleRadiusFactor, MinCapsuleRadiusFallback);
    }


    // Pass 2: 캡슐 생성 및 겹침 조정
    for (int32 CurrentBoneIndex = 0; CurrentBoneIndex < AllBones.Num(); ++CurrentBoneIndex)
    {
        const FBone& CurrentBoneData = AllBones[CurrentBoneIndex];
        const FName CurrentBoneName = FName(*CurrentBoneData.BoneName);
        const FBoneNode& CurrentBoneTreeNode = RefSkeletal.BoneTree[CurrentBoneIndex];

        UBodySetup* NewBodySetup = FObjectFactory::ConstructObject<UBodySetup>(this);
        NewBodySetup->BoneName = CurrentBoneName;
        FKSphylElem CapsuleShape;

        // 현재 본의 반지름 (Pass 1에서 계산)
        CapsuleShape.Radius = BoneRadii[CurrentBoneIndex];
        UE_LOG(LogLevel::Warning, TEXT("Bone: %s, Radius: %f"), *CurrentBoneName.ToString(), CapsuleShape.Radius);
        // 본의 원래 시작점과 끝점 (로컬 공간 기준)
        FVector BoneSegmentStart_Local = FVector::ZeroVector;
        FVector BoneSegmentEnd_Local;
        float AnatomicalBoneLength;

        int32 NumChildren = CurrentBoneTreeNode.ChildIndices.Num();
        if (NumChildren > 0)
        {
            // 1) 자식 위치 벡터 합산
            FVector AccumRel = FVector::ZeroVector;
            for (int32 ChildIdx : CurrentBoneTreeNode.ChildIndices)
            {
                AccumRel += AllBones[ChildIdx].LocalTransform.GetOrigin();
            }

            // 2) 평균 벡터 및 축 방향 계산
            FVector AvgRel = AccumRel / NumChildren;
            FVector BoneAxis = AvgRel.GetSafeNormal();

            // 3) 평균 벡터 크기를 해부학적 길이로 사용
            AnatomicalBoneLength = AvgRel.Magnitude();

            // 4) BoneAxis 방향으로 캡슐 끝점 설정
            BoneSegmentEnd_Local = BoneAxis * AnatomicalBoneLength;
        }
        else
        {
            // 말단 본 처리(기존 로직 유지)
            AnatomicalBoneLength = MinCapsuleRadiusFallback * 4.0f;
            BoneSegmentEnd_Local = AssumedLeafBoneDirection * AnatomicalBoneLength;
        }

        if (AnatomicalBoneLength < KINDA_SMALL_NUMBER)
        {
            AnatomicalBoneLength = KINDA_SMALL_NUMBER;
            BoneSegmentEnd_Local = AssumedLeafBoneDirection * AnatomicalBoneLength;
        }
        UE_LOG(LogLevel::Error, TEXT("Bone: %s, AnatomicalBoneLength: %f"), *CurrentBoneName.ToString(), AnatomicalBoneLength);
        FVector BoneDirection_Local = BoneSegmentEnd_Local.GetSafeNormal();

        float StartOffset = 0.0f;
        float EndOffset = 0.0f;

        // 부모 본의 반지름만큼 현재 캡슐의 시작점을 뒤로 민다
        int32 ParentBoneIndex = CurrentBoneData.ParentIndex; 
        
        if (ParentBoneIndex != INDEX_NONE && ParentBoneIndex >= 0 && ParentBoneIndex < BoneRadii.Num()) // 유효성 검사 추가
        {
            StartOffset = BoneRadii[ParentBoneIndex];
        
        }

        if (ParentBoneIndex != INDEX_NONE && ParentBoneIndex < BoneRadii.Num())
        {
            StartOffset = BoneRadii[ParentBoneIndex];
        }

        // StartOffset, EndOffset 계산 후
        // 자식 본의 반지름만큼 현재 캡슐의 끝점을 앞으로 당긴다
        // 또는 말단 본인 경우 자신의 반지름만큼 끝을 둥글게 처리하기 위해 당긴다
        if (CurrentBoneTreeNode.ChildIndices.Num() > 0)
        {
            int32 FirstChildBoneIndex = CurrentBoneTreeNode.ChildIndices[0];
            EndOffset = BoneRadii[FirstChildBoneIndex];
        }
        else // 말단 본
        {
            EndOffset = CapsuleShape.Radius; // 자신의 반지름만큼 끝을 줄여 둥근 캡슐 끝 형성
        }

        UE_LOG(LogLevel::Warning, TEXT("Bone: %s, StartOffset: %f, EndOffset: %f"), *CurrentBoneName.ToString(), StartOffset, EndOffset);
        // 실제 캡슐의 물리적 길이 (원통 부분)
        float CapsuleCylinderLength = AnatomicalBoneLength - StartOffset - EndOffset;

        // 캡슐 중심과 절반 높이 계산
        if (CapsuleCylinderLength < KINDA_SMALL_NUMBER) // 오프셋으로 인해 길이가 너무 짧아지거나 음수가 된 경우
        {
            CapsuleCylinderLength = KINDA_SMALL_NUMBER; // 매우 작은 길이로 설정
        
            // 캡슐을 원래 본 세그먼트의 중앙에 위치시키거나, 시작점에 위치
            CapsuleShape.Center = BoneDirection_Local * (AnatomicalBoneLength * 0.5f); // 또는 StartOffset * BoneDirection_Local
            CapsuleShape.Length = CapsuleCylinderLength * 0.5f; // Sphyl의 Length는 절반 높이
        }
        else
        {
            // 조정된 캡슐의 시작점은 원래 시작점에서 StartOffset만큼 이동
            // 캡슐의 중심은 이 조정된 시작점에서 CapsuleCylinderLength의 절반만큼 더 이동
            CapsuleShape.Center = BoneDirection_Local * (StartOffset + CapsuleCylinderLength * 0.5f);
            CapsuleShape.Length = CapsuleCylinderLength * 0.5f;
        }

        // 방향 설정 (이전과 동일)
        FVector CapsuleUpVector_UE = BoneDirection_Local; // 이미 정규화됨
        FVector CapsuleRightVector_UE;
        if (FMath::Abs(CapsuleUpVector_UE.Dot(FVector::XAxisVector)) < 0.95f) 
        {
            CapsuleRightVector_UE = CapsuleUpVector_UE.Cross(FVector::XAxisVector).GetSafeNormal();
        }
        else
        {
            CapsuleRightVector_UE = CapsuleUpVector_UE.Cross(FVector::YAxisVector).GetSafeNormal();
        }
        FVector CapsuleForwardVector_UE = CapsuleRightVector_UE.Cross(CapsuleUpVector_UE).GetSafeNormal();

        FMatrix RotationMat = FMatrix::Identity;
        RotationMat.SetAxis(0, CapsuleForwardVector_UE);
        RotationMat.SetAxis(1, CapsuleRightVector_UE);
        RotationMat.SetAxis(2, CapsuleUpVector_UE);
        CapsuleShape.Rotation = RotationMat.ToQuat();

        NewBodySetup->AggGeom.SphylElems.Add(CapsuleShape);
        BodySetups.Add(NewBodySetup);
    }

    UpdateBodySetupIndexMap();
    UpdateBoundsBodiesArray();
}
void UPhysicsAsset::AutoGenerateConstraints()
{
    if (!PreviewSkeletalMesh || !PreviewSkeletalMesh->GetSkeleton())
    {
        UE_LOG(LogLevel::Warning, "[UPhysicsAsset::AutoGenerateConstraints()] Invalid SkeletalMesh or Skeleton");
    }

    const FRefSkeletal& RefSkeletal = PreviewSkeletalMesh->GetSkeleton()->GetRefSkeletal();

    for (int32 RootIndex : RefSkeletal.RootBoneIndices)
    {
        GenerateConstraintRecursive(RefSkeletal, RootIndex);
    }
}


void UPhysicsAsset::GenerateConstraintRecursive(const FRefSkeletal& RefSkeletal, int32 ParentBoneIndex)
{
    const FBoneNode& ParentNode = RefSkeletal.BoneTree[ParentBoneIndex];

    for (int32 ChildIndex : ParentNode.ChildIndices) {
        const FName ParentBoneName = RefSkeletal.GetBoneName(ParentBoneIndex);
        const FName ChildBoneName = RefSkeletal.GetBoneName(ChildIndex);

        // Default Constraint 생성
        UPhysicsConstraintTemplate* NewConstraint = FObjectFactory::ConstructObject<UPhysicsConstraintTemplate>(this);
        NewConstraint->ConstraintBone1 = ParentBoneName;
        NewConstraint->ConstraintBone2 = ChildBoneName;
        NewConstraint->JointName = FName(*(ParentBoneName.ToString() + TEXT("_") + ChildBoneName.ToString()));

        // 기본 프로퍼티 설정.
        NewConstraint->TwistLimit = 180.f;
        NewConstraint->SwingLimit1 = 180.f;
        NewConstraint->SwingLimit2 = 180.0f;

        ConstraintSetup.Add(NewConstraint);

        // Child도 재귀 처리.
        GenerateConstraintRecursive(RefSkeletal, ChildIndex);
    }
}

void UPhysicsAsset::GetBodySetups(TArray<class UBodySetup*>& OutBodySetup) const
{
    OutBodySetup = BodySetups;
}

bool UPhysicsAsset::LoadFromFile(const FString& filepath)
{
    return Serializer::LoadFromFile(filepath);
}

bool UPhysicsAsset::SerializeToFile(std::ostream& Out)
{
    return UAsset::SerializeToFile(Out);
}

bool UPhysicsAsset::DeserializeFromFile(std::istream& In)
{
    return UAsset::DeserializeFromFile(In);
}

void UPhysicsAsset::PostLoad()
{
    UAsset::PostLoad();
}

UPhysicsAsset::UPhysicsAsset()
{
}

UPhysicsAsset::~UPhysicsAsset()
{
}

void UPhysicsAsset::Initialize()
{
    // NOTICE : MySkeletalMesh->SetPhysicsAsset(NewPhysicsAsset); 처럼 physicsasset의 previewMesh 설정 이후 호출해줘야 합니다.-
    AutoGenerateBodies();
    AutoGenerateConstraints();
}

int32 UPhysicsAsset::FindControllingBodyIndex(const class USkeletalMesh* skelMesh, int32 StartBoneIndex) const
{
    int32 BoneIndex = StartBoneIndex;
    while (BoneIndex != INDEX_NONE)
    {
        FName BoneName = skelMesh->GetSkeleton()->GetRefSkeletal().GetBoneName(BoneIndex);
        int32 BodyIndex = FindBodyIndex(BoneName);

        if (BodyIndex != INDEX_NONE)
            return BodyIndex;

        int32 ParentBoneIndex = skelMesh->GetSkeleton()->GetRefSkeletal().GetParentIndex(BoneIndex);

        if (ParentBoneIndex == BoneIndex)
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

        const UBodySetup* Body = BodySetups[BodyIndex];
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
    if (Scale3D.IsUniform())
    {
        const TArray<int32>* BodyIndexRefs = NULL;
        TArray<int32> AllBodies;
        // If we want to consider all bodies, make array with all body indices in
        if (MeshComponent->bConsiderAllBodiesForBounds)
        {
            AllBodies.AddUninitialized(BodySetups.Num());
            for (int32 i = 0; i < BodySetups.Num(); i++)
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

        for (int32 i = 0; i < BodySetupNum; i++)
        {
            const int32 BodyIndex = (*BodyIndexRefs)[i];
            UBodySetup* bs = BodySetups[BodyIndex];

            // Check if setup should be considered for bounds, or if all bodies should be considered anyhow
            if (bs->bConsiderForBounds || MeshComponent->bConsiderAllBodiesForBounds)
            {
                if (i + 1 < BodySetupNum)
                {
                    int32 NextIndex = (*BodyIndexRefs)[i + 1];
                    FPlatformMemory::Prefetch(BodySetups[NextIndex]);
                    FPlatformMemory::Prefetch(BodySetups[NextIndex], PLATFORM_CACHE_LINE_SIZE);
                }

                int32 BoneIndex = MeshComponent->GetBoneIndex(bs->BoneName);
                if (BoneIndex != INDEX_NONE)
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

    if (BoxSize.GetMin() < MinBoundSize)
    {
        const FVector ExpandByDelta(FMath::Max(0.0f, MinBoundSize - BoxSize.X), FMath::Max(0.0f, MinBoundSize - BoxSize.Y), FMath::Max(0.0f, MinBoundSize - BoxSize.Z));
        Box = Box.ExpandBy(ExpandByDelta * 0.5f);	//expand by applies to both directions with GetSize applies to total size so divide by 2
    }

    return Box;
}

FTransform UPhysicsAsset::GetSkelBoneTransform(int32 BoneIndex, const TArray<FTransform>& SpaceBases, const FTransform& LocalToWorld)
{
    if (BoneIndex != INDEX_NONE && BoneIndex < SpaceBases.Num())
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
    for (int32 i = 0; i < BodySetups.Num(); i++)
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
    if (BodyIndexA == BodyIndexB)
    {
        return;
    }

    const FRigidBodyIndexPair Key(BodyIndexA, BodyIndexB);

    // If its already in the disable table - do nothing
    if (CollisionDisableTable.Find(Key))
    {
        return;
    }

    CollisionDisableTable.Add(Key, false);
}

void UPhysicsAsset::EnableCollision(const int32 BodyIndexA, const int32 BodyIndexB)
{
    if (BodyIndexA == BodyIndexB)
    {
        return;
    }

    const FRigidBodyIndexPair Key(BodyIndexA, BodyIndexB);

    // If its not in table - do nothing
    if (!CollisionDisableTable.Find(Key))
    {
        return;
    }

    CollisionDisableTable.Remove(Key);
}

bool UPhysicsAsset::IsCollisionEnabled(const int32 BodyIndexA, const int32 BodyIndexB) const
{
    if (BodyIndexA == BodyIndexB)
    {
        return false;
    }

    if (CollisionDisableTable.Find(FRigidBodyIndexPair(BodyIndexA, BodyIndexB)))
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

    for (int32 i = 0; i < BodySetups.Num(); i++)
    {
        if (BodySetups[i] && BodySetups[i]->bConsiderForBounds)
        {
            BoundsBodies.Add(i);
        }
    }
}

void UPhysicsAsset::UpdateBodySetupIndexMap()
{
    // update BodySetupIndexMap
    BodySetupIndexMap.Empty();

    for (int32 i = 0; i < BodySetups.Num(); i++)
    {
        if (BodySetups[i])
        {
            BodySetupIndexMap.Add(BodySetups[i]->BoneName, i);
        }
    }
}


