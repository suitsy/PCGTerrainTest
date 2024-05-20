// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RTSCore_DebugGamePhase.generated.h"

class UCommonTextBlock;


UCLASS(Abstract)
class RTSCORE_API URTSCore_DebugGamePhase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* GamePhaseText;
};
