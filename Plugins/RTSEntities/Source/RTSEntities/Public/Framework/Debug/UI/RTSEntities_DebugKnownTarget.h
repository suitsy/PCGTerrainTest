// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/RTSAi_DataTypes.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "RTSEntities_DebugKnownTarget.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_DebugKnownTarget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetData(const FRTSEntities_TargetData& TargetData) const;

protected:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* TargetName;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* LastSeen;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* Location;
};
