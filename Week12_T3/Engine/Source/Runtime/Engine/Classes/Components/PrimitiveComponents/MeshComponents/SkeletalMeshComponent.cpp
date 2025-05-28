#include "SkeletalMeshComponent.h"

#include "LaunchEngineLoop.h"
#include "Engine/FBXLoader.h"
#include "Animation/AnimInstance.h"
#include "Animation/Skeleton.h"
#include "Engine/World.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Classes/Engine/FLoaderOBJ.h"
#include "GameFramework/Actor.h"
#include "Math/Transform.h"
#include "Renderer/Renderer.h"
#include "StaticMeshComponents/StaticMeshComponent.h"
#include "UObject/Casts.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/CustomAnimInstance/TestAnimInstance.h"
#include "Engine/Asset/AssetManager.h"
#include "Physics/PhysicsAsset.h"
#include "Physics/ConstraintInstance.h"
#include "Physics/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/PhysXSDKManager.h"

uint32 USkeletalMeshComponent::GetNumMaterials() const
{
    if (SkeletalMesh == nullptr) return 0;

    return SkeletalMesh->GetMaterials().Num();
}

UMaterial* USkeletalMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (SkeletalMesh != nullptr)
    {
        if (OverrideMaterials.IsValidIndex(ElementIndex))
        {
            return OverrideMaterials[ElementIndex];
        }

        if (SkeletalMesh->GetMaterials().IsValidIndex(ElementIndex))
        {
            return SkeletalMesh->GetMaterials()[ElementIndex].Material;
        }
    }
    return nullptr;
}

uint32 USkeletalMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    if (SkeletalMesh == nullptr) return -1;

    return SkeletalMesh->GetMaterialIndex(MaterialSlotName);
}

TArray<FName> USkeletalMeshComponent::GetMaterialSlotNames() const
{
    TArray<FName> MaterialNames;
    if (SkeletalMesh == nullptr) return MaterialNames;

    for (const FMaterialSlot& Material : SkeletalMesh->GetMaterials())
    {
        MaterialNames.Emplace(Material.MaterialSlotName);
    }

    return MaterialNames;
}

void USkeletalMeshComponent::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    if (SkeletalMesh == nullptr) return;
    SkeletalMesh->GetUsedMaterials(Out);
    for (int materialIndex = 0; materialIndex < GetNumMaterials(); materialIndex++)
    {
        if (OverrideMaterials[materialIndex] != nullptr)
        {
            Out[materialIndex] = OverrideMaterials[materialIndex];
        }
    }
}

int USkeletalMeshComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    if (!AABB.IntersectRay(rayOrigin, rayDirection, pfNearHitDistance)) return 0;
    int nIntersections = 0;
    if (SkeletalMesh == nullptr) return 0;

    FSkeletalMeshRenderData renderData = SkeletalMesh->GetRenderData();

    FSkeletalVertex* vertices = renderData.Vertices.GetData();
    int vCount = renderData.Vertices.Num();
    UINT* indices = renderData.Indices.GetData();
    int iCount = renderData.Indices.Num();

    if (!vertices) return 0;
    BYTE* pbPositions = reinterpret_cast<BYTE*>(renderData.Vertices.GetData());

    int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    float fNearHitDistance = FLT_MAX;
    for (int i = 0; i < nPrimitives; i++) {
        int idx0, idx1, idx2;
        if (!indices) {
            idx0 = i * 3;
            idx1 = i * 3 + 1;
            idx2 = i * 3 + 2;
        }
        else {
            idx0 = indices[i * 3];
            idx2 = indices[i * 3 + 1];
            idx1 = indices[i * 3 + 2];
        }

        // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
        uint32 stride = sizeof(FSkeletalVertex);
        FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
        FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
        FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

        float fHitDistance;
        if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance))
        {
            if (fHitDistance < fNearHitDistance)
            {
                pfNearHitDistance = fNearHitDistance = fHitDistance;
            }
            nIntersections++;
        }

    }
    return nIntersections;
}


void USkeletalMeshComponent::SetSkeletalMesh(USkeletalMesh* value)
{
    SkeletalMesh = value;
    VBIBTopologyMappingName = SkeletalMesh->GetFName();

    ResetToOriginPos();

    //value->UpdateBoneHierarchy();
    
    AABB = SkeletalMesh->GetRenderData().BoundingBox;

    // CreateBoneComponents();
}

