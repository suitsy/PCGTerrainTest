// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_PathingGrid.h"
#include "EnvQueryGenerator_SafeGroupLocations.generated.h"

class UNavigationSystemV1;
/**
 * 
 */
UCLASS()
class RTSAI_API UEnvQueryGenerator_SafeGroupLocations : public UEnvQueryGenerator_PathingGrid
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_SafeGroupLocations(const FObjectInitializer& ObjectInitializer);
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
	
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> Context;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue OffsetX;	

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue OffsetY;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderIntValue PositionsAmount;

	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FEnvDirection Direction;

	UPROPERTY(EditDefaultsOnly, Category=Pathfinding)
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	
	
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FEnvTraceData TraceData;
	
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	FAIDataProviderFloatValue MaxTraceDistance;

protected:
	void GenerateGridPoints(TArray<FNavLocation>& GridPoints, FEnvQueryInstance& QueryInstance, const float Spacing, const int32 ItemCount) const;
	void GetNearestGridPoint(FNavLocation& Point, TArray<FNavLocation>& GridPoints) const;
	void GenerateSourcePoint(FNavLocation& Point, const FVector& ContextPosition, const FRotator& Rotation, const int PosIndex, FVector LocalOffset, FVector2D& Spacings, const float PointSpacing, const bool bIsOddCount) const;
	bool TraceToPosition(FEnvQueryInstance& QueryInstance, const FVector& ContextPosition, const FVector& TargetPosition) const;
	void GetPathPointAtDistance(FNavLocation& Point, FEnvQueryInstance& QueryInstance, const FVector& ContextPosition, const int PosIndex, const float PathDistance) const;

	DECLARE_DELEGATE_RetVal_SevenParams(bool, FRunTraceSignature, const FVector&, const FVector&, AActor*, UWorld*, enum ECollisionChannel, const FCollisionQueryParams&, const FVector&);

	bool RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent);
	bool RunBoxTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent);
	bool RunSphereTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent);
	bool RunCapsuleTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent);

	/** Helper functions for accessing navigation data */
	ANavigationData* FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner) const;
	static FSharedConstNavQueryFilter GetQueryFilter(const ANavigationData& NavData, TSubclassOf<UNavigationQueryFilter> FilterClass);
	static FSharedConstNavQueryFilter GetQueryFilter(const ANavigationData& NavData, const UObject* Querier, TSubclassOf<UNavigationQueryFilter> FilterClass);

	
	/*void CreateSourcePositions(TArray<FNavLocation>& PositionsArray, FEnvQueryInstance& QueryInstance,
		const FVector& SourcePosition, const FRotator& Rotation) const;
	void ValidateSourcePoints(TArray<FNavLocation>& SourcePoints, TArray<FNavLocation>& GridPoints,	FEnvQueryInstance& QueryInstance, const FVector& SourcePosition) const;
	void GenerateSourcePoints(TArray<FNavLocation>& SourcePoints, FEnvQueryInstance& QueryInstance,
	const float Spacing, const FVector& SourcePosition, const FRotator& Rotation, float& TotalSpacing) const;
	bool RunLineTraceTo(const FVector& ItemPos, const FVector& ContextPos, AActor* ItemActor,
		UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params) const;
	float FindPathLengthTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const;
	FVector FindPositionAtPathLength(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const;
	*/
};

