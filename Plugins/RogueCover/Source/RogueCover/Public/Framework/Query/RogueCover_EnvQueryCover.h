// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "RogueCover_EnvQueryCover.generated.h"

class URogueCover_Point;
/**
 * 
 */
UCLASS()
class ROGUECOVER_API URogueCover_EnvQueryCover : public UEnvQueryItemType_VectorBase
{
	GENERATED_UCLASS_BODY()

public:
	typedef const URogueCover_Point* FValueType;

	static URogueCover_Point* GetValue(const uint8* RawData);
	static void SetValue(uint8* RawData, const URogueCover_Point* Value);

	virtual FVector GetItemLocation(const uint8* RawData) const override;

	virtual void AddBlackboardFilters(FBlackboardKeySelector& KeySelector, UObject* FilterOwner) const override;
	virtual bool StoreInBlackboard(FBlackboardKeySelector& KeySelector, UBlackboardComponent* Blackboard, const uint8* RawData) const override;
	virtual FString GetDescription(const uint8* RawData) const override;
};
