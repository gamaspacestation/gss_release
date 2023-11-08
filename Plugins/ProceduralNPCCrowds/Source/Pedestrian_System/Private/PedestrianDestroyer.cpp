//  Copyright (c) 2022 KomodoBit Games. All rights reserved.


#include "PedestrianDestroyer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Vector.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UPedestrianDestroyer::UPedestrianDestroyer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPedestrianDestroyer::BeginPlay()
{
	Super::BeginPlay();
	//Initialize do once bool with value
	bDo = false;

	
}

void UPedestrianDestroyer::DestroyByDistance(float DistanceToDestroy)
{

	if (UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) != NULL)
	{


		//Get Player Character Node equivelent
		ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		//Get owner of components location
		FVector Self = GetOwner()->GetActorLocation();
		//Get Player Character Location
		FVector GetPlayerCharacter = Character->GetActorLocation();
		//Get Distance To Node equivelent
		float DistanceFromCharacter = FVector::Distance(Self, GetPlayerCharacter);

			//Check Distance from player using above variables
			if (DistanceFromCharacter >= DistanceToDestroy)
			{
				//Start Do Once
				if (bDo == false)
				{
					bDo = true;
					//Destroy pedestrian from level
					GetOwner()->Destroy();
				}
			}
			//Reset Do Once
			else
			{
				bDo = false;
			}
		
	}
}

