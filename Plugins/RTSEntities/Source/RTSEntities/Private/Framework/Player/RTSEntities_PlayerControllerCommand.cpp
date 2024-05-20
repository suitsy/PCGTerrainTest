// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSEntities_PlayerControllerCommand.h"
#include "CommandSystem/RTSEntities_Command.h"
#include "CommandSystem/RTSEntities_NavigateTo.h"
#include "Engine/AssetManager.h"
#include "Framework/AI/RTSEntities_AiControllerCommand.h"
#include "Framework/Data/RTSEntities_FormationDataAsset.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"

ARTSEntities_PlayerControllerCommand::ARTSEntities_PlayerControllerCommand(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CommandState = ECommandState::None;
	GeneratingPreview = false;
}

void ARTSEntities_PlayerControllerCommand::Command()
{
	Super::Command();

	CommandState = ECommandState::Initiated;
	AssignCommandLocation();
}

void ARTSEntities_PlayerControllerCommand::CommandHold()
{
	Super::CommandHold();

	CommandState = ECommandState::Held;
	ForcePreview();
}

void ARTSEntities_PlayerControllerCommand::CommandEnd()
{
	Super::CommandEnd();

	// Create client command data, gather client params for server command creation
	const FRTSEntities_ClientCommandData ClientCommandData = CreateCommandClientData();

	// Issue command to server for queuing
	Server_IssueCommand(ClientCommandData);	
	
	// Reset command state
	CommandState = ECommandState::None;

	// Reset preview on command execution
	ResetCommandPreview(Client_PreviewNavigation);
}

void ARTSEntities_PlayerControllerCommand::WheelMouse(const float Input)
{
	Super::WheelMouse(Input);

	if(InputModifierKey == EInputModifierKey::Alt && HasSelectedEntities())
	{
		CycleFormation(Input);		
	}
}

void ARTSEntities_PlayerControllerCommand::SetPreviewCommand()
{
	Super::SetPreviewCommand();

	if(!IsLocalController())
	{
		return;
	}

	if(bPreviewCommand)
	{
		if(CommandState == ECommandState::None)
		{
			AssignCommandLocation();
		}
		
		PreviewTimerStart();		
	}
	else
	{
		PreviewTimerStop();	
	}
}

void ARTSEntities_PlayerControllerCommand::Server_IssueCommand_Implementation(const FRTSEntities_ClientCommandData& ClientCommandData)
{
	if(HasAuthority() && ClientCommandData.IsValid())
	{
		// Create the command, set callbacks, add to command queue 
		if(URTSEntities_Command* NewCommand = CreateCommand(ClientCommandData))
		{		
			// Bind to command complete
			NewCommand->OnCommandComplete.AddUObject(this, &ThisClass::CommandCompleted);
	
			// Bind to command failed
			NewCommand->OnCommandFailed.AddUObject(this, &ThisClass::CommandFailed);

			// Set should the command be queued or override current queue
			NewCommand->SetShouldQueue(ShouldQueueCommand());

			// Add command to the command queue
			Enqueue(NewCommand);

			// Check if command has navigation
			if(NewCommand->HasNavigation())
			{
				// Delayed dequeue for calculation of navigation  
				CreateCommandNavigation(NewCommand->GetId(), NewCommand->Data);
			}
			else
			{
				// Attempt to execute the next command
				Dequeue();
			}	
		}
	}
}

FRTSEntities_ClientCommandData ARTSEntities_PlayerControllerCommand::CreateCommandClientData()
{	
	// Get the command type based on key modifiers and assign if command contains navigation
	ERTSEntities_CommandType CommandType = ERTSEntities_CommandType::Idle;
	uint8 HasNavigation = false;
	GetCommandType(CommandType, HasNavigation);

	// Get a avg source location of the command
	FVector SourceLocation = FVector::ZeroVector;
	GetCommandCenterLocation(SourceLocation);
	
	FVector MouseLocation = FVector::ZeroVector;
	GetMousePositionOnTerrain(MouseLocation);	

	// Set location using the mouse location of the command state is None
	const FVector Destination = CommandState > ECommandState::None ? CommandLocation : MouseLocation;
	
	// Get the command rotation at the target location
	// If mouse dragged set orientation to that rotation, otherwise use the direction to location
	const FRotator Rotation = (MouseLocation - Destination).Length() > 150.f
		? FRotationMatrix::MakeFromX(MouseLocation - Destination).Rotator() 
		: FRotationMatrix::MakeFromX(Destination - SourceLocation).Rotator();

	// Generate client command data for issuing command to server
	FRTSEntities_ClientCommandData ClientCommandData = FRTSEntities_ClientCommandData(CommandType, HasNavigation);	
	ClientCommandData.SetTargetLocation(Destination);
	ClientCommandData.SetTargetRotation(Rotation);
	ClientCommandData.SetSourceLocation(SourceLocation);
	
	return ClientCommandData;
}

