// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RTSCharacter_AnimInstance.generated.h"

class IRTSCore_AiControllerInterface;
class AAIController;
class UCharacterMovementComponent;
class ARTSCharacter_Base;

/**
 * 
 */
UCLASS()
class RTSCHARACTER_API URTSCharacter_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	explicit URTSCharacter_AnimInstance(const FObjectInitializer& ObjectInitializer);
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	ARTSCharacter_Base* OwningCharacter;
	
	UPROPERTY(BlueprintReadOnly)
	AAIController* OwningController;	
	
	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* CharacterMovementComponent;

	UPROPERTY(BlueprintReadOnly)
	float Direction;
	
	UPROPERTY(BlueprintReadOnly)
	float Speed;
	
	UPROPERTY(BlueprintReadOnly)
	float Acceleration;
	
	UPROPERTY(BlueprintReadOnly)
	bool bHasVelocity;
	
	UPROPERTY(BlueprintReadOnly)
	bool bHasAcceleration;
	
	UPROPERTY(BlueprintReadOnly)
	float AimWeight;
	
	UPROPERTY(BlueprintReadOnly)
	float CurrentUprightAmount;
	
	UPROPERTY(BlueprintReadOnly)
	FRotator TargetLookAtRotation;
	
	UPROPERTY(BlueprintReadWrite)
	UAnimSequenceBase* UpperBodyPoseAnimation;

	UPROPERTY(BlueprintReadOnly)
	int32 BehaviourState;

	UPROPERTY(BlueprintReadOnly)
	int32 SpeedState;

	UPROPERTY(BlueprintReadOnly)
	int32 ConditionState;

	UPROPERTY(BlueprintReadOnly)
	int32 PostureState;

	UPROPERTY(BlueprintReadOnly)
	int32 NavigationState;
	
	UPROPERTY(BlueprintReadOnly)
	bool bInCombat;
};
