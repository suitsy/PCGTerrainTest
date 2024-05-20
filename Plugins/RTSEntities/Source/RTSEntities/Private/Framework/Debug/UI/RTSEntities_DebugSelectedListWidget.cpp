// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Debug/UI/RTSEntities_DebugSelectedListWidget.h"
#include "Components/VerticalBox.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Debug/UI/RTSEntities_DebugSelectedDisplayWidget.h"
#include "Framework/Player/RTSEntities_PlayerController.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"

void URTSEntities_DebugSelectedListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(ARTSEntities_PlayerController* RTSPlayer = Cast<ARTSEntities_PlayerController>(GetOwningPlayer()))
	{
		RTSPlayer->OnSelectedUpdated.AddDynamic(this, &URTSEntities_DebugSelectedListWidget::LoadSelectedList);
	}
}

TArray<UWidget*> URTSEntities_DebugSelectedListWidget::GetSelectedListWidgetList() const
{
	if(SelectedList)
	{
		return SelectedList->GetAllChildren();
	}

	return TArray<UWidget*>();
}

void URTSEntities_DebugSelectedListWidget::LoadSelectedList(FRTSEntities_PlayerSelections Selected)
{
	if(SelectedList)
	{
		ClearSelectedList();		

		for (int i = 0; i < Selected.Selections.Num(); ++i)
		{
			if(const URTSEntities_DeveloperSettings* DeveloperSettings = GetDefault<URTSEntities_DeveloperSettings>())
			{
				if(DeveloperSettings->DebugSelectedDisplayWidgetClass != nullptr)
				{
					if(URTSEntities_DebugSelectedDisplayWidget* SelectedDisplay = CreateWidget<URTSEntities_DebugSelectedDisplayWidget>(GetOwningPlayer(), DeveloperSettings->DebugSelectedDisplayWidgetClass.LoadSynchronous()))
					{
						SelectedDisplay->SetDisplayData(Selected.Selections[i].Group->GetData(), Selected.Selections[i]);						
						SelectedList->AddChildToVerticalBox(SelectedDisplay);
					}
				}
			}
		}		
	}
}

void URTSEntities_DebugSelectedListWidget::ClearSelectedList()
{
	TArray<UWidget*> DisplayWidgets = GetSelectedListWidgetList();
	for (int i = 0; i < DisplayWidgets.Num(); ++i)
	{
		if(URTSEntities_DebugSelectedDisplayWidget* DisplayWidget = Cast<URTSEntities_DebugSelectedDisplayWidget>(DisplayWidgets[i]))
		{
			DisplayWidget->OnClicked().RemoveAll(DisplayWidget);
			DisplayWidget->ClearData();
		}
	}

	SelectedList->ClearChildren();
}
