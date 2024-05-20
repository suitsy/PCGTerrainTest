// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "CommonUserTypes.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "RTSCore_SessionEntryWidget.generated.h"


class URTSCore_SessionButtonWidget;
class UCommonSession_SearchResult;


UCLASS(Abstract)
class RTSCORE_API URTSCore_SessionEntryWidget : public UCommonTabListWidgetBase, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UFUNCTION()
	void OnSessionSelected();

	UFUNCTION()
	void OnJoinSessionComplete(const FOnlineResultInformation& Result);
	
	UFUNCTION()
	void OnGameDataLoaded();
	
	UPROPERTY()
	UCommonSession_SearchResult* SessionSearchResult;	
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_SessionButtonWidget* SessionButton;

	UPROPERTY()
	FPrimaryAssetId GameDataId;

	FDelegateHandle JoinSessionHandle; 
};
