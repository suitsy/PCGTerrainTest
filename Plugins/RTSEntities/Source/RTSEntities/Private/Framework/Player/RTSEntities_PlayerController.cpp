// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSEntities_PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "CommandSystem/RTSEntities_Command.h"
#include "Engine/AssetManager.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/AI/RTSEntities_AIControllerCharacter.h"
#include "Framework/Data/RogueQuery.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Data/RTSCore_SystemStatics.h"
#include "Framework/Data/RTSEntities_AiDataAsset.h"
#include "Framework/Data/RTSEntities_EntityDataAsset.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Framework/Debug/RTSEntities_Debug.h"
#include "Framework/Debug/UI/RTSEntities_DebugSelected.h"
#include "Framework/Interfaces/RTSCore_EquipmentManagerInterface.h"
#include "Framework/Interfaces/RTSCore_InventoryInterface.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "Framework/Interfaces/RTSCore_TeamManager.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Player/RTSEntities_Hud.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Framework/Settings/RTSEntities_DeveloperSettings.h"
#include "Framework/System/RogueQuerySubsystem.h"
#include "Framework/UI/RTSEntities_BoxSelectWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ARTSEntities_PlayerController::ARTSEntities_PlayerController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	InputModifierKey = EInputModifierKey::NoModifier;
	Client_HitSelectable = nullptr;
	BoxSelectionWidget = nullptr;
	bIsBoxSelection = false;
	bPreviewCommand = false;
}

void ARTSEntities_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Selected, SharedParams);	
	DOREPLIFETIME(ThisClass, PlayerEntities);	
	DOREPLIFETIME(ThisClass, PlayerGroups);
	DOREPLIFETIME(ThisClass, Loadout);	
}

void ARTSEntities_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	// Debug Input
	Debug_ModifierKey();
#endif
}

void ARTSEntities_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(!BoxSelectionWidget && IsLocalController())
	{
		CreateBoxSelectionWidget();
	}

	if(HasAuthority())
	{
		if(IRTSCore_TeamInterface* TeamInterface = Cast<IRTSCore_TeamInterface>(PlayerState))
		{
			TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnTeamChanged);

			// Check is listen server client
			if(IsLocalController())
			{
				TeamInterface->SetInitialised();
			}
		}

		// Init player query data
		InitRogueQueryData();
	}
}

void ARTSEntities_PlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();		

	if(IsLocalController())
	{
		if(IRTSCore_TeamInterface* TeamInterface = Cast<IRTSCore_TeamInterface>(PlayerState))
		{
			TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnTeamChanged);
			TeamInterface->SetInitialised();
		}
	}
}

void ARTSEntities_PlayerController::Select()
{
	// Assign selection start parameters
	if(GetWorld())
	{
		// Assign select world location
		FVector StartLocation = FVector::ZeroVector;
		GetMousePositionOnTerrain(StartLocation);
		SelectStartWorldLocation = StartLocation;		

		// Update hit selectable on input
		Client_HitSelectable = GetHitSelectable(StartLocation);

		// Assign select viewport location
		SelectStartViewportLocation = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

		// Assign select screen location
		float X, Y;
		GetMousePosition(X,Y);
		SelectStartScreenLocation = FVector2D(X,Y);
	}	

	// If not adding to selection, clear existing selections
	if(InputModifierKey != EInputModifierKey::Ctrl)
	{
		ClearSelections();

		// Clear pending selections
		Client_PendingSelections.Clear();
	}
}

void ARTSEntities_PlayerController::SelectHold()
{
	if(!BoxSelectionWidget)
	{
		CreateBoxSelectionWidget();
	}

	// Check if the mouse has moved
	float DeltaX, DeltaY;
	GetInputMouseDelta(DeltaX, DeltaY);
	if(DeltaX != 0.f || DeltaY != 0.f)
	{
		// Remove single selection hit (if one)
		Client_HitSelectable = nullptr;
		
		// Perform box selection
		DrawSelectionBox();

		// Update selected entities in box
		UpdateBoxSelected();

		// Handle selection highlighting preview
		HandlePendingSelectionHighlight(true);
	}
}

void ARTSEntities_PlayerController::AssignSelectionLead()
{			
	int32 LeadSelectionIndex = 0;
	GetLeadSelectionIndex(Client_PendingSelections, LeadSelectionIndex);
	Client_PendingSelections.LeadSelectionIndex = LeadSelectionIndex;	
}

void ARTSEntities_PlayerController::SelectEnd()
{
	// Check if select is a box or single selection
	if(bIsBoxSelection)
	{
		// End the box selection
		EndDrawSelectionBox();

		// Handle box selection			
		HandleLocalSelection();

		// Assign this selections lead
		AssignSelectionLead();
		
		// Send box selection to server			
		Server_HandleSelection(Client_PendingSelections);
	}
	else
	{		
		// Handle a single selection on client
		HandleSingleSelection();		

		// Handle single selection				
		HandleLocalSelection();	

		// Assign this selections lead
		AssignSelectionLead();

		// Send box selection to server		
		Server_HandleSelection(Client_PendingSelections);
	}
}

void ARTSEntities_PlayerController::Command()
{
	// Initialise command, begin command previewing
	//SetCommandInitiated();
}

void ARTSEntities_PlayerController::CommandHold()
{
	// Update command preview while command is held and entities are selected
	/*if(HasSelectedEntities())
	{
		UpdateCommandPreview();
	}*/
}

void ARTSEntities_PlayerController::CommandEnd()
{
	// On command input release, if we have selected entities initiate the command
	/*if(HasSelectedEntities())
	{
		InitiateCommand();
	}*/

	// Reset command initiated
	//bCommandInitiated = false;
}

void ARTSEntities_PlayerController::DoubleTap()
{
	// If not adding to selection, clear existing selections
	if(InputModifierKey != EInputModifierKey::Ctrl)
	{
		ClearSelections();

		// Clear pending selections
		Client_PendingSelections.Clear();
	}

	// Update hit selectable on input
	FVector MouseLocation = FVector::ZeroVector;
	GetMousePositionOnTerrain(MouseLocation);
	Client_HitSelectable = GetHitSelectable(MouseLocation);
	
	// Get the entity class from the entity component
	TArray<AActor*> EntitiesToSelect;
	if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Client_HitSelectable))
	{			
		// Get the entity class 
		if(const TSubclassOf<APawn> EntityClass = Entity->GetEntityClass())
		{
			// Check each of the player's entities if on viewport and of same class			
			for (int i = 0; i < PlayerEntities.Num(); ++i)
			{
				FVector2D ScreenLocation;
				if (UGameplayStatics::ProjectWorldToScreen(this, PlayerEntities[i]->GetActorLocation(), ScreenLocation))
				{
					if (ScreenLocation.X >= 0 && ScreenLocation.X <= GEngine->GameViewport->Viewport->GetSizeXY().X &&
						ScreenLocation.Y >= 0 && ScreenLocation.Y <= GEngine->GameViewport->Viewport->GetSizeXY().Y &&
						PlayerEntities[i]->IsA(EntityClass))
					{
						// Entity is visible in the viewport
						EntitiesToSelect.Add(PlayerEntities[i]);
						DrawDebugSphere(GetWorld(), PlayerEntities[i]->GetActorLocation(), 100.f, 8, FColor::Emerald, false, 5.f, 0, 5.f);
					}
				}
			}
		}
	}

	// Populate client pending selections array
	CreateGroupSelections(EntitiesToSelect, Client_PendingSelections.Selections);

	// Handle single selection				
	HandleLocalSelection();	

	// Assign this selections lead
	AssignSelectionLead();

	// Send box selection to server		
	Server_HandleSelection(Client_PendingSelections);
}

