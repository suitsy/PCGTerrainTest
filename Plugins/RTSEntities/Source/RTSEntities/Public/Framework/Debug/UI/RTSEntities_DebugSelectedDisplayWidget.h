// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "RTSEntities_DebugSelectedDisplayWidget.generated.h"

class URTSEntities_DebugKnownTarget;
class UVerticalBox;
class URTSEntities_GroupDataAsset;
class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_DebugSelectedDisplayWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	void SetDisplayData(URTSEntities_GroupDataAsset* GroupData, const FRTSEntities_PlayerSelection& InPlayerSelection);
	void ClearData();
	
protected:
	void SetText(const FText& Text) const;
	void CreateUpdateTimer();
	void StopUpdateTimer();
	void UpdateDisplay();

	UFUNCTION()
	void OnStateUpdate(const FRTSEntities_GroupState StateUpdate);
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* GroupTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* NavStateTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* ActiveTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* BehaveStateTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* SpeedStateTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* ConditionStateTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* PostureStateTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* LastSeenTimeTextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* KnownTargets;

	UPROPERTY()
	URTSEntities_GroupDataAsset* Data;

	UPROPERTY()
	FRTSEntities_PlayerSelection PlayerSelection;

	UPROPERTY()
	FTimerHandle Handle_DisplayUpdate;
};