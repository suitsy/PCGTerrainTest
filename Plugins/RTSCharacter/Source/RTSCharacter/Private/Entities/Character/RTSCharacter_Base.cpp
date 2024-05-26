// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Character/RTSCharacter_Base.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/TimelineComponent.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Framework/Components/RogueRVO_Component.h"
#include "Framework/Components/RogueRVO_CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#if WITH_EDITOR
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#endif


ARTSCharacter_Base::ARTSCharacter_Base(const FObjectInitializer& ObjectInitializer) : 
Super(ObjectInitializer.SetDefaultSubobjectClass<URogueRVO_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	if(GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(RTS_TRACE_CHANNEL_ENTITIES, ECR_Block);
		GetMesh()->SetReceivesDecals(false);
	}

	TurnToLocationTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TurnToLocationTimeline"));
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate.Yaw = 135;
		MovementComponent->MaxAcceleration = 512;
		MovementComponent->BrakingFrictionFactor = 10;
	}

	// RVO	
	//RVOComponent = CreateDefaultSubobject<URogueRVO_Component>(TEXT("RVOComponent"));

	// Navigation Spline
	Navigation = CreateDefaultSubobject<USceneComponent>(TEXT("NavigationComponent"));
	NavigationPath = CreateDefaultSubobject<USplineComponent>(TEXT("NavigationSpline"));	
	NavigationPath->SetupAttachment(Navigation);
	NavigationPath->SetAbsolute(true, true);
	NavigationPath->SetClosedLoop(false);
	NavigationPath->Mobility = EComponentMobility::Movable;
}

void ARTSCharacter_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentUprightAmount);
	DOREPLIFETIME(ThisClass, UprightAmount);
}

void ARTSCharacter_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFocused && !bTurnToLocationTimelineExecuting)
	{
		FocusTrackTargetOnMove(DeltaTime);
	}
	else if(bIsObservingTarget && !bTurnToLocationTimelineExecuting)
	{
		FocusObservationTarget(DeltaTime);
	}
	// reset look when moving not in focus
	else if (GetVelocity().Length() > 10 && !TargetAimRotator.IsNearlyZero(5.f))
	{
		SetLookAtLocation(FVector::Zero(), true);
	}
	
	CurrentUprightAmount = FMath::Lerp(CurrentUprightAmount, UprightAmount, DeltaTime * MovementSmoothingCoefficient);
	
#if WITH_EDITOR
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSCharacter)
		{
			Debug();
		}
	}
#endif
}

void ARTSCharacter_Base::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);

	OutLocation = GetMesh()->GetSocketLocation(GetSightSocket());	
	OutRotation = FRotator(0.f, GetMesh()->GetSocketRotation(GetSightSocket()).Yaw, 0.f);
}

void ARTSCharacter_Base::BeginPlay()
{
	Super::BeginPlay();

	if(TurnDirectionCurve)
	{
		FOnTimelineFloat TurnDirectionUpdate;
		TurnDirectionUpdate.BindUFunction(this, FName("OnTurnDirectionUpdate"));
		TurnToLocationTimelineComponent->AddInterpFloat(TurnDirectionCurve, TurnDirectionUpdate);
	}

	if(TurnRotationCurve)
	{
		FOnTimelineFloat TurnRotationUpdate;
		TurnRotationUpdate.BindUFunction(this, FName("OnTurnRotationUpdate"));
		TurnToLocationTimelineComponent->AddInterpFloat(TurnRotationCurve, TurnRotationUpdate);
	}

	FOnTimelineEvent TurnToLocationFinished;
	TurnToLocationFinished.BindUFunction(this, FName("OnTurnToLocationFinished"));
	TurnToLocationTimelineComponent->SetTimelineFinishedFunc(TurnToLocationFinished);

	TurnToLocationTimelineComponent->SetPlayRate(1);
	TurnToLocationTimelineComponent->SetTimelineLength(1.f);
	TurnToLocationTimelineComponent->SetTimelineLengthMode(TL_TimelineLength);
}

void ARTSCharacter_Base::FocusTrackTargetOnMove(const float DeltaTime)
{
	if (GetVelocity().Length() == 0)
	{
		ProjectedDirection = 0;
		ProjectedSpeed = 0;
		return;
	}
	
	bWasAiming = bIsAiming;

	// Focus track while moving
	if (FocusTarget && FocusTarget->GetMesh())
	{
		TargetLookLocation = FocusTarget->GetMesh()->GetCenterOfMass();
	}
	
	SetAimRotatorToTargetLocation(TargetLookLocation);
	CalcAimDirectionRotation(TargetLookLocation, DeltaTime);
}

