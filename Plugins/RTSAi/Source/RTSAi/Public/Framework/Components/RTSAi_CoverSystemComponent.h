// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Components/RTSCore_GameStateComponent.h"
#include "RTSAi_CoverSystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnCoverGeneratedDelegate);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSAI_API URTSAi_CoverSystemComponent : public URTSCore_GameStateComponent
{
	GENERATED_BODY()

public:
	URTSAi_CoverSystemComponent(const FObjectInitializer& ObjectInitializer);
	void GenerateCover() const;

	FOnCoverGeneratedDelegate OnCoverGenerated;

protected:
	virtual void BeginPlay() override;
	void InitialiseCoverSystem();
};
