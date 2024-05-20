// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "RTSAi_GameState.generated.h"

class URTSAi_CoverSystemComponent;


UCLASS(Abstract)
class RTSAI_API ARTSAi_GameState : public ARTSCore_GameState
{
	GENERATED_BODY()

public:
	ARTSAi_GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	void GenerateCover();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	URTSAi_CoverSystemComponent* CoverSystem;

private:
	UPROPERTY()
	FGuid GenerateCoverTaskId;
};
