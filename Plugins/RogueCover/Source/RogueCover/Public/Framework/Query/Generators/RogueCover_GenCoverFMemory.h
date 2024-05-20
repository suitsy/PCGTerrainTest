// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "RogueCover_GenCoverFMemory.generated.h"


UCLASS()
class ROGUECOVER_API URogueCover_GenCoverFMemory : public UEnvQueryGenerator
{
	GENERATED_UCLASS_BODY()

	/** Square's extent */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue SquareExtent;

	/** Box's height */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	FAIDataProviderFloatValue BoxHeight;

	/** context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> GenerateAround;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
