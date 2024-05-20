// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Vehicle/RTSVehicles_Base.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Framework/Components/RogueRVO_Component.h"
#include "Framework/Components/RogueRVO_VehicleMovementComponent.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

ARTSVehicles_Base::ARTSVehicles_Base(const FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer.SetDefaultSubobjectClass<URogueRVO_VehicleMovementComponent>(AWheeledVehiclePawn::VehicleMovementComponentName))
{	
	if(GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(RTS_TRACE_CHANNEL_ENTITIES, ECR_Block);
		GetMesh()->SetReceivesDecals(false);
	}
	
	// States
	bDestroyed = false;
	bUseGeometricTracks = true;
	bGeoTracksFlipR = false;
	bGeoTracksFlipL = false;
	bEditorSplinePreview = true;
	bIsTurretRotating = false;
	bReverseTurnInReverse = true;

	// Tracks
	TrackThickness = 4.f;
	TrackFrequency = 1500.f;
	TracksAmount = 50.f;
	TilingSegmentLength = 70.f;
	SpeedInfluence = 0.3f;
	MaxSpeedInfluence = 0.6f;
	AccelerationInfluence = 0.2f;
	MaxAccelerationInfluence = 1.f;
	DecayRate = 0.3f;
	FrontSagTangent = 0.f;
	RearSagTangent = 0.f;
	SaggingMaxDistance = 20.f;
	CurrentRPMRatio = 0.f;
	ProportionalCoefficient = 5.f;
	InteractionAmplitudeMultiplier = 0.4f;
	RightTrackMID = nullptr;
	LeftTrackMID = nullptr;
	AmplitudeMultiplierTargetR = 0.f;
	AmplitudeMultiplierTargetL = 0.f;
	CurrentAmplitudeMultiplierR = 0.f;
	CurrentAmplitudeMultiplierL = 0.f;
	FilletsCompensation = 0.f;
	UniqueTrackMeshesAmount = 0.f;
	
	// Hull
	HullZRot = 0.f;
	HullDeltaZRot = 0.f;
	HullDeltaXLocation = 0.f;

	// Chassis
	ChassisDistanceXMoveComponentR = 0.f;
	ChassisDistanceXMoveComponentL = 0.f;
	ChassisDistanceZRotComponentR = 0.f;
	ChassisDistanceZRotComponentL = 0.f;
	ChassisAccelerationR = 0.f;
	ChassisAccelerationL = 0.f;
	ChassisDistanceR = 0.f;
	ChassisDistanceL = 0.f;
	ChassisDeltaDistanceR = 0.f;
	ChassisDeltaDistanceL = 0.f;
	SaggingDegreeR = 0.f;
	SaggingDegreeL = 0.f;

	// Wheels
	WheelRadiusFront = 22.5;
	WheelRadiusMiddle = 38.f;
	WheelRadiusRear = 22.f;
	WheelRadiusAccessory = 11.f;
	WheelStartingAngleGeoL = 0.f;
	WheelStartingAngleGeoR = 0.f;
	WheelStartingAngleUVR = 0.f;
	WheelStartingAngleUVL = 0.f;
	WheelSpeedCorrectionUV = 0.f;
	MiddleWheelXOffset = 0.f;
	WheelRotMiddleL = 0.f;
	WheelRotMiddleR = 0.f;
	WheelRotFrontL = 0.f;
	WheelRotFrontR = 0.f;
	WheelRotRearL = 0.f;
	WheelRotRearR = 0.f;
	WheelRotAccessoryL = 0.f;
	WheelRotAccessoryR = 0.f;
	CurrentRPMRatio = 0.f;

	// Turrets

	// Input
	MaxTurningSpeed = 45.f;	
	ForwardSpeed = 0.f;
	MaxSpeedKmh = 60.f;
	
	// Track creation
	CreateTrackMaterials();
	TrackPathR = CreateDefaultSubobject<USplineComponent>(TEXT("TrackPath_R"));
	TrackPathR->SetupAttachment(RootComponent);
	TrackPathR->SetClosedLoop(true);
	TrackPathR->DefaultUpVector = FVector(0.f,-1.f,0.f);
	TrackPathR->Mobility = EComponentMobility::Movable;
	TrackPathR->SetRelativeLocation(FVector(0.f, 133.6, 0.f));
	TrackPathR->SetRelativeRotation(FRotator(0.f,-180.f,0.f));
	TrackPathL = CreateDefaultSubobject<USplineComponent>(TEXT("TrackPath_L"));
	TrackPathL->SetupAttachment(RootComponent);
	TrackPathL->SetClosedLoop(true);
	TrackPathL->DefaultUpVector = FVector(0.f,-1.f,0.f);
	TrackPathL->Mobility = EComponentMobility::Movable;
	TrackPathL->SetRelativeLocation(FVector(0.f, -133.6, 0.f));
	TrackPathL->SetRelativeRotation(FRotator(0.f,-180.f,0.f));

	// Collision
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	// Navigation Spline	
	Navigation = CreateDefaultSubobject<USceneComponent>(TEXT("NavigationComponent"));
	NavigationPath = CreateDefaultSubobject<USplineComponent>(TEXT("NavigationSpline"));	
	NavigationPath->SetupAttachment(Navigation);
	NavigationPath->SetAbsolute(true, true);
	NavigationPath->SetClosedLoop(false);
	NavigationPath->Mobility = EComponentMobility::Movable;

	// RVO	
	RVOComponent = CreateDefaultSubobject<URogueRVO_Component>(TEXT("RVOComponent"));

	// Audio
	TankEngine = CreateDefaultSubobject<UAudioComponent>(TEXT("TankEngine"));
	TurretMotor = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretMotor"));
	Fire = CreateDefaultSubobject<UAudioComponent>(TEXT("Fire"));

	// Effects
	SlideBackLeft = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SlideBackLeft"));
	SlideBackLeft->SetupAttachment(RootComponent);
	SlideBackRight = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SlideBackRight"));
	SlideBackRight->SetupAttachment(RootComponent);
	P_Exhaust = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("P_Exhaust"));
	P_Exhaust->SetupAttachment(RootComponent);
	P_Exhaust1 = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("P_Exhaust1"));
	P_Exhaust1->SetupAttachment(RootComponent);
	P_Exhaust2 = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("P_Exhaust2"));
	P_Exhaust2->SetupAttachment(RootComponent);

	CreateEditorPreviewTracks();
}

void ARTSVehicles_Base::PostInitializeComponents()
{
	Super::PostInitializeComponents();	

	
}

