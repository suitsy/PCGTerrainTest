// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachine/MetaStates/Behaviour/Combat/RTSEntities_StateCombat.h"
#include "Data/RTSAi_DataTypes.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Framework/CoverSystem/RogueCover_Subsystem.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Query/RogueCover_EnvQueryCover.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"
#include "StateMachine/RTSEntities_GroupStateManager.h"


URTSEntities_StateCombat::URTSEntities_StateCombat()
{
}

void URTSEntities_StateCombat::Init(ARTSEntities_Group* InGroup)
{
	OwningGroup = InGroup;

	if(OwningGroup)
	{
		if(const URTSEntities_GroupDataAsset* GroupData = OwningGroup->GetData())
		{
			StateTimeout = GroupData->CombatTimeout;
		}
	}

	UE_LOG(LogRTSEntities, Warning, TEXT("[%s] Combat State Initialised"), *GetClass()->GetName());
}

void URTSEntities_StateCombat::OnEnter()
{
	StartCombat();

	if(URogueCover_Subsystem* CoverSubsystem = GetWorld()->GetSubsystem<URogueCover_Subsystem>())
	{
		if(IsNavigating())
		{
			// Check can reach destination
			if(CanReachDestination(FVector::ZeroVector))
			{
			
			}
			else
			{
				QueryCoverPositions();
			}
		}
		else
		{
			// Find cover


			// Assign cover to members
		
		}
	}
}

void URTSEntities_StateCombat::OnUpdate()
{
	Super::OnUpdate();
	
	// Current time greater than the last seen time + combat timeout then change state
	if(!HasKnownTargets())
	{
		SendTrigger(ERTSEntities_BehaviourTriggers::ToCautious);
	}
	else
	{
		// Assign group targets
		AssignCombatTargets();
	}		
}

void URTSEntities_StateCombat::OnExit()
{
	UE_LOG(LogRTSEntities, Warning, TEXT("[%s] OnExit State Combat"), *GetClass()->GetName());
}

void URTSEntities_StateCombat::StartCombat() const
{
	if(OwningGroup)
	{
		// Set group state to combat
		OwningGroup->SetState(ERTSCore_StateCategory::Behaviour, static_cast<int32>(ERTSCore_BehaviourState::Combat));

		// Notify group combat
		if(OwningGroup->GetGroupStateManager())
		{
			OwningGroup->GetGroupStateManager()->PushGroupBehaviourStateChange(ERTSCore_BehaviourState::Combat);
		}
		
		UE_LOG(LogRTSEntities, Warning, TEXT("[%s] OnEnter State Combat"), *GetClass()->GetName());
	}
}

bool URTSEntities_StateCombat::CanReachDestination(const FVector& Destination) const
{
	if(OwningGroup)
	{
		const FVector GroupLocation = OwningGroup->GetCenterPosition();
		const FVector DestinationDirection = (Destination - GroupLocation).GetSafeNormal();
		const FVector EnemyDirection = (EnemyLocation - GroupLocation).GetSafeNormal();
		const float DotProduct = FVector::DotProduct(DestinationDirection, EnemyDirection);

		return DotProduct < 0.25f;
	}

	return false;
}

void URTSEntities_StateCombat::QueryCoverPositions()
{
	if(const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>())
	{
		if(EntitiesSettings->QueryCoverPositions.LoadSynchronous() != nullptr)
		{
			const UEnvQuery* Query = Cast<UEnvQuery>(EntitiesSettings->QueryCoverPositions.LoadSynchronous());
			ERTSEntities_EQSExecute::ExecuteEnvironmentQuery(FEnvQueryRequest(Query, this), EEnvQueryRunMode::AllMatching, this, &ThisClass::OnQueryComplete_CoverPositions);
		}
	}
}

void URTSEntities_StateCombat::OnQueryComplete_CoverPositions(TSharedPtr<FEnvQueryResult> Result)
{
	TArray<URogueCover_Point*> CoverPoints;
	if (Result->IsSuccessful())
	{
		const URogueCover_EnvQueryCover* DefTypeOb = Result->ItemType->GetDefaultObject<URogueCover_EnvQueryCover>();
		check(DefTypeOb != nullptr);

		for (const FEnvQueryItem& Item : Result->Items)
		{
			URogueCover_Point* Point = DefTypeOb->GetValue(Result->RawData.GetData() + Item.DataOffset);
			Point->SetScore(Item.Score);
			CoverPoints.Add(Point);
		}
	}

	// Sort cover points by score
	CoverPoints.Sort([](const URogueCover_Point& A, const URogueCover_Point& B) {
		return A.GetScore() > B.GetScore();
	});

	CoverPoints.Num() > 0 ? AssignCoverPositions(CoverPoints) : TakeCover();	
}
