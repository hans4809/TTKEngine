#pragma once
#include <fbxsdk.h>

#include "Animation/AnimTypes.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Components/Mesh/StaticMesh.h"
#include "Container/Map.h"
#include "Container/Set.h"
#include "Skeletal/SkeletalDefine.h"

class UAnimDataModel;

class FFBXLoader
{
public:
    static bool InitFBXManager();
    static FSkeletalMeshRenderData ParseFBX(const FString& FilePath);
    
    static bool ParseSkeletalMeshFromFBX(const FString& FilePath, FSkeletalMeshRenderData& OutMeshData);
    static bool ParseSkeletonFromFBX(const FString& FilePath, FRefSkeletal& OutRefSkeletal);
    static bool ParseAnimationFromFBX(const FString& FilePath, UAnimDataModel*& OutAnimDataModel);
    static bool ParseMaterialFromFBX(const FString& FilePath, TArray<FObjMaterialInfo>& OutMaterialInfos);

private:
    static bool ParseSkeletalMeshAndSkeletonFromFBX(const FString& FilePath, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal);
public:
    //static USkeletalMesh* CreateSkeletalMesh(const FString& FilePath);
    //static USkeletalMesh* GetSkeletalMesh(const FString& FilePath);
    //static const TMap<FString, USkeletalMesh*>& GetSkeletalMeshes() { return SkeletalMeshMap;}
    
    static FSkeletalMeshRenderData GetSkeletalRenderData(const FString& FilePath);
    static FSkeletalMeshRenderData GetCopiedSkeletalRenderData(const FString& FilePath);
    static TMap<FName, FSkeletalMeshRenderData> GetAllSkeletalMeshData() { return SkeletalMeshData; }
    
    static FRefSkeletal GetRefSkeletal(FString FilePath);
    static FSkeletalMeshRenderData ParseBin(FString FilePath);
    static TMap<FName, FRefSkeletal> GetAllRefSkeletal() { return RefSkeletalData; }

    static TMap<FName, UAnimDataModel*> GetAllAnimData() { return AnimDataMap; }
    static UAnimDataModel* GetAnimData(const FString& FilePath);
    
    static void UpdateBoundingBox(FSkeletalMeshRenderData& MeshData);
private:
    // Skeletal
    static void ExtractFBXMeshData(const FbxScene* Scene, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal);
    static void ExtractBoneFromNode(FbxNode* Node, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal);
    static void ExtractMeshFromNode(FbxNode* Node, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal);
    static void ExtractMaterialFromNode(FbxNode* Node, TArray<FObjMaterialInfo>& OutMaterialInfos, TSet<FName>& ProcessedMaterials);
    static void ExtractVertices(FbxMesh* Mesh, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal);
    static void ExtractNormal(FbxMesh* Mesh, FSkeletalVertex& Vertex, int PolygonIndex, int VertexIndex);
    static void ExtractUV(FbxMesh* Mesh, FSkeletalVertex& Vertex, int PolygonIndex, int VertexIndex);
    static void ExtractTangent(FbxMesh* Mesh, FSkeletalVertex& Vertex, int PolygonIndex, int VertexIndex);
    static void ExtractSkinningData(FbxMesh* Mesh, FRefSkeletal& OutRefSkeletal);
    static void StoreWeights(FbxMesh* Mesh, FSkeletalVertex& Vertex, int PolygonIndex, int VertexIndex);
    static void StoreVertex(FSkeletalVertex& vertex, FSkeletalMeshRenderData& OutMeshData);
    static void ProcessSkinning(FbxSkin* Skin, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal, int BaseVertexIndex);
    static void ExtractIndices(FbxMesh* Mesh, FSkeletalMeshRenderData& OutMeshData, int BaseVertexIndex);
    static void ExtractMaterials(FbxNode* Node, FbxMesh* Mesh, FSkeletalMeshRenderData& OutMeshData, FRefSkeletal& OutRefSkeletal, int BaseIndexOffset);
    
    static FObjMaterialInfo ConvertFbxToObjMaterialInfo(FbxSurfaceMaterial* FbxMat, const FString& BasePath = TEXT(""));
    static FSkeletalVertex GetVertexFromControlPoint(FbxMesh* Mesh, int PolygonIndex, int VertexIndex);
    static bool IsTriangulated(FbxMesh* Mesh);
    
    // Anim
    static void ExtractFBXAnimData(const FbxScene* scene, const FString& FilePath);
    static void ExtractFBXAnimData(const FbxScene* scene, const FString& FilePath, UAnimDataModel*& OutAnimDataModel);
    
    static void ExtractAnimClip(FbxAnimStack* AnimStack, const TArray<FbxNode*>& BoneNodes, const FString& FilePath);
    static void ExtractAnimClip(FbxAnimStack* AnimStack, const TArray<FbxNode*>& BoneNodes, const FString& FilePath, UAnimDataModel*& OutAnimDataModel);

    static void ExtractAnimTrack(FbxNode* BoneNode, FRawAnimSequenceTrack& AnimTrack, const UAnimDataModel* AnimData);
    static void ExtractAnimTrack(FbxNode* BoneNode, FRawAnimSequenceTrack& AnimTrack, UAnimDataModel*& OutAnimDataModel);
    static void ExtractAnimCurve(FbxAnimLayer* AnimLayer, FbxNode* BoneNode, FRawAnimSequenceTrack& AnimTrack);

private:
    inline static FbxManager* FbxManager;

    inline static TMap<FName, FSkeletalMeshRenderData> SkeletalMeshData;
    inline static TMap<FString, USkeletalMesh*> SkeletalMeshMap;
    inline static TMap<FName, FRefSkeletal> RefSkeletalData;
    inline static TMap<FName, UAnimDataModel*> AnimDataMap;

    // data structure for parsing
    inline static TMap<FString, uint32> IndexMap;
    
    struct FBoneWeightInfo
    {
        int BoneIndex;
        float BoneWeight;
    };
    
    inline static TMap<uint32, TArray<FBoneWeightInfo>> SkinWeightMap;
    inline static TMap<FName, UAnimDataModel*> ParsedAnimData;
};