void ARTSVehicles_Base::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(!bDestroyed)
	{
		ForwardSpeed = GetVehicleMovement()->GetForwardSpeed();
		ChassisDistanceDefinition();
		SetTrackSaggingDegree();

		if(!bUseGeometricTracks)
		{
			UpdateTracksMaterials();
		}
		else
		{
			TrackPathShift();
			SetTracksTransform();
		}
		
		UpdateTracksPathAnimations();
		
		SetWheelsRotations();
		UpdateSuspensionOffset();
		UpdateSlideParticleLocation();
		UpdateTrackDecals();
		UpdateAntennaData();
		UpdateSound(DeltaSeconds);
	}	
}

void ARTSVehicles_Base::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);

	OutLocation = GetMesh()->GetSocketLocation(GetSightSocket());	
	OutRotation = FRotator(0.f, GetMesh()->GetSocketRotation(GetSightSocket()).Yaw, 0.f);
}

TArray<FRotator> ARTSVehicles_Base::GetTurretRotations() const
{
	if(GetController())
	{
		if(const IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(GetController()))
		{
			TArray<FRotator> Rotations = EntityEquipmentInterface->GetTurretRotations();
			return Rotations;
		}
	}

	return TArray<FRotator>{};
}

TArray<FRotator> ARTSVehicles_Base::GetWeaponRotations() const
{
	if(GetController())
	{
		if(const IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(GetController()))
		{
			return EntityEquipmentInterface->GetWeaponRotations();
		}
	}

	return TArray<FRotator>{};
}

void ARTSVehicles_Base::BeginPlay()
{
	Super::BeginPlay();

	if(GetMesh())
	{
		GetMesh()->SetAllBodiesPhysicsBlendWeight(0.f);
		//TrackPathR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		//TrackPathL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
	
	SplineMirrorCopy();
	TracksInstances_R = CreateTrackInstances(TrackPathR, bGeoTracksFlipR);
	TracksInstances_L = CreateTrackInstances(TrackPathL, bGeoTracksFlipL);
	FilletsCompensation = TrackThickness / 2.f;
	SetTrackSplines();
	SetSplinePointParameters();
	HideTracks();
	
}

void ARTSVehicles_Base::CreateTrackMaterials()
{
	if(GetMesh())
	{
		LeftTrackMID = GetMesh()->CreateDynamicMaterialInstance(GetMesh()->GetMaterialIndex(RTS_MATERIAL_SLOT_NAME_TRACK_L), BaseTrackMaterial);
		RightTrackMID = GetMesh()->CreateDynamicMaterialInstance(GetMesh()->GetMaterialIndex(RTS_MATERIAL_SLOT_NAME_TRACK_R), BaseTrackMaterial);
	}
}

void ARTSVehicles_Base::SplineMirrorCopy() const
{
	if(TrackPathR == nullptr || TrackPathL == nullptr)
	{
		return;
	}
	
	FTransform LeftTransform = TrackPathR->GetRelativeTransform();
	const FVector LeftLocation = LeftTransform.GetLocation();
	LeftTransform.SetLocation(FVector(LeftLocation.X, LeftLocation.Y * -1, LeftLocation.Z));
	TrackPathL->SetRelativeTransform(LeftTransform);
	
	const float FirstPointZLocation = TrackPathR->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Z;
	for (int i = 0; i < TrackPathR->GetNumberOfSplinePoints() - 2; ++i)
	{
		TrackPathL->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local);
	}

	for (int j = 0; j < TrackPathL->GetNumberOfSplinePoints(); ++j)
	{
		const bool BottomFrontSagPoint = TrackPathR->GetNumberOfSplinePoints() - 1 == j;		
		const float WrapIndex = j-1 == -1 ? TrackPathR->GetNumberOfSplinePoints() - 1 : j;		
		const bool BottomRearSagPoint = !(FMath::Abs(TrackPathR->GetLocationAtSplinePoint(j, ESplineCoordinateSpace::Local).Z - FirstPointZLocation) < 2.f)
		&& FMath::Abs(TrackPathR->GetLocationAtSplinePoint(WrapIndex, ESplineCoordinateSpace::Local).Z - FirstPointZLocation) < 2.f;

		
		const FVector SplinePointLocation = TrackPathR->GetLocationAtSplinePoint(j, ESplineCoordinateSpace::Local);
		const bool IsUnder = FMath::Abs(SplinePointLocation.Z - FirstPointZLocation) < 2.f;
		const float BottomPoint = IsUnder? MiddleWheelXOffset + SplinePointLocation.X : SplinePointLocation.X;		
		const float AdditionalOffset = (BottomFrontSagPoint || BottomRearSagPoint) ? MiddleWheelXOffset / 2 : 0.f;
		const float NewPointPositionX = BottomPoint + AdditionalOffset;
		
		const FVector PointArriveTangent = TrackPathR->GetArriveTangentAtSplinePoint(j, ESplineCoordinateSpace::Local);
		const float TangentCorrectionX = BottomRearSagPoint ? RearSagTangent : (BottomFrontSagPoint ? FrontSagTangent : 0.f);
		const FVector PointLeaveTangent = TrackPathR->GetLeaveTangentAtSplinePoint(j, ESplineCoordinateSpace::Local);
		
		TrackPathL->SetLocationAtSplinePoint(j,FVector(NewPointPositionX, SplinePointLocation.Y, SplinePointLocation.Z), ESplineCoordinateSpace::Local);
		TrackPathL->SetTangentsAtSplinePoint(j,
			FVector(PointArriveTangent.X + TangentCorrectionX, PointArriveTangent.Y, PointArriveTangent.Z),
			FVector(PointLeaveTangent.X + TangentCorrectionX, PointLeaveTangent.Y, PointLeaveTangent.Z),
			ESplineCoordinateSpace::Local
		);
	}
}

