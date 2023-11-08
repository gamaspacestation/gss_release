//  Copyright (c) 2022 KomodoBit Games. All rights reserved.


#include "BTTask_Wander.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AIController.h"
#include "Engine/EngineTypes.h"
#include "BehaviorTree/BTNode.h"
#include "NavigationSystem.h"


EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	const FVector Start = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	FVector ResultLocation;
	ANavigationData* NavData = (ANavigationData*)0;
	FSharedConstNavQueryFilter QueryFilter;
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(GetWorld(), Start, ResultLocation, WanderRadius, NavData, FilterClass);

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(RandomLocation.SelectedKeyName, ResultLocation);

	return EBTNodeResult::Succeeded;
}