int32 USkeletalMeshComponent::GetBoneIndex(FName BoneName) const
{
    int32 BoneIndex = INDEX_NONE;
    if (BoneName != NAME_None && GetSkeletalMesh())
    {
        BoneIndex = GetSkeletalMesh()->GetSkeleton()->GetRefSkeletal().FindBoneIndex(BoneName);
    }

    return BoneIndex;
}

FTransform USkeletalMeshComponent::GetBoneTransform(const int32 BoneIndex) const
{
    return FTransform(BoneWorldTransforms[BoneIndex]);
}

UAnimSingleNodeInstance* USkeletalMeshComponent::GetSingleNodeInstance() const
{
    return Cast<UAnimSingleNodeInstance>(AnimInstance);
}

void USkeletalMeshComponent::CreateBoneComponents()
{
    // 이미 할당된 component가 있다면 삭제
    for (auto& BoneComp : BoneComponents)
    {
        BoneComp->DestroyComponent();
    }

    //FManagerOBJ::CreateStaticMesh("Contents/helloBlender.obj");
    UStaticMesh* StaticMesh = UAssetManager::Get().Get<UStaticMesh>(TEXT("helloBlender"));

    for (const auto& Bone : GetSkeletalMesh()->GetRenderData().Bones)
    {
        UStaticMeshComponent* BoneComp = GetOwner()->AddComponent<UStaticMeshComponent>(EComponentOrigin::Runtime);
        BoneComp->SetStaticMesh(StaticMesh);
        BoneComp->SetWorldLocation(Bone.GlobalTransform.GetTranslationVector());
        BoneComp->SetFName(Bone.BoneName);
        BoneComponents.Add(BoneComp);
    }
}

USkeletalMesh* USkeletalMeshComponent::LoadSkeletalMesh(const FString& FileName)
{
    USkeletalMesh* SkeletalMesh = UAssetManager::Get().Get<USkeletalMesh>(FileName);
    SetSkeletalMesh(SkeletalMesh);

    return SkeletalMesh;
}

void USkeletalMeshComponent::UpdateBoneHierarchy()
{
    for (int i = 0; i < SkeletalMesh->GetRenderData().Vertices.Num(); ++i)
    {
        SkeletalMesh->GetRenderData().Vertices[i].Position = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawVertices[i].Position;
    }

    // 먼저 루트 뼈들의 글로벌 트랜스폼을 설정
    for (const int32 RootIndex : SkeletalMesh->GetSkeleton()->GetRefSkeletal().RootBoneIndices)
    {
        // 루트 뼈는 로컬 트랜스폼이 곧 글로벌 트랜스폼이 됨
        BoneWorldTransforms[RootIndex] = BoneLocalTransforms[RootIndex];
        BoneSkinningMatrices[RootIndex] = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones[RootIndex].InverseBindPoseMatrix * BoneWorldTransforms[RootIndex];

        // 재귀적으로 자식 뼈들의 글로벌 트랜스폼을 업데이트
        UpdateChildBones(RootIndex);
    }
}

void USkeletalMeshComponent::ResetToOriginPos()
{
    uint32 BoneNum = FMath::Min(SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones.Num(), SkeletalMesh->GetRenderData().Bones.Num());
    BoneLocalTransforms.SetNum(BoneNum);
    BoneWorldTransforms.SetNum(BoneNum);
    BoneSkinningMatrices.SetNum(BoneNum);

    for (int i = 0; i < SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones.Num() && i < SkeletalMesh->GetRenderData().Bones.Num(); ++i)
    {
        // 로컬 트랜스폼 복원
        BoneLocalTransforms[i] = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones[i].LocalTransform;
        BoneWorldTransforms[i] = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones[i].GlobalTransform;
        BoneSkinningMatrices[i] = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones[i].SkinningMatrix;
    }
}

void USkeletalMeshComponent::UpdateChildBones(int ParentIndex)
{
    // BoneTree 구조를 사용하여 현재 부모 뼈의 모든 자식을 찾음
    const FBoneNode& ParentNode = SkeletalMesh->GetSkeleton()->GetRefSkeletal().BoneTree[ParentIndex];

    // 모든 자식 뼈를 순회
    for (const int32 ChildIndex : ParentNode.ChildIndices)
    {
        // 자식의 글로벌 트랜스폼은 부모의 글로벌 트랜스폼과 자식의 로컬 트랜스폼을 결합한 것
        BoneWorldTransforms[ChildIndex] = BoneLocalTransforms[ChildIndex] * BoneWorldTransforms[ParentIndex];
        BoneSkinningMatrices[ChildIndex] = SkeletalMesh->GetSkeleton()->GetRefSkeletal().RawBones[ChildIndex].InverseBindPoseMatrix * BoneWorldTransforms[ChildIndex];

        // 재귀적으로 이 자식의 자식들도 업데이트
        UpdateChildBones(ChildIndex);
    }
}