void ARTSEntities_PlayerController::SetModifier(const EInputModifierKey NewModifier)
{
	InputModifierKey = NewModifier;
	
	/*if(ShouldPreviewCommand())
	{		
		StartCommandPreview();
	}
	else
	{
		// Stop previewing the command if we are already
		StopCommandPreview();
	}*/
}

void ARTSEntities_PlayerController::WheelMouse(const float Input)
{
	
}

void ARTSEntities_PlayerController::SetPreviewCommand()
{
	if(!IsLocalController())
	{
		return;
	}
	
	bPreviewCommand = InputModifierKey == EInputModifierKey::Space && Selected.Selections.Num() > 0;
}

TArray<AActor*> ARTSEntities_PlayerController::GetAllSelectedEntities()
{
	TArray<AActor*> SelectedEntities;
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		SelectedEntities.Append(Selected.Selections[i].Entities);
	}

	return SelectedEntities;
}

bool ARTSEntities_PlayerController::IsEntitySelected(AActor* Entity)
{
	if(Entity != nullptr)
	{
		for (int i = 0; i < Selected.Selections.Num(); ++i)
		{
			if(Selected.Selections[i].Entities.Contains(Entity))
			{
				return true;
			}
		}
	}
	
	return false;
}

AActor* ARTSEntities_PlayerController::GetSelectionLead(TArray<AActor*> Members)
{
	int32 LeaderIndex = 99;
	AActor* NewLeader = nullptr;
	for (int i = 0; i < Members.Num(); ++i)
	{
		if(const URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Members[i]))
		{
			if(Entity->GetIndex() < LeaderIndex)
			{
				NewLeader = Members[i];
				LeaderIndex = Entity->GetIndex();
			}
		}		
	}

	return NewLeader;
}

AActor* ARTSEntities_PlayerController::GetHitSelectable(const FVector& Location) const
{
	FHitResult OutHit;
	FVector TraceOrigin = Location;
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = Location;
	TraceEnd.Z -= 10000.f;
	
	if(GetWorld())
	{
		if(GetWorld()->LineTraceSingleByChannel(OutHit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_SELECTABLE))
		{
			if(OutHit.bBlockingHit)
			{
				// Check if we hit an actor and the actor has an entity component
				if(OutHit.GetActor() != nullptr && URTSEntities_Entity::FindEntityComponent(OutHit.GetActor()))
				{
					return OutHit.GetActor();
				}
			}
		}
	}
	
	return nullptr;
}

void ARTSEntities_PlayerController::CreateBoxSelectionWidget()
{
	if(const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>())
	{
		if(EntitiesSettings->BoxSelectionClass.LoadSynchronous())
		{
			BoxSelectionWidget = CreateWidget<URTSEntities_BoxSelectWidget>(this, EntitiesSettings->BoxSelectionClass.LoadSynchronous());
			if(BoxSelectionWidget)
			{
				BoxSelectionWidget->AddToViewport();
			}
		}
	}
}

void ARTSEntities_PlayerController::UpdateBoxSelected()
{	
	// Create the box selection planes
	TArray<FPlane> Planes;
	CreateBoxSelectionPlanes(Planes);	

	// Empty pending selection list
	TArray<AActor*> Client_PendingBoxSelectedEntities;

	// Update selection list to entities inside the box
	UpdatePendingBoxSelectionEntities(Planes, Client_PendingBoxSelectedEntities);

	// Create new pending selections array
	TArray<FRTSEntities_PlayerSelection> NewPendingSelections;	
		
	// Create group selections from boxed entities
	CreateGroupSelections(Client_PendingBoxSelectedEntities, NewPendingSelections);

	// Update pending box selections into valid group selections
	UpdatePendingBoxSelection(NewPendingSelections);		
}

void ARTSEntities_PlayerController::CreateBoxSelectionPlanes(TArray<FPlane>& Planes) const
{
	if(GetPawn())
	{
		if(const UCameraComponent* Camera = GetPawn()->GetComponentByClass<UCameraComponent>())
		{
			// Get the current box selection end points
			float X, Y;
			GetMousePosition(X,Y);
			const FVector2D EndScreenLocation = FVector2D(X,Y);
		 
			FVector LeftTop, LeftTopDir;
			DeprojectScreenPositionToWorld(FMath::Min(SelectStartScreenLocation.X, EndScreenLocation.X), FMath::Min(SelectStartScreenLocation.Y, EndScreenLocation.Y), LeftTop, LeftTopDir);				

			FVector RightTop, RightTopDir;
			DeprojectScreenPositionToWorld(FMath::Max(SelectStartScreenLocation.X, EndScreenLocation.X), FMath::Min(SelectStartScreenLocation.Y, EndScreenLocation.Y), RightTop, RightTopDir);

			FVector LeftBottom, LeftBottomDir;
			DeprojectScreenPositionToWorld(FMath::Min(SelectStartScreenLocation.X, EndScreenLocation.X), FMath::Max(SelectStartScreenLocation.Y, EndScreenLocation.Y), LeftBottom, LeftBottomDir);

			FVector RightBottom, RightBottomDir;
			DeprojectScreenPositionToWorld(FMath::Max(SelectStartScreenLocation.X, EndScreenLocation.X), FMath::Max(SelectStartScreenLocation.Y, EndScreenLocation.Y), RightBottom, RightBottomDir);
		
			Planes.Add(FPlane(LeftBottom,LeftTop, Camera->GetComponentLocation()));
			Planes.Add(FPlane(LeftTop,RightTop, Camera->GetComponentLocation()));
			Planes.Add(FPlane(RightTop,RightBottom, Camera->GetComponentLocation()));
			Planes.Add(FPlane(RightBottom,LeftBottom, Camera->GetComponentLocation()));
		}
	}
}

void ARTSEntities_PlayerController::UpdatePendingBoxSelectionEntities(TArray<FPlane>& Planes, TArray<AActor*>& Client_PendingBoxSelectedEntities) const
{	
	// Check players selectable entity list for selections within the selection box
	for (AActor* Entity : PlayerEntities )
	{
		bool bInside = true;

		// Check entity is within each plane (box selection) frustum
		for(int i = 0; i < 4; i++)
		{
			if(Planes[i].PlaneDot(Entity->GetActorLocation()) >= 0)
			{
				// Not inside selection, remove from box selection (In case previously added somehow?)
				Client_PendingBoxSelectedEntities.Remove(Entity);
				bInside = false;
				break;
			}						
		}

		// Add entity within selection box
		if(bInside)
		{
			Client_PendingBoxSelectedEntities.Add(Entity);
		}
	}
}

