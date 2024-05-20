// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RogueRVO_CharacterMovementComponent.h"
#include "Framework/Components/RogueRVO_Component.h"


URogueRVO_CharacterMovementComponent::URogueRVO_CharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bUseRogueRVO = true;

	// Enable RVO on character movement component
	bUseRVOAvoidance = true;	
}

void URogueRVO_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	
}

void URogueRVO_CharacterMovementComponent::CalcAvoidanceVelocity(float DeltaTime)
{	
	if(bUseRogueRVO && RVOComponentRef != nullptr)
	{
		Velocity = RVOComponentRef->GetRVOAvoidanceVelocity();
	}	
}


void URogueRVO_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// RVO on character movement comp needs to be enabled in order to call CalcAvoidanceVelocity override
	bUseRVOAvoidance = bUseRogueRVO;

	if(GetPawnOwner() && bUseRogueRVO)
	{		
		// Assign reference to Rogue RVO component on pawn owner
		RVOComponentRef = Cast<URogueRVO_Component>(GetPawnOwner()->GetComponentByClass(URogueRVO_Component::StaticClass()));

		// Disable RVO if failed to find Rogue RVO component and notify
		if(RVOComponentRef == nullptr)
		{
			bUseRogueRVO = false;

			// Disable RVO on character movement component
			bUseRVOAvoidance = false;
			
			UE_LOG(LogTemp, Warning, TEXT("[%s] Rogue RVO Movement Component failed to locate Rogue RVO Component, disabling RVO."), *GetClass()->GetName());
		}
	}
}
