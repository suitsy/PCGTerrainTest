#pragma once



#include "RTSVehicles_DataTypes.generated.h"

USTRUCT(BlueprintType)
struct FRTSVehicles_TankSplineAnim
{
	GENERATED_BODY()

	FRTSVehicles_TankSplineAnim(): AnimPointIndex(0), InteractWithWheel(false), VibrationMaxAmplitude(0), VibrationPhase(0),
	SaggingForward(0), SaggingBack(0) {}

	/** Specify the index of the spline point you want to animate **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 AnimPointIndex;

	/** Check for spline points above the wheels.When this option is enabled, the point will "lie" on the wheel if it is high enough. This avoids wheel-track crossing if the wheel can rise high enough to do so. **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool InteractWithWheel;

	/** Maximum vibration amplitude of a spline point **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float VibrationMaxAmplitude;

	/** Phase of oscillation, in degrees (360 = 1 full oscillation) **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float VibrationPhase;

	/** Offset spline point in perpendicular direction while moving forward **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SaggingForward;

	/** Offset of a spline point in the perpendicular direction when moving backwards **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SaggingBack;
};

USTRUCT()
struct FRTSVehicle_WheelRotationData
{
	GENERATED_BODY()

	FRTSVehicle_WheelRotationData():
		bIsLeftWheel(false),
		Distance(0),
		WheelRadius(0),
		TrackThickness(0),
		WheelSpeedCorrectionUV(0),
		WheelStartingAngleGeoL(0),
		WheelStartingAngleGeoR(0),
		WheelStartingAngleUVL(0),
		WheelStartingAngleUVR(0)
	{}

	FRTSVehicle_WheelRotationData(
		const bool IsLeftWheel,
		const float InDistance,
		const float InWheelRadius,
		const float InTrackThickness)
	:
		bIsLeftWheel(IsLeftWheel),
		Distance(InDistance),
		WheelRadius(InWheelRadius),
		TrackThickness(InTrackThickness),
		WheelSpeedCorrectionUV(0),
		WheelStartingAngleGeoL(0),
		WheelStartingAngleGeoR(0),
		WheelStartingAngleUVL(0),
		WheelStartingAngleUVR(0)
	{}

	FRTSVehicle_WheelRotationData(
		const bool IsLeftWheel,
		const float InDistance,
		const float InWheelRadius,
		const float InTrackThickness,
		const float InWheelSpeedCorrectionUV,
		const float InWheelStartingAngleGeoL,
		const float InWheelStartingAngleGeoR,
		const float InWheelStartingAngleUVL,
		const float InWheelStartingAngleUVR)
	:
		bIsLeftWheel(IsLeftWheel),
		Distance(InDistance),
		WheelRadius(InWheelRadius), 
		TrackThickness(InTrackThickness),
		WheelSpeedCorrectionUV(InWheelSpeedCorrectionUV),
		WheelStartingAngleGeoL(InWheelStartingAngleGeoL),
		WheelStartingAngleGeoR(InWheelStartingAngleGeoR),
		WheelStartingAngleUVL(InWheelStartingAngleUVL),
		WheelStartingAngleUVR(InWheelStartingAngleUVR)
	{}

	UPROPERTY()
	bool bIsLeftWheel;
	
	UPROPERTY()
	float Distance;

	UPROPERTY()
	float WheelRadius;

	UPROPERTY()
	float TrackThickness;

	UPROPERTY()
	float WheelSpeedCorrectionUV;

	UPROPERTY()
	float WheelStartingAngleGeoL;

	UPROPERTY()
	float WheelStartingAngleGeoR;

	UPROPERTY()
	float WheelStartingAngleUVL;

	UPROPERTY()
	float WheelStartingAngleUVR;	
};

USTRUCT(BlueprintType)
struct FRTSVehicles_AntennaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Stiffness;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Dampening;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Frequency;

	/** Rotation, at which the antenna will be in a state of equilibrium and around which oscillations will occur **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator Rotation;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GravityForce;
};

USTRUCT()
struct FRTSVehicle_WeaponData
{
	GENERATED_BODY()

	UPROPERTY()
	FName BoneName;

	UPROPERTY()
	float YawRotSpeed;

	UPROPERTY()
	FRotator CurrentRotation;

	UPROPERTY()
	FVector TargetPoint;	
};
