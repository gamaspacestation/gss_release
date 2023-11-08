/**
 * 
 *
 * 
 * 
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Misc/Optional.h"
#include "RealisticEyeMovementComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REALISTICEYEMOVEMENT_API URealisticEyeMovementComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	URealisticEyeMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetLookActor(class AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetLookComponent(class USceneComponent* Target);

	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetLookLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Target")
	void ClearLook();

	UFUNCTION(BlueprintPure, Category = "Eye")
	FORCEINLINE FRotator GetLookRotation() const { return CurrentRotation; }

private:
	FRotator CurrentRotation;

	UPROPERTY()
	TWeakObjectPtr<class USceneComponent> TargetComponent;
	TOptional<FVector> TargetPosition;
};
