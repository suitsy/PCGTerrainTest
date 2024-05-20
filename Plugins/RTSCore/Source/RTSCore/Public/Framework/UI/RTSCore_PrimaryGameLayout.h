// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrimaryGameLayout.h"
#include "RTSCore_PrimaryGameLayout.generated.h"



UCLASS(Abstract)
class RTSCORE_API URTSCore_PrimaryGameLayout : public UPrimaryGameLayout
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

protected:		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonActivatableWidgetStack* Game_Stack;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonActivatableWidgetStack* GameMenu_Stack;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonActivatableWidgetStack* Menu_Stack;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonActivatableWidgetStack* Modal_Stack;
};
