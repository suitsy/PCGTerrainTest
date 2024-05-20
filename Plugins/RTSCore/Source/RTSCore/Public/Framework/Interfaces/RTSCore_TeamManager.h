// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RTSCore_TeamManager.generated.h"

class UNiagaraComponent;
// This class does not need to be modified.
UINTERFACE()
class RTSCORE_API URTSCore_TeamManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RTSCORE_API IRTSCore_TeamManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ApplyTeamDataToMaterial(const uint8 NewTeamId, UMaterialInstanceDynamic* MaterialInstance) = 0;
	virtual void ApplyTeamDataToDecalComponent(const uint8 TeamId, UDecalComponent* DecalComponent) = 0;
	virtual void ApplyTeamDataToMeshComponent(const uint8 TeamId, UMeshComponent* MeshComponent) = 0;
	virtual void ApplyTeamDataToNiagaraComponent(const uint8 TeamId, UNiagaraComponent* NiagaraComponent) = 0;
	virtual void ApplyTeamDataToActor(const uint8 TeamId, AActor* TargetActor, bool bIncludeChildActors) = 0;
};
