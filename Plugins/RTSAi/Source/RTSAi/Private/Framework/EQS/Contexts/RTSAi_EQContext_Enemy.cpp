// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/EQS/Contexts/RTSAi_EQContext_Enemy.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Framework/Debug/RTSAi_Debug.h"
#include "Framework/Debug/RTSAi_TestingPawn.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#if WITH_EDITOR
#include "EngineUtils.h"
#endif

void URTSAi_EQContext_Enemy::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

#if WITH_EDITOR
	for (TActorIterator<ARTSAi_TestingPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if(const ARTSAi_TestingPawn* FoundTester = *ActorItr)
		{
			const int32 Team = FoundTester->GetTeamId();
			if(Team == 99)
			{
				UE_LOG(LogRTSAi, Warning, TEXT("[%s] Editor Test Target Assigned as Context for %s"), *GetClass()->GetName(), *QueryInstance.QueryName)
				UEnvQueryItemType_Actor::SetContextHelper(ContextData, FoundTester);
				return;
			}
		}
	}
#endif
	
	if(const IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>((Cast<AActor>((QueryInstance.Owner).Get())->GetInstigatorController())))
	{		
		//Set the context SeeingPawn to the provided context data
		UEnvQueryItemType_Point::SetContextHelper(ContextData, EntityEquipmentInterface->GetTargetLastKnownLocation());		
	}
}
