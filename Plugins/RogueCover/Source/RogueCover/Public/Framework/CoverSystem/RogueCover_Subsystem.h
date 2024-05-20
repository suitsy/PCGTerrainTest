// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Framework/Data/RogueCover_DataTypes.h"
#include "RogueCover_Subsystem.generated.h"


struct FRecastDebugGeometry;
class URogueCover_Point;
class ARecastNavMesh;
DECLARE_MULTICAST_DELEGATE(FOnCoverGenerationCompleteDelegate)


UCLASS()
class ROGUECOVER_API URogueCover_Subsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	URogueCover_Subsystem();
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void GenerateCoverPoints();
	TArray<URogueCover_Point*> GetCoverWithinBounds(const FBoxCenterAndExtent& BoundsIn) const;

	FOnCoverGenerationCompleteDelegate OnCoverGenerationComplete;

protected:
	void Generate(bool ForceRegeneration = false, bool DoAsync = false);
	ARecastNavMesh* GetNavMeshData(UWorld* World) const;
	void AnaliseMeshData(const FRecastDebugGeometry& NavMeshGeometry);
	FVector Get2DPerpVector(const FVector& v1) const;
	void TestAndAddPoint(const FVector& SegmentPoint, const FVector& SegmentDirection, UWorld* World, const FVector& CoverDirection);
	bool CoverExistWithinBounds(const FBoxCenterAndExtent& BoundsIn) const;
	FVector GetPlayerCameraLocation() const;
	URogueCover_Point* IsValidCoverPoint(UWorld* World, const FVector& SegmentPoint, const FVector& Segment, const FVector& TraceVec, const FVector& Perp) const;
	bool IsProvidingCover(UWorld* World, const FVector& SegmentPoint, const FVector& SegmentDirection, const FVector& TraceVec, const float StartHeight, const float EndHeight) const;
	bool TwoPassesCheck(UWorld* World, const FVector& SegmentPoint, const FVector& FirstTestDirection, const float& VerticalOffset, const FVector& CoverDirection) const;

#if WITH_EDITOR
	void DebugCoverPoints();
	void DrawDebugCover(UWorld* World, const FVector& StartLocation, const FVector& Direction1, const FVector& Direction2, const float& DistanceToDraw) const;
	void DrawOctreeBounds() const;
#endif
	
	UFUNCTION()
	void OnNavigationGenerationFinished(ANavigationData* NavData);
	
private:
	// Octree of all the generated cover points
	FCoverPointOctree* CoverPointOctree;

	/* Array of all cover points, used to keep GC from collecting the cp in Octree */
	UPROPERTY(Transient)
	TArray<URogueCover_Point*> AllCoverPoints;

	/* Array of all currently used as coverPoints */
	UPROPERTY(Transient)
	TArray<URogueCover_Point*> CoverPointsCurrentlyUsed;

	UPROPERTY()
	bool bIsRefreshing;

	UPROPERTY()
	bool bIsRefreshed;

	UPROPERTY()
	bool bHasGeneratedCovers; 
};
