// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Debug/UI/RTSEntities_DebugSelectedDisplayWidget.h"
#include "Components/VerticalBox.h"
#include "Framework/AI/RTSEntities_AiControllerBase.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/UI/RTSEntities_DebugKnownTarget.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"


void URTSEntities_DebugSelectedDisplayWidget::SetDisplayData(URTSEntities_GroupDataAsset* GroupData, const FRTSEntities_PlayerSelection& InPlayerSelection)
{
	if(GroupData)
	{
		Data = GroupData;
		PlayerSelection = InPlayerSelection;
		SetText(GroupData->Callsign);

		if(PlayerSelection.Group != nullptr)
		{
			PlayerSelection.Group->Register_OnStateUpdate(FOnStateUpdateDelegate::FDelegate::CreateUObject(this, &ThisClass::OnStateUpdate));
		}
		
		CreateUpdateTimer();
	}
}

void URTSEntities_DebugSelectedDisplayWidget::ClearData()
{
	Data = nullptr;
	PlayerSelection = FRTSEntities_PlayerSelection();
	StopUpdateTimer();
}

void URTSEntities_DebugSelectedDisplayWidget::SetText(const FText& Text) const
{
	if(GroupTextBlock)
	{
		GroupTextBlock->SetText(Text);
	}
}

void URTSEntities_DebugSelectedDisplayWidget::CreateUpdateTimer()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(Handle_DisplayUpdate, this, &URTSEntities_DebugSelectedDisplayWidget::UpdateDisplay, 1.f, true);
	}
}

void URTSEntities_DebugSelectedDisplayWidget::StopUpdateTimer()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Handle_DisplayUpdate);
	}
}

void URTSEntities_DebugSelectedDisplayWidget::UpdateDisplay()
{
	if(PlayerSelection.Group)
	{
		// Reset values
		float LastSeenTime = 0;
		int32 ActiveCount = 0;		
		for (int i = 0; i < PlayerSelection.Entities.Num(); ++i)
		{
			if(PlayerSelection.Entities[i] != nullptr)
			{
				if(const APawn* AiPawn = Cast<APawn>(PlayerSelection.Entities[i]))
				{
					if(const ARTSEntities_AiControllerBase* AiController = Cast<ARTSEntities_AiControllerBase>(AiPawn->GetController()))
					{
						if(AiController->GetState(ERTSCore_StateCategory::Navigation) == static_cast<int32>(ERTSAi_NavigationState::Navigating))
						{
							ActiveCount++;
						}
						
						if(AiController->GetLastSeenTargetTime() > LastSeenTime)
						{							
							LastSeenTime = AiController->GetLastSeenTargetTime();
							LastSeenTimeTextBlock->SetText(FText::AsNumber(LastSeenTime));
						}
					}
				}
			}
		}

		if(ActiveTextBlock)
		{
			ActiveTextBlock->SetText(FText::AsNumber(ActiveCount));
		}

		if(const URTSEntities_DeveloperSettings* DeveloperSettings = GetDefault<URTSEntities_DeveloperSettings>())
		{
			if(KnownTargets != nullptr)
			{
				KnownTargets->ClearChildren();
				for (int j = 0; j < PlayerSelection.Group->KnownTargets.Num(); ++j)
				{
					if(URTSEntities_DebugKnownTarget* KnownTargetWidget = CreateWidget<URTSEntities_DebugKnownTarget>(this, DeveloperSettings->DebugKnownTargetsWidgetClass.LoadSynchronous()))
					{
						KnownTargetWidget->SetData(PlayerSelection.Group->KnownTargets[j]);
						KnownTargets->AddChildToVerticalBox(KnownTargetWidget);
					}			
				}
			}
		}
	}
}

void URTSEntities_DebugSelectedDisplayWidget::OnStateUpdate(const FRTSEntities_GroupState StateUpdate)
{
	if(BehaveStateTextBlock)
	{
		BehaveStateTextBlock->SetText(UEnum::GetDisplayValueAsText(StateUpdate.Behaviour));
	}

	if(SpeedStateTextBlock)
	{
		SpeedStateTextBlock->SetText(UEnum::GetDisplayValueAsText(StateUpdate.Speed));
	}

	if(ConditionStateTextBlock)
	{
		ConditionStateTextBlock->SetText(UEnum::GetDisplayValueAsText(StateUpdate.Condition));
	}

	if(PostureStateTextBlock)
	{
		PostureStateTextBlock->SetText(UEnum::GetDisplayValueAsText(StateUpdate.Posture));
	}

	if(NavStateTextBlock)
	{
		NavStateTextBlock->SetText(UEnum::GetDisplayValueAsText(StateUpdate.Navigation));
	}
}
