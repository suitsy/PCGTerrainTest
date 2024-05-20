// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Components/RogueRVO_Component.h"
#include "Components/CapsuleComponent.h"
#include "Framework/RogueRVO_Subsystem.h"
#include "Framework/Components/RogueRVO_CharacterMovementComponent.h"
#include "Framework/Components/RogueRVO_VehicleMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"


URogueRVO_Component::URogueRVO_Component()
{
	PrimaryComponentTick.bCanEverTick = true;
	AvoidanceUID = 0;
	AvoidanceVelocity = FVector::ZeroVector;
	RVOAvoidanceRadius = 0.f;
	RVOAvoidanceHeight = 0.f;
	AvoidanceGroup.bGroup0 = true;
	GroupsToAvoid.Packed = 0xFFFFFFFF;
	GroupsToIgnore.Packed = 0;
	bWantsInitializeComponent = true;
	bWasAvoidanceUpdated = false;
	bUseRVOAvoidance = true;
}


void URogueRVO_Component::BeginPlay()
{
	Super::BeginPlay();

	
}

void URogueRVO_Component::InitializeAgentProperties()
{
	// Initialize height and radius based on actor bounds
	if (AActor* OwningActor = GetOwner())
	{
		// Get the actor pawn so we can get the movement component
		if(const APawn* Pawn = Cast<APawn>(OwningActor)){
		
			CharacterMovement = Cast<URogueRVO_CharacterMovementComponent>(Pawn->GetMovementComponent());
			if(CharacterMovement != nullptr)
			{
				// Cast to character to access capsule component
				if(const ACharacter* OwningCharacter = Cast<ACharacter>(OwningActor))
				{
					// Assign radius and height from capsule
					const UCapsuleComponent* CapsuleComp = OwningCharacter->GetCapsuleComponent();
					RVOAvoidanceRadius = CapsuleComp ? CapsuleComp->GetScaledCapsuleRadius() : 0.0f;
					RVOAvoidanceHeight = CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 0.0f;
				}
			}

			VehicleMovement = Cast<URogueRVO_VehicleMovementComponent>(Pawn->GetMovementComponent());
			if(VehicleMovement != nullptr)
			{
				// Assign radius and height from mesh shape of the vehicle
				TArray<UActorComponent*> MeshComponents;
				OwningActor->GetComponents(USkeletalMeshComponent::StaticClass(), MeshComponents);
				FVector BoxExtent;
				if (MeshComponents.Num() > 0 && MeshComponents.IsValidIndex(0))
				{
					// Assumes only one mesh component, may need to change this if using multiple meshes
					if (const USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(MeshComponents[0]))
					{
						// Get the bounding box of the mesh
						const FBoxSphereBounds Bounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());

						// Construct a collision shape from the bounding box
						BoxExtent = Bounds.BoxExtent;
					}
				}
				else
				{
					// If failed to get mesh just create a generic bounding box from actor bounds
					FVector ActorOrigin;
					OwningActor->GetActorBounds(true, ActorOrigin, BoxExtent, false);
				}
		
				RVOAvoidanceRadius = FMath::Max(BoxExtent.X * 2.f, BoxExtent.Y * 2.f);
				RVOAvoidanceHeight = BoxExtent.Z;
			}
		}	
		
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("[%s] %s: InitializeComponent - RVOAvoidanceRadius=%f; RVOAvoidanceHeight=%f"),
		*GetClass()->GetName(), *OwningActor->GetName(), RVOAvoidanceRadius, RVOAvoidanceHeight);
#endif
	}
}

