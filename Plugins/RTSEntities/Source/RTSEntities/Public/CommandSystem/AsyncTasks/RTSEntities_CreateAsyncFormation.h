// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Data/RTSEntities_FormationDataAsset.h"


class RTSENTITIES_API FRTSEntities_CreateAsyncFormation : public FNonAbandonableTask
{
public:
	FRTSEntities_CreateAsyncFormation(const FGuid NewCommandId,
	    const FRTSEntities_CommandData& NewCommandData, const FRTSEntities_PlayerSelections& NewSelected, FRTSEntities_Navigation& NewNavigation);

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FRTSEntities_CreateAsyncFormation, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork();
	
protected:
	/*URTSEntities_FormationDataAsset* GetFormationData();
	FRTSEntities_CommandData GetCommandData(const FVector& Offset);
	void CreateFormation();
	float GetSelectedAverageSpacing() const; 
	float GetSelectionSpacing(const FRTSEntities_PlayerSelection& SingleSelection) const; 
	float GetEntitiesAverageSpacing(const TArray<AActor*>& Entities, const float Spacing) const;
	float GetEntitySpacing(const AActor* Entity) const;
	TArray<FRTSEntities_FormationPosition> CreateFormationPositions(const float Spacing) const;
	bool NextPositionPermutation(int* Array, int Size) const;
	void AssignSelectionPositions(TArray<FRTSEntities_FormationPosition>& Positions) const;
	void UpdateSelectionAssignedPositions(TArray<FRTSEntities_FormationPosition>& Positions) const;
	void CreateEntityPositions(FRTSEntities_FormationPosition& Position) const;
	void AssignEntityPositions(FRTSEntities_FormationPosition& Position) const;
	void UpdateEntityPositions(FRTSEntities_FormationPosition& Position) const;*/

private:
	FGuid CommandId;
	FRTSEntities_CommandData CommandData;
	FRTSEntities_PlayerSelections Selected;
	FRTSEntities_Navigation& Navigation;
};
