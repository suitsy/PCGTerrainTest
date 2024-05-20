// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Entities/Data/RTSVehicles_DataTypes.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "Framework/Interfaces/RTSCore_AiInterface.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "Framework/Interfaces/RTSCore_VehicleInputInterface.h"
#include "RTSVehicles_Base.generated.h"

#define RTS_MATERIAL_SLOT_NAME_TRACK_L		FName(TEXT("Track_L"))
#define RTS_MATERIAL_SLOT_NAME_TRACK_R		FName(TEXT("Track_R"))
#define RTS_SOCKET_NAME_TRACK_R				FString("R_wheeltrack_0")
#define RTS_SOCKET_NAME_TRACK_L				FString("R_wheeltrack_0")
#define RTS_SOCKET_NAME_TURRET				FName(TEXT("turret"))
#define RTS_MATERIAL_PARAM_ENGINE_RPM		FName(TEXT("RPM"))
#define RTS_MATERIAL_PARAM_TRACK			FName(TEXT("OffsetV"))
#define RTS_FX_PARAM_INTENSITY				FName(TEXT("Intensity"))

class URogueRVO_Component;
class URTSVehicle_RVOAvoidanceComponent;
class UBoxComponent;
class UAIPerceptionStimuliSourceComponent;
class USplineComponent;
/**
 * 
 */