void URogueRVO_Component::InitializeAvoidance()
{
	if(bUseRVOAvoidance)
	{
		if (const AActor* OwningActor = GetOwner())
		{
			PrimaryComponentTick.SetTickFunctionEnable(true);
		
#if WITH_EDITOR	
			UE_LOG(LogTemp, Log, TEXT("[%s] %s: InitializeAvoidance"), *GetClass()->GetName(), *OwningActor->GetName());		
#endif
		
			// Reset id, register component with rvo subsystem
			if (URogueRVO_Subsystem* RogueRVOSubsystem = GetWorld()->GetSubsystem<URogueRVO_Subsystem>())
			{
				AvoidanceUID = 0;
				RogueRVOSubsystem->RegisterRVOComponent(this, AvoidanceWeight);
			}
		}
	}
}

void URogueRVO_Component::CalculateAvoidanceVelocity()
{
	// Always reset the avoidance velocity to the current velocity
	AvoidanceVelocity = GetVelocityForRVOConsideration();
	
	if(AvoidanceWeight >= 1.0f)
	{
		return;
	}

	const AActor* OwningActor = GetOwner();
	URogueRVO_Subsystem* RogueRVOSubsystem = GetWorld()->GetSubsystem<URogueRVO_Subsystem>();
	if(!OwningActor || !OwningActor->HasAuthority() || !RogueRVOSubsystem)
	{
		return;
	}	

	if (!AvoidanceVelocity.IsZero())
	{
		if (AvoidanceLockTimer > 0.0f)
		{
			AvoidanceVelocity = AvoidanceLockVelocity;
		}
		else
		{
			const FVector NewVelocity = RogueRVOSubsystem->GetAvoidanceVelocityForComponent(this);
			if (!NewVelocity.Equals(AvoidanceVelocity))
			{
				
#if WITH_EDITOR
				//UE_LOG(LogTemp, Verbose, TEXT("[%s] %s: RVO Update AvoidanceVelocity=%s"), *GetClass()->GetName(), *OwningActor->GetName(), *NewVelocity.ToCompactString());
#endif
				
				//Had to divert course, lock this avoidance move in for a short time. This will make us a VO, so unlocked others will know to avoid us.
				AvoidanceVelocity = NewVelocity;
			}			
			else
			{
				//Although we didn't divert course, our velocity for this frame is decided. We will not reciprocate anything further, so treat as a VO for the remainder of this frame.
				SetAvoidanceVelocityLock(RogueRVOSubsystem, RogueRVOSubsystem->LockTimeAfterClean);	//10 ms of lock time should be adequate.
			}
		}

		RogueRVOSubsystem->UpdateRVOComponent(this);
		bWasAvoidanceUpdated = true;
	}				
}

void URogueRVO_Component::UpdateAvoidance()
{
	if (!bUseRVOAvoidance)
	{
		return;
	}
	
	if(URogueRVO_Subsystem* RogueRVOSubsystem = GetWorld()->GetSubsystem<URogueRVO_Subsystem>())
	{
		if (RogueRVOSubsystem && !bWasAvoidanceUpdated)
		{
			RogueRVOSubsystem->UpdateRVOComponent(this);

			//Consider this a clean move because we didn't even try to avoid.
			SetAvoidanceVelocityLock(RogueRVOSubsystem, RogueRVOSubsystem->LockTimeAfterClean);
		}
	}
	
	bWasAvoidanceUpdated = false;
}

void URogueRVO_Component::SetAvoidanceVelocityLock(URogueRVO_Subsystem* Avoidance, float Duration)
{
	Avoidance->OverrideToMaxWeight(AvoidanceUID, Duration);
	AvoidanceLockVelocity = AvoidanceVelocity;
	AvoidanceLockTimer = Duration;
}


void URogueRVO_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AvoidanceLockTimer > 0.0f)
	{
		AvoidanceLockTimer -= DeltaTime;
	}
	
	CalculateAvoidanceVelocity();
	UpdateAvoidance();
}

void URogueRVO_Component::InitializeComponent()
{
	Super::InitializeComponent();

	if(GetOwner())
	{
		InitializeAgentProperties();
		InitializeAvoidance();
	}
}

