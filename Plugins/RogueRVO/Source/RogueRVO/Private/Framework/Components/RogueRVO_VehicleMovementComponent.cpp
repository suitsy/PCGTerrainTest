// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RogueRVO_VehicleMovementComponent.h"
#include "Framework/Components/RogueRVO_Component.h"
#include "Framework/Settings/RogueRVO_DeveloperSettings.h"


URogueRVO_VehicleMovementComponent::URogueRVO_VehicleMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	MaxVelocity = 600.f;
	bUseRogueRVO = true;
	MaxTurnRate = 12.f;
	AcceptanceDistance = 150.f;
	AcceptanceSpeed = 50.f;
	ArrivalDistance = 500.f;
	MaxDestinationBrake = 12.f;
}

void URogueRVO_VehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalculateDesiredVelocity();
	CalcVehicleInput();
}

void URogueRVO_VehicleMovementComponent::CalculateDesiredVelocity()
{
	if(GetPawnOwner())
	{
		// Calculate the direction from the vehicle to the target position
		const FVector TargetDirection = (NavData.NavTargetLocation - GetPawnOwner()->GetActorLocation()).GetSafeNormal();

		// Calculate the desired velocity based on the max speed
		DesiredVelocity = TargetDirection * GetMaxSpeed();

		if(bUseRogueRVO)
		{
			CalcAvoidanceVelocity();
		}
	}
}

void URogueRVO_VehicleMovementComponent::CalcAvoidanceVelocity()
{
	if(bUseRogueRVO && RVOComponentRef != nullptr)
	{
		DesiredVelocity = RVOComponentRef->GetRVOAvoidanceVelocity();
	}
}

void URogueRVO_VehicleMovementComponent::CalcVehicleInput()
{
	// Calculate the dot product of the desired velocity with the vehicle's forward vector
	const float Forward = FVector::DotProduct(DesiredVelocity.GetSafeNormal(), GetPawnOwner()->GetActorForwardVector());			
	SetThrottleInput(Forward <= 0.f ? 0.f : Forward * 0.5f);
		
	// Calculate the steering component by taking the dot product with the right vector
	const float Right = FVector::DotProduct(DesiredVelocity.GetSafeNormal(), GetPawnOwner()->GetActorRightVector());

	// Restrict steering based on a max turn rate
	const float MaxSteeringChange = GetMaxTurnRate() * GetWorld()->GetDeltaSeconds();
	SetSteeringInput(FMath::Clamp(Right, -MaxSteeringChange, MaxSteeringChange));

	// Approaching destination
	float BrakeValue = 0.f;
	if(NavData.bApproachingDestination && NavData.DistanceToDestination < GetArrivalDistance())
	{
		// Approaching destination
		if(GetSpeed() > GetAcceptanceSpeed() && NavData.DistanceToDestination < GetAcceptanceDistance())
		{		
			const float DestinationBrakeAmount = FMath::GetMappedRangeValueClamped(FVector2d(0, GetArrivalDistance() + GetMaxSpeed()), FVector2d(0.f, MaxDestinationBrake), NavData.DistanceToDestination + GetSpeed());
			BrakeValue = BrakeCurve != nullptr ? BrakeCurve->GetFloatValue(DestinationBrakeAmount) : DestinationBrakeAmount;
		}
	}
	else
	{
		// Check we are on the path
		if(NavData.bOnPath)
		{
			if(NavData.bSlowForCorner)
			{
				// Handle braking for path cornering when on path
				const float BrakeRatio = FMath::GetMappedRangeValueClamped(FVector2d(0.f, 2.f), FVector2d(0.f, 1.f), GetSpeed() / GetMaxSpeed() + NavData.TurnRatio);
				BrakeValue= SteeringDampeningCurve ? SteeringDampeningCurve->GetFloatValue(BrakeRatio) : 0.f;					
			}
		}
		else
		{
			// Limit speed to 50% if off path
			if(GetSpeed() > GetMaxSpeed() * 0.5f)
			{
				BrakeValue = 0.1f;
			}
		}
	}

	SetBrakeInput(BrakeValue);
}

void URogueRVO_VehicleMovementComponent::ApplyVehicleInput(const FRogueRVO_VehicleInput VehicleInput)
{
	// Calc amount to scale down the steering based on vehicle speed
	const float SpeedScalingFactor = FMath::Clamp(GetForwardSpeed() / GetMaxVelocity(), 0.5f, 0.0f);
	const float ScaledSteeringForce = VehicleInput.Steering * SpeedScalingFactor;

	// Ensure speed has not exceeded max speed, reset throttle to zero this tick if it has
	float MaxCheckedThrottle = GetForwardSpeed() < GetMaxVelocity() ? VehicleInput.Throttle : 0.f;

	// Assign the final throttle value from the throttle curve
	MaxCheckedThrottle = MaxCheckedThrottle > 0.f && ThrottleCurve != nullptr ? ThrottleCurve->GetFloatValue(MaxCheckedThrottle) : MaxCheckedThrottle;

	SetYawInput(ScaledSteeringForce);
	SetThrottleInput(MaxCheckedThrottle);
	SetBrakeInput(VehicleInput.Braking);

#if WITH_EDITOR
	DebugVehicleInput(MaxCheckedThrottle, ScaledSteeringForce, VehicleInput.Braking);
#endif
}

float URogueRVO_VehicleMovementComponent::GetSpeed() const
{
	return GetPawnOwner() && FMath::Abs(GetPawnOwner()->GetVelocity().Length()) > 10.f ? FMath::Abs(GetPawnOwner()->GetVelocity().Length()) : 0.f; 
}

void URogueRVO_VehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetPawnOwner() && bUseRogueRVO)
	{
		// Assign reference to Rogue RVO component on pawn owner
		RVOComponentRef = Cast<URogueRVO_Component>(GetPawnOwner()->GetComponentByClass(URogueRVO_Component::StaticClass()));

		// Disable RVO if failed to find Rogue RVO component and notify
		if(RVOComponentRef == nullptr)
		{
			bUseRogueRVO = false;

			UE_LOG(LogTemp, Warning, TEXT("[%s] Rogue RVO Movement Component failed to locate Rogue RVO Component, disabling RVO."), *GetClass()->GetName());
		}
	}
}

void URogueRVO_VehicleMovementComponent::DebugVehicleInput(const float Throttle, const float InSteering,
	const float Brake) const
{
	if(const URogueRVO_DeveloperSettings* Settings = GetDefault<URogueRVO_DeveloperSettings>())
	{
		if(Settings->DebugEnabled)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("ThrottleInput: %f | SteeringInput: %f | BrakeInput: %f | %s | %s"),
					Throttle, InSteering, Brake, GetHandbrakeInput() ? TEXT("HandBrakeON") : TEXT("HandBrakeOFF"), IsParked() ? TEXT("Parked") : TEXT("-")));
		}
	}
}

