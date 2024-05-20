// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_MultiplayerMainMenuWidget.h"
#include "PrimaryGameLayout.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_ButtonBase.h"
#include "Kismet/KismetSystemLibrary.h"
/** Required Includes - do not remove below **/
#include "Framework/UI/MultiplayerMenu/RTSCore_HostSessionWidget.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_JoinSessionWidget.h"


void URTSCore_MultiplayerMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if(HostButton)
	{
		HostButton->OnClicked().AddUObject(this, &URTSCore_MultiplayerMainMenuWidget::OnHostButtonClicked);
	}

	if(JoinButton)
	{
		JoinButton->OnClicked().AddUObject(this, &URTSCore_MultiplayerMainMenuWidget::OnJoinButtonClicked);
	}

	if(ExitButton)
	{
		ExitButton->OnClicked().AddUObject(this, &URTSCore_MultiplayerMainMenuWidget::OnExitButtonClicked);
	}
}

UWidget* URTSCore_MultiplayerMainMenuWidget::NativeGetDesiredFocusTarget() const
{
	if(HostButton)
	{
		return HostButton;
	}
	
	return Super::NativeGetDesiredFocusTarget();
}

void URTSCore_MultiplayerMainMenuWidget::OnHostButtonClicked()
{
	if(HostWidgetClass == nullptr)
	{
		return;
	}	

	if(const UWorld* WorldContext = GetWorld())
	{
		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(WorldContext))
		{
			RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(RTSCore_UILayerTags::TAG_UI_LAYER_MENU, true, HostWidgetClass,
				[this](EAsyncWidgetLayerState State, UCommonActivatableWidget* Widget) {
				switch (State)
				{
					case EAsyncWidgetLayerState::AfterPush:
						if(IRTSCore_GamePhaseInterface* PhaseInterface = Cast<IRTSCore_GamePhaseInterface>(Widget))
						{
							PhaseInterface->SetTaskId(MainMenuTaskId);
						}
						return;
					case EAsyncWidgetLayerState::Canceled:
						return;
					default: ;
				}
			});
		}
	}
	
}

void URTSCore_MultiplayerMainMenuWidget::OnJoinButtonClicked()
{
	if(JoinWidgetClass == nullptr)
	{
		return;
	}

	if(GetWorld())
	{
		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(GetWorld()))
		{
			RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(RTSCore_UILayerTags::TAG_UI_LAYER_MENU, true, JoinWidgetClass,
				[this](EAsyncWidgetLayerState State, UCommonActivatableWidget* Screen) {
				switch (State)
				{
					case EAsyncWidgetLayerState::AfterPush:
						return;
					case EAsyncWidgetLayerState::Canceled:
						return;
					default: ;
				}
			});
		}
	}
}

void URTSCore_MultiplayerMainMenuWidget::OnExitButtonClicked()
{
	if(GetOwningPlayer())
	{
		if(const UWorld* WorldContext = GetWorld())
		{
			UKismetSystemLibrary::QuitGame(WorldContext, GetOwningPlayer(), EQuitPreference::Quit, false);
		}
	}
}