void ARTSEntities_PlayerController::CreateGroupSelections(TArray<AActor*>& Client_PendingBoxSelectedEntities, TArray<FRTSEntities_PlayerSelection>& NewPendingSelections)
{
	const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>();
	if(!Settings)
	{
		return;
	}
	
	// From the pending box selected entities, build list of unique groups for this tick to be considered for selection 	
	for (int i = 0; i < Client_PendingBoxSelectedEntities.Num(); ++i)
	{
		// Cast to entity interface
		if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Client_PendingBoxSelectedEntities[i]))
		{
			// Check if the group has been create for this entity
			const bool bGroupAdded = NewPendingSelections.Contains(Entity->GetGroup());

			// List for selection members
			TArray<AActor*> SelectionMembers;
						
			// Assign selection members based on selection type
			switch (InputModifierKey)
			{
				case EInputModifierKey::Ctrl:
					break;
				case EInputModifierKey::Shift:
					{
						if(!bGroupAdded)
						{
							if(Entity->GetGroup())
							{
								Entity->GetGroup()->GetEntities(SelectionMembers);
							}
						}
					}
					break;
				case EInputModifierKey::Alt:
				{
					// Add subgroup members to selection if not group only selections
					if(!Settings->bAllowGroupSelectionOnly)
					{
						Entity->GetSubGroupMembers(SelectionMembers);
						
						if(bGroupAdded)
						{
							// Add the members to the existing group if existing
							for (int j = 0; j < NewPendingSelections.Num(); ++j)
							{
								// Find the entity's group
								if(NewPendingSelections[j].Group == Entity->GetGroup())
								{
									for (int k = 0; k < SelectionMembers.Num(); ++k)
									{
										NewPendingSelections[j].Entities.AddUnique(SelectionMembers[k]);
									}																	
								}
							}
						}
					}
				}
					break;
				case EInputModifierKey::Space:
					break;					
				default:
				{
					// Check if individual entity selection is enabled
					if(Settings->bAllowGroupSelectionOnly && !bGroupAdded)
					{
						Entity->GetGroup()->GetEntities(SelectionMembers);
					}
					else
					{
						if(bGroupAdded)
						{
							// Add the member to the existing group if individual selection enabled
							for (int j = 0; j < NewPendingSelections.Num(); ++j)
							{
								// Find the entity's group
								if(NewPendingSelections[j].Group == Entity->GetGroup())
								{
									// Add the entity to the existing new pending selection
									NewPendingSelections[j].Entities.AddUnique(Client_PendingBoxSelectedEntities[i]);
									break;
								}
							}
						}
						else
						{							
							SelectionMembers.Add(Client_PendingBoxSelectedEntities[i]);
						}
					}
				}
			}
			
			if(!bGroupAdded)
			{
				// Group has not been added, create the selection with the entity as the member
				
				FRTSEntities_PlayerSelection NewSelection = Entity->CreatePlayerSelection(SelectionMembers);
				if(NewSelection.IsValid())
				{					
					NewPendingSelections.Add(NewSelection);
				}				
			}
		}		
	}
}

void ARTSEntities_PlayerController::UpdatePendingBoxSelection(TArray<FRTSEntities_PlayerSelection>& NewPendingSelections)
{
	const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>();
	if(!Settings)
	{
		return;
	}
	
	/**
	 *  Compare the NewPendingSelections to the previous ticks pending selections (Client_PendingSelections)
	 *	The Client_PendingSelections have had their highlight updated and need client action to un-highlight
	 *	NewPendingSelections will contain only the new (additional) selections after this loop
	 *  **/
	TArray<FRTSEntities_PlayerSelection> RemainPendingList;
	for (int i = NewPendingSelections.Num() - 1; i >= 0; --i)
	{
		bool bRemove = false;
		
		// Check pending list for group
		for (int j = Client_PendingSelections.Selections.Num() - 1; j >= 0; --j)
		{
			// Check if the group was already pending selection
			if(Client_PendingSelections.Selections[j].Group == NewPendingSelections[i].Group)
			{
				// If group is found add to remain pending 
				RemainPendingList.Add(Client_PendingSelections.Selections[j]);
				
				// Check if partial group selections are allowed
				if(Settings->bAllowGroupSelectionOnly)
				{
					// If group only, remove group from current pending
					Client_PendingSelections.Selections.RemoveAt(j);
					bRemove = true;
					
				}
				else
				{
					// Partial group selections are allowed
					
					// Get the index of the current remain pending group
					const int32 RemainPendingGroupIndex = RemainPendingList.Num() - 1;

					// Check we have a valid remain pending group index
					if(RemainPendingList.IsValidIndex(RemainPendingGroupIndex))
					{
						// Clear the members list on the last added remain pending group
						RemainPendingList[RemainPendingGroupIndex].Entities.Empty();

						// Add new pending selection entities to remain pending selection
						for (int k = NewPendingSelections[i].Entities.Num() - 1; k >= 0; --k)
						{
							// Add to remain pending
							RemainPendingList[RemainPendingGroupIndex].Entities.Add(NewPendingSelections[i].Entities[k]);

							// Remove member from current pending
							Client_PendingSelections.Selections[j].Entities.Remove(NewPendingSelections[i].Entities[k]);

							// Remove member from new pending
							NewPendingSelections[i].Entities.RemoveAt(k);
						}

						// Check the existing pending group has no members to be un selected
						if(Client_PendingSelections.Selections[j].Entities.Num() == 0)
						{
							Client_PendingSelections.Selections.RemoveAt(j);
						}

						// Check the new pending group has no members pending selection
						if(NewPendingSelections[i].Entities.Num() == 0)
						{
							bRemove = true;
						}
					}
				}

				// We found the group and can break out of checking rest of existing groups for match
				break;				
				
				// If the selection is not for full groups, check if subgroup is pending
				/*if(BoxSelectionType != 0)
				{
					// If the sub group id matches skip adding group
					if(Client_PendingSelections[k].SubGroupId == NewPendingSelections[j].SubGroupId)
					{
						RemainPendingList.Add(Client_PendingSelections[k]);
						Client_PendingSelections.RemoveAt(k);
						bRemove = true;
						break;
					}
				}
				else
				{
					
				}*/
			}
		}

		// If group added to remain pending remove from the new pending selections
		if(bRemove)
		{
			NewPendingSelections.RemoveAt(i);
		}
	}

	// The remaining groups in the pending list are no longer pending selections and need to be deselected / un-highlighted 
	HandlePendingSelectionHighlight(false);

	// Clear pending selections
	Client_PendingSelections.Clear();

	// Add the remain pending groups back to the client pending list
	Client_PendingSelections.Selections = RemainPendingList;

	// Add the new pending groups to the client pending list
	Client_PendingSelections.Selections.Append(NewPendingSelections);

	int32 NumEntities = 0;
	for (int l = 0; l < Client_PendingSelections.Selections.Num(); ++l)
	{
		NumEntities += Client_PendingSelections.Selections[l].Entities.Num();
	}

	/** Due to allowing subgroups to be selected we need to check for duplicate groups in the client pending selections list,
	 *  if both subgroups exist they need to be merged into a GroupALL selection **/
	CheckForDuplicateSelections();	
}