void ARTSEntities_PlayerControllerCommand::AssignCommandLocation()
{	
	FVector MouseLocation = FVector::ZeroVector;
	GetMousePositionOnTerrain(MouseLocation);
	CommandLocation = MouseLocation;
}

URTSEntities_Command* ARTSEntities_PlayerControllerCommand::CreateCommand(const FRTSEntities_ClientCommandData& ClientCommandData)
{
	if(HasAuthority() && ClientCommandData.IsValid())
	{
		// Get reference to entities settings
		if(URTSEntities_GroupDataAsset* LeadGroupData = Selected.GetLeadGroup()->GetData())
		{
			// Get the class for the current requested command type
			if(const TSoftClassPtr<URTSEntities_Command>* LeadCommandClassPtr = LeadGroupData->Commands.Find(ClientCommandData.CommandType))
			{
				// Create a new command from requested type
				if(URTSEntities_Command* NewCommand = NewObject<URTSEntities_Command>(this, LeadCommandClassPtr->LoadSynchronous()))
				{
					FRTSEntities_CommandData CommandData = FRTSEntities_CommandData();
					UpdateCommandData(ClientCommandData, CommandData, Selected);
					NewCommand->Data = CommandData;
					
					return NewCommand;
				}
			}
		}		
	}	

	return nullptr;
}

void ARTSEntities_PlayerControllerCommand::GetCommandType(ERTSEntities_CommandType& CommandType, uint8& HasNavigation) const
{
	switch (InputModifierKey)
	{
	case EInputModifierKey::Ctrl:
		CommandType =  ERTSEntities_CommandType::NavigateTo;
		HasNavigation = true;
		break;
	case EInputModifierKey::Shift:
		CommandType =  ERTSEntities_CommandType::NavigateToFast;
		HasNavigation = true;
		break;
	case EInputModifierKey::Alt:
		CommandType =  ERTSEntities_CommandType::NavigateToSlow;
		HasNavigation = true;
		break;
		/*case EInputModifierKey::Space:
			 *CommandType =  ERTSEntities_CommandType::Cover;
			break;*/		
	case EInputModifierKey::ShiftCtrl:
		CommandType =  ERTSEntities_CommandType::NavigateToFast;	
		HasNavigation = true;		
		break;
	case EInputModifierKey::ShiftAlt:
		CommandType =  ERTSEntities_CommandType::NavigateTo;
		HasNavigation = true;
		break;
	case EInputModifierKey::ShiftSpace:
		CommandType =  ERTSEntities_CommandType::NavigateToFast;
		HasNavigation = true;
		break;
	case EInputModifierKey::CtrlAlt:
		CommandType =  ERTSEntities_CommandType::NavigateToSlow;
		HasNavigation = true;			
		break;
	case EInputModifierKey::CtrlSpace:
		CommandType =  ERTSEntities_CommandType::NavigateToSlow;
		HasNavigation = true;
		break;
	case EInputModifierKey::AltSpace:
		break;
	default:
		{
			CommandType =  ERTSEntities_CommandType::NavigateTo;
			HasNavigation = true;
		}			
	}
}

void ARTSEntities_PlayerControllerCommand::UpdateCommandData(const FRTSEntities_ClientCommandData& ClientCommandData, FRTSEntities_CommandData& CommandData,
	const FRTSEntities_PlayerSelections& InSelected, const uint8 Preview) const
{
	// Assign new command id, if preview assign invalid guid
	Preview ? CommandData.Id = FGuid() : CommandData.Id = FGuid::NewGuid();

	// Update CommandType
	CommandData.CommandType = ClientCommandData.CommandType;

	// Update HasNavigation
	CommandData.HasNavigation = ClientCommandData.HasNavigation;

	/** Filter selected based on selections that are able to do the command type
	 *  Command Data handles assignment of source data based on the selection passed in **/	
	FRTSEntities_PlayerSelections SelectedCopy = InSelected;
	ValidateSelections(SelectedCopy, CommandData.CommandType);
	CommandData.SetSelected(SelectedCopy);
	
	// Assign command target data
	CommandData.TargetTransform = ClientCommandData.TargetTransform;
	
	// Assign command target if one is under mouse cursor
	CommandData.TargetActor = GetHitSelectable(CommandData.TargetTransform.GetLocation());

	// Assign command navigation data if navigation is required
	if(CommandData.HasNavigation)
	{
		GetCommandNavigationData(CommandData, SelectedCopy);
	}
}

