// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQueryGenerator.h"
#include "RogueQueryGenerator_Grid.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, MinimalAPI)
class URogueQueryGenerator_Grid : public URogueQueryGenerator
{
	GENERATED_BODY()

public:
	URogueQueryGenerator_Grid();	
	virtual void GenerateItems(FRogueQueryInstance& QueryInstance) override;
	
	/** half of square's extent, like a radius */
	UPROPERTY(EditDefaultsOnly, Category=Generator, meta=(DisplayName="GridHalfSize"))
	float GridSize = 500.f;

	/** generation density */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	float SpaceBetween = 100.f;
};
