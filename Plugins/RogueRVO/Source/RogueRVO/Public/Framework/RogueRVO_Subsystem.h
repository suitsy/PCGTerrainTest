// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavEdgeProviderInterface.h"
#include "Data/RogueRVO_DataTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueRVO_Subsystem.generated.h"

class URogueRVO_Component;
class IRVOAvoidanceInterface;

UCLASS(Config=Game)
class ROGUERVO_API URogueRVO_Subsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	URogueRVO_Subsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Add the RVO component for avoidance */
	void RegisterRVOComponent(URogueRVO_Component* RVOComponent, const float AvoidanceWeight = 0.5f);
	
	/** Update the RVO avoidance data for the participating component */
	void UpdateRVOComponent(URogueRVO_Component* RVOComponent);

	/** Calculate avoidance velocity for component (avoids collisions with the supplied component) */
	UFUNCTION(BlueprintCallable, Category="AI")
	FVector GetAvoidanceVelocityForComponent(URogueRVO_Component* RVOComponent);

	/** Only use if you want manual velocity planning. Provide your AvoidanceUID in order to avoid colliding with yourself. */
	FVector GetAvoidanceVelocityIgnoringUID(const FRogueRVO_NavAvoidanceData& AvoidanceData, float DeltaTime, int32 IgnoreThisUID);

	/** For Duration seconds, set this object to ignore all others. */
	void OverrideToMaxWeight(int32 AvoidanceUID, float Duration);

	/** Used to manually mark data associated with given ID as 'dead' (and reusable) */
	void RemoveAvoidanceObject(const int32 AvoidanceUID);
	
	/** How long an avoidance UID must not be updated before the system will put it back in the pool. Actual delay is up to 150% of this value. */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float DefaultTimeToLive;

	/** Multiply the radius of all STORED avoidance objects by this value to allow a little extra room for avoidance maneuvers. */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float ArtificialRadiusExpansion;

	/** Allowable height margin between obstacles and agents. This is over and above the difference in agent heights. */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float HeightCheckMargin;

	/** This is how far forward in time (seconds) we extend our velocity cones and thus our prediction */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float DeltaTimeToPredict;

	/** How long to stay on course (barring collision) after making an avoidance move */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float LockTimeAfterAvoid;

	/** How long to stay on course (barring collision) after making an unobstructed move (should be > 0.0, but can be less than a full frame) */
	UPROPERTY(EditAnywhere, Category="Avoidance", config, meta=(ClampMin = "0.0"))
	float LockTimeAfterClean;

protected:
	/** Get appropriate UID for use when reporting to this function or requesting RVO assistance. */
	UFUNCTION(BlueprintCallable, Category="AI")
	int32 GetNewAvoidanceUID();

	/** Try to set a timer for RemoveOutdatedObjects */
	void RequestUpdateTimer();

	/** Cleanup AvoidanceObjects, called by timer */
	void RemoveOutdatedObjects();

	/** This is called by our blueprint-accessible function after it has packed the data into an object. */
	void UpdateRVO_Internal(int32 AvoidanceUID, const FRogueRVO_NavAvoidanceData& AvoidanceData);

	/** This is called by our blueprint-accessible functions, and permits the user to ignore self, or not. Important in case the user isn't in the avoidance manager. */
	FVector GetAvoidanceVelocity_Internal(const FRogueRVO_NavAvoidanceData& AvoidanceData, float DeltaTime, int32 *IgnoreThisUID = NULL);

	/** All objects currently part of the avoidance solution. This is pretty transient stuff. */
	TMap<int32, FRogueRVO_NavAvoidanceData> AvoidanceObjects;

	/** This is a pool of keys to be used when new objects are created. */
	TArray<int32> NewKeyPool;	

	/** Handle for efficient management of RemoveOutdatedObjects timer */
	FTimerHandle TimerHandle_RemoveOutdatedObjects;

	/** Allow auto removing dead entries in AvoidanceObjects **/
	uint32 bAutoPurgeOutdatedObjects : 1;

	/** Set when RemoveOutdatedObjects timer is already requested */
	uint32 bRequestedUpdateTimer : 1;

	/** Keeping this here to avoid constant allocation */
	TArray<FRogueRVO_VelocityAvoidanceCone> AllCones;

	/** Provider of navigation edges to consider for avoidance */
	TWeakObjectPtr<UObject> EdgeProviderOb;
	INavEdgeProviderInterface* EdgeProviderInterface;

	void SetNavEdgeProvider(INavEdgeProviderInterface* InEdgeProvider);
	static bool AvoidsNavEdges(const FVector& OrgLocation, const FVector& TestVelocity, const TArray<FNavEdgeSegment>& NavEdges, float MaxZDiff);
	static FVector AvoidCones(TArray<FRogueRVO_VelocityAvoidanceCone>& AllCones, const FVector& BasePosition, const FVector& DesiredPosition, const int NumConesToTest);
	
private:
	
#if WITH_EDITOR
	bool IsDebugOnForUID(int32 AvoidanceUID);
	void Debug_RVO(const FRogueRVO_NavAvoidanceData& AvoidanceData, const FVector& Velocity, float DeltaTime) const;
#endif	
};
