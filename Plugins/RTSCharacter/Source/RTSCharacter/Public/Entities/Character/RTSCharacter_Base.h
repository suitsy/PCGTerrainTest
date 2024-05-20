// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Interfaces/RTSCore_AiInterface.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "GameFramework/Character.h"
#include "RTSCharacter_Base.generated.h"

class URogueRVO_Component;
class UAIPerceptionStimuliSourceComponent;
class UTimelineComponent;


UCLASS()
class RTSCHARACTER_API ARTSCharacter_Base : public ACharacter, public IRTSCore_AiInterface, public IRTSCore_EntityInterface
{
	GENERATED_BODY()

public:
	ARTSCharacter_Base(const FObjectInitializer& ObjectInitializer); 
	virtual void Tick(float DeltaTime) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	// Public Get Functions
	bool HasFocusTarget() const { return bIsFocused; }
	bool IsAiming() const { return bIsAiming; }
	float GetTurnDirection() const { return TurnDirection; }
	FRotator GetCurrentLookAtRotator() const { return CurrentLookAtRotator; }
	float GetProjectedSpeed() const { return ProjectedSpeed; }
	float GetProjectedDirection() const { return ProjectedDirection; }
	float GetCurrentUprightAmount() const { return CurrentUprightAmount; }

protected:
	virtual void BeginPlay() override;
	void FocusTrackTargetOnMove(const float DeltaTime);
	void FocusObservationTarget(const float DeltaTime);
	void SetAimRotatorToTargetLocation(const FVector& Location);
	void CalcAimDirectionRotation(const FVector& Location, const float DeltaTime);
	static float GetTurnDirection(float A, float B);
	void SetLookAtLocation(const FVector& Location, const bool bResetLook);
	void ResetAimRotator();
	
	// Turn Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Rotation")
	UCurveFloat* TurnRotationCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Rotation")
	UCurveFloat* TurnDirectionCurve;

	UPROPERTY(EditAnywhere, Category = "RTSC_Settings|Rotation")
	FVector2D AimRotationHorizontalConstraint = FVector2D(-60.f, 60.f);
	
	UPROPERTY(EditAnywhere, Category = "RTSC_Settings|Rotation")
	FVector2D AimRotationVerticalConstraint = FVector2D(-10.f, 90.f);
	
	UPROPERTY(BlueprintReadOnly, Category = "RTSC_Settings|Rotation")
	FVector TargetLookLocation;
	
	UPROPERTY(BlueprintReadOnly, Category = "RTSC_Settings|Rotation")
	FRotator TargetAimRotator;

	UPROPERTY(EditAnywhere, Category = "RTSC_Settings|Rotation")
	float Rotate360TimeCoefficient = 2.f;

	UPROPERTY()
	bool bAimOnly = false;

	UPROPERTY()
	bool bResetCurrentLook = false;

	UPROPERTY()	
	float CurrentYaw;

	UPROPERTY()
	float TurnDirection;

	UPROPERTY()
	float TargetTurnDirection;

	UPROPERTY()
	FRotator LastLookAtRotator;

	UPROPERTY()
	FRotator CurrentLookAtRotator;

	UPROPERTY()
	bool bTurnToLocationTimelineExecuting = false;

	UPROPERTY()
	FRotator TargetTurnRotation;

	UPROPERTY()
	float ActorLookAtRotationYaw;

	UPROPERTY()
	float ProjectedDirection;
	
	UPROPERTY()
	float ProjectedSpeed;
	
	UPROPERTY()
	float RelativeYaw;

	// Movement Parameters
	UPROPERTY(EditAnywhere, Category = "RTSC_Settings|Movement")
	float MovementSmoothingCoefficient = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Movement")
	float DefaultFocusWalkSpeed = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Movement")
	float FocusMovementBackWalkMultiplier = .5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Movement")
	FVector2D FocusMovementForwardWalkSpeedLimits = FVector2D(200.f, 500.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSC_Settings|Movement")
	FVector2D FocusMovementBackWalkSpeedLimits = FVector2D(150.f, 300.f);

	// Combat Parameters
	UPROPERTY()
	ACharacter* FocusTarget;

	// Character State Parameters	
	UPROPERTY()
	bool bWasAiming;

	UPROPERTY()
	bool bIsAiming;
	
	UPROPERTY()
	bool bIsFocused;
	
	UPROPERTY()
	bool bIsObservingTarget;

	UPROPERTY()
	float CurrentUprightAmount = 1.f;

	UPROPERTY()
	float UprightAmount = 1.f;

	UPROPERTY()
	uint8 BehaviourState = 0;

private:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> Navigation;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USplineComponent> NavigationPath;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<URogueRVO_Component> RVOComponent;
	
	UFUNCTION()
	void OnTurnDirectionUpdate(float Alpha);
	
	UFUNCTION()
	void OnTurnRotationUpdate(float Alpha);
	
	UFUNCTION()
	void OnTurnToLocationFinished();
	
	UPROPERTY()
	UTimelineComponent* TurnToLocationTimelineComponent;

	
#if WITH_EDITOR
	void Debug() const;
#endif

	
	/** Entities Interface **/
public:
	virtual TObjectPtr<USplineComponent> GetNavigationSpline() const override;
	virtual void CreatePerceptionStimuliSourceComponent() override;
	virtual FName GetSightSocket() const override;
	virtual FVector GetSightSocketLocation() const override;
	virtual USceneComponent* GetEntityMesh() const override;	
	virtual UShapeComponent* GetCollisionBox() const override;
	
	/** Ai Interface **/
public:
	virtual void SetObservationLocation(const FVector& Location) override;	
	virtual void PlayMontage(UAnimMontage* MontageToPlay) override;
	virtual void Crouch() override { UprightAmount = 0.f; }
	virtual void Stand() override { UprightAmount = 1.f - FMath::RandRange(-0.1f, 0.f); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;

	/** Rogue RVO **/	
	virtual FVector GetRVOAvoidanceVelocity() const;
};
