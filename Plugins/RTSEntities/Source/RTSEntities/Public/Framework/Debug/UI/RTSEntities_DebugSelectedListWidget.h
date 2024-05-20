// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "RTSEntities_DebugSelectedListWidget.generated.h"

class UVerticalBox;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectSelectedDelegate, const AActor*, Selected);


UCLASS()
class RTSENTITIES_API URTSEntities_DebugSelectedListWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;	
	TArray<UWidget*> GetSelectedListWidgetList() const;
	
	FOnSelectSelectedDelegate OnSelectSelected;

protected:
	UFUNCTION()
	void LoadSelectedList(FRTSEntities_PlayerSelections Selected);
	
	void ClearSelectedList();
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* SelectedList; 
};
