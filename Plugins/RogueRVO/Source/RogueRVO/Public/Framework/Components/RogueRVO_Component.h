// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/RVOAvoidanceInterface.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "Components/ActorComponent.h"
#include "RogueRVO_Component.generated.h"

class URogueRVO_VehicleMovementComponent;
class URogueRVO_CharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROGUERVO_API URogueRVO_Component : public UActorComponent, public IRVOAvoidanceInterface
{
	GENERATED_BODY()

public:
	URogueRVO_Component();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void OnRegister() override;
	virtual void Deactivate() override;

	// IRVOAvoidanceInterface
	virtual void SetRVOAvoidanceUID(int32 UID) override;
	virtual int32 GetRVOAvoidanceUID() override;
	virtual void SetRVOAvoidanceWeight(float Weight) override;
	virtual float GetRVOAvoidanceWeight() override;
	virtual FVector GetRVOAvoidanceOrigin() override;
	virtual float GetRVOAvoidanceRadius() override;
	virtual float GetRVOAvoidanceHeight() override;
	virtual float GetRVOAvoidanceConsiderationRadius() override;
	virtual FVector GetVelocityForRVOConsideration() override;
	virtual FVector GetRVOAvoidanceVelocity();
	virtual void SetAvoidanceGroupMask(int32 GroupFlags) override;
	virtual int32 GetAvoidanceGroupMask() override;
	virtual void SetGroupsToAvoidMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToAvoidMask() override;
	virtual void SetGroupsToIgnoreMask(int32 GroupFlags) override;
	virtual int32 GetGroupsToIgnoreMask() override;

protected:
	virtual void BeginPlay() override;
	void InitializeAgentProperties();
	void InitializeAvoidance();
	void CalculateAvoidanceVelocity();
	void UpdateAvoidance();
	void SetAvoidanceVelocityLock(class URogueRVO_Subsystem* Avoidance, float Duration);

	/** If set, component will use RVO avoidance. This only runs on the server. */
	UPROPERTY(Category="Rogue RVO", EditAnywhere, BlueprintReadOnly)
	uint8 bUseRVOAvoidance:1;

	UPROPERTY(Transient)
	TObjectPtr<URogueRVO_CharacterMovementComponent> CharacterMovement;

	UPROPERTY(Transient)
	TObjectPtr<URogueRVO_VehicleMovementComponent> VehicleMovement;	

private:
	UPROPERTY()
	int32 AvoidanceUID;

	UPROPERTY()
	FVector AvoidanceVelocity = FVector();

	/** Forced avoidance velocity, used when AvoidanceLockTimer is > 0 */	
	UPROPERTY()
	FVector AvoidanceLockVelocity = FVector();

	/** Remaining time of avoidance velocity lock */
	UPROPERTY()
	float AvoidanceLockTimer = 10.f;

	/** Was avoidance updated in this frame? */
	UPROPERTY()
	bool bWasAvoidanceUpdated = false;

	/** De facto default value 0.5 (due to that being the default in the avoidance registration function), indicates RVO behavior. */
	UPROPERTY(Category = "Avoidance", EditAnywhere)
	float AvoidanceWeight = 0.5f;

	/** Vehicle Radius to use for RVO avoidance (usually half of vehicle width) */
	UPROPERTY(Category = "Avoidance", VisibleAnywhere)
	float RVOAvoidanceRadius = 0.f;

	/** Vehicle Height to use for RVO avoidance (usually vehicle height) */
	UPROPERTY(Category = "Avoidance", VisibleAnywhere)
	float RVOAvoidanceHeight = 0.f;

	/** Area Radius to consider for RVO avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere)
	float AvoidanceConsiderationRadius = 1600.0f;

	/** Moving actor's group mask */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup;

	/** Will avoid other agents if they are in one of specified groups */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid;

	/** Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore;
};