void ARTSEntities_PlayerControllerCommand::ValidateSelections(FRTSEntities_PlayerSelections& SelectedToValidate,
	const ERTSEntities_CommandType& Type) const
{
	// Check all groups can perform this command type, remove from command selection if can not
	for (int i = SelectedToValidate.Selections.Num() - 1; i >= 0; --i)
	{
		if(SelectedToValidate.Selections[i].IsValid())
		{
			if(URTSEntities_GroupDataAsset* GroupData = SelectedToValidate.Selections[i].Group->GetData())
			{
				const TSoftClassPtr<URTSEntities_Command>* CommandClassPtr = GroupData->Commands.Find(Type);
				if(CommandClassPtr == nullptr)
				{
					SelectedToValidate.Selections.RemoveAt(i);
				}
			}
		}
	}
}

void ARTSEntities_PlayerControllerCommand::GetCommandCenterLocation(FVector& CenterLocation) const
{
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		if(Selected.Selections[i].IsValid())
		{
			if(const AActor* GroupLead = Selected.Selections[i].Group->GetLead(Selected.Selections[i].Entities))
			{
				CenterLocation += GroupLead->GetActorLocation();
			}
		}		
	}
	
	if(Selected.Selections.Num() > 0)
	{
		CenterLocation /= Selected.Selections.Num();
	}
}

void ARTSEntities_PlayerControllerCommand::GetCommandNavigationData(FRTSEntities_CommandData& CommandData, const FRTSEntities_PlayerSelections& InSelected) const
{
	if(InSelected.Selections.IsValidIndex(CommandData.LeadSelectionIndex) && InSelected.Selections[CommandData.LeadSelectionIndex].Lead != nullptr)
	{
		if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(InSelected.Selections[CommandData.LeadSelectionIndex].Lead))
		{
			CommandData.Navigation = FRTSEntities_Navigation(
				EntityComponent->GetFormation(),
				EntityComponent->GetGroupSpacing(),
				EntityComponent->GetEntitySpacing(),
				CommandData.GetLocation(),
				CommandData.GetRotation()
			);
		}
	}
}

void ARTSEntities_PlayerControllerCommand::CreateCommandNavigation(const FGuid Id, FRTSEntities_CommandData& CommandData)
{
	if(!CommandData.IsValid() || !CommandData.Navigation.IsValid())
	{
		return;
	}		
		
	// Get command data to be used for the formation
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, Id, CommandData]
	{
		
		if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSEntities_FormationDataAsset* FormationData = Cast<URTSEntities_FormationDataAsset>(AssetManager->GetPrimaryAssetObject(CommandData.Navigation.FormationId)))
			{
				// Create navigation data for the command using the formation and command data 
				FRTSEntities_Navigation NavigationResult = CommandData.Navigation;					
				NavigationResult.Offset = FormationData->Offset;
				
				FormationData->CreateFormation(NavigationResult, CommandData.Selected.Selections);				

				AsyncTask(ENamedThreads::GameThread, [this, Id, NavigationResult]
				{					
					OnNavigationCreated(Id, NavigationResult);		
				});
			}
		}
	});	
}

void ARTSEntities_PlayerControllerCommand::OnNavigationCreated(const FGuid Id, const FRTSEntities_Navigation& Navigation)
{
	if(!Navigation.IsValid())
	{
		return;
	}
 	
	if(Id.IsValid())
	{
		for (int i = 0; i < CommandQueue.Num(); ++i)
		{
			if(CommandQueue[i]->GetId() == Id)
			{
				if(URTSEntities_NavigateTo* NavCommand = Cast<URTSEntities_NavigateTo>(CommandQueue[i]))
				{
					// Override existing navigation on command (if any)
					NavCommand->SetNavigation(Navigation);

					// Call a dequeue to attempt execution of the next command in queue
					Dequeue();
				}
			}
		}
	}
	else
	{
		Client_PreviewNavigation = Navigation;
		PreviewNavigation(Client_PreviewNavigation, true);
		GeneratingPreview = false;
	}	
}

bool ARTSEntities_PlayerControllerCommand::ShouldQueueCommand()
{
	switch (InputModifierKey)
	{
	case EInputModifierKey::Ctrl:
	case EInputModifierKey::CtrlAlt:
	case EInputModifierKey::CtrlSpace:
	case EInputModifierKey::ShiftCtrl: return true;			
	default: return false;
	}
}

void ARTSEntities_PlayerControllerCommand::Enqueue(URTSEntities_Command* NewCommand)
{	
	if(HasAuthority() && NewCommand != nullptr)
	{		
		CommandQueue.Add(NewCommand);
		NewCommand->SetQueued();
	}
}