UCLASS()
class RTSVEHICLES_API ARTSVehicles_Base : public AWheeledVehiclePawn, public IRTSCore_AiInterface, public IRTSCore_VehicleInputInterface, public IRTSCore_EntityInterface
{
	GENERATED_BODY()

public:
	explicit ARTSVehicles_Base(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	
	/** Animation Getters **/
	float GetWheelRotMiddleL() const { return WheelRotMiddleL; }
	float GetWheelRotMiddleR() const { return WheelRotMiddleR; }
	float GetWheelRotFrontL() const { return WheelRotFrontL; }
	float GetWheelRotFrontR() const { return WheelRotFrontR; }
	float GetWheelRotRearL() const { return WheelRotRearL; }
	float GetWheelRotRearR() const { return WheelRotRearR; }
	float GetWheelRotAccessoryL() const { return WheelRotAccessoryL; }
	float GetWheelRotAccessoryR() const { return WheelRotAccessoryR; }
	TArray<float> GetWheelZOffsets() const { return WheelZOffsets; }
	float GetSaggingDegreeL() const { return SaggingDegreeL; }
	float GetSaggingDegreeR() const { return SaggingDegreeR; }
	TArray<double> GetVibrationOffsetL() const { return VibrationOffsetL; }
	TArray<double> GetVibrationOffsetR() const { return VibrationOffsetR; }
	bool GetChassisLockedR()const { return bChassisLockedR; }
	bool GetChassisLockedL()const { return bChassisLockedL; }
	TArray<FRotator> GetAntennaRotation() const { return AntennaRotation; }
	TArray<FRotator> GetTurretRotations() const;
	TArray<FRotator> GetWeaponRotations() const;
	
protected:
	virtual void BeginPlay() override;
	void CreateTrackMaterials();
	void SplineMirrorCopy() const;
	TArray<UInstancedStaticMeshComponent*> CreateTrackInstances(USplineComponent* TrackPath, const bool GeoTracksFlip);
	void CreateEditorPreviewTracks() const;
	void SetTrackSplines();
	void SetSplinePointParameters();
	int32 FindSplineXClosestPoint(const int32 Index) const;
	void ChassisDistanceDefinition();
	void SetTrackSaggingDegree();
	float SaggingCalculation(const float CurrentSaggingDegree, const float ChassisDeltaDistance, const bool IsLocked) const;
	void UpdateTracksMaterials() const;
	void UpdateTracksPathAnimations();
	FVector PointLocationCalculation(const FRTSVehicles_TankSplineAnim& TankSplineAnimData, const int32 Index, const bool IsLeftSide);
	float VibrationCalculation(const float VibrationAmplitude, const float VibrationPhase) const;
	void TrackPathShift() const;
	void SetTracksTransform() const;
	float WheelRotationDefinition(const FRTSVehicle_WheelRotationData& WheelData) const;
	void UpdateAntennaData();

	float GetSpeedInfluence(const float ChassisDeltaDistance) const;
	float GetAccelerationInfluence(const float ChassisAcceleration) const;
	void HideTracks();
	void ShowUVTracks(const bool bShow) const;

	void SetWheelsRotations();
	void UpdateSuspensionOffset();
	void UpdateSlideParticleLocation() const;
	void UpdateSound(const float DeltaTime);
	void UpdateTrackDecals();

	UFUNCTION(Server, Unreliable)
	void Server_UpdateTrackDecals(UParticleSystemComponent* Particle, const bool bActive, const float Intensity);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_UpdateTrackDecals(UParticleSystemComponent* Particle, const bool bActive, const float Intensity);

	/** Use this variable to display geometry tracks in the viewport */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings")
	bool bEditorSplinePreview;

	/**
	 * Specify the geometric radius of the front wheel.
	 * Used to calculate the angle of rotation during animation
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelRadiusFront;

	/**
	 * Specify the geometric radius of the middle wheel.
	 * Used to calculate the angle of rotation during animation
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelRadiusMiddle;

	/**
	 * Specify the geometric radius of the rear wheel.
	 * Used to calculate the angle of rotation during animation
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelRadiusRear;

	/**
	 * Specify the geometric radius of the accessory wheel.
	 * Used to calculate the angle of rotation during animation
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelRadiusAccessory;

	/**
	 * This angle is additionally added to the rotation of the front and rear wheels.
	 * Adjust it so that the teeth of the drive sprocket fall into the holes of the tracks.
	 * Works when using geometric tracks.
	 * For right wheel
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelStartingAngleGeoR;

	/**
	 * This angle is additionally added to the rotation of the front and rear wheels.
	 * Adjust it so that the teeth of the drive sprocket fall into the holes of the tracks.
	 * Works when using geometric tracks.
	 * For left wheel
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelStartingAngleGeoL;

	/**
	 * This angle is additionally added to the rotation of the front and rear wheels.
	 * Adjust it so that the teeth of the drive sprocket fall into the holes of the tracks.
	 * Works when using UV tracks.
	 * For right wheel
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelStartingAngleUVR;

	/**
	 * This angle is additionally added to the rotation of the front and rear wheels.
	 * Adjust it so that the teeth of the drive sprocket fall into the holes of the tracks.
	 * Works when using UV tracks.
	 * For left wheel
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelStartingAngleUVL;

	/**
	 * Speeds up or slows down the rotation of the front and rear wheels.
	 * Use this variable to synchronize the rotation of the drive wheel with the tracks.
	 * If the wheel is spinning too fast, reduce this value, if it is slow, increase it.
	 * 
	 * It only has an effect when using UV tracks
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float WheelSpeedCorrectionUV;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	bool bUseGeometricTracks;

	/** Shows how far the chassis will travel before maximum sag */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks")
	float SaggingMaxDistance;

	/**
	 * Specify the minimum distance between the wheel and the ground.
	 * Used to calculate wheel animation
	 * (for Z-axis offset and rotation adjustments)
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	float TrackThickness;

	/** In order for the UV tracks to move the correct distance, you must specify the exact length of the repeating section of the track mesh, cm */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks")
	float TilingSegmentLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks|Animation")
	bool bInvertTrackDirection;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks|Animation")
	float SpeedInfluence;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks|Animation")
	float MaxSpeedInfluence;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks|Animation")
	float AccelerationInfluence;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks|Animation")
	float MaxAccelerationInfluence;

	/** The higher the parameter, the faster the tracks stop oscillating **/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks|Vibration")
	float DecayRate;

	/** Use this array to assign additional spline animations */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Chassis")
	TArray<FRTSVehicles_TankSplineAnim> TankSplineAnim;

	/** Specify track meshes that will move along the spline */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Chassis")
	TArray<UStaticMesh*> TrackStaticMeshes;

	/**
	 * Use this variable if the center left and right wheels are not symmetrical (this happens on tanks with torsion bar suspension).
	 * This variable shifts the bottom points of the left spline by a given distance (cm) in relation to the right spline
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	float MiddleWheelXOffset;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Wheels")
	TArray<float> WheelZOffsets;

	/**
	 * Setting tangents at the front bottom sag point
	 * (for the left spline of geometric tracks)
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	float FrontSagTangent;

	/**
	 * Setting tangents at the rear bottom sag point
	 * (for the left spline of geometric tracks)
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	float RearSagTangent;

	/** Flips the right geometric tracks along the Y axis */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	bool bGeoTracksFlipR;

	/** Flips the left geometric tracks along the Y axis */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	bool bGeoTracksFlipL;

	/**
	 * Additionally reduces the amplitude when the track touches the wheel
	 * 
	 * When set to 0, vibration stops completely when touched
	 * When set to 1, the amplitude does not change when touched
	 * 
	 * It begins to operate smoothly when the distance between the wheel
	 * and the track is less than the amplitude. The smaller this distance, the stronger the effect
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	float InteractionAmplitudeMultiplier;

	/** Maximum track oscillation frequency */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Tracks")
	float TrackFrequency;

	/** Specify the number of tracks on one spline */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks")
	int32 TracksAmount;

	/** The reverse of the turn when moving back */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Steering")
	bool bReverseTurnInReverse;

	/** The reverse of the turn when moving back */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Settings|Antenna")
	TArray<FRTSVehicles_AntennaData> AntennaData;

	/**
	 * Maximum linear speed limiter.
	 * If the speed is greater than this value, then the forward motion input will be reset to zero
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings")
	float MaxSpeedKmh;

	/**
	 * Maximum turn speed limit, deg/sec.
	 * If the angular velocity of the tank exceeds this value, then the Yaw input is set to zero
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Steering")
	float MaxTurningSpeed;

	/** Curve that establishes the relationship between the linear speed of the tank (x-axis) and the multiplier of the maximum turning speed (y-axis) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> SteeringCurve;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> SteeringDampeningCurve;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> CollisionSteeringCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> CollisionBrakingCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> BrakingCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> CorneringCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Control")
	TObjectPtr<UCurveFloat> ThrottleCurve;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|Tracks")
	float ProportionalCoefficient;

private:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USplineComponent> TrackPathR;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USplineComponent> TrackPathL;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> Navigation;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<USplineComponent> NavigationPath;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess = true))
	TObjectPtr<URogueRVO_Component> RVOComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere,  meta=(AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> Fire;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> TankEngine;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> TurretMotor;	

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UParticleSystemComponent> SlideBackLeft;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UParticleSystemComponent> SlideBackRight;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UParticleSystemComponent> P_Exhaust2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UParticleSystemComponent> P_Exhaust1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess = true))
	TObjectPtr<UParticleSystemComponent> P_Exhaust;
	
	UPROPERTY()
	float HullZRot;
	
	UPROPERTY()
	float HullDeltaZRot;
	
	UPROPERTY()
	float ChassisDistanceZRotComponentR;
	
	UPROPERTY()
	float ChassisDistanceZRotComponentL;
	
	UPROPERTY()
	bool bChassisLockedR;
	
	UPROPERTY()
	bool bChassisLockedL;

	/** Moving the body along the X axis in 1 frame. Used to calculate the movement of the tracks along the spline and sag when braking. **/
	UPROPERTY()
	float HullDeltaXLocation;
	
	UPROPERTY()
	float ChassisDistanceXMoveComponentR;
	
	UPROPERTY()
	float ChassisDistanceXMoveComponentL;

	/** Acceleration of right tracks used to calculate tracks vibration **/
	UPROPERTY()
	float ChassisAccelerationR;

	/** Acceleration of left tracks used to calculate track vibration **/
	UPROPERTY()
	float ChassisAccelerationL;
	
	UPROPERTY()
	float ChassisDistanceR;
	
	UPROPERTY()
	float ChassisDeltaDistanceR;
	
	UPROPERTY()
	float ChassisDistanceL;
	
	UPROPERTY()
	float ChassisDeltaDistanceL;

	/** Controls the sag of the tracks changes in the range from 0 to 1 **/
	UPROPERTY()
	float SaggingDegreeR;

	/** Controls the sag of the tracks changes in the range from 0 to 1 **/
	UPROPERTY()
	float SaggingDegreeL;

	UPROPERTY()
	UMaterialInstanceDynamic* RightTrackMID;

	UPROPERTY()
	UMaterialInstanceDynamic* LeftTrackMID;
	
	UPROPERTY()
	float AmplitudeMultiplierTargetR;
	
	UPROPERTY()
	float AmplitudeMultiplierTargetL;
	
	UPROPERTY()
	float CurrentAmplitudeMultiplierR;

	/** Reduces amplitude at low speeds, always <1 for right tracks**/
	UPROPERTY()
	float CurrentAmplitudeMultiplierL;

	/** Vectors that are perpendicular to the spline at the points of the spline **/
	UPROPERTY()
	TArray<FVector> SplinePointPerpendicularVectors;

	/** The initial position of the spline points **/
	UPROPERTY()
	TArray<FVector> SplinePointLocations;

	/**
	 * Indices of points under the wheels whose 3 coordinates will be used to offset points above the wheels.
	 * Used when option "InteractWithWheel" is enabled.
	 */
	UPROPERTY()
	TArray<int32> CopyPointIndices;

	/**
	 * Calculated relative vibration offset, in the range -1 to 1.
	 * For the right side
	 */
	UPROPERTY()
	TArray<double> VibrationOffsetR;

	/**
	 * Calculated relative vibration offset, in the range -1 to 1.
	 * For the left side
	 */
	UPROPERTY()
	TArray<double> VibrationOffsetL;

	UPROPERTY()
	float FilletsCompensation;
	
	UPROPERTY()
	TArray<UInstancedStaticMeshComponent*> TracksInstances_R;

	UPROPERTY()
	TArray<UInstancedStaticMeshComponent*> TracksInstances_L;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseTrackMaterial;

	/** The number of static track meshes used in the animation **/
	UPROPERTY()
	int32 UniqueTrackMeshesAmount;

	/** The current angle that the middle left wheel is turned to */
	UPROPERTY()
	float WheelRotMiddleL;

	/** The current angle that the middle right wheel is turned to */
	UPROPERTY()
	float WheelRotMiddleR;

	/** The current angle that the front left wheel is turned to */
	UPROPERTY()
	float WheelRotFrontL;

	/** The current angle that the front right wheel is turned to */
	UPROPERTY()
	float WheelRotFrontR;

	/** The current angle that the rear left wheel is turned to */
	UPROPERTY()
	float WheelRotRearL;

	/** The current angle that the rear right wheel is turned to */
	UPROPERTY()
	float WheelRotRearR;

	/** The current angle that the accessory left wheel is turned to */
	UPROPERTY()
	float WheelRotAccessoryL;

	/** The current angle that the accessory right wheel is turned to */
	UPROPERTY()
	float WheelRotAccessoryR;

	UPROPERTY()
	float CurrentRPMRatio;

	UPROPERTY()
	bool bIsTurretRotating;

	UPROPERTY()
	bool bDestroyed;

	UPROPERTY()
	float ForwardSpeed;

	// Antenna	
	UPROPERTY()
	FVector InvertedLocalTurretSpeed;

	UPROPERTY()
	FVector InvertedLocalTurretAcceleration;

	UPROPERTY()
	FVector InvertedLocalHullSpeed;

	UPROPERTY()
	FVector InvertedLocalHullAcceleration;

	UPROPERTY()
	TArray<FVector> AntennaCurrentSpeed;

	UPROPERTY()
	TArray<FRotator> AntennaRotation;

	UPROPERTY()
	TArray<FRotator> TurretRotations;

	UPROPERTY()
	TArray<FRotator> GunRotations;

	// Turret
	UPROPERTY()
	FVector TargetPoint;

	
	/** Vehicle Input **/
public:
	/** IRTSCore_VehicleInputInterface **/
	virtual void ApplyVehicleInput(const FRTSCore_VehicleInput& VehicleInput) override;	
	virtual void SetThrottle(float Throttle) override;
	virtual void SetSteering(float Steering) override;
	virtual void SetBrake(const float Brake) override;
	virtual void SetHandbrake(const bool bHandbrake) override;
	virtual void SetParked(const bool bIsParked) override;
	virtual float GetThrottle() const override;
	virtual float GetBrake() const override;
	virtual float GetMass() const override;
	virtual bool IsParked() const override;
	virtual FVector GetRVOAvoidanceVelocity() const override;
	
	virtual float GetMaxSpeed() const override { return MaxSpeedKmh; }
	virtual float GetMaxTurningSpeed() const override { return MaxTurningSpeed; }
	virtual float GetThrottleInput() const override;
	virtual bool GetHandBrakeInput() const override;
	virtual float GetBrakeInput() const override;
	virtual bool IsMaxAngularVelocity() const override;
	virtual TObjectPtr<UCurveFloat> GetBrakingCurve() const override { return BrakingCurve; }
	virtual TObjectPtr<UCurveFloat> GetCorneringCurve() const override { return CorneringCurve; }
	virtual TObjectPtr<UCurveFloat> GetThrottleCurve() const override { return ThrottleCurve; }
	virtual TObjectPtr<UCurveFloat> GetSteeringCurve() const override { return SteeringCurve; }
	virtual TObjectPtr<UCurveFloat> GetSteeringDampeningCurve() const override { return SteeringDampeningCurve; }
	virtual TObjectPtr<UCurveFloat> GetCollisionSteeringCurve() const override { return CollisionSteeringCurve; }
	virtual TObjectPtr<UCurveFloat> GetCollisionBrakeCurve() const override { return CollisionBrakingCurve; }
	/** End IRTSCore_VehicleInputInterface **/

private:
	
#if WITH_EDITOR
	void DebugVehicleInput(const float Throttle, const float InSteering, const float Brake) const;	
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
	virtual void Crouch() override {  }
	virtual void Stand() override {  }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
};