TArray<UInstancedStaticMeshComponent*> ARTSVehicles_Base::CreateTrackInstances(USplineComponent* TrackPath, const bool GeoTracksFlip)
{
	TArray<UInstancedStaticMeshComponent*> TrackInstances;
	if(TrackPath != nullptr)
	{
		UniqueTrackMeshesAmount = 0.f;
		for (int i = 0; i < TrackStaticMeshes.Num(); ++i)
		{
			if(TrackStaticMeshes[i] != nullptr)
			{
				UniqueTrackMeshesAmount++;
			}
		}

		if(UniqueTrackMeshesAmount > 0)
		{
			for (int j = 0; j < TrackStaticMeshes.Num(); ++j)
			{
				// Create an instance of the static mesh
				UInstancedStaticMeshComponent* InstancedStaticMesh = NewObject<UInstancedStaticMeshComponent>(this);
				InstancedStaticMesh->RegisterComponent();
				InstancedStaticMesh->SetStaticMesh(TrackStaticMeshes[j]);

				// Attach the instance to the spline component
				InstancedStaticMesh->AttachToComponent(TrackPath, FAttachmentTransformRules::KeepRelativeTransform);

				FTransform InstanceTransform = FTransform::Identity;
				InstanceTransform.SetScale3D(FVector(1.f, GeoTracksFlip ? -1.f : 1.f, 1.f));
				InstancedStaticMesh->SetRelativeTransform(InstanceTransform);

				// Disable collision for the instance
				InstancedStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				// Add instances based on TracksAmount
				const float SplineInputKey = (TracksAmount / UniqueTrackMeshesAmount);
				for (int32 k = 0; k <= SplineInputKey; ++k)
				{
					// Add the instance
					InstancedStaticMesh->AddInstance(FTransform::Identity, false);
				}

				TrackInstances.Add(InstancedStaticMesh);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] UniqueTrackMeshesAmount is Zero!"), *GetClass()->GetName());
		}
	}

	return TrackInstances;
}

void ARTSVehicles_Base::CreateEditorPreviewTracks() const
{
	if(bEditorSplinePreview)
	{
		if(TracksInstances_R.Num() > 0 && TracksInstances_L.Num() > 0)
		{
			SetTracksTransform();
		}
	}
}

void ARTSVehicles_Base::SetTrackSplines()
{
	for (int l = 0; l < 2; ++l)
	{
		const bool IsLeftSide = l == 0;
		USplineComponent* TrackPath = IsLeftSide ? TrackPathL : TrackPathR;

		if(TrackPath == nullptr)
		{
			continue;
		}
		
		for (int i = 0; i < TrackPath->GetNumberOfSplinePoints(); ++i)
		{
			const FVector Location = TrackPath->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			const FVector Right = TrackPath->GetRightVectorAtSplinePoint(i, ESplineCoordinateSpace::Local) * (TrackThickness / -2.f);
			TrackPath->SetLocationAtSplinePoint(i, Location + Right, ESplineCoordinateSpace::Local);

			IsLeftSide ? VibrationOffsetL.Add(0.f) : VibrationOffsetR.Add(0.f);			
		}
	}
}

void ARTSVehicles_Base::SetSplinePointParameters()
{
	for (int i = 0; i < TankSplineAnim.Num(); ++i)
	{
		const FTransform SplinePointTransform = TrackPathR->GetTransformAtSplinePoint(TankSplineAnim[i].AnimPointIndex, ESplineCoordinateSpace::Local);
		SplinePointLocations.Add(SplinePointTransform.GetLocation());
		SplinePointPerpendicularVectors.Add(UKismetMathLibrary::Conv_RotatorToVector(UKismetMathLibrary::ComposeRotators(SplinePointTransform.GetRotation().Rotator(), FRotator(90.f,0.f,0.f))));

		if(TankSplineAnim[i].InteractWithWheel)
		{
			CopyPointIndices.Add(FindSplineXClosestPoint(TankSplineAnim[i].AnimPointIndex));
		}
		else
		{
			CopyPointIndices.Add(0);
		}
	}
}

int32 ARTSVehicles_Base::FindSplineXClosestPoint(const int32 Index) const
{
	int32 ClosestPointIndex = 0;
	float MinimalDeltaXLocation = 0.f;
	const float CurrentPointXLocation = TrackPathR->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::Local).X;
	for (int i = 0; i < TrackPathR->GetNumberOfSplinePoints(); ++i)
	{
		const float NewMinimalDeltaXLocation = FMath::Abs(CurrentPointXLocation - TrackPathR->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local).X);
		if(i != Index && (NewMinimalDeltaXLocation < MinimalDeltaXLocation || MinimalDeltaXLocation == 0.f))
		{
			MinimalDeltaXLocation = NewMinimalDeltaXLocation;
			ClosestPointIndex = i;
		}
	}	

	return ClosestPointIndex;
}

void ARTSVehicles_Base::ChassisDistanceDefinition()
{
	// Set chassis distance Z rotation component
	if(GetMesh())
	{
		HullDeltaZRot = HullZRot - GetMesh()->GetComponentRotation().Yaw;
		HullZRot = GetMesh()->GetComponentRotation().Yaw;

		if(!bChassisLockedR)
		{
			ChassisDistanceZRotComponentR -= (HullDeltaZRot * ProportionalCoefficient);
		}

		if(!bChassisLockedL)
		{
			ChassisDistanceZRotComponentL -= (HullDeltaZRot * ProportionalCoefficient);
		}
	}

	// Set Chassis distance X movement component
	HullDeltaXLocation = ForwardSpeed * GetWorld()->GetDeltaSeconds();

	if(!bChassisLockedR)
	{
		ChassisDistanceXMoveComponentR += HullDeltaXLocation;
	}
	
	if(!bChassisLockedL)
	{
		ChassisDistanceXMoveComponentL += HullDeltaXLocation;
	}
	

	// Set chassis acceleration definition
	ChassisAccelerationR = ((ChassisDistanceXMoveComponentR - ChassisDistanceZRotComponentR) - ChassisDistanceR) - ChassisDeltaDistanceR;
	ChassisAccelerationL = ((ChassisDistanceXMoveComponentL - ChassisDistanceZRotComponentL) - ChassisDistanceL) - ChassisDeltaDistanceL;

	// Set chassis distance R and L definitions
	ChassisDeltaDistanceR = (ChassisDistanceXMoveComponentR - ChassisDistanceZRotComponentR) - ChassisDistanceR;
	ChassisDistanceR = ChassisDistanceXMoveComponentR - ChassisDistanceZRotComponentR;
	ChassisDeltaDistanceL = (ChassisDistanceXMoveComponentL - ChassisDistanceZRotComponentL) - ChassisDistanceL;
	ChassisDistanceL = ChassisDistanceXMoveComponentL - ChassisDistanceZRotComponentL;

	/** Bug fix.
	 *  When the value of the variable reaches a large value, the calculation becomes inaccurate,
	 *  as a result of which the wheels and tracks move with large oscillations.
	 *  Therefore, the values of the variables are periodically reset to zero. At high speed, the reset is imperceptible
	 **/
	if(FMath::Abs(ForwardSpeed) > 300.f && FMath::Abs(ChassisDistanceR) > TrackPathR->GetSplineLength())
	{
		ChassisDistanceZRotComponentR = 0.f;
		ChassisDistanceXMoveComponentR = 0.f;
		ChassisDistanceR = 0.f;
		ChassisDistanceZRotComponentL = 0.f;
		ChassisDistanceXMoveComponentL = 0.f;
		ChassisDistanceL = 0.f;
	}
}

