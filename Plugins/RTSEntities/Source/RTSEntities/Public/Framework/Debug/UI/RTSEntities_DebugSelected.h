// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RTSEntities_DebugSelected.generated.h"

class UCommonTextBlock;
class UCommonBorder;
/**
 * 
 */
UCLASS(Abstract)
class RTSENTITIES_API URTSEntities_DebugSelected : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

protected:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* TextBlock;
	
};
