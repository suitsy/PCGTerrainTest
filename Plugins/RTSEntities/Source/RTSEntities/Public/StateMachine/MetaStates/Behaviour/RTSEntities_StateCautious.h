// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateMachine/RTSEntities_StateMachine.h"
#include "RTSEntities_StateCautious.generated.h"

/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_StateCautious : public URTSEntities_StateMachine
{
	GENERATED_BODY()

public:
	URTSEntities_StateCautious();
	virtual void Init(ARTSEntities_Group* InGroup) override;	

protected:	
	virtual void OnEnter() override;
	virtual void OnUpdate() override;
	virtual void OnExit() override;
};
