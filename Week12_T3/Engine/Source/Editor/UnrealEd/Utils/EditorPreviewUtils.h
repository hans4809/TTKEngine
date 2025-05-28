#pragma once

#include "Components/Mesh/SkeletalMesh.h"
#include "Actors/SkeletalMeshActor.h"
#include "EditorEngine.h"
#include "Engine/FEditorStateManager.h"
#include "Physics/PhysicsAsset.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
namespace PhysicsPreviewUtils
{
    /**
    * SkeletalMesh 및 PhysicsAsset을 프리뷰하기 위한 World와 Actor를 세팅합니다.
    *
    * @param SkeletalMesh       프리뷰할 SkeletalMesh
    * @param PhysicsAsset       프리뷰할 PhysicsAsset
    * @param OutWorld           생성된 World 반환
    * @param OutSkeletalActor   생성된 SkeletalMeshActor 반환
    */
    void SetupPhysicsAssetPreview(const FString& FileName)
    {
        {
            UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
            if (!EditorEngine )
            {
                return;
            }

            // Preview World 생성
            UWorld*  NewWorld = EditorEngine->CreatePreviewWindow(EditorPreviewPhysicsAsset);

            NewWorld->ClearSelectedActors();

            // SkySphere 등 배경 설정 (생략 가능)
            // ...

            // SkeletalMeshActor 생성
            ASkeletalMeshActor* SkeletalMeshActor = NewWorld->SpawnActor<ASkeletalMeshActor>();
            USkeletalMeshComponent* SkeletalMeshComp = SkeletalMeshActor->GetSkeletalMeshComponent();
            
            SkeletalMeshComp->LoadSkeletalMesh(FileName);
            
            UPhysicsAsset* NewPhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);

            // FIXME : 추후 PhysicsAsset FileSystem 완성 시 로드 방식 변경.
            //UPhysicsAsset* MyPhisicsAsset = LoadPhysicsAsset(FilePath);
            
            // SkeletalMesh - PhysicsAsset 간의 연결
            // TODO : skeletalmesh 안에서 physicsasset owner 설정 중. 추후 PhysicsAsset이 owner 가지고 있는 구조는 개선 예정.
            USkeletalMesh* SkeletalMesh = SkeletalMeshActor->GetSkeletalMeshComponent()->GetSkeletalMesh();
            SkeletalMesh->SetPhysicsAsset(NewPhysicsAsset);

            // SkeletalMeshComponent에 설정
            SkeletalMeshComp->SetSkeletalMesh(SkeletalMesh);

            NewPhysicsAsset->Initialize();

            NewWorld->SetSelectedActor(SkeletalMeshActor);
        }
    }
}