void ARTSCharacter_Base::FocusObservationTarget(const float DeltaTime)
{
	if(TargetLookLocation != FVector::ZeroVector)
	{
		SetAimRotatorToTargetLocation(TargetLookLocation);
		CalcAimDirectionRotation(TargetLookLocation, DeltaTime);
	}
}

void ARTSCharacter_Base::SetAimRotatorToTargetLocation(const FVector& Location)
{

	const FRotator RelativeLookRotator = UKismetMathLibrary::FindRelativeLookAtRotation(GetTransform(), Location);
	TargetAimRotator.Roll = FMath::Clamp(RelativeLookRotator.Pitch * -1, AimRotationVerticalConstraint.Y * -1, AimRotationVerticalConstraint.X * -1);
	TargetAimRotator.Yaw = FMath::Clamp(RelativeLookRotator.Yaw, AimRotationHorizontalConstraint.X, AimRotationHorizontalConstraint.Y);
	TargetAimRotator.Pitch = 0;
	CurrentYaw = TargetAimRotator.Yaw;
	RelativeYaw = RelativeLookRotator.Yaw;
}

void ARTSCharacter_Base::CalcAimDirectionRotation(const FVector& Location, const float DeltaTime)
{
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Location);
	const FRotator ActorRotation = GetActorRotation();
	const float ForwardAmount = UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::Normal(GetVelocity(), .0001f), GetCapsuleComponent()->GetForwardVector());
	const float UprightAmountCoefficient = 150.f - (CurrentUprightAmount * 150.f);
	
	ProjectedDirection = FMath::Lerp(ProjectedDirection, GetVelocity().Length() * GetTurnDirection(ActorRotation.Yaw, LookAtRotation.Yaw) * (1 - UKismetMathLibrary::Abs(ForwardAmount)), DeltaTime * MovementSmoothingCoefficient);
	ProjectedSpeed = FMath::Lerp(ProjectedSpeed, GetVelocity().Length() * ForwardAmount, DeltaTime * MovementSmoothingCoefficient);
	CurrentLookAtRotator = UKismetMathLibrary::RLerp(CurrentLookAtRotator, TargetAimRotator, DeltaTime * MovementSmoothingCoefficient, true);
	SetActorRotation(UKismetMathLibrary::RLerp(FRotator(0, ActorRotation.Yaw, 0), FRotator(0, LookAtRotation.Yaw, 0), DeltaTime * MovementSmoothingCoefficient, true));

	const float DesiredWalkSpeed = ForwardAmount < 0 ?
		FMath::Clamp(DefaultFocusWalkSpeed * FocusMovementBackWalkMultiplier - UprightAmountCoefficient, FocusMovementBackWalkSpeedLimits.X, FocusMovementBackWalkSpeedLimits.Y - UprightAmountCoefficient)
		: (FMath::Clamp(DefaultFocusWalkSpeed - UprightAmountCoefficient, FocusMovementForwardWalkSpeedLimits.X, FocusMovementForwardWalkSpeedLimits.Y));

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(GetCharacterMovement()->MaxWalkSpeed, DesiredWalkSpeed, DeltaTime * MovementSmoothingCoefficient);
}

float ARTSCharacter_Base::GetTurnDirection(float A, float B)
{
	float Direction = A - B;
	if (UKismetMathLibrary::Abs(Direction) > 180)
	{
		if (A < B)
		{
			Direction = A + 360 - B;
		}
		else
		{
			Direction = A - 360 + B;
		}
	}
	
	return UKismetMathLibrary::SafeDivide(Direction, UKismetMathLibrary::Abs(Direction));
}

void ARTSCharacter_Base::SetLookAtLocation(const FVector& Location, const bool bResetLook)
{
	bResetCurrentLook = bResetLook;
	if (bResetCurrentLook)
	{
		ResetAimRotator();
	}
	else
	{
		TargetLookLocation = Location;
		SetAimRotatorToTargetLocation(TargetLookLocation);
	}

	const float NewRate = UKismetMathLibrary::SafeDivide(1, UKismetMathLibrary::SafeDivide(UKismetMathLibrary::Abs(CurrentLookAtRotator.Yaw - RelativeYaw) * Rotate360TimeCoefficient, 360));
	TurnToLocationTimelineComponent->SetPlayRate(FMath::Clamp(NewRate, .1f, 5.f));
	CurrentYaw = RelativeYaw == CurrentYaw ? 0 : CurrentYaw;
	ActorLookAtRotationYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLookLocation).Yaw - CurrentYaw;
	TargetTurnRotation = GetActorRotation();
	TargetTurnDirection = GetTurnDirection(ActorLookAtRotationYaw, TargetTurnRotation.Yaw);

	if (!bTurnToLocationTimelineExecuting)
	{
		TurnToLocationTimelineComponent->PlayFromStart();
	}
	else
	{
		LastLookAtRotator = CurrentLookAtRotator;
		bTurnToLocationTimelineExecuting = false;
		TurnDirection = 0;
		TurnToLocationTimelineComponent->Stop();
		
		if (bResetCurrentLook)
		{
			TurnToLocationTimelineComponent->PlayFromStart();
		}
		else
		{
			TurnToLocationTimelineComponent->Play();
		}
		
		TurnToLocationTimelineComponent->SetNewTime(.5f);
	}
}

