// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "AI/RVOAvoidanceInterface.h"
#include "GameFramework/MovementComponent.h"
#include "Framework/Interfaces/RTSCore_VehicleInputInterface.h"
#include "RTSVehicle_RVOAvoidanceComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("VehicleRVO"), STATGROUP_VehicleRVO, STATCAT_Advanced);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSVEHICLES_API URTSVehicle_RVOAvoidanceComponent : public UMovementComponent, public IRVOAvoidanceInterface
{
	GENERATED_BODY()

	// Sets default values for this component's properties
	URTSVehicle_RVOAvoidanceComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void Deactivate() override;

	/** Change avoidance state and register with RVO manager if necessary */
	UFUNCTION(BlueprintCallable, Category = "Avoidance")
	void SetAvoidanceEnabled(bool bEnable);

	/* System-wide switch that turns the whole behavior on/off for all vehilcles */
	static void SetAvoidanceSystemEnabled(bool bEnable);

	static bool IsAvoidanceSystemEnabled();

	// Inherited via IRVOAvoidanceInterface
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

private:
	bool InitVehicleControls();
	IRTSCore_VehicleInputInterface* GetVehicleControls() const;
	FString GetVehicleName() const;

	void InitializeFromVehicleProperties(AActor* Vehicle);
	void InitializeAvoidance();

	/** Update RVO Avoidance for simulation */
	void UpdateAvoidance(float DeltaTime);

	/** called in Tick to update data in RVO avoidance manager */
	void UpdateDefaultAvoidance();

	/** lock avoidance velocity */
	void SetAvoidanceVelocityLock(class UAvoidanceManager* Avoidance, float Duration);
	void CalculateAvoidanceVelocity(float DeltaTime);
	void UpdateVehicleMovementFromAvoidance();
	void NotifyRVOStatus(bool bEnabled);

private:
	int32 AvoidanceUID {};

	UPROPERTY()
	TScriptInterface<IRTSCore_VehicleInputInterface> VehicleControls {};

	/** De facto default value 0.5 (due to that being the default in the avoidance registration function), indicates RVO behavior. */
	UPROPERTY(Category = "Avoidance", EditAnywhere)
	float AvoidanceWeight{ 0.5f };

	/** Vehicle Radius to use for RVO avoidance (usually half of vehicle width) */
	UPROPERTY(Category = "Avoidance", VisibleAnywhere)
	float RVOAvoidanceRadius;

	/** Vehicle Height to use for RVO avoidance (usually vehicle height) */
	UPROPERTY(Category = "Avoidance", VisibleAnywhere)
	float RVOAvoidanceHeight;

	/** Area Radius to consider for RVO avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere)
	float AvoidanceConsiderationRadius{ 1600.0f };

	/** Moving actor's group mask */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup {};

	/** Will avoid other agents if they are in one of specified groups */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid {};

	/** Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid */
	UPROPERTY(Category = "Avoidance", EditAnywhere, AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore {};

	/** Value by which to alter steering per frame based on calculated avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOSteeringStep{ 0.25f };

	/** Value by which to alter throttle per frame based on calculated avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float RVOThrottleStep{ 0.3f };

	/** If set, component will use RVO avoidance */
	UPROPERTY(Category = "Avoidance", EditAnywhere)
	bool bEnableAvoidanceMovementUpdates{ false };

	/** If set, component RVO avoidance will be enabled for all vehicles */
	static inline bool bUseRVOAvoidance{ true };

	/** Calculated avoidance velocity used to adjust steering and throttle */
	FVector AvoidanceVelocity{ 0 };

	/** forced avoidance velocity, used when AvoidanceLockTimer is > 0 */
	FVector AvoidanceLockVelocity{ 0 };

	/** remaining time of avoidance velocity lock */
	float AvoidanceLockTimer{};

	/** Was avoidance updated in this frame? */
	bool bWasAvoidanceUpdated{};

	bool bDidAvoidanceAlterVelocity{};
};

inline bool URTSVehicle_RVOAvoidanceComponent::IsAvoidanceSystemEnabled()
{
	return bUseRVOAvoidance;
}