void ARTSVehicles_Base::SetTrackSaggingDegree()
{
	SaggingDegreeR = SaggingCalculation(SaggingDegreeR, ChassisDeltaDistanceR, bChassisLockedR);
	SaggingDegreeL = SaggingCalculation(SaggingDegreeL, ChassisDeltaDistanceL, bChassisLockedL);
}

float ARTSVehicles_Base::SaggingCalculation(const float CurrentSaggingDegree, const float ChassisDeltaDistance,	const bool IsLocked) const
{
	return FMath::Clamp(((IsLocked ? (HullDeltaXLocation * -1) : ChassisDeltaDistance) / SaggingMaxDistance) + CurrentSaggingDegree, 0.f, 1.f);
}

void ARTSVehicles_Base::UpdateTracksMaterials() const
{
	for (int i = 0; i < 2; ++i)
	{
		const bool IsLeftSide = i == 0;
		UMaterialInstanceDynamic* Material = IsLeftSide ? LeftTrackMID : RightTrackMID;
		const float ChassisDistance = IsLeftSide ? ChassisDistanceL : ChassisDistanceR;
		
		if(Material != nullptr)
		{
			double Remainder;
			const float Size = UKismetMathLibrary::FMod(static_cast<double>(ChassisDistance / TilingSegmentLength), static_cast<double>(1.0f), Remainder);
			Material->SetScalarParameterValue(RTS_MATERIAL_PARAM_TRACK, bInvertTrackDirection ? Size * -1.f : Size);
		}
	}
}

void ARTSVehicles_Base::UpdateTracksPathAnimations()
{
	AmplitudeMultiplierTargetR = GetSpeedInfluence(ChassisDeltaDistanceR) + GetAccelerationInfluence(ChassisAccelerationR);
	CurrentAmplitudeMultiplierR = FMath::FInterpConstantTo((AmplitudeMultiplierTargetR > CurrentAmplitudeMultiplierR ? AmplitudeMultiplierTargetR : CurrentAmplitudeMultiplierR), AmplitudeMultiplierTargetR, GetWorld()->GetDeltaSeconds(), DecayRate);
	AmplitudeMultiplierTargetL = GetSpeedInfluence(ChassisDeltaDistanceL) + GetAccelerationInfluence(ChassisAccelerationL);
	CurrentAmplitudeMultiplierL = FMath::FInterpConstantTo(AmplitudeMultiplierTargetL > CurrentAmplitudeMultiplierL ? AmplitudeMultiplierTargetL : CurrentAmplitudeMultiplierL, AmplitudeMultiplierTargetL, GetWorld()->GetDeltaSeconds(), DecayRate);

	for (int i = 0; i < TankSplineAnim.Num(); ++i)
	{
		TrackPathR->SetLocationAtSplinePoint(TankSplineAnim[i].AnimPointIndex, PointLocationCalculation(TankSplineAnim[i], i, false), ESplineCoordinateSpace::Local);
		TrackPathL->SetLocationAtSplinePoint(TankSplineAnim[i].AnimPointIndex, PointLocationCalculation(TankSplineAnim[i], i, true), ESplineCoordinateSpace::Local);
	}
}

FVector ARTSVehicles_Base::PointLocationCalculation(const FRTSVehicles_TankSplineAnim& TankSplineAnimData, const int32 Index, const bool IsLeftSide)
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		const int32 MiddleWheelsAmount = WheeledVehicleMovementComponent->GetNumWheels() / 2;

		if(SplinePointPerpendicularVectors.IsValidIndex(Index) && SplinePointLocations.IsValidIndex(Index))
		{
			const float SaggingDegree = IsLeftSide ? SaggingDegreeL : SaggingDegreeR;
			const float AmplitudeBeforeInteraction = IsLeftSide ? CurrentAmplitudeMultiplierL : CurrentAmplitudeMultiplierR * TankSplineAnimData.VibrationMaxAmplitude;
			const USplineComponent* TrackPath = IsLeftSide ? TrackPathL : TrackPathR;
			

			const FVector Sagging = FMath::InterpEaseInOut(TankSplineAnimData.SaggingBack, TankSplineAnimData.SaggingForward, SaggingDegree, 1.0f) * SplinePointPerpendicularVectors[Index];
			const FVector StartLocation = SplinePointLocations[Index];
			FVector RearSagPoint = FVector::ZeroVector;
			FVector FrontSagPoint = FVector::ZeroVector;

			/** To correct the bending in the wrong direction.
			 *  The lower sagging points are displaced along the Z-axis by a distance equal to half the suspension offset at the nearest wheel.
			 *  Also on the left spline there is a shift along the X axis for asymmetric chassis **/
			//const float X = (TrackPath == TrackPathL) ? MiddleWheelXOffset / 2.f : 0.f;

			// Rear sag point
			if(TankSplineAnimData.AnimPointIndex == MiddleWheelsAmount)
			{
				const float WheelIndex = IsLeftSide ? (MiddleWheelsAmount * 2) - 1 : MiddleWheelsAmount - 1;
				if(WheeledVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex))
				{					
					const float Z = WheeledVehicleMovementComponent->Wheels[WheelIndex]->GetSuspensionOffset() / 2.f;
					RearSagPoint = FVector(0.f, 0.f, Z);
				}
			}

			// Front sag point
			if(TankSplineAnimData.AnimPointIndex == TrackPath->GetNumberOfSplinePoints() - 1)
			{
				const float WheelIndex = IsLeftSide ? MiddleWheelsAmount : 0.f;
				if(WheeledVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex))
				{
					const float Z = WheeledVehicleMovementComponent->Wheels[WheelIndex]->GetSuspensionOffset() / 2.f;
					FrontSagPoint = FVector(0.f, 0.f, Z);
				}
			}

			// Calculate spline point location
			const FVector SplinePointLocation = Sagging + StartLocation + RearSagPoint + FrontSagPoint;


			// Calculate min Z location for interaction with wheel
			float MinZLocation = 0.f;			
			if(CopyPointIndices.IsValidIndex(Index))
			{
				MinZLocation = TankSplineAnimData.InteractWithWheel ? (TrackPath->GetLocationAtSplinePoint(CopyPointIndices[Index], ESplineCoordinateSpace::Local).Z + (WheelRadiusMiddle * 2.f) + TrackThickness) : 0.f;
			}

			// Calculate vibration offset
			const float DistWheelToTrack = SplinePointLocation.Z - MinZLocation;
			const float VibrationAmplitude = FMath::GetMappedRangeValueClamped(FVector2D(0.f,AmplitudeBeforeInteraction), FVector2D(InteractionAmplitudeMultiplier,1.f), DistWheelToTrack) * AmplitudeBeforeInteraction;
			const float CurrentVibrationOffset = VibrationCalculation(VibrationAmplitude, TankSplineAnimData.VibrationPhase);
			if(IsLeftSide)
			{
				if(VibrationOffsetL.IsValidIndex(TankSplineAnimData.AnimPointIndex))
				{
					VibrationOffsetL[TankSplineAnimData.AnimPointIndex] = CurrentVibrationOffset;
				}
			}
			else
			{
				if(VibrationOffsetR.IsValidIndex(TankSplineAnimData.AnimPointIndex))
				{
					VibrationOffsetR[TankSplineAnimData.AnimPointIndex] = CurrentVibrationOffset;
				}
			}

			if(TankSplineAnimData.InteractWithWheel)
			{
				return (FVector(SplinePointLocation.X, SplinePointLocation.Y, FMath::Clamp(SplinePointLocation.Z, MinZLocation, 10000.f)) + (CurrentVibrationOffset * SplinePointPerpendicularVectors[Index]));
			}
			else
			{
				return SplinePointLocation + (CurrentVibrationOffset * SplinePointPerpendicularVectors[Index]);
			}
		}		
	}

	return FVector::ZeroVector;	
}