void ARTSEntities_PlayerController::HandlePendingSelectionHighlight(const bool bHighlight)
{
	// Highlight the pending selections
	for (int i = 0; i < Client_PendingSelections.Selections.Num(); ++i)
	{
		// Update highlight for each group member
		for (int j = 0; j < Client_PendingSelections.Selections[i].Entities.Num(); ++j)
		{
			if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Client_PendingSelections.Selections[i].Entities[j]))
			{
				Entity->Highlight(this, bHighlight);
			}
		}
	}
}

void ARTSEntities_PlayerController::HighlightGroupSelection(const FRTSEntities_PlayerSelection& Selection, const bool bHighlight)
{
	// Update highlight for each group member
	for (int i = 0; i < Selection.Entities.Num(); ++i)
	{
		if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Selection.Entities[i]))
		{
			Entity->Highlight(this, bHighlight);
		}
	}
}

void ARTSEntities_PlayerController::CheckForDuplicateSelections()
{
	TArray<FRTSEntities_PlayerSelection> CheckedGroupList;
	TSet<ARTSEntities_Group*> CheckedGroupsSet;
	for (int n = Client_PendingSelections.Selections.Num() - 1; n >= 0; --n)
	{
		// Check if the group is not already in CheckedGroupsSet
		if (!CheckedGroupsSet.Contains(Client_PendingSelections.Selections[n].Group))
		{
			// Group not in CheckedGroupList, add and continue to the next check
			CheckedGroupList.Add(Client_PendingSelections.Selections[n]);
			CheckedGroupsSet.Add(Client_PendingSelections.Selections[n].Group);
			continue;
		}
		
		// If the group is already in the CheckGroupList
		/*for (int i = CheckedGroupList.Num() - 1; i >= 0; --i)
		{
			// Get a referenced to the group in CheckGroupList
			if(CheckedGroupList[i].Group == Client_PendingSelections[n].Group)
			{
				// Check if either of the groups is a GroupAll
				if(CheckedGroupList[i].SubGroupId == ESubGroupId::GroupAll || Client_PendingSelections[n].SubGroupId == ESubGroupId::GroupAll)
				{
					// If one of the groups is a GroupAll we need to remove the subgroup
					if(CheckedGroupList[i].SubGroupId != ESubGroupId::GroupAll)
					{
						CheckedGroupList.RemoveAt(i);
					}
					
					if(Client_PendingSelections[n].SubGroupId != ESubGroupId::GroupAll)
					{
						Client_PendingSelections.RemoveAt(n);
					}
				}
				else
				{
					// Neither are a GroupAll, check if SubGroupId is different
					if (CheckedGroupList[i].SubGroupId != Client_PendingSelections[n].SubGroupId)
					{
						// If the SubGroupId is different we have a full group, need to update this group selection to a GroupAll
						TArray<AActor*> GroupMembers;
						CheckedGroupList[i].Group->GetMembersBySubgroupId(GroupMembers, ESubGroupId::GroupAll);
						CheckedGroupList[i].Members = GroupMembers;
						CheckedGroupList[i].SubGroupId = ESubGroupId::GroupAll;
						CheckedGroupList[i].Lead = GetSelectionLead(GroupMembers);
					}
					else
					{
						// SubGroupId is the same we some how have a duplicate, skip adding this selection to checked
					}
				}					
			}
		}*/
	}
	
	// Empty remaining selections and add the checked selections to pending
	Client_PendingSelections.Clear();
	Client_PendingSelections.Selections = CheckedGroupList;
}

void ARTSEntities_PlayerController::ClearSelections()
{
	// Clear local selected effects, highlight, decals etc
	ClearSelectedEffects();
	HandlePendingSelectionHighlight(false);
	SetPreviewCommand();

	// Clear actual selected list
	Server_ClearSelections();
}

void ARTSEntities_PlayerController::DrawSelectionBox()
{
	if(UWorld* WorldContext = GetWorld())
	{		
		if(BoxSelectionWidget)
		{
			bIsBoxSelection = true;
			BoxSelectionWidget->DrawSelectionBox(bIsBoxSelection, SelectStartViewportLocation, UWidgetLayoutLibrary::GetMousePositionOnViewport(WorldContext));			
		}		
	}
}

void ARTSEntities_PlayerController::EndDrawSelectionBox()
{
	if(BoxSelectionWidget)
	{
		bIsBoxSelection = false;
		BoxSelectionWidget->DrawSelectionBox(bIsBoxSelection, FVector2D::ZeroVector, FVector2D::ZeroVector);
	}	
}

void ARTSEntities_PlayerController::HandleSingleSelection()
{
	// Check if the single select hit something selectable
	if(Client_HitSelectable != nullptr)
	{
		// Cast to the entity interface
		if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Client_HitSelectable))
		{
			// Check the player owns the hit selectable
			if(Entity->IsOwningPlayer(this))
			{
				// List for selection members
				TArray<AActor*> SelectionMembers;
				
				// Check for modifier - switch single selection to a group or subgroup selection
				switch (InputModifierKey)
				{
					case EInputModifierKey::Ctrl:
						{
							//Client_PendingSelections.Add(Entity->CreatePlayerSelection(Client_HitSelectable));
						}
						break;
					case EInputModifierKey::Shift:
						{
							// Assign selection members
							Entity->GetGroup()->GetEntities(SelectionMembers);
							
							// Select hit actors group								
							Client_PendingSelections.Selections.Add(Entity->CreatePlayerSelection(SelectionMembers));							
						}
						break;
					case EInputModifierKey::Alt:
						{
							// Assign selection members
							Entity->GetSubGroupMembers(SelectionMembers);
							
							// Select hit actors sub group
							Client_PendingSelections.Selections.Add(Entity->CreatePlayerSelection(SelectionMembers));
						}
						break;
					case EInputModifierKey::Space: {}
						break;
					default:
					{
						// Assign selection members
						SelectionMembers.Add(Client_HitSelectable);

						// Select hit actor only
						Client_PendingSelections.Selections.Add(Entity->CreatePlayerSelection(SelectionMembers));
					}
						
				}
			}	
		}
	}
	else
	{
		// If not adding to selection, clear existing selections
		if(InputModifierKey != EInputModifierKey::Ctrl)
		{
			ClearSelections();
		}
	}
}

void ARTSEntities_PlayerController::HandleLocalSelection()
{
	if(!IsLocalController())
	{
		return;
	}
	
	// Perform client side selection of all entities in pending selections
	for (int i = 0; i < Client_PendingSelections.Selections.Num(); ++i)
	{
		for (int j = 0; j < Client_PendingSelections.Selections[i].Entities.Num(); ++j)
		{
			if(URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Client_PendingSelections.Selections[i].Entities[j]))
			{
				EntityComponent->Select(this);
				EntityComponent->Highlight(this);
			}
		}	
	}
}

