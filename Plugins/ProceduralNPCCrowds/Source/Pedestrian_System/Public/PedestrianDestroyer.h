//  Copyright (c) 2022 KomodoBit Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PedestrianDestroyer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PEDESTRIAN_SYSTEM_API UPedestrianDestroyer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPedestrianDestroyer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, META = (DisplayName = "Remove Pedestrian", Category = "Procedural NPC Crowds"))
		void DestroyByDistance(float DistanceToDestroy);

	//creat bool for do once
	bool bDo;

		
};
