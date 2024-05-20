// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "Engine/DataAsset.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "RogueQueryGenerator.generated.h"

struct FRecastDebugPathfindingData;
/**
 * 
 */
UCLASS(MinimalAPI, Abstract)
class URogueQueryGenerator : public UDataAsset
{
	GENERATED_BODY()

public:
	URogueQueryGenerator();
	ROGUEQUERY_API virtual void PostInitProperties() override;
	ROGUEQUERY_API virtual void PostLoad() override;
	ROGUEQUERY_API virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	ROGUEQUERY_API virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void GenerateItems(FRogueQueryInstance& QueryInstance) { checkNoEntry(); }
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category=Generator)
	FName GeneratorName;
	
	/** trace params */
	UPROPERTY(EditDefaultsOnly, Category = Projection)
	FRogueQueryTraceData ProjectionData;

	/** generation on nav mesh only */
	UPROPERTY(EditDefaultsOnly, Category=Navigation)
	uint8 NavPointsOnly:1 = true;
	
	/** pathfinding direction */
	UPROPERTY(EditDefaultsOnly, Category = Navigation)
	FAIDataProviderBoolValue PathToItem;

	/** multiplier for max distance between point and source */
	UPROPERTY(EditDefaultsOnly, Category = Navigation)
	FAIDataProviderFloatValue ScanRangeMultiplier;

protected:
	void FinishGeneration(FRogueQueryInstance& QueryInstance, TArray<FNavLocation>& GeneratedPoints) const;	
};
