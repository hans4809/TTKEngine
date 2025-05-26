#include "FireBallComponent.h"

#include "GameFramework/Actor.h"

UFireBallComponent::~UFireBallComponent()
{
}

void UFireBallComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void UFireBallComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UFireBallComponent::PostDuplicate()
{
    Super::PostDuplicate();
}