void ARTSEntities_PlayerController::ClearSelectedEffects()
{	
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		for (int j = 0; j < Selected.Selections[i].Entities.Num(); ++j)
		{
			if(URTSEntities_Entity* Entity = URTSEntities_Entity::FindEntityComponent(Selected.Selections[i].Entities[j]))
			{
				Entity->Select(this, false);				
			}
		}
	}
}

void ARTSEntities_PlayerController::GetLeadSelectionIndex(const FRTSEntities_PlayerSelections& CommandSelections, int32& LeadSelectionIndex) const
{
	// Calculate center position
	const FVector SourcePosition = GetSelectionCenterPosition(CommandSelections);
		
	// Get the group lead closest to the center of the selection
	if(CommandSelections.Selections.Num() > 1)
	{
		float Distance = MAX_FLT;
		for (int j = 0; j < CommandSelections.Selections.Num(); ++j)
		{
			const float SelectionDistance = FVector::DistSquared(CommandSelections.Selections[j].Lead->GetActorLocation(), SourcePosition);
			if(SelectionDistance < Distance)
			{
				Distance = SelectionDistance;
				LeadSelectionIndex = j;
			}
		}
	}
}

FVector ARTSEntities_PlayerController::GetSelectionCenterPosition(const FRTSEntities_PlayerSelections& Selected)
{
	FVector CenterPosition = FVector::ZeroVector;
	for (int i = 0; i < Selected.Selections.Num(); ++i)
	{
		CenterPosition += Selected.Selections[i].GetSelectionCenterPosition();
	}
	
	if(Selected.Selections.Num() > 0)
	{
		CenterPosition /= Selected.Selections.Num();
	}

	return CenterPosition;
}

void ARTSEntities_PlayerController::CycleFormation(const float Input)
{
	// Check if the lead selection is not valid, re-assign one if invalid
	if(!Selected.Selections.IsValidIndex(Selected.LeadSelectionIndex))
	{
		int32 NewLeadSelectionIndex = -1;
		GetLeadSelectionIndex(Selected, NewLeadSelectionIndex);
		Selected.LeadSelectionIndex = NewLeadSelectionIndex;
	}

	if(Selected.Selections.IsValidIndex(Selected.LeadSelectionIndex))
	{
		// Get the lead groups next formation
		const FPrimaryAssetId NewFormation = Selected.GetNextFormation(Input);

		if(NewFormation.IsValid())
		{
			for (int i = 0; i < Selected.Selections.Num(); ++i)
			{
				if(Selected.Selections[i].IsValid() && Selected.Selections[i].Group != nullptr)
				{
					Selected.Selections[i].Group->SetEntitiesFormation(Selected.Selections[i].Entities, NewFormation);
				}
			}
		}
	}
}

void ARTSEntities_PlayerController::Server_ClearSelections_Implementation()
{
	if(Selected.Selections.Num() > 0)
	{		
		Selected.Clear();		
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Selected, this);
	}
}

void ARTSEntities_PlayerController::Server_HandleSelection_Implementation(const FRTSEntities_PlayerSelections& ReceivedSelections)
{
	if(!HasAuthority())
	{
		return;
	}
	
	// Add selections to the selections array and replicate
	Selected = ReceivedSelections;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Selected, this);
}

void ARTSEntities_PlayerController::OnRep_Selections()
{
	// Broadcast selected changed
	OnSelectedUpdated.Broadcast(Selected);	

	SetPreviewCommand();
}

void ARTSEntities_PlayerController::OnRep_PlayerGroups()
{

	if(IsLocalPlayerController())
	{
		for (int i = 0; i < PlayerGroups.Num(); ++i)
		{			
			if(PlayerGroups[i] != nullptr)
			{
				PlayerGroups[i]->CreateGroupDisplay();			
			}
		}
	}
}

void ARTSEntities_PlayerController::CreateGroups(const TArray<FPrimaryAssetId>& InGroupAssetIds)
{
	if(!HasAuthority())
	{
		return;
	}

	// Assign group assets
	GroupAssetIds = InGroupAssetIds;
	
	// Generate group start locations
	GenerateGroupSpawnLocations();

	// Begin spawning player groups
	for (int i = 0; i < GroupAssetIds.Num(); ++i)
	{
		if(GroupAssetIds[i].IsValid())
		{
			CreateGroup(GroupAssetIds[i], i);			
		}
	}

	OnPlayerEntitiesLoaded.Broadcast(this);
}

void ARTSEntities_PlayerController::CreateGroupsEQS(const TArray<FPrimaryAssetId>& InGroupAssetIds)
{
	if(!HasAuthority())
	{
		return;
	}

	// Assign group assets
	GroupAssetIds = InGroupAssetIds;

	// Generate group start locations
	GenerateGroupSpawnLocations();

	// Begin spawning player groups
	for (int i = 0; i < GroupAssetIds.Num(); ++i)
	{
		if(GroupAssetIds[i].IsValid())
		{
			CreateGroupEQS(GroupAssetIds[i], i);
			CreateGroupEntitiesEQS(PlayerGroups[i], i);
		}
	}

	OnPlayerEntitiesLoaded.Broadcast(this);	
}

void ARTSEntities_PlayerController::CreateGroupEQS(const FPrimaryAssetId& GroupAssetId, const int32 GroupSpawnIndex)
{
	if(!HasAuthority())
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = GetPawn();
	SpawnParams.Owner = this;
	if(UWorld* WorldContext = GetWorld())
	{
		FTransform Position = FTransform::Identity;
		if(GroupSpawnLocation.IsValidIndex(GroupSpawnIndex))
		{
			Position.SetLocation(GroupSpawnLocation[GroupSpawnIndex]);
		}
		
		if(ARTSEntities_Group* NewGroup = WorldContext->SpawnActor<ARTSEntities_Group>(ARTSEntities_Group::StaticClass(), Position, SpawnParams))
		{
			PlayerGroups.Add(NewGroup);
			NewGroup->Initialise(this, GroupAssetId);
			OnRep_PlayerGroups();
		}
	}
}

void ARTSEntities_PlayerController::CreateGroupEntitiesEQS(ARTSEntities_Group* Group, const int32 GroupSpawnIndex)
{
	const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if(!Group || !HasAuthority() || !AssetManager)
	{
		return;
	}
	
	// Generate groups entity spawn locations
	if(const URTSEntities_GroupDataAsset* GroupData = Group->GetData())
	{		
		// Get groups spacing area requirements
		FVector2D SpacingArea = FVector2D();					
		GetGroupSpawnSpaceRequirement(GroupData, GroupData->Members, SpacingArea);
		
		if(const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>())
		{
			if(URogueQuery* Query = Cast<URogueQuery>(AssetManager->GetPrimaryAssetObject(Settings->FormationPositionsQuery)))
			{
				if(URogueQuerySubsystem* RogueQuerySubsystem = GetWorld()->GetSubsystem<URogueQuerySubsystem>())
				{
					FRogueQueryCustomData CustomData = FRogueQueryCustomData(true);
					CustomData.SpaceBetween = SpacingArea.X;
					CustomData.NumPositions = GroupData->Members.Num() + 1;
					const FRogueQueryRequest QueryRequest = FRogueQueryRequest(Group, Query, GroupSpawnLocation[GroupSpawnIndex], CustomData);					
					RogueQuerySubsystem->Query(QueryRequest, FRogueQueryFinishedSignature::CreateUObject(this, &ThisClass::OnQueryEntityPositions));
				}
			}
		}
	}
}

