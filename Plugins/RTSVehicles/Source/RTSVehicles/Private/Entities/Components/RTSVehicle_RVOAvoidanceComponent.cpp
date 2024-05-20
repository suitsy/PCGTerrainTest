// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Components/RTSVehicle_RVOAvoidanceComponent.h"
#include "AI/Navigation/AvoidanceManager.h"
#include "Framework/Settings/RTSCore_DeveloperSettings.h"

DECLARE_CYCLE_STAT(TEXT("RVO::TickComponent"), STAT_TickComponent, STATGROUP_VehicleRVO);
DECLARE_CYCLE_STAT(TEXT("RVO::CalculateAvoidanceVelocity"), STAT_CalculateAvoidanceVelocity, STATGROUP_VehicleRVO);
DECLARE_CYCLE_STAT(TEXT("RVO::UpdateAvoidance"), STAT_UpdateAvoidance, STATGROUP_VehicleRVO);
DECLARE_CYCLE_STAT(TEXT("RVO::UpdateVehicleMovementFromAvoidance"), STAT_UpdateVehicleMovementFromAvoidance, STATGROUP_VehicleRVO);

// Sets default values for this component's properties
URTSVehicle_RVOAvoidanceComponent::URTSVehicle_RVOAvoidanceComponent()
{
	PrimaryComponentTick.bCanEverTick = bUseRVOAvoidance;

	AvoidanceVelocity = FVector::ZeroVector;
	AvoidanceLockVelocity = FVector::ZeroVector;
	AvoidanceLockTimer = 0.0f;

	AvoidanceGroup.bGroup0 = true;
	GroupsToAvoid.Packed = 0xFFFFFFFF;
	GroupsToIgnore.Packed = 0;
}

bool URTSVehicle_RVOAvoidanceComponent::InitVehicleControls()
{
	auto Owner = GetOwner();
	if(!Owner)
	{
		return false;
	}

	auto VehicleControlsPtr = Cast<IRTSCore_VehicleInputInterface>(Owner);
	if (!VehicleControlsPtr)
	{
		TArray<UActorComponent*> Components = Owner->GetComponentsByInterface(URTSCore_VehicleInputInterface::StaticClass());
		if (Components.Num())
		{
			VehicleControlsPtr = Cast<IRTSCore_VehicleInputInterface>(Components[0]);
		}
	}

	VehicleControls.SetObject(Cast<UObject>(VehicleControlsPtr));
	return VehicleControls.GetObject() != nullptr;
}

IRTSCore_VehicleInputInterface* URTSVehicle_RVOAvoidanceComponent::GetVehicleControls() const
{
	return Cast<IRTSCore_VehicleInputInterface>(VehicleControls.GetObject());
}

FString URTSVehicle_RVOAvoidanceComponent::GetVehicleName() const
{
	auto Vehicle = GetOwner();
	
	return Vehicle ? *Vehicle->GetName() : TEXT("NULL");
}

void URTSVehicle_RVOAvoidanceComponent::InitializeFromVehicleProperties(AActor* Vehicle)
{
	// Initialize height and radius based on actor bounds
	check(Vehicle);

	FVector ActorOrigin;
	FVector BoxExtent;

	Vehicle->GetActorBounds(true, ActorOrigin, BoxExtent, false);

	RVOAvoidanceRadius = FMath::Max(BoxExtent.X * 1.25f, BoxExtent.Y * 1.25f);
	RVOAvoidanceHeight = BoxExtent.Z;

	UE_LOG(LogTemp, Log, TEXT("%s: InitializeComponent - RVOAvoidanceRadius=%f; RVOAvoidanceHeight=%f"),
		*GetVehicleName(), RVOAvoidanceRadius, RVOAvoidanceHeight);
}

void URTSVehicle_RVOAvoidanceComponent::InitializeAvoidance()
{
	if (!bUseRVOAvoidance)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s: InitializeAvoidance"), *GetVehicleName());

	PrimaryComponentTick.SetTickFunctionEnable(bUseRVOAvoidance);

	UAvoidanceManager* AvoidanceManager = GetWorld()->GetAvoidanceManager();
	if (!AvoidanceManager)
	{
		return;
	}

	if(const URTSCore_DeveloperSettings* DeveloperSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(DeveloperSettings->DebugRTSVehicles && DeveloperSettings->DebugAiCollision)
		{
			AvoidanceManager->AvoidanceDebugForAll(true);			
		}
	}

	// reset id, RegisterMovementComponent call is required to initialize update timers in avoidance manager
	AvoidanceUID = 0;
	AvoidanceManager->RegisterMovementComponent(this, AvoidanceWeight);
}

void URTSVehicle_RVOAvoidanceComponent::UpdateAvoidance(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAvoidance);

	UpdateDefaultAvoidance();
}

