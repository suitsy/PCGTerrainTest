// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "GameFramework/Actor.h"
#include "RTSEntities_MarkerActor.generated.h"

class URTSEntities_MarkerComponent;

UCLASS()
class RTSENTITIES_API ARTSEntities_MarkerActor : public AActor
{
	GENERATED_BODY()

public:
	ARTSEntities_MarkerActor();
	void InitDestinationDecal(const FVector& InDecalSize, UMaterialInstanceDynamic* InMaterial);
	void UpdateDestinationDecal(const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus Status) const;
	void InitNiagaraSystem();
	void UpdateNiagaraSystem(const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus Status) const;
	void Disable() const;
	void Enable() const;
	
protected:
	virtual void BeginPlay() override;
	void SetDecalStatus(const ERTSEntities_CommandStatus Status) const;

	UPROPERTY()
	UDecalComponent* Decal;

	UPROPERTY()
	URTSEntities_MarkerComponent* MarkerComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;
};
