// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Components/RTSCore_GameStateComponent.h"
#include "Framework/Interfaces/RTSCore_EquipmentManagerInterface.h"
#include "RTSEquipment_ManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSEQUIPMENT_API URTSEquipment_ManagerComponent : public URTSCore_GameStateComponent, public IRTSCore_EquipmentManagerInterface
{
	GENERATED_BODY()

public:
	URTSEquipment_ManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void AddEquipmentComponent(APlayerController* NewOwningPlayer, AActor* Entity) override;
};
