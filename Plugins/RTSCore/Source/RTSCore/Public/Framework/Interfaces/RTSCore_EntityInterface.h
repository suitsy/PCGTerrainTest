// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "UObject/Interface.h"
#include "RTSCore_EntityInterface.generated.h"

class USplineComponent;

UINTERFACE()
class RTSCORE_API URTSCore_EntityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_EntityInterface
{
	GENERATED_BODY()

public:
	virtual TObjectPtr<USplineComponent> GetNavigationSpline() const = 0;
	virtual void CreatePerceptionStimuliSourceComponent() = 0;
	virtual FName GetSightSocket() const = 0;
	virtual FVector GetSightSocketLocation() const = 0;
	virtual USceneComponent* GetEntityMesh() const = 0;
	virtual UShapeComponent* GetCollisionBox() const = 0;
	virtual void Crouch() = 0;
	virtual void Stand() = 0;
};
