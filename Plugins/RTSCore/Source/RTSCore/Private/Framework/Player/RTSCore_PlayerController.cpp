// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSCore_PlayerController.h"
#include "GameUIManagerSubsystem.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Framework/Player/RTSCore_Hud.h"
#include "Framework/Data/RTSCore_GameDataAsset.h"


ARTSCore_PlayerController::ARTSCore_PlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerStartLocation = FVector::ZeroVector;
}

void ARTSCore_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ARTSCore_PlayerController::BeginPlay()
{
	Super::BeginPlay();	
}

void ARTSCore_PlayerController::CreateMainMenu(const FGuid& TaskId)
{
	Client_CreateMainMenu(TaskId);
}

void ARTSCore_PlayerController::Client_CreateMainMenu_Implementation(const FGuid& TaskId)
{
	if(!IsLocalController() || !GetWorld())
	{
		return;
	}
	
	// Ensure we have a reference to the game state
	if(const ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
	{
		// Pass the menu class for this game mode to the hud for display
		if(ARTSCore_Hud* RTSHud = Cast<ARTSCore_Hud>(GetHUD()))
		{
			if(RTSGameState->GetGameData())
			{
				RTSHud->ShowMainMenu(RTSGameState->GetGameData()->MultiplayerMenuClass, TaskId);
			}
		}		
	}
}

void ARTSCore_PlayerController::ReportClientGamePhaseTaskComplete(const FGuid TaskId)
{
	Server_ReportClientGamePhaseTaskComplete(TaskId);
}

void ARTSCore_PlayerController::Server_ReportClientGamePhaseTaskComplete_Implementation(const FGuid TaskId)
{
	// Check we have a reference to the world
	if(HasAuthority() && GetWorld())
	{
		// Ensure we have a reference to the game state
		if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
		{
			// Send task complete registration to game state
			RTSGameState->RegistrationPhaseTaskComplete(TaskId);			
		}
	}
}


void ARTSCore_PlayerController::GetMousePositionOnTerrain(FVector& TerrainPosition) const
{
	FVector WorldLocation, WorldDirection;
	DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	FHitResult OutHit;
	if(const UWorld* WorldContext = GetWorld())
	{
		if(WorldContext->LineTraceSingleByChannel(OutHit, WorldLocation, WorldLocation + (WorldDirection * 100000.f), RTS_TRACE_CHANNEL_TERRAIN))
		{
			if(OutHit.bBlockingHit)
			{
				TerrainPosition = OutHit.Location;
			}
		}
	}
}

void ARTSCore_PlayerController::GetTerrainPosition(FVector& TerrainPosition) const
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	FVector TraceOrigin = TerrainPosition;
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = TerrainPosition;
	TraceEnd.Z -= 10000.f;

	/**
	 * Ensure a Terrain trace channel is added to DefaultEngine.ini and the Terrain is set to block this channel
	 * +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Terrain")
	 */
	
	if(GetWorld())
	{
		if(GetWorld()->LineTraceSingleByChannel(Hit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_TERRAIN, CollisionParams))
		{
			TerrainPosition = Hit.ImpactPoint;
		}
	}
}

FVector ARTSCore_PlayerController::GetGridPosition(const FVector& SourceLocation, const int32 TotalPositions, const int32 Index, const float Spacing, const int32 NumRows)
{
	// Handle the error - number of rows zero
	if (NumRows == 0 || Index == 0)
	{		
		return SourceLocation;
	}
	
	const int32 NumColumns = FMath::CeilToInt(static_cast<float>(TotalPositions) / static_cast<float>(NumRows));

	// Calculate row and column indices from the position index
	const int32 Row = Index / NumColumns;
	const int32 Col = Index % NumColumns;
	
	// Calculate an offset from the source spawn position
	const float OffsetX = (Col - (NumColumns - 1) * 0.5f) * Spacing;
	const float OffsetY = (Row - (NumRows - 1) * 0.5f) * Spacing;
	FVector GridLocation = SourceLocation + FVector(OffsetX, OffsetY, 0.0f);
	GetTerrainPosition(GridLocation);

	return GridLocation;
}
