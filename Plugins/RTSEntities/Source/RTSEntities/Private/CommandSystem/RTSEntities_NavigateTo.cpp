// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandSystem/RTSEntities_NavigateTo.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Async/Async.h"

URTSEntities_NavigateTo::URTSEntities_NavigateTo()
{	
}

void URTSEntities_NavigateTo::Execute()
{
	if(!Valid())
	{
		Fail();
		return;
	}

	if(HasAuthority())
	{
		// Binds on complete
		Super::Execute();

		if(Data.Status == ERTSEntities_CommandStatus::Active)
		{
			ExecuteNavigation();				
		}
	}
}

void URTSEntities_NavigateTo::Undo()
{
	
}

void URTSEntities_NavigateTo::Redo()
{
	
}

void URTSEntities_NavigateTo::Complete(const FGuid CompleteId, const ARTSEntities_Group* Group, const ERTSEntities_CommandStatus StatusUpdate, const FVector& Location)
{
	Super::Complete(CompleteId, Group, StatusUpdate, GetAvgDestinationLocation());

	// Update groups completed location
	for (int i = 0; i < Data.Navigation.Positions.Num(); ++i)
	{
		if(Data.Navigation.Positions[i].Selection.Group == Group)
		{
			Data.Navigation.Positions[i].Destination = Location;
		}
	}
}

/*void URTSEntities_NavigateTo::CreateNavigation(const uint8 PreviewOnly)
{	
	if(URTSEntities_FormationDataAsset* FormationData = GetFormationData())
	{
		// Get command data to be used for the formation
		const FRTSEntities_CommandData CommandData = GetCommandData(FormationData->Offset);

		// Check command data is valid
		if(CommandData.IsValid())
		{
			bNavProcessing = true;
			TWeakObjectPtr<URTSEntities_NavigateTo> NavigationCommand = this;
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [NavigationCommand, FormationData, CommandData, PreviewOnly]
			{
				const FRTSEntities_CommandData Async_CommandData = CommandData;
				const FRTSEntities_PlayerSelections Async_Selected = NavigationCommand->Selected;
				
				// Create formation for selected
				FRTSEntities_Navigation NavigationResult = FormationData->CreateFormation(Async_CommandData, Async_Selected);

				AsyncTask(ENamedThreads::GameThread, [NavigationCommand, NavigationResult, PreviewOnly]
				{
					NavigationCommand->Navigation = NavigationResult;
					NavigationCommand->OnNavigationCreated(PreviewOnly);
				});					
			});
		}
	}	
}

void URTSEntities_NavigateTo::OnNavigationCreated(const uint8 PreviewOnly)
{
	GenerateFormationReferencePaths();		
	bNavProcessing = false;
}*/

void URTSEntities_NavigateTo::ExecuteNavigation()
{
	if(!HasAuthority())
	{
		return;
	}
	
	for (int i = 0; i < Data.Selected.Selections.Num(); ++i)
	{
		if(Data.Selected.Selections[i].IsValid())
		{
			if(!Data.Selected.Selections[i].Group->AssignedCommand(Data.Id) && !Data.Selected.Selections[i].Group->HasCompletedCommand(Data.Id))
			{
				Data.Selected.Selections[i].Group->ExecuteNavigationCommand(this);
			}
		}
	}

#if WITH_EDITOR
	Debug_Formation();
#endif
}

bool URTSEntities_NavigateTo::Valid() const
{
	return Super::Valid();
}

/*URTSEntities_FormationDataAsset* URTSEntities_NavigateTo::GetFormationData()
{
	if(Selected.Selections.IsValidIndex(GetLeadSelectionIndex()) && Selected.Selections[GetLeadSelectionIndex()].Lead != nullptr)
	{
		if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Selected.Selections[GetLeadSelectionIndex()].Lead))
		{
			return EntityComponent->GetFormationData();
		}
	}

	return nullptr;
}*/

/*FRTSEntities_CommandData URTSEntities_NavigateTo::GetCommandData(const FVector& Offset)
{
	if(Selected.Selections.IsValidIndex(GetLeadSelectionIndex()) && Selected.Selections[GetLeadSelectionIndex()].Lead != nullptr)
	{
		if(const URTSEntities_Entity* EntityComponent = URTSEntities_Entity::FindEntityComponent(Selected.Selections[GetLeadSelectionIndex()].Lead))
		{
			return FRTSEntities_CommandData(
				GetLocation(),
				GetRotation(),
				EntityComponent->GetGroupSpacing(),
				EntityComponent->GetEntitySpacing(),
				Offset
			);
		}
	}

	return FRTSEntities_CommandData();
}*/

