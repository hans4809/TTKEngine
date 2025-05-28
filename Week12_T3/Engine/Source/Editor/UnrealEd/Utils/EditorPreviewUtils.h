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

            // 1) 끝에서부터 '_' 위치 찾기
            int32 UnderscoreIndex = FileName.Find(
                FString(TEXT("_")),            // 찾을 문자열
                ESearchCase::IgnoreCase,       // 대소문자 무시
                ESearchDir::FromEnd,           // 뒤에서부터 검색
                -1                             
            );

            // 2) '_' 포함 뒤쪽 부분 제거
            //    UnderscoreIndex가 INDEX_NONE(-1)이면 '_'가 없는 것이므로 원본 그대로 사용
            FString BaseName = (UnderscoreIndex != INDEX_NONE)
                ? FileName.Left(UnderscoreIndex)  // :contentReference[oaicite:1]{index=1}
                : FileName;

            FString PhysicsAssetName = BaseName + TEXT("_") + UPhysicsAsset::StaticClass()->GetName();
            
            UPhysicsAsset* PhysicsAsset = UAssetManager::Get().Get<UPhysicsAsset>(PhysicsAssetName);

            bool isNew = false;
            if (PhysicsAsset == nullptr)
            {
                PhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(nullptr);
                isNew = true;
            }
            
            // SkeletalMesh - PhysicsAsset 간의 연결
            // TODO : skeletalmesh 안에서 physicsasset owner 설정 중. 추후 PhysicsAsset이 owner 가지고 있는 구조는 개선 예정.
            USkeletalMesh* SkeletalMesh = SkeletalMeshComp->GetSkeletalMesh();
            SkeletalMesh->SetPhysicsAsset(PhysicsAsset);

            if (isNew)
            {
                PhysicsAsset->Initialize();
                FAssetDescriptor desc = SkeletalMesh->GetDescriptorCopy();
                desc.AssetName = PhysicsAssetName;
                desc.AssetExtension = TEXT(".ttalkak");
                FString FinalFileName = PhysicsAssetName + desc.AssetExtension;
                
                std::filesystem::path absPath = std::filesystem::path(desc.AbsolutePath);
                std::filesystem::path parentAbsPath = absPath.parent_path();
                std::filesystem::path finalAbsPath = parentAbsPath / FinalFileName;
                desc.AbsolutePath = FString(finalAbsPath.generic_wstring().c_str());
                
                std::filesystem::path relPath = std::filesystem::path(desc.RelativePath);
                std::filesystem::path parentRelPath = relPath.parent_path();
                std::filesystem::path finalRelPath = parentRelPath / FinalFileName;
                desc.RelativePath = FString(finalRelPath.generic_wstring().c_str());

                Serializer::SaveToFile(PhysicsAsset, desc.AbsolutePath);
            }
            
            NewWorld->SetSelectedActor(SkeletalMeshActor);
        }
    }
}