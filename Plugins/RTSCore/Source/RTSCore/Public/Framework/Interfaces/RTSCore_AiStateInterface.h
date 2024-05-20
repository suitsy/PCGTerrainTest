// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_AiStateInterface.generated.h"

enum class ERTSCore_StateCategory : uint8;

// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_AiStateInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_AiStateInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual int32 GetState(const ERTSCore_StateCategory Category) const = 0;
	virtual void SetState(const ERTSCore_StateCategory Category, const int32 NewState) = 0;
};
