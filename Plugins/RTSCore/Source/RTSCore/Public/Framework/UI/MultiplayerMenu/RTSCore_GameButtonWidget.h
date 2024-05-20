// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSCore_ButtonBase.h"
#include "RTSCore_GameButtonWidget.generated.h"


UCLASS(Abstract)
class RTSCORE_API URTSCore_GameButtonWidget : public URTSCore_ButtonBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetGameData(const FPrimaryAssetId& GameDataAssetId);
	void UpdateButtonText(const FText& GameMapText, const FText& GameNameText, const FText& GameDescText);
	
protected:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* Name;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* Desc;
};
