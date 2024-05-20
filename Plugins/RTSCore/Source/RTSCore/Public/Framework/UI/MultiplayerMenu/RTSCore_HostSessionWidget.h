// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonSessionSubsystem.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "Framework/Interfaces/RTSCore_GamePhaseInterface.h"
#include "RTSCore_HostSessionWidget.generated.h"

class URTSCore_GameDisplayListWidget;
class UCommonUserInfo;
class URTSCore_ButtonBase;
class UCommonTextBlock;


UCLASS(Abstract)
class RTSCORE_API URTSCore_HostSessionWidget : public UCommonActivatableWidget, public IRTSCore_GamePhaseInterface
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;	
	virtual void SetTaskId(const FGuid TaskId) override { HostMenuTaskId = TaskId; }

protected:
	void InitTextDisplays() const;
	bool AttemptOnlineLogin();
	void HostSession();
	UCommonSession_HostSessionRequest* CreateHostingRequest() const;

	UFUNCTION()
	void OnUserOnlineLogin(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	UFUNCTION()
	void OnSessionCreated(const FOnlineResultInformation& Result);

	UFUNCTION()
	void OnLaunchGame();

	UFUNCTION()
	void OnBackMenu();

	UFUNCTION()
	void OnGameListCreated();

	UFUNCTION()
	void OnGameSelected(const FPrimaryAssetId& SelectedGameData);
	
	UPROPERTY()
	FPrimaryAssetId GameDataId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_GameDisplayListWidget* GameList;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* SelectedGameTitleText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* LaunchButton;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* BackButton;

	/** General Settings **/
	UFUNCTION()
	void OnGameSettingDifficultyChanged();

	UFUNCTION()
	void OnGameSettingVictoryConditionChanged();
	
	UPROPERTY()
	TEnumAsByte<ERTSCore_GameSetting_Difficulty> GameSetting_Difficulty;

	UPROPERTY()
	TEnumAsByte<ERTSCore_GameSetting_VictoryCondition> GameSetting_VictoryCondition;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* GameSettingDifficultyText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* ChangeGameSettingDifficultyButton;
		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* GameSettingVictoryConditionText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* ChangeGameSettingVictoryConditionButton;

	/** NetMode Settings **/	
	UFUNCTION()
	void OnNetworkModeButtonClicked();
	
	UPROPERTY()
	ECommonSessionOnlineMode NetMode;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonTextBlock* NetworkModeText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URTSCore_ButtonBase* ChangeNetModeButton;

private:
	UPROPERTY()
	FGuid HostMenuTaskId;
};