float ARTSVehicles_Base::VibrationCalculation(const float VibrationAmplitude, const float VibrationPhase) const
{
	return FMath::Sin(UE_DOUBLE_PI/(180.0) * (VibrationPhase + (GetWorld()->GetTimeSeconds() * TrackFrequency))) * VibrationAmplitude; 
}

void ARTSVehicles_Base::TrackPathShift() const
{
	for (int i = 0; i < 2; ++i)
	{
		const bool IsLeftSide = i == 0;
		USplineComponent* TrackPath = IsLeftSide ? TrackPathL : TrackPathR;
		if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
		{
			const int32 HalfLength = (WheeledVehicleMovementComponent->Wheels.Num() / 2) - 1;
			for (int j = 0; j < HalfLength; ++j)
			{
				const FVector Location = TrackPath->GetLocationAtSplinePoint(j, ESplineCoordinateSpace::Local);
				const float WheelIndex = IsLeftSide ? j + HalfLength : j;
				if(WheeledVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex))
				{
					const float Z = WheeledVehicleMovementComponent->Wheels[WheelIndex]->GetSuspensionOffset() + TrackThickness + (FilletsCompensation * -1.f);
					TrackPath->SetLocationAtSplinePoint(j, FVector(Location.X, Location.Y, Z), ESplineCoordinateSpace::Local);
				}
			}
		}
	}
}

void ARTSVehicles_Base::SetTracksTransform() const
{
	for (int l = 0; l < 2; ++l)
	{
		const bool IsLeftSide = l == 0;
		TArray<UInstancedStaticMeshComponent*> TrackInstances = IsLeftSide ? TracksInstances_L : TracksInstances_R;
		USplineComponent* TrackPath = IsLeftSide ? TrackPathL : TrackPathR;
		const float ChassisDistance = IsLeftSide ? ChassisDistanceL : ChassisDistanceR;
		
		if(TrackPath != nullptr && UniqueTrackMeshesAmount > 0)
		{
			const float TrackPathLength = TrackPath->GetSplineLength();
			const float TrackTypeCount = static_cast<float>(TracksAmount / UniqueTrackMeshesAmount);
			const float TrackInterval = TrackPathLength / TrackTypeCount;

			for (int i = 0; i < TrackInstances.Num(); ++i)
			{
				const float SplineLength = ChassisDistance + ((TrackInterval / UniqueTrackMeshesAmount) * i);
				double RemainderA;
				double RemainderB;
				UKismetMathLibrary::FMod(static_cast<double>(SplineLength), static_cast<double>(TrackPathLength), RemainderA);
				UKismetMathLibrary::FMod(static_cast<double>(TrackPathLength) + RemainderA, static_cast<double>(TrackPathLength), RemainderB);
				const double ChassisCurrentDistance = RemainderB;
				const int TracksBeforeSplineEnd = UKismetMathLibrary::FMod(static_cast<double>(TrackPathLength) - ChassisCurrentDistance, static_cast<double>(TrackInterval), RemainderA);

				if(TrackInstances.IsValidIndex(i) && TrackInstances[i] != nullptr)
				{
					for (int j = 0; j <= TracksBeforeSplineEnd; ++j)
					{
						const float CurrentTrackDistance = j * TrackInterval + ChassisCurrentDistance;
						const FTransform TrackTransform = TrackPath->GetTransformAtDistanceAlongSpline(CurrentTrackDistance, ESplineCoordinateSpace::Local);
						const FVector RightVector = TrackPath->GetRightVectorAtDistanceAlongSpline(CurrentTrackDistance, ESplineCoordinateSpace::Local) * FilletsCompensation;
						FTransform NewTrackTransform = TrackTransform;							
						NewTrackTransform.SetLocation(TrackTransform.GetLocation() + RightVector);
						TrackInstances[i]->UpdateInstanceTransform(j, NewTrackTransform, false, true);
			
					}

					const int32 CountIndex = TracksBeforeSplineEnd + 1;
					const int32 LastIndex = TrackInstances[i]->GetInstanceCount() - 1;
					if(CountIndex <= LastIndex)
					{
						for (int k = CountIndex; k <= LastIndex; ++k)
						{
							const float CurrentTrackDistance = k * TrackInterval + (ChassisCurrentDistance - TrackPathLength);
							const FTransform TrackTransform = TrackPath->GetTransformAtDistanceAlongSpline(CurrentTrackDistance, ESplineCoordinateSpace::Local);
							const FVector RightVector = TrackPath->GetRightVectorAtDistanceAlongSpline(CurrentTrackDistance, ESplineCoordinateSpace::Local) * FilletsCompensation;
							FTransform NewTrackTransform = TrackTransform;
							NewTrackTransform.SetLocation(TrackTransform.GetLocation() + RightVector);
							TrackInstances[i]->UpdateInstanceTransform(k, NewTrackTransform, false, true);
						}
					}
				}
			}
		}
	}
}

