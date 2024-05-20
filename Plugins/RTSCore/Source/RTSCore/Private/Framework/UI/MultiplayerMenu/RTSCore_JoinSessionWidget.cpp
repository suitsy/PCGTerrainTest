// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_JoinSessionWidget.h"
#include "CommonBorder.h"
#include "CommonGameInstance.h"
#include "CommonListView.h"
#include "CommonSessionSubsystem.h"
#include "PrimaryGameLayout.h"
#include "Framework/Debug/RTSCore_Debug.h"
#include "Framework/UI/MultiplayerMenu/RTSCore_ButtonBase.h"

void URTSCore_JoinSessionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if(RefreshButton)
	{
		RefreshButton->OnClicked().AddUObject(this, &URTSCore_JoinSessionWidget::OnRefreshList);
	}
	
	if(BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &URTSCore_JoinSessionWidget::OnBackMenu);
	}

	SetSpinnerDisplay(false);
}

void URTSCore_JoinSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	StartSearch();	
}

UWidget* URTSCore_JoinSessionWidget::NativeGetDesiredFocusTarget() const
{
	if(ListView)
	{
		return ListView;
	}
	
	return Super::NativeGetDesiredFocusTarget();
}

void URTSCore_JoinSessionWidget::StartSearch()
{
	UE_LOG(LogRTSMultiMenu, Warning, TEXT("[%s] - Start Search"), *GetClass()->GetSuperClass()->GetName());

	if(bSearchInProgress || !GetOwningPlayer() || !GetOwningPlayer()->GetWorld() || !ListView)
	{
		UE_LOG(LogRTSMultiMenu, Warning, TEXT("[%s] - Search Failed"), *GetClass()->GetSuperClass()->GetName());
		return;
	}

	bSearchInProgress = true;
	NoSessionsDisplay->SetVisibility(ESlateVisibility::Collapsed);
	ListView->ClearListItems();
	SetSpinnerDisplay(true);

	if(const UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(GetOwningPlayer()->GetWorld()->GetGameInstance()))
	{
		if(UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>())
		{
			if(UCommonSession_SearchSessionRequest* SessionSearchRequest = SessionSubsystem->CreateOnlineSearchSessionRequest())
			{
				SessionSearchRequest->OnSearchFinished.AddUObject(this, &URTSCore_JoinSessionWidget::OnSessionSearchComplete, SessionSearchRequest);
				SessionSubsystem->FindSessions(GetOwningPlayer(), SessionSearchRequest);
			}
			else
			{
				UE_LOG(LogRTSMultiMenu, Error, TEXT("[%s] - Failed to Create Session Search"), *GetClass()->GetSuperClass()->GetName());
			}
		}
	}
	
}

void URTSCore_JoinSessionWidget::SetSpinnerDisplay(const bool bSpinnerState) const
{
	if(SpinnerDisplay)
	{
		SpinnerDisplay->SetVisibility(bSpinnerState ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void URTSCore_JoinSessionWidget::OnRefreshList()
{
	if(!bSearchInProgress)
	{
		StartSearch();
	}
}

void URTSCore_JoinSessionWidget::OnSessionSearchComplete(bool bSucceeded, const FText& ErrorMessage,
	UCommonSession_SearchSessionRequest* SessionSearchRequest)
{
	bSearchInProgress = false;
	SetSpinnerDisplay(false);
	
	if(bSucceeded && SessionSearchRequest)
	{
		if(SessionSearchRequest->Results.Num() > 0)
		{
			ListView->SetListItems(SessionSearchRequest->Results);
			ListView->SetUserFocus(GetOwningPlayer());
			NoSessionsDisplay->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			NoSessionsDisplay->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		NoSessionsDisplay->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogRTSMultiMenu, Error, TEXT("[%s] - Failed Session Search"), *GetClass()->GetSuperClass()->GetName());		
	}
}

void URTSCore_JoinSessionWidget::OnBackMenu()
{
	if(const UWorld* WorldContext = GetWorld())
	{
		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(WorldContext))
		{
			RootLayout->FindAndRemoveWidgetFromLayer(this);
		}
	}
}