void ARTSEntities_PlayerController::OnQueryEntityPositions(const FRogueQueryInstance& Result)
{	
	if(!HasAuthority() || !Result.IsSuccessful())
	{
		return;
	}

	ARTSEntities_Group* Group = Cast<ARTSEntities_Group>(Result.Owner.Get());
	const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>();
	if(!AssetManager || !EntitiesSettings || !Group)
	{
		return;
	}

	URTSEntities_GroupDataAsset* GroupData = Group->GetData();
	if(!GroupData || Result.Items.Num() < GroupData->Members.Num())
	{
		return;
	} 
	
	TArray<AActor*> GroupEntities;			
	for (int i = 0; i < GroupData->Members.Num(); ++i)
	{
		if(!GroupData->Members[i].EntityData.IsValid() || !Result.Items.IsValidIndex(i))
		{
			continue;
		}
		
		// Get the data class from the asset id for this entity
		if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(GroupData->Members[i].EntityData)))
		{
			FTransform Position{};				
			//EntityLocation.Z += 100.f; // Lift up slightly, generally half capsule size					
			Position.SetLocation(Result.Items[i].Location);
			//DrawDebugSphere(GetWorld(), Result.Items[i].Location, 50.f, 8, FColor::Black, false, 7.f, 0, 1.f);
		
			// Get the entity type from the entities settings 
			if(const TSoftClassPtr<APawn>* EntityClassPtr = EntitiesSettings->EntitiesMap.Find(EntityData->EntityClass))
			{
				// Check received a valid soft class pointer
				if (EntityClassPtr)
				{
					// Spawn entity
					AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(EntityClassPtr->LoadSynchronous(), Position, this, GetPawn(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
					if(NewActor)
					{
						FRTSEntities_InitialisationData EntityInitData = FRTSEntities_InitialisationData(
							NewActor,
							GroupData->Members[i],
							EntityData->bUseDecal,
							EntityData->DecalSizeModifier,
							Group,
							GroupData->DefaultFormation
						);
						if(GroupData->Members[i].Index == -1)
						{
							EntityInitData.EntityConfigData.Index = i;
						}
					
						CreateEntityComponents(EntityInitData);
						AssignEntityTeam(NewActor);								
						AssignAIController(NewActor, EntityData);
						GroupEntities.Add(NewActor);

						// Remove spawn position
						//ResultItems.RemoveAt(ResultItems.Num() - 1);

						// Finish spawning the character
						UGameplayStatics::FinishSpawningActor(NewActor, Position);
						PlayerEntities.Add(NewActor);
					}
				}			
			}	
		}		

		// Initialise group data
		Group->AssignEntities(GroupEntities);	
	}
}

void ARTSEntities_PlayerController::GenerateGroupSpawnLocations()
{
	GroupSpawnLocation.Empty();
	TArray<float> CurrentGroupSpawnSpacingX;
	CurrentGroupSpawnSpacingX.Init(0.f, 2);

	// Begin calculating each groups location
	for (int CurrentGroupIndex = 0; CurrentGroupIndex < GroupAssetIds.Num(); ++CurrentGroupIndex)
	{
		if(GroupAssetIds[CurrentGroupIndex].IsValid())
		{
			if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
			{
				if(const URTSEntities_GroupDataAsset* GroupData = Cast<URTSEntities_GroupDataAsset>(AssetManager->GetPrimaryAssetObject(GroupAssetIds[CurrentGroupIndex])))
				{
					// Get groups spacing area requirements
					FVector2D SpacingArea = FVector2D();
					
					GetGroupSpawnSpaceRequirement(GroupData, GroupData->Members, SpacingArea);

					// Update group spawn location for first group
					if(CurrentGroupIndex == 0)
					{
						GroupSpawnLocation.Add(PlayerStartLocation);

						// Adjust start offset to group area spacing X in both directions
						if(CurrentGroupSpawnSpacingX.IsValidIndex(0))
						{
							// Up
							CurrentGroupSpawnSpacingX[0] += SpacingArea.X;
						}

						if(CurrentGroupSpawnSpacingX.IsValidIndex(1))
						{
							// Down
							CurrentGroupSpawnSpacingX[1] += -SpacingArea.X;
						}
						
						continue;
					}

					// Update entity spawn locations for remaining entities alternative left and right
					const float Iteration = CurrentGroupIndex % 2;
					const int32 OffsetIndex = FMath::CeilToInt32(Iteration);
					if(CurrentGroupSpawnSpacingX.IsValidIndex(OffsetIndex))
					{
						// Add entity spacing to current spacing for this offset
						CurrentGroupSpawnSpacingX[OffsetIndex] += OffsetIndex == 0 ? SpacingArea.X : -SpacingArea.X;

						// Create new location using the new entity offset on the X
						FVector GroupLocation = PlayerStartLocation;
						GroupLocation.X += CurrentGroupSpawnSpacingX[OffsetIndex];					

						// Add new location
						GroupSpawnLocation.Add(GroupLocation);
						CurrentGroupSpawnSpacingX[OffsetIndex] += OffsetIndex == 0 ? SpacingArea.X : -SpacingArea.X;
					}	 			
				}
			}
		}
	}
}

void ARTSEntities_PlayerController::CreateGroup(const FPrimaryAssetId& GroupAssetId, const int32 GroupSpawnIndex)
{
	if(!HasAuthority())
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = GetPawn();
	SpawnParams.Owner = this;
	if(UWorld* WorldContext = GetWorld())
	{
		if(ARTSEntities_Group* NewGroup = WorldContext->SpawnActor<ARTSEntities_Group>(ARTSEntities_Group::StaticClass(), FTransform::Identity, SpawnParams))
		{
			PlayerGroups.Add(NewGroup);
			NewGroup->Initialise(this, GroupAssetId);
			CreateGroupEntities(GroupAssetId, NewGroup, GroupSpawnIndex);
			OnRep_PlayerGroups();
		}
	}
}

