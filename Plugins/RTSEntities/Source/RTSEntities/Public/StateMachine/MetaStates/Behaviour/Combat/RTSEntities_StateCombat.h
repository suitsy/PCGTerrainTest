// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateMachine/RTSEntities_StateMachine.h"
#include "RTSEntities_StateCombat.generated.h"

/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_StateCombat : public URTSEntities_StateMachine
{
	GENERATED_BODY()

public:
	URTSEntities_StateCombat();
	virtual void Init(ARTSEntities_Group* InGroup) override;

protected:	
	virtual void OnEnter() override;
	virtual void OnUpdate() override;
	virtual void OnExit() override;

	void StartCombat() const;
	bool CanReachDestination(const FVector& Destination) const;

	void QueryCoverPositions();	
	void OnQueryComplete_CoverPositions(TSharedPtr<FEnvQueryResult> Result);
};