/*void URTSEntities_NavigateTo::GenerateFormationReferencePaths()
{	
	if(Navigation.IsValid())
	{
		for (int i = 0; i < Navigation.Positions.Num(); ++i)
		{
			if(Navigation.Positions[i].IsValid() && Navigation.Positions[i].Selection.IsValid() && Navigation.Positions[i].Selection.Lead != nullptr)
			{
				// Set form up point to start position 
				FVector FormUpPoint = Navigation.Positions[i].SourceLocation;				
	
				// Generate a temp guide path from group navigation data
				const UNavigationPath* GuidePath = GeneratePath(FormUpPoint, Navigation.Positions[i].Destination, Navigation.Positions[i].Selection.Lead);
				
				// Get the minimum of: 50% of the path length or the groups max form up distance
				const float HalfPathLength = GuidePath->GetPathLength() * 0.5f;
				const float MaxFormUpDist = Navigation.Positions[i].Selection.Group->GetMaxFormUpDistance();
				const float FormUpDistance = FMath::Min(HalfPathLength, MaxFormUpDist);
		
				// Interpolate along the path until we find two points that overlap the form up point
				float DistanceAlongPath = 0;
				for (int32 j = 0; j < GuidePath->PathPoints.Num() - 1; j++)
				{
					const float SegmentLength = (GuidePath->PathPoints[j + 1] - GuidePath->PathPoints[j]).Length();
					if (DistanceAlongPath + SegmentLength >= FormUpDistance)
					{
						// Segment is further than the form up point, lerp from the last point the remaining distance
						FormUpPoint = FMath::Lerp(GuidePath->PathPoints[j], GuidePath->PathPoints[j + 1], (FormUpDistance - DistanceAlongPath) / SegmentLength);
						break;
					}
					else
					{
						DistanceAlongPath += SegmentLength;
					}			
				}
				
				
				// Generate groups reference path points from group navigation data
				Navigation.Positions[i].ReferencePathPoints = GeneratePath(FormUpPoint, Navigation.Positions[i].Destination, Navigation.Positions[i].Selection.Lead)->PathPoints;				
			}
		}
	}
}*/

/*UNavigationPath* URTSEntities_NavigateTo::GeneratePath(const FVector& Start, const FVector& End, AActor* Context)
{
	if(GetWorld())
	{
		if(UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem()))
		{
			// Validate start nav position to nav mesh
			FNavLocation NavLocationStart;
			NavSystem->ProjectPointToNavigation(Start, NavLocationStart);
		
			// Validate destination nav position to nav mesh
			FNavLocation NavLocationEnd;
			NavSystem->ProjectPointToNavigation(End, NavLocationEnd);

			// Generate group path for entity offset reference
			return NavSystem->FindPathToLocationSynchronously(
				GetWorld(),
				NavLocationStart,
				NavLocationEnd,
				Context
			);
		}
	}

	return nullptr;
}*/

FVector URTSEntities_NavigateTo::GetAvgDestinationLocation()
{
	FVector AvgLocation = FVector::ZeroVector;
	if(Data.Navigation.IsValid() && Data.Navigation.Positions.Num() > 0)
	{
		
		for (int i = 0; i < Data.Navigation.Positions.Num(); ++i)
		{
			AvgLocation += Data.Navigation.Positions[i].Destination;
		}
	
		return AvgLocation /= Data.Navigation.Positions.Num();
	}

	return AvgLocation;
}

#if WITH_EDITOR
void URTSEntities_NavigateTo::Debug_Formation() const
{
	if(Data.Navigation.IsValid())
	{
		if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
		{
			if(CoreSettings->DebugRTSEntities && CoreSettings->DebugEntityFormations)
			{
				for (int i = 0; i < Data.Navigation.Positions.Num(); ++i)
				{
					DrawDebugSphere(GetWorld(), Data.Navigation.Positions[i].Destination, 50.f, 8, FColor::Silver, false, 15.f, 0, 5.f);
	
					for (int j = 0; j < Data.Navigation.Positions[i].EntityPositions.Num(); ++j)
					{
						DrawDebugSphere(GetWorld(), Data.Navigation.Positions[i].EntityPositions[j].Destination, 30.f, 8, FColor::Emerald, false, 15.f, 0, 5.f);
						//DrawDebugDirectionalArrow(GetWorld(), Formation.Positions[i].EntityPositions[j].Owner->GetActorLocation(), Formation.Positions[i].EntityPositions[j].Destination, 200.f, FColor::Emerald, false, 15.f, 0, 5.f);
					}

					for (int k = 0; k < Data.Navigation.Positions[i].ReferencePathPoints.Num(); ++k)
					{
						DrawDebugSphere(GetWorld(), Data.Navigation.Positions[i].ReferencePathPoints[k], 30.f, 8, FColor::Red, false, 10.f, 0, 5.f);
						if(Data.Navigation.Positions[i].ReferencePathPoints.IsValidIndex(k+1))
						{
							DrawDebugDirectionalArrow(GetWorld(), Data.Navigation.Positions[i].ReferencePathPoints[k], Data.Navigation.Positions[i].ReferencePathPoints[k+1], 1000.f, FColor::Red, false, 10.f, 0, 4.f);
						}
					}
				}
			}
		}
	}
}
#endif
