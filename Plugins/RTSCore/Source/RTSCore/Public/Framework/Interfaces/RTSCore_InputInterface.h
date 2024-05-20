// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "UObject/Interface.h"
#include "RTSCore_InputInterface.generated.h"


// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_InputInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_InputInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Select() = 0;
	virtual void SelectHold() = 0;
	virtual void SelectEnd() = 0;
	virtual void Command() = 0;
	virtual void CommandHold() = 0;
	virtual void CommandEnd() = 0;
	virtual void DoubleTap() = 0;
	virtual void SetModifier(const EInputModifierKey NewModifier) = 0;
	virtual void WheelMouse(const float Input) = 0;
	virtual void SetPreviewCommand() = 0;
	
};