void ARTSEntities_PlayerController::CreateGroupEntities(const FPrimaryAssetId& GroupAssetId, ARTSEntities_Group* NewGroup, const int32 GroupSpawnIndex)
{
	const URTSEntities_DeveloperSettings* EntitiesSettings = GetDefault<URTSEntities_DeveloperSettings>();
	if(!EntitiesSettings || !HasAuthority())
	{
		return;
	}
	
	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(URTSEntities_GroupDataAsset* GroupData = Cast<URTSEntities_GroupDataAsset>(AssetManager->GetPrimaryAssetObject(GroupAssetId)))
		{
			TArray<AActor*> GroupEntities;
			GenerateEntitySpawnLocation(GroupData, GroupSpawnIndex);
			
			for (int i = 0; i < GroupData->Members.Num(); ++i)
			{				
				if(!GroupData->Members[i].EntityData.IsValid() || !EntitySpawnLocation.IsValidIndex(i))
				{
					continue;
				}

				// Get the data class from the asset id for this entity
				if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(GroupData->Members[i].EntityData)))
				{
					FTransform Position{};				
					//EntityLocation.Z += 100.f; // Lift up slightly, generally half capsule size					
					Position.SetLocation(EntitySpawnLocation[i]);			
					
					// Get the entity type from the entities settings 
					if(const TSoftClassPtr<APawn>* EntityClassPtr = EntitiesSettings->EntitiesMap.Find(EntityData->EntityClass))
					{
						// Check received a valid soft class pointer
						if (EntityClassPtr)
						{
							// Spawn entity
							AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(EntityClassPtr->LoadSynchronous(), Position, this, GetPawn(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
							if(NewActor)
							{
								FRTSEntities_InitialisationData EntityInitData = FRTSEntities_InitialisationData(
									NewActor,
									GroupData->Members[i],
									EntityData->bUseDecal,
									EntityData->DecalSizeModifier,
									NewGroup,
									GroupData->DefaultFormation
								);
								if(GroupData->Members[i].Index == -1)
								{
									EntityInitData.EntityConfigData.Index = i;
								}
								
								CreateEntityComponents(EntityInitData);
								AssignEntityTeam(NewActor);								
								AssignAIController(NewActor, EntityData);
								GroupEntities.Add(NewActor);
							}

							// Finish spawning the character
							UGameplayStatics::FinishSpawningActor(NewActor, Position);
							PlayerEntities.Add(NewActor);
						}
						
					}	
				}
			}

			// Initialise group data
			NewGroup->AssignEntities(GroupEntities);	
		}
	}
}

void ARTSEntities_PlayerController::GenerateEntitySpawnLocations()
{
	
}

void ARTSEntities_PlayerController::GenerateEntitySpawnLocation(const URTSEntities_GroupDataAsset* GroupData, const int32 GroupSpawnIndex)
{
	EntitySpawnLocation.Empty();
	TArray<float> CurrentEntitySpawnSpacing;
	CurrentEntitySpawnSpacing.Init(0.f, 2);
	FVector StartGroupSpawnLocation = GroupSpawnLocation.IsValidIndex(GroupSpawnIndex) ? GroupSpawnLocation[GroupSpawnIndex] : PlayerStartLocation;
	
	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		for (int MemberIndex = 0; MemberIndex < GroupData->Members.Num(); ++MemberIndex)
		{
			if(!GroupData->Members[MemberIndex].EntityData.IsValid())
			{
				continue;
			}

			// Get the data class from the asset id for this entity
			if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(GroupData->Members[MemberIndex].EntityData)))
			{
				// Assign first member to the center position of the group spawn location
				if(MemberIndex == 0)
				{
					URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), StartGroupSpawnLocation);
					EntitySpawnLocation.Add(StartGroupSpawnLocation);					
					continue;
				}

				// Update entity spawn locations for remaining entities alternative left and right
				const float Iteration = MemberIndex % 2;
				const int32 OffsetIndex = FMath::CeilToInt32(Iteration);
				if(CurrentEntitySpawnSpacing.IsValidIndex(OffsetIndex))
				{
					// Add entity spacing to current spacing for this offset
					CurrentEntitySpawnSpacing[OffsetIndex] += OffsetIndex == 0 ? EntityData->Spacing + 500.f * (Iteration + MemberIndex) : -EntityData->Spacing + 500.f * (Iteration + MemberIndex);

					// Create new location using the new entity offset on the X
					FVector EntityLocation = StartGroupSpawnLocation;
					EntityLocation.Y += CurrentEntitySpawnSpacing[OffsetIndex];					

					// Add new location
					URTSCore_SystemStatics::GetTerrainPosition(GetWorld(), EntityLocation);
					EntitySpawnLocation.Add(EntityLocation);
				}				
			}
		}
	}
}

void ARTSEntities_PlayerController::GetGroupSpawnSpaceRequirement(const URTSEntities_GroupDataAsset* GroupData, const TArray<FRTSEntities_EntityConfigData>& EntityConfigData, FVector2D& AreaSpacing) const
{
	AreaSpacing.Y = 0.f;	
	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{			
		AreaSpacing.X = 0.f;
		for (int i = 0; i < EntityConfigData.Num(); ++i)
		{
			// Get the data class from the asset id for this entity	
			if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(EntityConfigData[i].EntityData)))
			{
				AreaSpacing.Y += EntityData->Spacing + GroupData->DefaultEntitySpacing;

				// Assign largest spacing requirement to to area spacing X value
				if(EntityData->Spacing + GroupData->DefaultEntitySpacing > AreaSpacing.X)
				{
					AreaSpacing.X = EntityData->Spacing + GroupData->DefaultEntitySpacing;
				}
			}
		}

		if(AreaSpacing.X < 25.f)
		{
			AreaSpacing.X = EntityConfigData.Num() * 0.5f * 125.f;
		}
	}
}

void ARTSEntities_PlayerController::CreateEntityComponents(const FRTSEntities_InitialisationData& EntityInitData)
{
	if(!EntityInitData.IsValid() || !HasAuthority())
	{
		return;
	}
	
	if(URTSEntities_Entity* EntityComponent = NewObject<URTSEntities_Entity>(EntityInitData.Entity, TEXT("EntityComponent")))
	{
		//EntityInitData.Entity->AddComponent(FName(TEXT("EntityComponent")), false,  FTransform::Identity, EntityComponent);
		EntityComponent->Initialise(this, EntityInitData);
		EntityComponent->RegisterComponent();		
		EntityInitData.Entity->AddInstanceComponent(EntityComponent);
	}

	if(EntityInitData.bUseDecal)
	{		
		if(URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(EntityInitData.Entity))
		{
			EntityComponent->CreateDecalComponent();
		}
	}
	
	CreateEquipmentComponent(EntityInitData);
}

void ARTSEntities_PlayerController::CreateEquipmentComponent(const FRTSEntities_InitialisationData& EntityInitData)
{
	if(HasEquipmentManagerInterface())
	{			
		EquipmentManagerInterface.GetInterface()->AddEquipmentComponent(this, EntityInitData.Entity);

		if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSEntities_EntityDataAsset* EntityData = Cast<URTSEntities_EntityDataAsset>(AssetManager->GetPrimaryAssetObject(EntityInitData.EntityConfigData.EntityData)))
			{
				if(IRTSCore_InventoryInterface* EquipmentInterface = Cast<IRTSCore_InventoryInterface>(EntityInitData.Entity->FindComponentByInterface<URTSCore_InventoryInterface>()))
				{					
					EquipmentInterface->SetEquipmentData(this, EntityData->Equipment);					
				}
			}
		}
	}
}

