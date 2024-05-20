// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "RTSEntities_GroupDisplay.generated.h"

class UImage;
class UTextBlock;
class ARTSEntities_Group;


UCLASS(Abstract)
class RTSENTITIES_API URTSEntities_GroupDisplay : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void Init(ARTSEntities_Group* NewOwningGroup);
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Icon;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Callsign;

protected:
	
	UPROPERTY()
	ARTSEntities_Group* OwningGroup;
};
