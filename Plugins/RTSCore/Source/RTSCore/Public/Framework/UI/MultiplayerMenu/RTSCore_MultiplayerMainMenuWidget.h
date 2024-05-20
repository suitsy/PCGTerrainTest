// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Framework/Interfaces/RTSCore_GamePhaseInterface.h"
#include "RTSCore_MultiplayerMainMenuWidget.generated.h"


class URTSCore_ButtonBase;
class URTSCore_JoinSessionWidget;
class URTSCore_HostSessionWidget;

UCLASS(Abstract)
class RTSCORE_API URTSCore_MultiplayerMainMenuWidget : public UCommonActivatableWidget, public IRTSCore_GamePhaseInterface
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual void SetTaskId(const FGuid TaskId) override { MainMenuTaskId = TaskId; }

protected:
	UFUNCTION()
	void OnHostButtonClicked();
	
	UFUNCTION()
	void OnJoinButtonClicked();
	
	UFUNCTION()
	void OnExitButtonClicked();	

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* HostButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* JoinButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* ExitButton;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSoftClassPtr<URTSCore_HostSessionWidget> HostWidgetClass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSoftClassPtr<URTSCore_JoinSessionWidget> JoinWidgetClass;

private:
	UPROPERTY()
	FGuid MainMenuTaskId;
};
