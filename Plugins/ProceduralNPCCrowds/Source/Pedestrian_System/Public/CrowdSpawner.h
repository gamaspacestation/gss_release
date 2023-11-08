//  Copyright (c) 2022 KomodoBit Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSpawner.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PEDESTRIAN_SYSTEM_API UCrowdSpawner : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCrowdSpawner();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, META = (DisplayName = "SpawnPedestrian", Category = "Procedural NPC Crowds"))
	void CrowdSpawnByDistance(float DistanceToSpawn, int32 MaxSpawnAmount, FVector CharacterHeight, FVector ProceduralSeperation, TSubclassOf<AActor> NPCToSpawn, FRotator SpawnRotation);


private:
	//create bool for do once
	bool bDo;
};
