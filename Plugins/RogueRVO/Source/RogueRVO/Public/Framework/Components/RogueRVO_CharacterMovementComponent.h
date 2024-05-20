// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RogueRVO_CharacterMovementComponent.generated.h"


class URogueRVO_Component;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROGUERVO_API URogueRVO_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	URogueRVO_CharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void CalcAvoidanceVelocity(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<URogueRVO_Component> RVOComponentRef;	

	/** If set, component will use Rogue RVO avoidance. This only runs on the server. */
	UPROPERTY(Category=RogueRVO, EditAnywhere, BlueprintReadOnly)
	uint8 bUseRogueRVO:1;
};
