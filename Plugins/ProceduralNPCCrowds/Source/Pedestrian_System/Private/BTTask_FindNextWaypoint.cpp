//  Copyright (c) 2022 KomodoBit Games. All rights reserved.

#include "BTTask_FindNextWaypoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AIController.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetArrayLibrary.h"
#include "BehaviorTree/BTNode.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"




EBTNodeResult::Type UBTTask_FindNextWaypoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//Start Fvector from "Controlled Pawn" in blueprints
	const FVector Start = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation() + OwnerComp.GetAIOwner()->GetPawn()->GetActorForwardVector();

	// End FVector From "Controlled Pawn" in blueprints with parameters for tracedistance and start
	const FVector End = OwnerComp.GetAIOwner()->GetPawn()->GetActorForwardVector() * TraceDistance + Start;


	//holds no values, just used for sphere trace syntax
	TArray<AActor*> ActorsToIgnore;

	//Array of the HitResults from the objects hit
	TArray<FHitResult> HitArray;

	//Setting the object channel to use
	TArray < TEnumAsByte < EObjectTypeQuery > > nameofobjects;
	nameofobjects.Add(ObjectToUse);

	

	// Sphere Trace for multi objects with branch check for hit result
	const bool Hit = UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, End, SphereRadius, nameofobjects, false, ActorsToIgnore, DebugDrawTypes, HitArray, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
	{
		if (Hit)
		{
			// Setting the blackboard key to be the vector of the impact point vector.
			// NOTE: HitArray is getting a random index from the length of the break hit result which converts array to single.
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(NextWaypointVector.SelectedKeyName, HitArray[rand() % HitArray.Num()].ImpactPoint);
			
			
			return EBTNodeResult::Succeeded;
		}
		else
		{

			//Start wandering towards nearest PathPoint if did not hit anything with SphereTrace
			const FVector StartWander = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
			float radius = 1000;
			FVector ResultLocation;
			ANavigationData* NavData = (ANavigationData*)0;
			FSharedConstNavQueryFilter QueryFilter;
			TSubclassOf<UNavigationQueryFilter> FilterClass;

			//Get Random Reachable point in navigable radius function
			UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(GetWorld(), StartWander, ResultLocation, radius, NavData, FilterClass);

			//Set BlackBoard Value for where to go
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(FindWayPoint.SelectedKeyName, ResultLocation);

		
			return EBTNodeResult::Succeeded;
		}
			
		
				return EBTNodeResult::Succeeded;
			
	}

		
	
}




	



		

