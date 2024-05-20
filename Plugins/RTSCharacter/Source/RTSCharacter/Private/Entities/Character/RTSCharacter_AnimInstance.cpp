// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Character/RTSCharacter_AnimInstance.h"
#include "AIController.h"
#include "KismetAnimationLibrary.h"
#include "Entities/Character/RTSCharacter_Base.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "Framework/Interfaces/RTSCore_AiStateInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

URTSCharacter_AnimInstance::URTSCharacter_AnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OwningCharacter = nullptr;
	OwningController = nullptr;
	CharacterMovementComponent = nullptr;
	Direction = 0.f;
	Speed = 0.f;
	bHasVelocity = false;
	AimWeight = 0.f;
	Acceleration = 0.f;
	bHasAcceleration = false;
	TargetLookAtRotation = FRotator::ZeroRotator;
	CurrentUprightAmount = 1.f;
	UpperBodyPoseAnimation = nullptr;
	BehaviourState = 0.f;
	SpeedState = 0.f;
	ConditionState = 0.f;
	PostureState = 0.f;
	NavigationState = 0.f;
	bInCombat = false;
}

void URTSCharacter_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ARTSCharacter_Base>(TryGetPawnOwner());
	if(OwningCharacter)
	{
		CharacterMovementComponent = OwningCharacter->GetCharacterMovement();		
	}
}

void URTSCharacter_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !CharacterMovementComponent)
	{
		return;
	}
	
	// Set Speed Direction
	Speed = OwningCharacter->GetVelocity().Length();
	bHasVelocity = Speed > 0.f;

	// Set Acceleration Direction
	Acceleration = CharacterMovementComponent->GetCurrentAcceleration().Length();
	bHasAcceleration = Acceleration > 0.f;	

	
	if (OwningCharacter->HasFocusTarget() && bHasVelocity)
	{
		Speed = OwningCharacter->GetProjectedSpeed();
		Direction = OwningCharacter->GetProjectedDirection();
	}
	else
	{		
		if (Speed > 0)
		{
			Direction = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(), OwningCharacter->GetActorRotation());
		}
		else
		{
			Direction = OwningCharacter->GetTurnDirection();
		}

	}
	
	// Set Aim Weight
	AimWeight = FMath::Lerp(AimWeight, OwningCharacter->IsAiming() ? .9f : 0, DeltaSeconds * 5.f);

	// Set LookAt Rotation
	TargetLookAtRotation = OwningCharacter->GetCurrentLookAtRotator();

	// Set Current Upright Amount
	CurrentUprightAmount = OwningCharacter->GetCurrentUprightAmount();

	if(!OwningController)
	{
		if(const APawn* Pawn = Cast<APawn>(OwningCharacter))
		{
			OwningController = Cast<AAIController>(Pawn->GetController());
		}		
	}

	if(OwningController)
	{
		if(const IRTSCore_AiStateInterface* AiStateInterface = Cast<IRTSCore_AiStateInterface>(OwningController))
		{
			// Set current states
			BehaviourState = AiStateInterface->GetState(ERTSCore_StateCategory::Behaviour);
			SpeedState = AiStateInterface->GetState(ERTSCore_StateCategory::Speed);
			ConditionState = AiStateInterface->GetState(ERTSCore_StateCategory::Condition);
			PostureState = AiStateInterface->GetState(ERTSCore_StateCategory::Posture);
			NavigationState = AiStateInterface->GetState(ERTSCore_StateCategory::Navigation);

			// Check in combat
			bInCombat = BehaviourState > 2;
		}
	}	
}