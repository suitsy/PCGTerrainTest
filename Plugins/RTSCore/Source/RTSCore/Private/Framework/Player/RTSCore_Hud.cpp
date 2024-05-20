// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSCore_Hud.h"
#include "PrimaryGameLayout.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/Interfaces/RTSCore_GamePhaseInterface.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Framework/UI/UIExtensionSubsystem.h"

ARTSCore_Hud::ARTSCore_Hud(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ARTSCore_Hud::BeginPlay()
{
	Super::BeginPlay();

	if(const URTSCore_DeveloperSettings* DeveloperSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		PlayerUIConfigurations = DeveloperSettings->UIConfigurations;
	}

	SyncHudGamePhase();
}

void ARTSCore_Hud::ShowMainMenu(const TSoftClassPtr<UCommonActivatableWidget>& MenuClass, const FGuid TaskId)
{
	if(MenuClass == nullptr)
	{
		UE_LOG(LogRTSCore, Error, TEXT("[%s] Invalid reference to menu class!"), *GetNameSafe(this));
		return;
	}	

	if(GetWorld())
	{
		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(GetWorld()))
		{
			RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(RTSCore_UILayerTags::TAG_UI_LAYER_MENU, true, MenuClass,
				[this, TaskId](EAsyncWidgetLayerState State, UCommonActivatableWidget* Widget)
			{
				switch (State)
					{
						case EAsyncWidgetLayerState::AfterPush:
							// Assign any UI params
							if(IRTSCore_GamePhaseInterface* PhaseInterface = Cast<IRTSCore_GamePhaseInterface>(Widget))
							{
								PhaseInterface->SetTaskId(TaskId);
							}
							return;
						case EAsyncWidgetLayerState::Canceled:
							// Perform any cleanup required
							
							return;
						default: ;
					}
			});			
		}
		else
		{
			UE_LOG(LogRTSCore, Error, TEXT("[%s] Failed to get primary game layout!"), *GetNameSafe(this));
		}
	}
}

void ARTSCore_Hud::LoadUIConfiguration(const FRTSCore_UIConfiguration& UIConfiguration) const
{
	if(!GetWorld())
	{
		return;
	}
	
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetOwningPlayerController()->Player))
	{
		if (const TSubclassOf<UCommonActivatableWidget> WidgetClass = UIConfiguration.LayoutData.LayoutClass.LoadSynchronous())
		{
			UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, UIConfiguration.LayoutData.LayerID, WidgetClass);
		}

		if(UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
		{
			for (const FRTSCore_UIWidgetData& Entry : UIConfiguration.Widgets)
			{
				ExtensionSubsystem->RegisterExtensionAsWidgetForContext(Entry.SlotID, LocalPlayer, Entry.WidgetClass.LoadSynchronous(), -1);
			}
		}
	}		
}

void ARTSCore_Hud::SyncHudGamePhase()
{
	if(GetWorld())
	{
		if(ARTSCore_GameState* CoreGameState = GetWorld()->GetGameState<ARTSCore_GameState>())
		{
			if(CoreGameState->GetGamePhaseState() == ERTSCore_GamePhaseState::Loading)
			{
				GetWorldTimerManager().SetTimerForNextTick(this, &ARTSCore_Hud::SyncHudGamePhase);
			}
			else
			{
				// Bind to game phase updates
				CoreGameState->OnGamePhaseChangedChecked().AddUObject(this, &ThisClass::OnGamePhaseChanged);

				// Set game phase
				OnGamePhaseChanged(CoreGameState->GetGamePhaseState());
			}
		}
	}
}

void ARTSCore_Hud::OnGamePhaseChanged(const ERTSCore_GamePhaseState NewState)
{
	if(GamePhaseState != NewState)
	{
		GamePhaseState = NewState;
		for (int i = 0; i < PlayerUIConfigurations.Num(); ++i)
		{
			if(!PlayerUIConfigurations[i].IsValid() || PlayerUIConfigurations[i].GamePhase != NewState)
			{
				continue;
			}

			LoadUIConfiguration(PlayerUIConfigurations[i]);								
		}
	}
}