float ARTSVehicles_Base::WheelRotationDefinition(const FRTSVehicle_WheelRotationData& WheelData) const
{
	float WheelStartingAngle;
	if(WheelData.bIsLeftWheel)
	{
		WheelStartingAngle = bUseGeometricTracks ? WheelData.WheelStartingAngleGeoL : WheelData.WheelStartingAngleUVL;
	}
	else
	{
		WheelStartingAngle = bUseGeometricTracks ? WheelData.WheelStartingAngleGeoR : WheelData.WheelStartingAngleUVR;
	}
	
	const float Circumference = 2.f * (WheelData.WheelRadius + WheelData.TrackThickness + (bUseGeometricTracks ? 0.f : WheelData.WheelSpeedCorrectionUV)) * UE_PI;
	return (-360 * WheelData.Distance / Circumference) + WheelStartingAngle;
}

void ARTSVehicles_Base::UpdateAntennaData()
{
	if(AntennaRotation.Num() != AntennaData.Num() || AntennaCurrentSpeed.Num() != AntennaData.Num())
	{
		AntennaRotation.Init(FRotator::ZeroRotator, AntennaData.Num());
		AntennaCurrentSpeed.Init(FVector::ZeroVector, AntennaData.Num());
	}
	
	if(GetMesh())
	{
		const FRotator TurretRotation = GetMesh()->GetSocketRotation(RTS_SOCKET_NAME_TURRET);
		const FRotator VehicleRotation = GetMesh()->GetComponentRotation() * -2.f;
		const FRotator InvertedTurretWorldRotation = FRotator(FQuat(VehicleRotation) * FQuat(TurretRotation));
		const FVector TurretVelocity = GetMesh()->GetPhysicsLinearVelocity(RTS_SOCKET_NAME_TURRET);
		const FVector NewInvertedLocalTurretSpeed = InvertedTurretWorldRotation.RotateVector(TurretVelocity);
		InvertedLocalTurretAcceleration = (InvertedLocalTurretSpeed - NewInvertedLocalTurretSpeed) * -1;
		InvertedLocalTurretSpeed = NewInvertedLocalTurretSpeed;

		for (int i = 0; i < AntennaData.Num(); ++i)
		{
			FRotator AntennaDeltaRotation = AntennaRotation[i] - AntennaData[i].Rotation;
			AntennaDeltaRotation.Normalize();
			const FVector InvertedPitchRoll = FVector(AntennaDeltaRotation.Pitch, AntennaDeltaRotation.Roll, AntennaDeltaRotation.Yaw);
			const FVector ElasticForce = InvertedPitchRoll * -1.f * FMath::Abs(AntennaData[i].Stiffness);
			const FVector FrictionForce = AntennaCurrentSpeed[i] * -1.f *  FMath::Abs(AntennaData[i].Dampening);

			// Set antenna speed
			FRotator DeltaRotation = TurretRotation - AntennaData[i].Rotation;
			DeltaRotation.Normalize();
			FVector UnRotated = DeltaRotation.UnrotateVector(FVector(0.f, 0.f, AntennaData[i].GravityForce));
			UnRotated.X *= -1.f;
			AntennaCurrentSpeed[i] += (InvertedLocalTurretAcceleration + ElasticForce + FrictionForce + UnRotated.X + UnRotated.Y + UnRotated.Z) * GetWorld()->GetDeltaSeconds();

			// Set antenna rotation
			const FVector AntennaMovement = AntennaCurrentSpeed[i] * GetWorld()->GetDeltaSeconds() * FMath::Abs(AntennaData[i].Frequency);
			const FRotator AntennaMoveRotation = FRotator(FQuat(FRotator(AntennaMovement.X, 0.f, AntennaMovement.Y)) * FQuat(AntennaRotation[i]));
			AntennaRotation[i] = FRotator(FMath::Clamp(AntennaMoveRotation.Pitch, -60, 60), 0.f, FMath::Clamp(AntennaMoveRotation.Roll, -60, 60));
		}
	}
}

float ARTSVehicles_Base::GetSpeedInfluence(const float ChassisDeltaDistance) const
{
	return FMath::Clamp(FMath::Abs(ChassisDeltaDistance * SpeedInfluence), 0.f, MaxSpeedInfluence);
}

float ARTSVehicles_Base::GetAccelerationInfluence(const float ChassisAcceleration) const
{
	return GetVehicleMovementComponent()->GetHandbrakeInput() ? FMath::Clamp(FMath::Abs(ChassisAcceleration * AccelerationInfluence), 0.f, MaxAccelerationInfluence) : 0.f;
}

void ARTSVehicles_Base::HideTracks()
{
	if(bUseGeometricTracks)
	{
		ShowUVTracks(false);
	}
	else
	{
		for (int i = 0; i < TracksInstances_R.Num(); ++i)
		{
			if(TracksInstances_R[i] != nullptr)
			{
				TracksInstances_R[i]->SetVisibility(false, false);
			}
		}
		
		for (int j = 0; j < TracksInstances_L.Num(); ++j)
		{
			if(TracksInstances_L[j] != nullptr)
			{
				TracksInstances_L[j]->SetVisibility(false, false);
			}
		}
	}
}

void ARTSVehicles_Base::ShowUVTracks(const bool bShow) const
{
	if(GetMesh())
	{
		for (int i = 0; i < GetMesh()->GetNumLODs(); ++i)
		{
			const int32 LeftMaterialIndex = GetMesh()->GetMaterialIndex(RTS_MATERIAL_SLOT_NAME_TRACK_L);
			GetMesh()->ShowMaterialSection(LeftMaterialIndex, LeftMaterialIndex, bShow, i);
			const int32 RightMaterialIndex = GetMesh()->GetMaterialIndex(RTS_MATERIAL_SLOT_NAME_TRACK_R);
			GetMesh()->ShowMaterialSection(RightMaterialIndex, RightMaterialIndex, bShow, i);
		}
	}
}

