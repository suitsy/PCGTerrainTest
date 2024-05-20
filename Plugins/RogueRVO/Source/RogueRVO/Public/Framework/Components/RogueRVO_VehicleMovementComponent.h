// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Framework/Data/RogueRVO_DataTypes.h"

#include "RogueRVO_VehicleMovementComponent.generated.h"


class URogueRVO_Component;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROGUERVO_API URogueRVO_VehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()

public:
	explicit URogueRVO_VehicleMovementComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CalculateDesiredVelocity();
	void CalcAvoidanceVelocity();
	void CalcVehicleInput();
	void ApplyVehicleInput(const FRogueRVO_VehicleInput VehicleInput);

	// Getters
	FVector GetDesiredVelocity() const { return DesiredVelocity; }
	float GetMaxVelocity() const { return MaxVelocity; }
	float GetMaxTurnRate() const { return MaxTurnRate; }
	float GetSpeed() const;
	float GetAcceptanceDistance() const { return AcceptanceDistance; }
	float GetArrivalDistance() const { return ArrivalDistance; }
	float GetAcceptanceSpeed() const { return AcceptanceSpeed; }

	// Setters
	void UpdateNavData(const FRogueRVO_NavData& NavDataUpdate) { NavData = NavDataUpdate; }
	void SetMaxVelocity(const float NewMaxVelocity) { MaxVelocity = NewMaxVelocity; }
	void SetMaxTurnRate(const float NewMaxTurnRate) { MaxTurnRate = NewMaxTurnRate; }
	void SetAcceptanceDistance(const float Distance) { AcceptanceDistance = Distance; }
	void SetArrivalDistance(const float Distance) { ArrivalDistance = Distance; }
	void SetAcceptanceSpeed(const float Speed) { AcceptanceSpeed = Speed; }
	void SetMaxDestinationBrake(const float Brake) { MaxDestinationBrake = Brake; }
	
protected:
	virtual void BeginPlay() override;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="RogueRVO|Curves")
	TObjectPtr<UCurveFloat> ThrottleCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="RogueRVO|Curves")
	TObjectPtr<UCurveFloat> BrakeCurve;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="RogueRVO|Curves")
	TObjectPtr<UCurveFloat> SteeringDampeningCurve;

	/** Maximum linear speed limiter, if the speed is greater than this velocity, forward input will be set to zero */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float MaxVelocity;

	/** Maximum linear turn rate, reduces the rate of turn */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float MaxTurnRate;

	/** Maximum brake applied once under the arrival distance to a destination */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float MaxDestinationBrake;

	/** Distance the agent needs to be to destination for navigation to complete */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float AcceptanceDistance;

	/** Distance the agent will begin reducing speed from destination */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float ArrivalDistance;

	/** Speed the agent needs to be under at destination for navigation to complete */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float AcceptanceSpeed;

	UPROPERTY()
	TObjectPtr<URogueRVO_Component> RVOComponentRef;

	/** If set, component will use Rogue RVO avoidance. This only runs on the server. */
	UPROPERTY(Category=RogueRVO, EditAnywhere, BlueprintReadOnly)
	uint8 bUseRogueRVO:1;

	UPROPERTY()
	FVector DesiredVelocity;

	UPROPERTY(Transient)
	FRogueRVO_NavData NavData;
	
private:
	
#if WITH_EDITOR
	void DebugVehicleInput(const float Throttle, const float InSteering, const float Brake) const;	
#endif
};
