// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"
#include "RTSEntities_SelectedDecalComponent.generated.h"

class URTSEntities_Entity;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSENTITIES_API URTSEntities_SelectedDecalComponent : public UDecalComponent
{
	GENERATED_BODY()

public:
	URTSEntities_SelectedDecalComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	UFUNCTION()
	static URTSEntities_SelectedDecalComponent* FindSelectedDecalComponent(const AActor* Entity) { return (Entity ? Entity->FindComponentByClass<URTSEntities_SelectedDecalComponent>() : nullptr); }
	
protected:
	virtual void OnRegister() override;

	UFUNCTION()
	void UpdateSelected(const bool bSelected);

	UPROPERTY()
	URTSEntities_Entity* EntityComponent;
};
