// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueQuery_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RogueQuery.generated.h"

class URogueQueryGenerator;
class URogueQueryTest;

/**
 * 
 */
UCLASS(BlueprintType, MinimalAPI)
class URogueQuery : public UPrimaryDataAsset
{
	GENERATED_BODY()	

public:
	URogueQuery();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}
	
	ROGUEQUERY_API virtual void PostInitProperties() override;
	ROGUEQUERY_API virtual void PostLoad() override;
	ROGUEQUERY_API virtual void PostRename(UObject* OldOuter, const FName OldName) override;
	ROGUEQUERY_API virtual void PostDuplicate(bool bDuplicateForPIE) override;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category=Query)
	FName QueryName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category=Query)
	TEnumAsByte<ERogueQueryRunMode::Type> Mode = ERogueQueryRunMode::AllMatching;

	/** display query grid when using debug actor */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Generator")
	TObjectPtr<URogueQueryGenerator> Generator;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category=Tests)
	TArray<TObjectPtr<URogueQueryTest>> Tests;
};