void URTSVehicle_RVOAvoidanceComponent::UpdateDefaultAvoidance()
{
	if (!bUseRVOAvoidance)
	{
		return;
	}

	UAvoidanceManager* AvoidanceManager = GetWorld()->GetAvoidanceManager();
	if (AvoidanceManager && !bWasAvoidanceUpdated)
	{
		AvoidanceManager->UpdateRVO(this);

		//Consider this a clean move because we didn't even try to avoid.
		SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterClean);
	}

	bWasAvoidanceUpdated = false;		//Reset for next frame
}

void URTSVehicle_RVOAvoidanceComponent::SetAvoidanceVelocityLock(UAvoidanceManager* Avoidance, float Duration)
{
	Avoidance->OverrideToMaxWeight(AvoidanceUID, Duration);
	AvoidanceLockVelocity = AvoidanceVelocity;
	AvoidanceLockTimer = Duration;
}

void URTSVehicle_RVOAvoidanceComponent::CalculateAvoidanceVelocity(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_CalculateAvoidanceVelocity);

	if (!bUseRVOAvoidance)
	{
		return;
	}

	// since we don't assign the avoidance velocity but instead use it to adjust steering and throttle,
	// always reset the avoidance velocity to the current velocity
	AvoidanceVelocity = GetVelocityForRVOConsideration();

	if (!bEnableAvoidanceMovementUpdates || AvoidanceWeight >= 1.0f)
	{
		return;
	}

	UAvoidanceManager* AvoidanceManager = GetWorld()->GetAvoidanceManager();
	auto MyOwner = GetOwner();

	if (!AvoidanceManager || !MyOwner)
	{
		return;
	}

	if (MyOwner->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const bool bShowDebug = AvoidanceManager->IsDebugEnabled(AvoidanceUID);
#endif

	if (!AvoidanceVelocity.IsZero())
	{
		//See if we're doing a locked avoidance move already, and if so, skip the testing and just do the move.
		if (AvoidanceLockTimer > 0.0f)
		{
			AvoidanceVelocity = AvoidanceLockVelocity;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bShowDebug)
			{
				DrawDebugLine(GetWorld(), GetRVOAvoidanceOrigin(), GetRVOAvoidanceOrigin() + AvoidanceVelocity, FColor::Blue, true, 0.5f, SDPG_MAX);
			}
#endif
		}
		else
		{
			FVector NewVelocity = AvoidanceManager->GetAvoidanceVelocityForComponent(this);
			if (!NewVelocity.Equals(AvoidanceVelocity))		//Really want to branch hint that this will probably not pass
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s: RVO Update AvoidanceVelocity=%s"), *GetVehicleName(), *NewVelocity.ToCompactString());
				//Had to divert course, lock this avoidance move in for a short time. This will make us a VO, so unlocked others will know to avoid us.
				AvoidanceVelocity = NewVelocity;
				SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterAvoid);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				if (bShowDebug)
				{
					DrawDebugLine(GetWorld(), GetRVOAvoidanceOrigin(), GetRVOAvoidanceOrigin() + AvoidanceVelocity, FColor::Red, true, 20.0f, SDPG_MAX, 10.0f);
				}
#endif
			}
			else
			{
				//Although we didn't divert course, our velocity for this frame is decided. We will not reciprocate anything further, so treat as a VO for the remainder of this frame.
				SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterClean);	//10 ms of lock time should be adequate.
			}
		}

		AvoidanceManager->UpdateRVO(this);
		bWasAvoidanceUpdated = true;
	}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	else if (bShowDebug)
	{
		DrawDebugLine(GetWorld(), GetRVOAvoidanceOrigin(), GetRVOAvoidanceOrigin() + GetVelocityForRVOConsideration(), FColor::Yellow, true, 0.05f, SDPG_MAX);
	}

	if (bShowDebug)
	{
		FVector UpLine(0, 0, 500);
		DrawDebugLine(GetWorld(), GetRVOAvoidanceOrigin(), GetRVOAvoidanceOrigin() + UpLine, (AvoidanceLockTimer > 0.01f) ? FColor::Red : FColor::Blue, true, 0.05f, SDPG_MAX, 5.0f);
	}
#endif
}

void URTSVehicle_RVOAvoidanceComponent::UpdateVehicleMovementFromAvoidance()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateVehicleMovementFromAvoidance);

	//bool RVOActive = SetAvoidanceSteering();
	//RVOActive |= SetAvoidanceThrottle();

	NotifyRVOStatus(true);
}

void URTSVehicle_RVOAvoidanceComponent::NotifyRVOStatus(bool bEnabled)
{
	if (bEnabled == bDidAvoidanceAlterVelocity)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s: NotifyRVOStatus changed: %s"), *GetVehicleName(), bEnabled ? TEXT("TRUE") : TEXT("FALSE"));

	bDidAvoidanceAlterVelocity = bEnabled;
}

