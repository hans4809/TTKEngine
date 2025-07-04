#include "AnimInstance.h"
#include "Components/PrimitiveComponents/MeshComponents/SkeletalMeshComponent.h"
#include "UObject/Casts.h"
#include "Animation/AnimSequence.h"
#include "AnimData/AnimDataModel.h"
#include "Math/JungleMath.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationRuntime.h"
#include "Container/Array.h"

AActor* UAnimInstance::GetOwningActor() const
{
    return GetOwningComponent()->GetOwner();
}

USkeletalMeshComponent* UAnimInstance::GetOwningComponent() const
{
    return CastChecked<USkeletalMeshComponent>(GetOuter());
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds) const
{
    if (CurrentSequence)
    {
        for (FAnimNotifyEvent& Notify : CurrentSequence->Notifies)
        {
            // 시간 조건에 맞으면 Notify 실행
            if (!Notify.OnNotify.IsBound())
                continue;
            
            if (Notify.Duration == 0.f)
            {
                if (Notify.TriggerTime <= CurrentTime && Notify.bIsTriggered == false)
                {
                    Notify.OnNotify.Execute();
                    Notify.bIsTriggered = true;
                }
            }
            else
            {
                if (Notify.TriggerTime <= CurrentTime && CurrentTime < Notify.TriggerTime + Notify.Duration)
                {
                    Notify.OnNotify.Execute();
                    Notify.bIsTriggered = true;
                }
            }
        }
    }
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    TriggerAnimNotifies(DeltaSeconds);
    UpdateCurveValues(DeltaSeconds);
}

void UAnimInstance::UpdateAnimation(UAnimSequence* AnimSequence, float DeltaTime)
{
    const UAnimDataModel* DataModel = AnimSequence->GetDataModel();
    TArray<FName> BoneNames;
    DataModel->GetBoneTrackNames(BoneNames);

    USkeletalMeshComponent* SkeletalMeshComp = GetOwningComponent();

    CurrentPose.Pose.BoneTransforms.Empty();
    FAnimExtractContext Context(CurrentTime, true, false);
    AnimSequence->GetAnimationPose(CurrentPose, Context);

    int32 boneNum = FMath::Min(SkeletalMeshComp->BoneLocalTransforms.Num(), CurrentPose.Pose.BoneTransforms.Num());

    for (int32 i = 0; i < boneNum; ++i)
    {
        const FTransform& BoneTransform = CurrentPose.Pose.BoneTransforms[i];
        const FMatrix TransformMatrix = JungleMath::CreateModelMatrix(
            BoneTransform.GetLocation(),
            BoneTransform.GetRotation(),
            BoneTransform.GetScale()
        );
       SkeletalMeshComp->BoneLocalTransforms[i] = TransformMatrix;
    }
    
    CurrentTime += DeltaTime;
    if (CurrentTime > DataModel->GetPlayLength())
    {
        CurrentTime = 0.0f;
        AnimSequence->ResetNotifies();
    }
}

void UAnimInstance::BlendAnimations(UAnimSequence* FromSequence, UAnimSequence* ToSequence, float DeltaTime)
{
    if (!FromSequence || !ToSequence)
        return;

    BlendTime += DeltaTime;
    float BlendAlpha = FMath::Clamp(BlendTime / BlendDuration, 0.0f, 1.0f);
    
    FPoseContext &FromPose = OriginalPose;
    FPoseContext &ToPose = TargetPose;

    //FAnimExtractContext FromContext(CurrentTime, true, false);
    //FAnimExtractContext ToContext(0, true, false);


    //FromSequence->GetAnimationPose(FromPose, FromContext); 
    //ToSequence->GetAnimationPose(ToPose, ToContext); 

    CurrentPose.Pose.BoneTransforms.Empty();
    FAnimationRuntime::BlendTwoPosesTogether(FromPose.Pose, ToPose.Pose, 
        FromPose.Curve,ToPose.Curve, BlendAlpha, CurrentPose.Pose, CurrentPose.Curve);


    USkeletalMeshComponent* SkeletalMeshComp = GetOwningComponent();
    USkeletalMesh* SkeletalMesh = SkeletalMeshComp->GetSkeletalMesh();

    int32 boneNum = FMath::Min(SkeletalMeshComp->BoneLocalTransforms.Num(), CurrentPose.Pose.BoneTransforms.Num());

    for (int32 i = 0; i < boneNum; ++i)
    {
        const FTransform& BoneTransform = CurrentPose.Pose.BoneTransforms[i];
        const FMatrix TransformMatrix = JungleMath::CreateModelMatrix(
            BoneTransform.GetLocation(),
            BoneTransform.GetRotation(),
            BoneTransform.GetScale()
        );
        SkeletalMeshComp->BoneLocalTransforms[i] = TransformMatrix;
    }
    
    if (BlendAlpha >= 1.0f)
    {
        bIsBlending = false;
        CurrentTime = 0.0f;
    }
}

void UAnimInstance::CapturePose()
{
    OriginalPose.Pose.BoneTransforms.Empty();
    TargetPose.Pose.BoneTransforms.Empty();

    FAnimExtractContext TargetContext(0, true, false);
    CurrentSequence->GetAnimationPose(TargetPose, TargetContext);

    OriginalPose = CurrentPose;
}

void UAnimInstance::UpdateCurveValues(float DeltaSeconds) const
{
    if (CurrentSequence)
    {
        // 현재 애니메이션 시퀀스에서 커브 정보를 가져옴
        //for (const FCurve& Curve : CurrentSequence->Curves)
        //{
        //    // 시간에 따른 커브 값 평가
        //    float Value = Curve.Evaluate(CurrentTime);

        //    // BlendedCurve에 저장
        //    BlendedCurve.SetCurveValue(Curve.CurveName, Value);
        //}
    }
}