bool ARTSEntities_PlayerController::HasEquipmentManagerInterface()
{
	if(EquipmentManagerInterface.GetInterface() == nullptr)
	{
		if(const UWorld* WorldContext = GetWorld())
		{
			if(const AGameStateBase* GameState = WorldContext->GetGameState<AGameStateBase>())	
			{
				TArray<UActorComponent*> Components = GameState->GetComponentsByInterface(URTSCore_EquipmentManagerInterface::StaticClass());
				if(Components.IsValidIndex(0))
				{
					EquipmentManagerInterface.SetObject(Components[0]); // Set the object implementing the interface
					EquipmentManagerInterface.SetInterface(Cast<IRTSCore_EquipmentManagerInterface>(Components[0]));
				}
			}
		}
	}

	return EquipmentManagerInterface.GetInterface() != nullptr;
}

void ARTSEntities_PlayerController::AssignEntityTeam(AActor* Entity)
{
	if(!HasAuthority())
	{
		return;
	}
	
	if(URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Entity))
	{
		if(const IRTSCore_TeamInterface* TeamInterface = Cast<IRTSCore_TeamInterface>(PlayerState))
		{
			EntityComponent->SetGenericTeamId(TeamInterface->GetGenericTeamId());
		}
	}
}

void ARTSEntities_PlayerController::AssignAIController(AActor* Entity, const URTSEntities_EntityDataAsset* EntityData) const
{
	if(!GetWorld() || !EntityData || !EntityData->AiDataAssetId.IsValid())
	{
		return;
	}
	
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{		
		if(const URTSEntities_AiDataAsset* AiData = Cast<URTSEntities_AiDataAsset>(AssetManager->GetPrimaryAssetObject(EntityData->AiDataAssetId)))
		{
			if(AiData->AiControllerClass.LoadSynchronous() != nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Instigator = GetPawn();
				SpawnParams.Owner = GetPawn();			
	
				if(ARTSEntities_AiControllerBase* AIController = AIController = GetWorld()->SpawnActor<ARTSEntities_AiControllerBase>(AiData->AiControllerClass.LoadSynchronous(), FTransform::Identity, SpawnParams))
				{
					if(APawn* EntityPawn = Cast<APawn>(Entity))
					{
						// Set ai data 
						AIController->SetAiData(EntityData->AiDataAssetId);		

						// Posses the pawn with ai controller
						AIController->Possess(EntityPawn);
					}
				}
			}
		}		
	}	
}

bool ARTSEntities_PlayerController::HasTeamsManagerInterface()
{
	if(TeamsManagerInterface.GetInterface() == nullptr)
	{
		if(const UWorld* WorldContext = GetWorld())
		{
			if(const AGameStateBase* GameState = WorldContext->GetGameState<AGameStateBase>())	
			{
				TArray<UActorComponent*> Components = GameState->GetComponentsByInterface(URTSCore_TeamManagerInterface::StaticClass());
				if(Components.IsValidIndex(0))
				{
					TeamsManagerInterface.SetObject(Components[0]); // Set the object implementing the interface
					TeamsManagerInterface.SetInterface(Cast<IRTSCore_TeamManagerInterface>(Components[0]));
				}
			}
		}
	}

	return TeamsManagerInterface.GetInterface() != nullptr;
}

bool ARTSEntities_PlayerController::HasTeamsInitiated() const
{
	if(const IRTSCore_TeamInterface* TeamInterface = Cast<IRTSCore_TeamInterface>(PlayerState))
	{
		return TeamInterface->IsInitialised();
	}

	return false;
}

void ARTSEntities_PlayerController::OnTeamChanged(UObject* ObjectChangingTeam, uint8 OldTeamId, uint8 NewTeamId)
{
	// Configure team related items
	UE_LOG(LogRTSEntities, Log, TEXT("[%s] changed team to team index %d"), *GetPathName(this), NewTeamId);

	if(HasTeamsManagerInterface())
	{
		if(IsLocalController() && BoxSelectionWidget != nullptr)
		{			
			if(UMaterialInstanceDynamic* MaterialInstance = BoxSelectionWidget->GetDynamicMaterial())
			{
				TeamsManagerInterface.GetInterface()->ApplyTeamDataToMaterial(NewTeamId, MaterialInstance);	
			}						
		}
	}
}


void ARTSEntities_PlayerController::InitRogueQueryData()
{
	if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		AllQueryAssets.Empty();
		const FPrimaryAssetType FormationAssetType(RTS_DATA_ASSET_TYPE_QUERYDATA);	
		AssetManager->GetPrimaryAssetIdList(FormationAssetType, AllQueryAssets);

		if(AllQueryAssets.Num() > 0)
		{
			AssetManager->LoadPrimaryAssets(AllQueryAssets);
		}
		else
		{
			UE_LOG(LogRTSEntities, Error, TEXT("[%s] - Failed to Get Query Assets"), *GetClass()->GetSuperClass()->GetName());
		}
	}
}

#if WITH_EDITOR

void ARTSEntities_PlayerController::Debug_MousePosition(const FVector& MouseLocation) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSEntities && GetWorld())
		{
			if(const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>())
			{
				//DrawDebugSphere(GetWorld(), MouseLocation, 40.f, 8, FColor::Silver, false, Settings->PreviewInterval);
			}
		}
	}
}

void ARTSEntities_PlayerController::Debug_CommandDestination() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSEntities && GetWorld())
		{
			//DrawDebugSphere(GetWorld(), CommandTargetWorldLocation, 40.f, 8, FColor::Black, false, 7.f);
		}
	}
}

void ARTSEntities_PlayerController::Debug_CommandPreviewRotation(const FVector& MouseLocation) const
{
	/*if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSEntities && GetWorld() && bCommandInitiated)
		{		
			if(const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>())
			{
				FVector StartArrow = PreviewCommandData.CommandNavData.DestinationLocation;
				FVector EndArrow = MouseLocation;
				StartArrow.Z += 100.f;
				EndArrow.Z += 100.f;
				DrawDebugDirectionalArrow(GetWorld(), StartArrow, EndArrow, 2000.f, FColor::Silver, false, Settings->PreviewInterval, 0, 10.f);
			}
		}
	}*/
}

void ARTSEntities_PlayerController::Debug_ModifierKey() const
{	
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSPlayer && CoreSettings->DebugPlayerInput)
		{
			RTSENTITIES_PRINT_TICK(FColor::Cyan, FString::Printf(TEXT("Modifier Key: %s"), *UEnum::GetValueAsString(InputModifierKey)));
		}
	}	
}

void ARTSEntities_PlayerController::Client_Debug_Selected_Implementation() const
{
	if(!IsLocalController())
	{
		return;
	}
	
	if(Selected.Selections.Num() > 0)
	{
		if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
		{
			if(CoreSettings->DebugRTSEntities)
			{
				if(const URTSEntities_DeveloperSettings* Settings = GetDefault<URTSEntities_DeveloperSettings>())
				{
					if(Settings->DebugSelectedWidgetClass)
					{
						if(ARTSEntities_Hud* RTSHud = Cast<ARTSEntities_Hud>(GetHUD()))
						{
							RTSHud->ShowSelectedDebug(Settings->DebugSelectedWidgetClass);
						}
					}
				}
			}
		}
	}
}

#endif