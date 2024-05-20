// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "GameFramework/Actor.h"
#include "RTSEntities_Decal.generated.h"

UCLASS()
class RTSENTITIES_API ARTSEntities_Decal : public AActor
{
	GENERATED_BODY()

public:
	ARTSEntities_Decal();
	void InitDestinationMarker(const FVector& InDecalSize, UMaterialInstanceDynamic* DecalMaterial);
	void UpdateDestinationMarker(const FVector& NewLocation, const FRotator& NewRotation, const ERTSEntities_CommandStatus Status) const;
	void Disable() const;
	void Enable() const;
	
protected:
	virtual void BeginPlay() override;
	void SetDecalStatus(const ERTSEntities_CommandStatus Status) const;

	UPROPERTY()
	UDecalComponent* Decal;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance;
};