void USkeletalMeshComponent::PlayAnimation(UAnimSequence* NewAnimToPlay, bool bLooping)
{
    SetAnimSequence(NewAnimToPlay);
    Play(bLooping);
}

void USkeletalMeshComponent::SetAnimSequence(UAnimSequence* NewAnimToPlay)
{
    if (NewAnimToPlay == nullptr)
    {
        return;
    }
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetCurrentSequence(NewAnimToPlay);
    }
}

UAnimSequence* USkeletalMeshComponent::GetAnimSequence() const
{

    return GetSingleNodeInstance()->GetCurrentSequence();
}

void USkeletalMeshComponent::SkinningVertex()
{
    for (auto& Vertex : SkeletalMesh->GetRenderData().Vertices)
    {
        Vertex.SkinningVertex(SkeletalMesh->GetRenderData().Bones);
    }

    FFBXLoader::UpdateBoundingBox(SkeletalMesh->GetRenderData());
    AABB = SkeletalMesh->GetRenderData().BoundingBox;

    SkeletalMesh->SetData(SkeletalMesh->GetRenderData(), SkeletalMesh->GetSkeleton()); // TODO: Dynamic VertexBuffer Update하게 바꾸기
}

// std::unique_ptr<FActorComponentInfo> USkeletalMeshComponent::GetComponentInfo()
// {
//     auto Info = std::make_unique<FStaticMeshComponentInfo>();
//     SaveComponentInfo(*Info);
//     
//     return Info;
// }

// void UStaticMeshComponent::SaveComponentInfo(FActorComponentInfo& OutInfo)
// {
//     FStaticMeshComponentInfo* Info = static_cast<FStaticMeshComponentInfo*>(&OutInfo);
//     Super::SaveComponentInfo(*Info);
//
//     Info->StaticMeshPath = staticMesh->GetRenderData()->PathName;
// }

// void UStaticMeshComponent::LoadAndConstruct(const FActorComponentInfo& Info)
// {
//     Super::LoadAndConstruct(Info);
//
//     const FStaticMeshComponentInfo& StaticMeshInfo = static_cast<const FStaticMeshComponentInfo&>(Info);
//     UStaticMesh* Mesh = FManagerOBJ::CreateStaticMesh(FString::ToFString(StaticMeshInfo.StaticMeshPath));
//     SetStaticMesh(Mesh);
// }

void USkeletalMeshComponent::PostDuplicate()
{
    ResetToOriginPos();
}

void USkeletalMeshComponent::BeginPlay()
{
    UMeshComponent::BeginPlay();
    animTime = 0.f;
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    if (AnimInstance)
    {
        AnimInstance->NativeUpdateAnimation(DeltaTime);
    }

    UpdateBoneHierarchy();
    if (GEngineLoop.Renderer.GetSkinningMode() == ESkinningType::CPU)
    {
        SkeletalMesh->UpdateSkinnedVertices();
        bCPUSkinned = true;
    }
    else
    {
        if (bCPUSkinned)
        {
            SkeletalMesh->ResetToOriginalPose();
            bCPUSkinned = false;
        }
    }
}

void USkeletalMesh::ResetToOriginalPose()
{
    // 본 트랜스폼 복원
    for (int i = 0; i < Skeleton->GetRefSkeletal().RawVertices.Num() && i < SkeletalMeshRenderData.Bones.Num(); i++)
    {
        // 로컬 트랜스폼 복원
        SkeletalMeshRenderData.Bones[i].LocalTransform = Skeleton->GetRefSkeletal().RawBones[i].LocalTransform;
        SkeletalMeshRenderData.Bones[i].GlobalTransform = Skeleton->GetRefSkeletal().RawBones[i].GlobalTransform;
        SkeletalMeshRenderData.Bones[i].SkinningMatrix = Skeleton->GetRefSkeletal().RawBones[i].SkinningMatrix;
    }

    // 스키닝 적용
    UpdateSkinnedVertices();
}

void USkeletalMesh::SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset)
{
    MyPhysicsAsset = InPhysicsAsset;

    if (InPhysicsAsset)
    {
        InPhysicsAsset->SetPreviewMesh(this);
    }
}

