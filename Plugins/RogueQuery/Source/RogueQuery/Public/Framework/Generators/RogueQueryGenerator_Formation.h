// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQueryGenerator.h"
#include "RogueQueryGenerator_Formation.generated.h"

/**
 * 
 */
UCLASS()
class ROGUEQUERY_API URogueQueryGenerator_Formation : public URogueQueryGenerator
{
	GENERATED_BODY()

public:
	URogueQueryGenerator_Formation();
	void SetCustomData(const FRogueQueryCustomData& Data);
	virtual void GenerateItems(FRogueQueryInstance& QueryInstance) override;
	
	/** vector to calculate position offsets */
	UPROPERTY(EditDefaultsOnly, Category=Generator, meta=(DisplayName="Formation Offset"))
	FVector Offset;

	/** spacing between each position */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	float SpaceBetween;

	/** number of positions required */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	int32 NumPositions;
};
