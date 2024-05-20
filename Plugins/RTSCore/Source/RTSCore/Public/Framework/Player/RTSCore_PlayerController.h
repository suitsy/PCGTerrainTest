// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "RTSCore_PlayerController.generated.h"

enum class ERTSCore_GamePhaseState : uint8;
/**
 * 
 */
UCLASS()
class RTSCORE_API ARTSCore_PlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	ARTSCore_PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaSeconds) override;
	
	/** Utility Functions **/
	void GetMousePositionOnTerrain(FVector& TerrainPosition) const;
	void GetTerrainPosition(FVector& TerrainPosition) const;
	FVector GetGridPosition(const FVector& SourceLocation, const int32 TotalPositions, const int32 Index, const float Spacing = 200.f, const int32 NumRows = 1);
	virtual void SetPlayerStartLocation(const FVector StartLocation) { PlayerStartLocation = StartLocation; }
	virtual FVector GetPlayerStartLocation() const { return PlayerStartLocation; }
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	FVector PlayerStartLocation;

	UPROPERTY()
	TArray<FVector> GroupSpawnLocation;

	UPROPERTY()
	TArray<FVector> EntitySpawnLocation;
	
	/** UI Management **/
public:
	void CreateMainMenu(const FGuid& TaskId);
	
protected:	
	UFUNCTION(Client, Reliable)
	void Client_CreateMainMenu(const FGuid& TaskId);

	/** Game Phase State Handling**/
public:
	void ReportClientGamePhaseTaskComplete(const FGuid TaskId);
	
protected:
	UFUNCTION(Server, Reliable)
	void Server_ReportClientGamePhaseTaskComplete(const FGuid TaskId);
	
};