UPhysicsAsset* USkeletalMesh::GetPhysicsAsset() const
{
    return MyPhysicsAsset;
}

void USkeletalMeshComponent::Play(bool bLooping)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetPlaying(true);
        SingleNodeInstance->SetLooping(bLooping);
    }
}

void USkeletalMeshComponent::Stop()
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetPlaying(false);
    }
}

void USkeletalMeshComponent::SetPlaying(bool bPlaying)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetPlaying(bPlaying);
    }
}

bool USkeletalMeshComponent::IsPlaying() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->IsPlaying();
    }

    return false;
}

void USkeletalMeshComponent::SetReverse(bool bIsReverse)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetReverse(bIsReverse);
    }
}

bool USkeletalMeshComponent::IsReverse() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->IsReverse();
    }
}

void USkeletalMeshComponent::SetPlayRate(float Rate)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetPlayRate(Rate);
    }
}

float USkeletalMeshComponent::GetPlayRate() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->GetPlayRate();
    }

    return 0.f;
}

void USkeletalMeshComponent::SetLooping(bool bIsLooping)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetLooping(bIsLooping);
    }
}

bool USkeletalMeshComponent::IsLooping() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->IsLooping();
    }
    return false;
}

int USkeletalMeshComponent::GetCurrentKey() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->GetCurrentKey();
    }
    return 0;
}

void USkeletalMeshComponent::SetCurrentKey(int InKey)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetCurrentKey(InKey);
    }
}

void USkeletalMeshComponent::SetElapsedTime(float InElapsedTime)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetElapsedTime(InElapsedTime);
    }
}

float USkeletalMeshComponent::GetElapsedTime() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->GetElapsedTime();
    }
    return 0.f;
}

int32 USkeletalMeshComponent::GetLoopStartFrame() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->GetLoopStartFrame();
    }
    return 0;
}

void USkeletalMeshComponent::SetLoopStartFrame(int32 InLoopStartFrame)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetLoopStartFrame(InLoopStartFrame);
    }
}

int32 USkeletalMeshComponent::GetLoopEndFrame() const
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        return SingleNodeInstance->GetLoopEndFrame();
    }
    return 0;
}

void USkeletalMeshComponent::SetLoopEndFrame(int32 InLoopEndFrame)
{
    if (UAnimSingleNodeInstance* SingleNodeInstance = GetSingleNodeInstance())
    {
        SingleNodeInstance->SetLoopEndFrame(InLoopEndFrame);
    }
}

