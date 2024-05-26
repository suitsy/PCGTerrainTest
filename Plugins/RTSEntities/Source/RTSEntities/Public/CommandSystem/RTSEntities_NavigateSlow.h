// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_NavigateTo.h"
#include "RTSEntities_NavigateSlow.generated.h"

/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_NavigateSlow : public URTSEntities_NavigateTo
{
	GENERATED_BODY()

public:
	URTSEntities_NavigateSlow();	
	
	virtual void ExecuteNavigation() override;
};
