/**
 * 
 *
 * 
 * 
 */

#include "RealisticEyeMovementComponent.h"

URealisticEyeMovementComponent::URealisticEyeMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URealisticEyeMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TargetComponent.IsValid())
	{
		SetLookLocation(TargetComponent->GetComponentLocation());
	}

	const FVector EyeLocation = GetComponentLocation();
	const FRotator EyeRotation = GetComponentRotation();

	const FRotator DesiredRotation = TargetPosition.IsSet() ? FRotationMatrix::MakeFromX(TargetPosition.GetValue() - EyeLocation).Rotator() : FRotator::ZeroRotator;
	
	const FRotator DeltaRotator = DesiredRotation - EyeRotation;

	const float HorizontalDistance = DeltaRotator.Yaw - CurrentRotation.Yaw;
	const float VerticalDistance = DeltaRotator.Pitch - CurrentRotation.Pitch;

	const float MaxHorizontalSpeed = 473 * (1 - FMath::Exp(-HorizontalDistance / 7.8f));
	const float MaxVerticalSpeed = 473 * (1 - FMath::Exp(-VerticalDistance / 7.8f));

	CurrentRotation.Yaw = FMath::RInterpTo(FRotator(0.f, CurrentRotation.Yaw, 0.f), FRotator(0.f, DeltaRotator.Yaw, 0.f), DeltaTime, MaxHorizontalSpeed).Yaw;
	CurrentRotation.Pitch = FMath::RInterpTo(FRotator(CurrentRotation.Pitch, 0.f, 0.f), FRotator(DeltaRotator.Pitch, 0.f, 0.f), DeltaTime, MaxVerticalSpeed).Pitch;
}

void URealisticEyeMovementComponent::SetLookActor(AActor* Actor)
{
	if (Actor == nullptr)
	{
		SetLookComponent(nullptr);
		return;
	}

	SetLookComponent(Actor->GetRootComponent());
}

void URealisticEyeMovementComponent::SetLookComponent(USceneComponent* Component)
{
	if (Component == nullptr)
	{
		ClearLook();
		return;
	}

	TargetComponent = Component;
	SetLookLocation(TargetComponent->GetComponentLocation());
}

void URealisticEyeMovementComponent::SetLookLocation(const FVector& Position)
{
	TargetPosition = Position;
}

void URealisticEyeMovementComponent::ClearLook()
{
	TargetComponent = nullptr;
	TargetPosition.Reset();
}