void USkeletalMeshComponent::SetAnimationMode(EAnimationMode InAnimationMode)
{
    AnimationMode = InAnimationMode;
}
int32 USkeletalMeshComponent::FindBodyIndex(FName BoneName) const
{
    if (BoneName == NAME_None)
    {
        return INDEX_NONE;
    }

    if (SkeletalMesh && SkeletalMesh->GetPhysicsAsset())
    {
        if (BoneName == NAME_None)
        {
            return INDEX_NONE;
        }

        for (int32 i = 0; i < Bodies.Num(); ++i)
        {
            if (Bodies[i] && Bodies[i]->AssociatedBoneName == BoneName)
            {
                return i;
            }
        }
    }
    return INDEX_NONE;
}
void USkeletalMeshComponent::CreatePhysicsState()
{
    if (!SkeletalMesh)
    {
        return;
    }
    UPhysicsAsset* Asset = SkeletalMesh->GetPhysicsAsset();
    if (!Asset)
    {
        UPhysicsAsset* NewPhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);
        SkeletalMesh->SetPhysicsAsset(NewPhysicsAsset);
        Asset = SkeletalMesh->GetPhysicsAsset();
        Asset->Initialize();
    }

    for (FBodyInstance* BI : Bodies)
    {
        BI->ReleasePhysicsState();
        delete BI;
    }
    Bodies.Empty();
    for (FConstraintInstance* CI : Constraints)
    {
        CI->TermConstraint(GetWorld()->GetPhysicsScene());
        delete CI;
    }
    Constraints.Empty();

    FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
    physx::PxPhysics* PxSDK = FPhysXSDKManager::GetInstance().GetPhysicsSDK();
    TArray< UBodySetup*> BodySetups;
    Asset->GetBodySetups(BodySetups);

    for (UBodySetup* BS : BodySetups)
    {
        if (!BS) continue;

        int32 BoneIdx = GetBoneIndex(BS->BoneName);

        if (BS->BoneName != NAME_None && BoneIdx == INDEX_NONE)
        {
            continue;
        }
        FBodyInstance* BI = new FBodyInstance();

        BI->Initialize(this, PxSDK);

        BI->AssociatedBoneName = BS->BoneName; // AssociatedBoneName 설정!

        FTransform BoneGlobalTransform = (BoneIdx != INDEX_NONE) ? GetBoneTransform(BoneIdx) : GetWorldTransform();

        EPhysBodyType BodyType = EPhysBodyType::Dynamic;
        UPhysicalMaterial* PhysMat = BS->GetDefaultMaterial();
        if (!PhysMat)
        {
            break;
        }
        if (BI->CreatePhysicsState(BoneGlobalTransform, BodyType))
        {

            for (auto& E : BS->AggGeom.SphereElems)
                BI->AddSphereGeometry(E.Radius, PhysMat, FTransform(FQuat::Identity, E.Center, FVector::OneVector));
            
            for (auto& E : BS->AggGeom.BoxElems)
                BI->AddBoxGeometry(FVector(E.X, E.Y, E.Z) * 0.5f, PhysMat, FTransform(FQuat::Identity, E.Center, FVector::OneVector));
            
            for (auto& E : BS->AggGeom.SphylElems)
            {
                FTransform ElemLocalTM = E.GetTransform();        // {Center, Rotation, Scale}
                //FTransform ShapeLocalTM =   * BoneGlobalTransform;
                BI->AddCapsuleGeometry(E.Radius, E.Length, PhysMat, ElemLocalTM);
            }
            
            for (auto& E : BS->AggGeom.ConvexElems)
                BI->AddConvexGeometry(E.CookedPxConvexMesh, PhysMat, E.GetTransform(), FVector::OneVector);

            BI->UpdateMassAndInertia(BS->CalculateMass(this));
            BI->AddObject(PhysScene);
            Bodies.Add(BI);
        }
    }
    // 4) ConstraintSetup → FConstraintInstance → PxJoint
    for (UPhysicsConstraintTemplate* Tpl : Asset->ConstraintSetup)
    {
        if (!Tpl) continue;

        int32 i1 = FindBodyIndex(Tpl->ConstraintBone1);
        int32 i2 = FindBodyIndex(Tpl->ConstraintBone2);
        if (i1 == INDEX_NONE || i2 == INDEX_NONE) continue;

        FConstraintInstance* CI = new FConstraintInstance();
        CI->JointName = Tpl->JointName;
        CI->ConstraintBone1 = Tpl->ConstraintBone1;
        CI->ConstraintBone2 = Tpl->ConstraintBone2;
        CI->ProfileInstance.CopyFrom(Tpl->DefaultProfile);

        int32 ChildSkelBoneIndex = GetBoneIndex(Tpl->ConstraintBone2);

        FBodyInstance* BodyInst1 = Bodies[i1];
        FBodyInstance* BodyInst2 = Bodies[i2];

        FTransform Actor1WorldTransform = BodyInst1->GetGlobalPose();
        Actor1WorldTransform.SetScale(FVector::OneVector);
        
        FTransform Actor2WorldTransform = BodyInst2->GetGlobalPose();
        Actor2WorldTransform.SetScale(FVector::OneVector);

        //FTransform JointAnchorWorldTransform = GetBoneTransform(ChildSkelBoneIndex);

        //JointAnchorWorldTransform.SetLocation(JointAnchorWorldTransform.GetLocation());
        //JointAnchorWorldTransform.SetRotation(Actor1WorldTransform.GetRotation());
        //JointAnchorWorldTransform.SetScale(FVector::OneVector);

        FTransform JointAnchorLocationOnly = GetBoneTransform(ChildSkelBoneIndex); // 위치 정보만 사용
        FTransform AlignedJointAnchorWorldTransform;
        AlignedJointAnchorWorldTransform.SetLocation(JointAnchorLocationOnly.GetLocation());
        AlignedJointAnchorWorldTransform.SetRotation(Actor1WorldTransform.GetRotation());
        AlignedJointAnchorWorldTransform.SetScale(FVector::OneVector);

        CI->Pos1 = AlignedJointAnchorWorldTransform * Actor1WorldTransform.Inverse();

        CI->Pos2 = AlignedJointAnchorWorldTransform * Actor2WorldTransform.Inverse();

        CI->InitConstraint(this, Bodies[i1], Bodies[i2], PhysScene);
        
        Constraints.Add(CI);
    }
}