void ARTSVehicles_Base::SetWheelsRotations()
{
	// Front left
	WheelRotFrontL = WheelRotationDefinition(FRTSVehicle_WheelRotationData(
		true,
		ChassisDistanceL,
		WheelRadiusFront,
		TrackThickness,
		WheelSpeedCorrectionUV,
		WheelStartingAngleGeoL,
		WheelStartingAngleGeoR,
		WheelStartingAngleUVL,
		WheelStartingAngleUVR
	));

	// Front right
	WheelRotFrontR = WheelRotationDefinition(FRTSVehicle_WheelRotationData(
		false,
		ChassisDistanceR,
		WheelRadiusFront,
		TrackThickness,
		WheelSpeedCorrectionUV,
		WheelStartingAngleGeoL,
		WheelStartingAngleGeoR,
		WheelStartingAngleUVL,
		WheelStartingAngleUVR
	));

	// Middle left
	WheelRotMiddleL = WheelRotationDefinition(FRTSVehicle_WheelRotationData(true, ChassisDistanceL, WheelRadiusMiddle, TrackThickness));
	
	// Middle right
	WheelRotMiddleR = WheelRotationDefinition(FRTSVehicle_WheelRotationData(false, ChassisDistanceR, WheelRadiusMiddle, TrackThickness));

	// Rear left
	WheelRotRearL = WheelRotationDefinition(FRTSVehicle_WheelRotationData(
		true,
		ChassisDistanceL,
		WheelRadiusRear,
		TrackThickness,
		WheelSpeedCorrectionUV,
		WheelStartingAngleGeoL,
		WheelStartingAngleGeoR,
		WheelStartingAngleUVL,
		WheelStartingAngleUVR
	));

	// Rear right
	WheelRotRearR = WheelRotationDefinition(FRTSVehicle_WheelRotationData(
		false,
		ChassisDistanceR,
		WheelRadiusRear,
		TrackThickness,
		WheelSpeedCorrectionUV,
		WheelStartingAngleGeoL,
		WheelStartingAngleGeoR,
		WheelStartingAngleUVL,
		WheelStartingAngleUVR
	));

	// Accessory left
	WheelRotAccessoryL = WheelRotationDefinition(FRTSVehicle_WheelRotationData(true, ChassisDistanceL, WheelRadiusAccessory, TrackThickness));

	// Accessory right
	WheelRotAccessoryR = WheelRotationDefinition(FRTSVehicle_WheelRotationData(false, ChassisDistanceR, WheelRadiusAccessory, TrackThickness));	
}

void ARTSVehicles_Base::UpdateSuspensionOffset()
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheelZOffsets.Empty();		
		for (int i = 0; i < WheeledVehicleMovementComponent->Wheels.Num(); ++i)
		{
			WheelZOffsets.Add(WheeledVehicleMovementComponent->Wheels[i]->GetSuspensionOffset());			
		}
	}
}

void ARTSVehicles_Base::UpdateSlideParticleLocation() const
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		if(GetMesh())
		{
			const int32 MidPoint = WheeledVehicleMovementComponent->Wheels.Num() / 2;
			SlideBackRight->SetWorldLocation(GetMesh()->GetSocketLocation(FName(*FString::Printf(TEXT("%s_%d"), *RTS_SOCKET_NAME_TRACK_R, MidPoint))));
			SlideBackLeft->SetWorldLocation(GetMesh()->GetSocketLocation(FName(*FString::Printf(TEXT("%s_%d"), *RTS_SOCKET_NAME_TRACK_L, MidPoint))));
		}
	}
}

void ARTSVehicles_Base::UpdateSound(const float DeltaTime)
{
	if(bDestroyed)
	{
		if(TurretMotor != nullptr)
		{
			TurretMotor->Stop();
		}
		
		if(TankEngine != nullptr)
		{
			TankEngine->Stop();
		}
	}
	else
	{
		if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
		{
			const float TargetRPM = WheeledVehicleMovementComponent->GetEngineRotationSpeed() / WheeledVehicleMovementComponent->GetEngineMaxRotationSpeed() * 100.f;
			CurrentRPMRatio = FMath::FInterpConstantTo(CurrentRPMRatio, TargetRPM, DeltaTime, 30.f);

			if(TankEngine != nullptr)
			{
				TankEngine->SetFloatParameter(RTS_MATERIAL_PARAM_ENGINE_RPM, CurrentRPMRatio);
				if(!TankEngine->IsPlaying())
				{
					TankEngine->FadeIn(0.25, 1.f);
				}
			}
			
			if(P_Exhaust != nullptr)
			{
				P_Exhaust->SetFloatParameter(RTS_MATERIAL_PARAM_ENGINE_RPM, CurrentRPMRatio);
			}

			if(P_Exhaust1 != nullptr)
			{
				P_Exhaust1->SetFloatParameter(RTS_MATERIAL_PARAM_ENGINE_RPM, CurrentRPMRatio);
			}
			
			if(P_Exhaust2 != nullptr)
			{
				P_Exhaust2->SetFloatParameter(RTS_MATERIAL_PARAM_ENGINE_RPM, CurrentRPMRatio);
			}

			if(TurretMotor != nullptr)
			{
				if(bIsTurretRotating)
				{
					if(!TurretMotor->IsPlaying())
					{
						TurretMotor->FadeIn(0.25, 1.f, 0.f);
					}
				}
				else
				{
					TurretMotor->FadeOut(0.25, 0.f);
				}
			}
		}
	}
}

void ARTSVehicles_Base::UpdateTrackDecals()
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		const float Speed = WheeledVehicleMovementComponent->GetForwardSpeed();
		if(SlideBackRight != nullptr && SlideBackLeft != nullptr && Speed != 0.f)
		{
			Server_UpdateTrackDecals(SlideBackRight.Get(), Speed != 0.f, FMath::Abs(Speed));
			Server_UpdateTrackDecals(SlideBackLeft.Get(), Speed != 0.f, FMath::Abs(Speed));
		}
	}
}

void ARTSVehicles_Base::Server_UpdateTrackDecals_Implementation(UParticleSystemComponent* Particle, const bool bActive, const float Intensity)
{
	NetMulticast_UpdateTrackDecals(Particle, bActive, Intensity);
}

void ARTSVehicles_Base::NetMulticast_UpdateTrackDecals_Implementation(UParticleSystemComponent* Particle, const bool bActive, const float Intensity)
{
	Particle->SetActive(bActive);
	Particle->SetFloatParameter(RTS_FX_PARAM_INTENSITY, Intensity);
}

