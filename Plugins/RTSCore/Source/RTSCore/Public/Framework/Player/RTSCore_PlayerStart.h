// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerStart.h"
#include "RTSCore_PlayerStart.generated.h"

UCLASS(Blueprintable, MinimalAPI)
class ARTSCore_PlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ARTSCore_PlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	bool IsClaimed() const { return ClaimingController != nullptr; }
	bool TryClaim(AController* OccupyingController);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FGenericTeamId TeamId;

	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;
};