void URogueRVO_Component::OnRegister()
{
	const ENetMode NetMode = GetNetMode();
	if (bUseRVOAvoidance && NetMode == NM_Client)
	{
		bUseRVOAvoidance = false;
	}
	
	Super::OnRegister();	
}

void URogueRVO_Component::Deactivate()
{
	Super::Deactivate();
	
	if(URogueRVO_Subsystem* RogueRVOSubsystem = GetWorld()->GetSubsystem<URogueRVO_Subsystem>())
	{
		RogueRVOSubsystem->RemoveAvoidanceObject(AvoidanceUID);
	}	
}

void URogueRVO_Component::SetRVOAvoidanceUID(int32 UID)
{
	AvoidanceUID = UID;
}

int32 URogueRVO_Component::GetRVOAvoidanceUID()
{
	return AvoidanceUID;
}

void URogueRVO_Component::SetRVOAvoidanceWeight(float Weight)
{
	AvoidanceWeight = Weight;
}

float URogueRVO_Component::GetRVOAvoidanceWeight()
{
	return AvoidanceWeight;
}

FVector URogueRVO_Component::GetRVOAvoidanceOrigin()
{
	if (const AActor* OwningActor = GetOwner())
	{
		if(CharacterMovement != nullptr)
		{			
			return CharacterMovement->GetActorFeetLocation();
		}
		
		return OwningActor->GetActorLocation();		
	}
	
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("%s: GetRVOAvoidanceOrigin - OwningActor is NULL"), *GetName());
#endif
	
	return FVector();
}

float URogueRVO_Component::GetRVOAvoidanceRadius()
{
	return RVOAvoidanceRadius;
}

float URogueRVO_Component::GetRVOAvoidanceHeight()
{
	return RVOAvoidanceHeight;
}

float URogueRVO_Component::GetRVOAvoidanceConsiderationRadius()
{
	return AvoidanceConsiderationRadius;
}

FVector URogueRVO_Component::GetVelocityForRVOConsideration()
{
	if(const AActor* OwningActor = GetOwner())
	{
		FVector Velocity2D = OwningActor->GetVelocity();
		
		if(CharacterMovement != nullptr)
		{
			Velocity2D = CharacterMovement->GetLastUpdateVelocity();
		}
		
		if(VehicleMovement != nullptr)
		{
			Velocity2D = VehicleMovement->GetDesiredVelocity();
		}

		// Zero the 3D space
		Velocity2D.Z = 0.f;

		/*if(const IRTSCore_VehicleInputInterface* VehicleInputInterface = Cast<IRTSCore_VehicleInputInterface>(OwningActor))
		{
			const float BrakeAmount = VehicleInputInterface->GetBrakeInput();
			Velocity2D *= (1 + BrakeAmount);
		}	*/	

		return Velocity2D;
	}
	
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("%s: GetVelocityForRVOConsideration - OwningActor is NULL"), *GetName());
#endif
	
	return FVector();
}

FVector URogueRVO_Component::GetRVOAvoidanceVelocity()
{
	return AvoidanceVelocity;
}

void URogueRVO_Component::SetAvoidanceGroupMask(int32 GroupFlags)
{
	AvoidanceGroup.SetFlagsDirectly(GroupFlags);
}

int32 URogueRVO_Component::GetAvoidanceGroupMask()
{
	return AvoidanceGroup.Packed;
}

void URogueRVO_Component::SetGroupsToAvoidMask(int32 GroupFlags)
{
	GroupsToAvoid.SetFlagsDirectly(GroupFlags);
}

int32 URogueRVO_Component::GetGroupsToAvoidMask()
{
	return GroupsToAvoid.Packed;
}

void URogueRVO_Component::SetGroupsToIgnoreMask(int32 GroupFlags)
{
	GroupsToIgnore.SetFlagsDirectly(GroupFlags);
}

int32 URogueRVO_Component::GetGroupsToIgnoreMask()
{
	return GroupsToIgnore.Packed;
}