void ARTSVehicles_Base::ApplyVehicleInput(const FRTSCore_VehicleInput& VehicleInput)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		const float VehicleSpeed = Chaos::CmSToKmH(WheeledVehicleMovementComponent->GetForwardSpeed());
		const float SpeedScalingFactor = FMath::Clamp(VehicleSpeed / MaxSpeedKmh, 0.5f, 0.0f);
		const float ScaledSteeringForce = VehicleInput.Steering * SpeedScalingFactor;
		//const float TurnSpeed = FMath::GetMappedRangeValueClamped(FVector2d(0, MaxSpeedKmh * 0.25f), FVector2d(0.5f, 5.f), VehicleSpeed / MaxSpeedKmh * 0.25f);
		//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("SpeedScalingFactor: %f | TurnSpeed: %f "), SpeedScalingFactor, VehicleInput.Steering * SpeedScalingFactor));							
		
		//CurrentSteering = FMath::FInterpTo(CurrentSteering, ScaledSteeringForce, GetWorld()->GetDeltaSeconds(), 5.f);
		WheeledVehicleMovementComponent->SetYawInput(ScaledSteeringForce);
			
		const float MaxCheckedThrottle = VehicleSpeed < MaxSpeedKmh ? VehicleInput.Throttle : 0.f;
		float ThrottleSpeedScale = MaxCheckedThrottle;	
		if(MaxCheckedThrottle > 0 && ThrottleCurve != nullptr)
		{
			ThrottleSpeedScale = ThrottleCurve->GetFloatValue(MaxCheckedThrottle);	
		}
			
		WheeledVehicleMovementComponent->SetThrottleInput(ThrottleSpeedScale);
			//WheeledVehicleMovementComponent->SetThrottleInput(FMath::FInterpTo(WheeledVehicleMovementComponent->GetThrottleInput(), MaxCheckedThrottle, GetWorld()->GetDeltaSeconds(), 1.f));

		WheeledVehicleMovementComponent->SetBrakeInput(VehicleInput.Braking);
			//WheeledVehicleMovementComponent->SetBrakeInput(FMath::FInterpTo(WheeledVehicleMovementComponent->GetBrakeInput(), VehicleInput.Braking, GetWorld()->GetDeltaSeconds(), 1.f));

#if WITH_EDITOR
		DebugVehicleInput(ThrottleSpeedScale, ScaledSteeringForce, VehicleInput.Braking);
#endif
	}
}

void ARTSVehicles_Base::SetThrottle(const float Throttle)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheeledVehicleMovementComponent->SetThrottleInput(Throttle);
	}
}

void ARTSVehicles_Base::SetSteering(const float Steering)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheeledVehicleMovementComponent->SetYawInput(Steering);
	}
}

void ARTSVehicles_Base::SetBrake(const float Brake)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheeledVehicleMovementComponent->SetBrakeInput(Brake);
	}
}

void ARTSVehicles_Base::SetHandbrake(const bool bHandbrake)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheeledVehicleMovementComponent->SetHandbrakeInput(bHandbrake);
	}
}

void ARTSVehicles_Base::SetParked(const bool bIsParked)
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		WheeledVehicleMovementComponent->SetBrakeInput(bIsParked ? 1.f : 0.f);
		WheeledVehicleMovementComponent->SetHandbrakeInput(bIsParked);
		WheeledVehicleMovementComponent->SetThrottleInput(0.f);
		WheeledVehicleMovementComponent->SetYawInput(0.f);
		WheeledVehicleMovementComponent->SetParked(bIsParked);
	}
}

float ARTSVehicles_Base::GetThrottle() const
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->GetThrottleInput();
	}

	return 0.f;
}

float ARTSVehicles_Base::GetBrake() const
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->GetBrakeInput();
	}

	return 0.f;
}

float ARTSVehicles_Base::GetMass() const
{
	return GetVehicleMovementComponent() ? GetVehicleMovementComponent()->Mass : 5000.f;
}

bool ARTSVehicles_Base::IsParked() const
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->IsParked();
	}

	return false;
}

FVector ARTSVehicles_Base::GetRVOAvoidanceVelocity() const
{	
	if(RVOComponent != nullptr)
	{
		return RVOComponent->GetRVOAvoidanceVelocity();
	}

	return FVector::ZeroVector;
}

float ARTSVehicles_Base::GetThrottleInput() const
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->GetThrottleInput();
	}
	
	return 0.f;
}

bool ARTSVehicles_Base::GetHandBrakeInput() const
{
	if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->GetHandbrakeInput();
	}
	
	return false;
}

float ARTSVehicles_Base::GetBrakeInput() const
{
	if(UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		return WheeledVehicleMovementComponent->GetBrakeInput();
	}
	
	return 0.f;
}

bool ARTSVehicles_Base::IsMaxAngularVelocity() const
{
	return GetMesh() != nullptr ? FMath::Abs(GetMesh()->GetPhysicsAngularVelocityInDegrees().Z) > MaxTurningSpeed : false;
}

void ARTSVehicles_Base::DebugVehicleInput(const float Throttle, const float InSteering, const float Brake) const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSVehicles && CoreSettings->DebugRTSVehicle_Input)
		{
			if(const UChaosWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("ThrottleInput: %f | SteeringInput: %f | BrakeInput: %f | %s | %s"),
					Throttle, InSteering, Brake, WheeledVehicleMovementComponent->GetHandbrakeInput() ? TEXT("HandBrakeON") : TEXT("HandBrakeOFF"), WheeledVehicleMovementComponent->IsParked() ? TEXT("Parked") : TEXT("-")));
			}							
		}
	}
}

TObjectPtr<USplineComponent> ARTSVehicles_Base::GetNavigationSpline() const
{
	return NavigationPath;
}

void ARTSVehicles_Base::CreatePerceptionStimuliSourceComponent()
{
	StimuliSourceComponent = NewObject<UAIPerceptionStimuliSourceComponent>(this, TEXT("StimuliSourceComponent"));
	if(StimuliSourceComponent)
	{
		StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

FName ARTSVehicles_Base::GetSightSocket() const
{
	return FName(TEXT("view_port"));
}

FVector ARTSVehicles_Base::GetSightSocketLocation() const
{
	if(!GetMesh())
	{
		return FVector::ZeroVector;
	}
	
	return GetMesh()->GetSocketLocation(GetSightSocket());
}

USceneComponent* ARTSVehicles_Base::GetEntityMesh() const
{
	return GetMesh();
}

UShapeComponent* ARTSVehicles_Base::GetCollisionBox() const
{
	return CollisionBox;
}

void ARTSVehicles_Base::SetObservationLocation(const FVector& Location)
{	
}

void ARTSVehicles_Base::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (MontageToPlay != nullptr && GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(MontageToPlay);
		}
	}
}
