// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RTSCore_JoinSessionWidget.generated.h"


class URTSCore_ButtonBase;
class UCommonBorder;
class UCommonListView;
class UCommonSession_SearchSessionRequest;


UCLASS(Abstract)
class RTSCORE_API URTSCore_JoinSessionWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	void StartSearch();
	void SetSpinnerDisplay(const bool bSpinnerState) const;
	
	UFUNCTION()
	void OnRefreshList();
	
	UFUNCTION()
	void OnSessionSearchComplete(bool bSucceeded, const FText& ErrorMessage, UCommonSession_SearchSessionRequest* SessionSearchRequest);

	UFUNCTION()
	void OnBackMenu();	
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonListView* ListView;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* RefreshButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* BackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonBorder* NoSessionsDisplay;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonBorder* SpinnerDisplay;

	UPROPERTY()
	bool bSearchInProgress = false;	
};