void ARTSEntities_PlayerControllerCommand::Dequeue()
{
	if(HasAuthority())
	{
		// Check each command down the queue if it can be executed
		for (int i = 0; i < CommandQueue.Num(); ++i)
		{
			// Ensure valid command
			if(CommandQueue[i] == nullptr)
			{
				continue;
			}
			
			// Build a queue list so no command with a prior entity can be executed
			TArray<FRTSEntities_PlayerSelection> QueuedSelections;
			QueuedSelections.Append(CommandQueue[i]->GetSelections());

			// Check command can execute, execute immediately if not queued
			if(!CommandQueue[i]->ShouldQueue() || CanExecute(CommandQueue[i]->GetId(), QueuedSelections))
			{
				CommandQueue[i]->Execute();	
			}			
		}
	}
}

bool ARTSEntities_PlayerControllerCommand::CanExecute(const FGuid Id, const TArray<FRTSEntities_PlayerSelection>& QueuedSelections)
{
	for (int i = 0; i < QueuedSelections.Num(); ++i)
	{
		// Check for active command for this selection group members
		if(QueuedSelections[i].Group->HasActiveCommandFor(QueuedSelections[i].Entities))
		{
			return false;
		}				
	}

	return true;
}

void ARTSEntities_PlayerControllerCommand::CommandCompleted(const FGuid CommandId, const uint8 Success)
{
	if(HasAuthority())
	{
		// Move the completed command out of the queue
		ArchiveCommand(CommandId, Success);		

		// Attempt to execute the next command
		Dequeue();
	}
}

void ARTSEntities_PlayerControllerCommand::CommandFailed(const FGuid CommandId)
{
	CommandCompleted(CommandId, false);
}

void ARTSEntities_PlayerControllerCommand::ArchiveCommand(const FGuid Id, const uint8 Success)
{
	if(!HasAuthority())
	{
		return;
	}
	
	for (int i = CommandQueue.Num()	 - 1; i >= 0; --i)
	{
		if(CommandQueue[i] == nullptr)
		{
			continue;
		}
		
		if(CommandQueue[i]->GetId() == Id)
		{
			if(Success)
			{
				CommandQueueHistory.Add(CommandQueue[i]);
			}
			CommandQueue.RemoveAt(i);
			break;
		}
	}		
		
	// Check the size of the command history, trim oldest command if greater than command history size
	if(const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>())
	{
		if(CommandQueueHistory.Num() > EntitiesSettings->CommandHistorySize)
		{
			CommandQueueHistory.RemoveAt(0);
		}
	}	
}

void ARTSEntities_PlayerControllerCommand::PreviewTimerStart()
{
	if(IsLocalController() && !GetWorldTimerManager().IsTimerActive(TimerHandle_Preview))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Preview, this, &ThisClass::PreviewTimer, 0.15f, true);
	}
}

void ARTSEntities_PlayerControllerCommand::PreviewTimerStop()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Preview);
	PreviewNavigation(Client_PreviewNavigation, false);
}

void ARTSEntities_PlayerControllerCommand::PreviewTimer()
{
	if(IsLocalController() && !GeneratingPreview)
	{
		// Create client command data, gather client params for server command creation
		const FRTSEntities_ClientCommandData ClientCommandData = CreateCommandClientData();

		// Generate command data for the preview
		FRTSEntities_CommandData CommandData = FRTSEntities_CommandData();
		UpdateCommandData(ClientCommandData, CommandData, Selected, true);

		if(CommandData.HasNavigation)
		{
			// Calculate navigation data
			GeneratingPreview = true;
			CreateCommandNavigation(CommandData.Id, CommandData);
		}
	}
}

void ARTSEntities_PlayerControllerCommand::ResetCommandPreview(const FRTSEntities_Navigation& NavData)
{
	PreviewTimerStop();
}

void ARTSEntities_PlayerControllerCommand::PreviewNavigation(const FRTSEntities_Navigation& Navigation, const uint8 ShowPreview)
{
	if(!IsLocalController())
	{
		return;
	}
	
	for (int i = 0; i < Navigation.Positions.Num(); ++i)
	{
		for (int j = 0; j < Navigation.Positions[i].EntityPositions.Num(); ++j)
		{
			if(const APawn* AiPawn = Cast<APawn>(Navigation.Positions[i].EntityPositions[j].Owner))
			{
				if(ARTSEntities_AiControllerCommand* AiControllerCommand = Cast<ARTSEntities_AiControllerCommand>(AiPawn->GetController()))
				{
					AiControllerCommand->PreviewNavigation(ShowPreview ? Navigation.Positions[i].EntityPositions[j] : FRTSEntities_EntityPosition());						
				}
			}
		}
	}	
}
