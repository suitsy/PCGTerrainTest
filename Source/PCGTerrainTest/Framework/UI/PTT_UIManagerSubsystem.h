// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "PTT_UIManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PCGTERRAINTEST_API UPTT_UIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	UPTT_UIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void SwitchPolicy(UGameUIPolicy* InPolicy);

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	
	FTSTicker::FDelegateHandle TickHandle;
};
