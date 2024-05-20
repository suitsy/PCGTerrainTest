// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EQSTestingPawn.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "RTSAi_TestingPawn.generated.h"

class UAISenseConfig_Sight;
class URTSAi_DataAsset;
class UAIPerceptionStimuliSourceComponent;

UCLASS()
class RTSAI_API ARTSAi_TestingPawn : public AEQSTestingPawn, public IRTSCore_TeamInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARTSAi_TestingPawn();

	virtual bool IsInitialised() const override { return true; }
	virtual void SetInitialised() override {}
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual int32 GetTeamId() const { return GenericTeamIdToInteger(TeamId); }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Testing)
	FGenericTeamId TeamId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Testing)
	bool bDetectInGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
};
