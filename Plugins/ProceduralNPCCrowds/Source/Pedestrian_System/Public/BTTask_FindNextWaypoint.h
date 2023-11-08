//  Copyright (c) 2022 KomodoBit Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BTTask_FindNextWaypoint.generated.h"


/**
 * 
 */
UCLASS()
class PEDESTRIAN_SYSTEM_API UBTTask_FindNextWaypoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Tracing)
		float TraceDistance = 600.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Tracing)
		float SphereRadius = 300.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Tracing)
		TEnumAsByte<EDrawDebugTrace::Type> DebugDrawTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tracing)
		TEnumAsByte<EObjectTypeQuery> ObjectToUse;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
		FBlackboardKeySelector NextWaypointVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
		FBlackboardKeySelector FindWayPoint;


private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	


	
};