void ARTSCharacter_Base::ResetAimRotator()
{
	TargetAimRotator = FRotator::ZeroRotator;
	CurrentYaw = 0;
	RelativeYaw = 0;
}

void ARTSCharacter_Base::OnTurnDirectionUpdate(float Alpha)
{
	if(bAimOnly || bResetCurrentLook || CurrentYaw == 0 || GetVelocity().Length() != 0)
	{
		return;
	}
	
	TurnDirection = Alpha * 180 * TargetTurnDirection;
}

void ARTSCharacter_Base::OnTurnRotationUpdate(float Alpha)
{
	if (LastLookAtRotator.Equals(FRotator::ZeroRotator, 5) && TargetAimRotator.Equals(FRotator::ZeroRotator, 5.f))
	{
		TurnToLocationTimelineComponent->Stop();
	}
	else
	{		
		CurrentLookAtRotator = UKismetMathLibrary::RLerp(LastLookAtRotator, TargetAimRotator, Alpha, true);

		// Update aiming body
		bTurnToLocationTimelineExecuting = true;
	}

	if (bAimOnly || bResetCurrentLook || CurrentYaw == 0 || GetVelocity().Length() != 0)
	{
		return;
	}

	// Update actor rotation
	FRotator NewActorRotation = UKismetMathLibrary::RLerp(TargetTurnRotation, FRotator(0, ActorLookAtRotationYaw, 0), Alpha, true);
	NewActorRotation.Roll = TargetTurnRotation.Roll;
	NewActorRotation.Pitch = TargetTurnRotation.Pitch;
	SetActorRotation(NewActorRotation);
}

void ARTSCharacter_Base::OnTurnToLocationFinished()
{
	LastLookAtRotator = CurrentLookAtRotator;
	bTurnToLocationTimelineExecuting = false;
	TurnDirection = 0;
	bResetCurrentLook = false;
	bIsObservingTarget = false;
}

#if WITH_EDITOR
void ARTSCharacter_Base::Debug() const
{
	if(const UWorld* WorldContext = GetWorld())
	{
		// Draw Aim Line
		const FVector HandR = GetMesh()->GetSocketLocation("hand_r_socket");
		const FVector HandL = GetMesh()->GetSocketLocation("index_03_l");
		const FVector Direction = GetActorForwardVector();
		const FVector End = HandR + Direction * 10000;

		/*DrawDebugLine(
			WorldContext,
			HandR,
			End,
			FColor::Red,
			false,
			-1.f,
			0,
			0.3f
		);*/
	}
}
#endif

void ARTSCharacter_Base::SetObservationLocation(const FVector& Location)
{
	if(Location != FVector::ZeroVector)
	{
		bIsObservingTarget = true;
		SetLookAtLocation(Location, false);
	}
}



void ARTSCharacter_Base::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (MontageToPlay != nullptr && GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(MontageToPlay);
		}
	}
}

TObjectPtr<USplineComponent> ARTSCharacter_Base::GetNavigationSpline() const
{
	return NavigationPath;
}

void ARTSCharacter_Base::CreatePerceptionStimuliSourceComponent()
{
	StimuliSourceComponent = NewObject<UAIPerceptionStimuliSourceComponent>(this, TEXT("StimuliSourceComponent"));
	if(StimuliSourceComponent)
	{
		StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

FName ARTSCharacter_Base::GetSightSocket() const
{
	return FName(TEXT("head_socket"));
}

FVector ARTSCharacter_Base::GetSightSocketLocation() const
{
	if(!GetMesh())
	{
		return FVector::ZeroVector;
	}
	
	return GetMesh()->GetSocketLocation(GetSightSocket());
}

USceneComponent* ARTSCharacter_Base::GetEntityMesh() const
{
	return GetMesh();
}

UShapeComponent* ARTSCharacter_Base::GetCollisionBox() const
{
	return GetCapsuleComponent();
}

FVector ARTSCharacter_Base::GetRVOAvoidanceVelocity() const
{
	if(RVOComponent != nullptr)
	{
		return RVOComponent->GetRVOAvoidanceVelocity();
	}
	else if(GetMovementComponent())
	{
		return GetMovementComponent()->Velocity;
	}

	return FVector::ZeroVector;
}