void URTSVehicle_RVOAvoidanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_TickComponent);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUseRVOAvoidance)
	{
		return;
	}

	if (AvoidanceLockTimer > 0.0f)
	{
		AvoidanceLockTimer -= DeltaTime;
	}

	CalculateAvoidanceVelocity(DeltaTime);
	UpdateAvoidance(DeltaTime);

	if (bEnableAvoidanceMovementUpdates)
	{
		UpdateVehicleMovementFromAvoidance();
	}
}

void URTSVehicle_RVOAvoidanceComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (auto Vehicle = GetOwner(); Vehicle && InitVehicleControls())
	{
		InitializeFromVehicleProperties(Vehicle);
		InitializeAvoidance();
	}
	else
	{
		ensureMsgf(false, TEXT("URTSVehicle_RVOAvoidanceComponent - Could not find vehicle controls implementer on owner - %s"),
			GetOwner() ? *GetOwner()->GetClass()->GetName() : TEXT("NULL"));

		SetAvoidanceEnabled(false);

		return;
	}
}

void URTSVehicle_RVOAvoidanceComponent::Deactivate()
{
	Super::Deactivate();

	if(AvoidanceUID)
	{
		UAvoidanceManager* AvoidanceManager = GetWorld()->GetAvoidanceManager();
		if (AvoidanceManager)
		{
			AvoidanceManager->RemoveAvoidanceObject(AvoidanceUID);
		}
	}
}

void URTSVehicle_RVOAvoidanceComponent::SetAvoidanceEnabled(bool bEnable)
{
	if (bEnableAvoidanceMovementUpdates == bEnable)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%s: ToggleSwervingAvoidance=%s"), *GetVehicleName(), bEnable ? TEXT("TRUE") : TEXT("FALSE"));

	bEnableAvoidanceMovementUpdates = bEnable;

	if (!bEnable)
	{
		NotifyRVOStatus(false);
	}
}

void URTSVehicle_RVOAvoidanceComponent::SetAvoidanceSystemEnabled(bool bEnable)
{
	bUseRVOAvoidance = bEnable;
}

void URTSVehicle_RVOAvoidanceComponent::SetRVOAvoidanceUID(int32 UID)
{
	AvoidanceUID = UID;
}

int32 URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceUID()
{
	return AvoidanceUID;
}

void URTSVehicle_RVOAvoidanceComponent::SetRVOAvoidanceWeight(float Weight)
{
	AvoidanceWeight = Weight;
}

float URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceWeight()
{
	return AvoidanceWeight;
}

FVector URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceOrigin()
{
	auto Vehicle = GetOwner();
	if (!Vehicle)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: GetRVOAvoidanceOrigin - Vehicle is NULL"), *GetName());
		return FVector();
	}

	return Vehicle->GetActorLocation();
}

float URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceRadius()
{
	return RVOAvoidanceRadius;
}

float URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceHeight()
{
	return RVOAvoidanceHeight;
}

float URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceConsiderationRadius()
{
	return AvoidanceConsiderationRadius;
}

FVector URTSVehicle_RVOAvoidanceComponent::GetVelocityForRVOConsideration()
{
	auto Vehicle = GetVehicleControls();
	if (!Vehicle)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: GetVelocityForRVOConsideration - Vehicle is NULL"), *GetName());
		return FVector();
	}

	FVector Velocity2D;
	Velocity2D.Z = 0.f;

	return Velocity2D;
}

FVector URTSVehicle_RVOAvoidanceComponent::GetRVOAvoidanceVelocity()
{
	return AvoidanceVelocity;
}

void URTSVehicle_RVOAvoidanceComponent::SetAvoidanceGroupMask(int32 GroupFlags)
{
	AvoidanceGroup.SetFlagsDirectly(GroupFlags);
}

int32 URTSVehicle_RVOAvoidanceComponent::GetAvoidanceGroupMask()
{
	return AvoidanceGroup.Packed;
}

void URTSVehicle_RVOAvoidanceComponent::SetGroupsToAvoidMask(int32 GroupFlags)
{
	GroupsToAvoid.SetFlagsDirectly(GroupFlags);
}

int32 URTSVehicle_RVOAvoidanceComponent::GetGroupsToAvoidMask()
{
	return GroupsToAvoid.Packed;
}

void URTSVehicle_RVOAvoidanceComponent::SetGroupsToIgnoreMask(int32 GroupFlags)
{
	GroupsToIgnore.SetFlagsDirectly(GroupFlags);
}

int32 URTSVehicle_RVOAvoidanceComponent::GetGroupsToIgnoreMask()
{
	return GroupsToIgnore.Packed;
}
