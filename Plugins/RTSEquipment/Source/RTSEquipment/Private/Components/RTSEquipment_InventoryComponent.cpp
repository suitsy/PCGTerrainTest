// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RTSEquipment_InventoryComponent.h"
#include "Data/RTSEquipment_ItemAmmunitionDataAsset.h"
#include "Data/RTSEquipment_ItemDataAsset.h"
#include "Data/RTSEquipment_ItemWeaponBaseDataAsset.h"
#include "Data/Weapons/RTSEquipment_WeaponTurretDataAsset.h"
#include "Debug/RTSEquipment_Debug.h"
#include "Engine/AssetManager.h"
#include "Equipment/RTSEquipment_Item.h"
#include "Equipment/Weapons/RTSEquipment_WeaponItem.h"
#include "Framework/Interfaces/RTSCore_EntityEquipmentInterface.h"
#include "Framework/Interfaces/RTSCore_EntityInterface.h"
#include "Framework/Managers/RTSCore_GameState.h"
#include "Net/UnrealNetwork.h"


URTSEquipment_InventoryComponent::URTSEquipment_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	EquippedSlot = -1;
}

void URTSEquipment_InventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	EquipmentUpdate();
}

void URTSEquipment_InventoryComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, OwningPlayer, COND_OwnerOnly);	
	DOREPLIFETIME(ThisClass, Inventory);	
	DOREPLIFETIME(ThisClass, EquippedSlot);
	DOREPLIFETIME(ThisClass, EquipmentData);
	DOREPLIFETIME(ThisClass, TurretRotations);
	DOREPLIFETIME(ThisClass, WeaponRotations);
	DOREPLIFETIME(ThisClass, RawTurretRotations);
	DOREPLIFETIME(ThisClass, RawWeaponRotations);
}

void URTSEquipment_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(GetOwner(), TEXT("[%s] - Failed to get owner!"), *GetClass()->GetName());
}

void URTSEquipment_InventoryComponent::AssignEntityEquipment(APawn* Pawn, AController* OldController, AController* NewController)
{
	if(IRTSCore_EntityEquipmentInterface* AiControllerInterface = Cast<IRTSCore_EntityEquipmentInterface>(NewController))
	{
		AiControllerInterface->AssignEquipment(this);
	}

	Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &URTSEquipment_InventoryComponent::AssignEntityEquipment);
}

void URTSEquipment_InventoryComponent::SetEquipmentData(APlayerController* NewOwningPlayer, const TArray<FPrimaryAssetId>& NewEquipmentData)
{
	if(NewOwningPlayer && NewOwningPlayer->HasAuthority())
	{
		OwningPlayer = NewOwningPlayer;
		EquipmentData = NewEquipmentData;
		
		// Set equipped slot to no slot
		EquippedSlot = -1;

		// Assign equipment component reference to ai controller 
		if(APawn* AiPawn = GetPawn())
		{
			AiPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &URTSEquipment_InventoryComponent::AssignEntityEquipment);			
		}

		// Register this component with the preload game phase state
		if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
		{
			RTSGameState->CallOrRegisterForEvent_OnGamePhasePreGameplay(FOnGamePhasePreGameplayDelegate::FDelegate::CreateUObject(this, &ThisClass::LoadEquipmentData), CreateEntityEquipmentTaskId, FString("EquipmentManager - Load Entity Equipment"));
		}
	}
}

void URTSEquipment_InventoryComponent::EquipItem(const int32 Slot)
{
	if(GetOwner() && GetOwner()->HasAuthority())
	{
		// Un-equip any equipped items
		UnEquipItem();
	
		// Get new slot data
		if(Inventory.IsValidIndex(Slot))
		{
			// Remove any unequipped actors unless they are meant to remain on equip			
			for (const FRTSEquipment_SlotItem Item : Inventory[Slot].UnEquippedSpawnedActors)
			{
				if(!Item.bRemainsDisplayedEquipped)
				{
					Item.Actor->BeginDestroy();
				}
			}
	
			// Create equipped actors
			CreateEquippedActors(Slot);
			
			// Update equipped slot to new slot
			EquippedSlot = Slot;

			// Assign data to equipped actors
			for (int i = 0; i < Inventory[EquippedSlot].EquippedSpawnedActors.Num(); ++i)
			{
				if(ARTSEquipment_Item* Item = Cast<ARTSEquipment_Item>(Inventory[EquippedSlot].EquippedSpawnedActors[i].Actor))
				{
					Item->OnEquipped(Inventory[Slot].AssetId);
				}				
			}			
		}	
	}
}

void URTSEquipment_InventoryComponent::UnEquipItem()
{
	// Check if the equipped slot is valid
	if(Inventory.IsValidIndex(EquippedSlot))
	{		
		// Remove any spawned actors for equipped slot
		for (int i = 0; i < Inventory[EquippedSlot].EquippedSpawnedActors.Num(); ++i)
		{
			if(Inventory[EquippedSlot].EquippedSpawnedActors[i].Actor != nullptr)
			{
				Inventory[EquippedSlot].EquippedSpawnedActors[i].Actor->BeginDestroy();
			}
		}

		// Create unequipped actors
		CreateUnEquippedActors(EquippedSlot);		
	}
}

void URTSEquipment_InventoryComponent::EquipWeapon(const ERTSEquipment_WeaponSlot WeaponSlot)
{
	// Iterate through the inventory to find the equipment with the desired slot
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i].Type == ERTSEquipment_Type::Weapon && Inventory[i].WeaponSlot == WeaponSlot)
		{
			// Equip the found weapon
			EquipItem(i);
			return;
		}
	}	
}

/** Returns the ammunition slot in the inventory **/
int32 URTSEquipment_InventoryComponent::GetAmmunitionSlot(const FPrimaryAssetId& AmmoType)
{
	// Iterate through the inventory to find the equipment with the desired slot
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i].Type == ERTSEquipment_Type::Ammunition && Inventory[i].AmmunitionType == AmmoType)
		{
			return i;
		}
	}

	return -1;
}

void URTSEquipment_InventoryComponent::LoadEquipmentData()
{
	if(GetOwner() && GetOwner()->HasAuthority())
	{
		if(UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(EquipmentData.Num() > 0)
			{
				const TArray<FName> Bundles;
				const FStreamableDelegate LoadedDelegate = FStreamableDelegate::CreateUObject(this, &URTSEquipment_InventoryComponent::OnEquipmentDataLoaded);
				AssetManager->LoadPrimaryAssets(EquipmentData, Bundles, LoadedDelegate);
			}
			else
			{
				UE_LOG(LogRTSEquipment, Error, TEXT("[%s] - Failed to get equipment data assets"), *GetClass()->GetSuperClass()->GetName());
			}
		}
	}
}

void URTSEquipment_InventoryComponent::OnEquipmentDataLoaded()
{
	for (int j = 0; j < EquipmentData.Num(); ++j)
	{
		CreateInventorySlot(EquipmentData[j]); 		
	}

	if(EquippedSlot == -1)
	{
		EquipWeapon(ERTSEquipment_WeaponSlot::Primary);
	}

	if(const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if(IRTSCore_EntityEquipmentInterface* EntityEquipmentInterface = Cast<IRTSCore_EntityEquipmentInterface>(Pawn->GetController()))
		{
			EntityEquipmentInterface->OnEquipmentCreated();
		}
	}

	if(ARTSCore_GameState* RTSGameState = Cast<ARTSCore_GameState>(GetWorld()->GetGameState()))
	{
		// Register the load load outs task complete
		RTSGameState->RegistrationPhaseTaskComplete(CreateEntityEquipmentTaskId);
	}
}

void URTSEquipment_InventoryComponent::CreateInventorySlot(const FPrimaryAssetId& EquipmentDataId)
{
	if(GetOwner() && GetOwner()->HasAuthority() && EquipmentDataId.IsValid())
	{
		FRTSEquipment_Slot NewSlot = FRTSEquipment_Slot(EquipmentDataId);

		if(IsItemType<URTSEquipment_ItemDataAsset>(EquipmentDataId))
		{
			if(URTSEquipment_ItemDataAsset* ItemData = GetData<URTSEquipment_ItemDataAsset>(EquipmentDataId))
			{
				// Assign equipment type
				NewSlot.Type = ItemData->Type;
				
				for (const FRTSEquipment_ItemData Equipment : ItemData->ActorsToSpawn)
				{
					for (FRTSEquipment_ItemActorData ItemActorData : Equipment.UnEquippedActorData)
					{
						if(ItemActorData.bDisplayUnEquipped)
						{
							CreateUnEquippedActor(NewSlot, ItemActorData);
						}
					}				
				}
			}
		}

		// Check if the new item is a weapon so we can assign its weapon slot data
		if(IsItemType<URTSEquipment_ItemWeaponBaseDataAsset>(EquipmentDataId))
		{
			if(URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(EquipmentDataId))
			{
				// Assign equipment type
				NewSlot.Type = ItemData->Type;
				NewSlot.WeaponSlot = ItemData->GetWeaponSlot();
				NewSlot.AmmunitionType = ItemData->GetAmmunitionType();
			}
		}

		if(IsItemType<URTSEquipment_ItemAmmunitionDataAsset>(EquipmentDataId))
		{
			if(URTSEquipment_ItemAmmunitionDataAsset* ItemData = GetData<URTSEquipment_ItemAmmunitionDataAsset>(EquipmentDataId))
			{
				// Assign equipment type
				NewSlot.Type = ItemData->Type;
				NewSlot.StoredMagazineCount = ItemData->MagazineCount;
			}
		}

		if(IsItemType<URTSEquipment_WeaponTurretDataAsset>(EquipmentDataId))
		{
			if(URTSEquipment_WeaponTurretDataAsset* ItemData = GetData<URTSEquipment_WeaponTurretDataAsset>(EquipmentDataId))
			{
				// Assign equipment type
				NewSlot.Type = ItemData->Type;
				
				while (TurretRotations.Num() < ItemData->AnimationIndex + 1)
				{
					TurretRotations.Add(FRotator::ZeroRotator);
					RawTurretRotations.Add(FRotator::ZeroRotator);
					WeaponRotations.Add(FRotator::ZeroRotator);
					RawWeaponRotations.Add(FRotator::ZeroRotator);
				}
			}
		}
			
		Inventory.Add(NewSlot);
	}
}

void URTSEquipment_InventoryComponent::OnEquippedWeaponFired()
{
	const int32 AmmoSlot = GetAmmunitionSlot(Inventory[EquippedSlot].AmmunitionType);
	if(Inventory.IsValidIndex(AmmoSlot))
	{
		if(const URTSEquipment_ItemAmmunitionDataAsset* AmmoItemData = GetData<URTSEquipment_ItemAmmunitionDataAsset>(Inventory[AmmoSlot].AmmunitionType))
		{
			Inventory[EquippedSlot].MagazineAmmunitionCount = FMath::Clamp(Inventory[EquippedSlot].MagazineAmmunitionCount - 1, 0, AmmoItemData->MagazineCapacity);
		}
	}
}

void URTSEquipment_InventoryComponent::OnEquippedWeaponReload()
{
	const int32 AmmoSlot = GetAmmunitionSlot(Inventory[EquippedSlot].AmmunitionType);
	if(Inventory.IsValidIndex(AmmoSlot) && Inventory[AmmoSlot].StoredMagazineCount > 0)
	{
		Inventory[AmmoSlot].StoredMagazineCount -= 1;
		if(const URTSEquipment_ItemAmmunitionDataAsset* AmmoItemData = GetData<URTSEquipment_ItemAmmunitionDataAsset>(Inventory[AmmoSlot].AmmunitionType))
		{
			Inventory[EquippedSlot].MagazineAmmunitionCount = AmmoItemData->MagazineCapacity;
		}
	}
}

void URTSEquipment_InventoryComponent::CreateUnEquippedActors(const int32 Slot)
{
	if(URTSEquipment_ItemDataAsset* ItemData = GetData<URTSEquipment_ItemDataAsset>(Inventory[Slot].AssetId))
	{
		// Create unequipped actors
		for (const FRTSEquipment_ItemData Equipment : ItemData->ActorsToSpawn)
		{
			for (FRTSEquipment_ItemActorData ItemActorData : Equipment.UnEquippedActorData)
			{
				if(ItemActorData.bDisplayUnEquipped && !ItemActorData.bRemainsDisplayedEquipped)
				{
					CreateUnEquippedActor(Inventory[Slot], ItemActorData);
				}
			}				
		}
	}
}

void URTSEquipment_InventoryComponent::CreateUnEquippedActor(FRTSEquipment_Slot& Slot, const FRTSEquipment_ItemActorData& SlotData) const
{
	Slot.UnEquippedSpawnedActors.Add(CreateEquipment(SlotData));		
}

void URTSEquipment_InventoryComponent::CreateEquippedActors(const int32 Slot)
{
	if(URTSEquipment_ItemDataAsset* ItemData = GetData<URTSEquipment_ItemDataAsset>(Inventory[Slot].AssetId))
	{	
		// Create equipped actors
		for (const FRTSEquipment_ItemData Equipment : ItemData->ActorsToSpawn)
		{
			for (FRTSEquipment_ItemActorData ItemActorData : Equipment.EquippedActorData)
			{
				CreateEquippedActor(Inventory[Slot], ItemActorData);
			}			
		}					
	}
}

void URTSEquipment_InventoryComponent::CreateEquippedActor(FRTSEquipment_Slot& Slot, const FRTSEquipment_ItemActorData& SlotData) const
{
	Slot.EquippedSpawnedActors.Add(CreateEquipment(SlotData));
}

FRTSEquipment_SlotItem URTSEquipment_InventoryComponent::CreateEquipment(const FRTSEquipment_ItemActorData& SlotData) const
{
	if(SlotData.ItemToSpawn.LoadSynchronous() != nullptr)
	{		
		if(GetPawn())
		{
			USceneComponent* AttachTarget = GetPawn()->GetRootComponent();
			if(const IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetPawn()))
			{
				AttachTarget = EntityInterface->GetEntityMesh();
			}

			if(AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SlotData.ItemToSpawn.LoadSynchronous(), FTransform::Identity, GetPawn()))
			{
				NewActor->FinishSpawning(FTransform::Identity, true);
				NewActor->SetActorRelativeTransform(SlotData.AttachTransform);
				NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SlotData.AttachSocket);				
				return FRTSEquipment_SlotItem(NewActor, SlotData.bRemainsDisplayedEquipped);
			}
		}
	}

	return FRTSEquipment_SlotItem();
}

void URTSEquipment_InventoryComponent::EquipmentUpdate()
{
	for (int i = 0; i < Inventory.Num(); ++i)
	{
		if(Inventory[i].Type == ERTSEquipment_Type::Weapon)
		{
			if(IsItemType<URTSEquipment_WeaponTurretDataAsset>(Inventory[i].AssetId))
			{
				UpdateTurretRotation(Inventory[i]);
			}
		}
	}
}

void URTSEquipment_InventoryComponent::UpdateTurretRotation(FRTSEquipment_Slot& EquipmentSlot)
{
	// Check we have a valid owner and world
	if(!GetOwner() && GetWorld())
	{
		return;
	}

	// Get data for turret
	if(const URTSEquipment_WeaponTurretDataAsset* TurretData = GetData<URTSEquipment_WeaponTurretDataAsset>(EquipmentSlot.AssetId))
	{
		// Get Entity interface for actor mesh reference
		if(GetPawn())
		{
			if(const IRTSCore_EntityInterface* EntityInterface = Cast<IRTSCore_EntityInterface>(GetPawn()->GetController()))
			{
				if(const USkeletalMeshComponent* ActorMesh = Cast<USkeletalMeshComponent>(EntityInterface->GetEntityMesh()))
				{
					const FRotator CurrentTurretRotation = ActorMesh->GetSocketRotation(TurretData->GetTurretBoneName());
					const FVector TurretLocation = ActorMesh->GetSocketLocation(TurretData->GetTurretBoneName());
					const FVector AimPoint = GetTurretAimPoint(TurretLocation, EquipmentSlot.Target);
					FVector SocketToTargetDirTurret = AimPoint - TurretLocation;

					//DrawDebugDirectionalArrow(GetWorld(), TurretLocation, TurretLocation + SocketToTargetDirTurret.GetSafeNormal() * 1000.f, 50.f, FColor::Red, false, 0.01f, 0, 4.f);

					if(EquipmentSlot.Target != nullptr)
					{
						CalculateBallistics(SocketToTargetDirTurret);
					}

					// Calculate turret rotation
					const FRotator ParentBoneRotation = ActorMesh->GetSocketRotation(ActorMesh->GetParentBone(TurretData->GetTurretBoneName()));
					const FVector TurretTargetVectorProjectedXY = ParentBoneRotation.UnrotateVector(FVector::VectorPlaneProject(SocketToTargetDirTurret, ActorMesh->GetUpVector()));		
					const FRotator TargetRotationTurret = TurretTargetVectorProjectedXY.ToOrientationRotator();

					if(RawTurretRotations.IsValidIndex(TurretData->AnimationIndex) && TurretRotations.IsValidIndex(TurretData->AnimationIndex))
					{
						const FRotator ActorRotation = FRotator(0.f, GetOwner()->GetActorRotation().Yaw, 0.f);
						FRotator TurretDeltaRotation = RawTurretRotations[TurretData->AnimationIndex] - ActorRotation;
						TurretDeltaRotation.Normalize();				
						const FRotator InterpTurretRotation = FMath::RInterpTo(TurretDeltaRotation, TargetRotationTurret, GetWorld()->GetDeltaSeconds(), TurretData->GetRotationSpeed());
						const FRotator RangeClampedTurretRotation = FRotator(0.f, FMath::Clamp(InterpTurretRotation.Yaw, TurretData->GetHorizontalRange().X, TurretData->GetHorizontalRange().Y), 0.f);
					
						// Assign rotation for animation
						RawTurretRotations[TurretData->AnimationIndex] = FRotator(FQuat(ActorRotation) * FQuat(RangeClampedTurretRotation));
						TurretRotations[TurretData->AnimationIndex] = RangeClampedTurretRotation; 
					}
			
					// Calculate weapon rotation (pitch)
					const FVector WeaponLocation = ActorMesh->GetSocketLocation(TurretData->GetTurretWeaponBoneName());
					const FRotator WeaponRotation = ActorMesh->GetSocketRotation(TurretData->GetTurretWeaponBoneName());
					FVector SocketToTargetDirWeapon = AimPoint - WeaponLocation;

					if(EquipmentSlot.Target != nullptr)
					{
						CalculateBallistics(SocketToTargetDirWeapon);
					}
				
					const FVector WeaponTargetVectorComponentSpace = GetOwner()->GetActorRotation().UnrotateVector(SocketToTargetDirWeapon);
					const FRotator RotFromX = WeaponTargetVectorComponentSpace.ToOrientationRotator();
					const FRotator WeaponTargetRotation = FRotator(RotFromX.Pitch, 0.f, 0.f);				

					//DrawDebugDirectionalArrow(GetWorld(), WeaponLocation, WeaponLocation + SocketToTargetDirWeapon.GetSafeNormal() * 1000.f, 50.f, FColor::Emerald, false, 0.01f, 0, 4.f);

				

					if(RawWeaponRotations.IsValidIndex(TurretData->AnimationIndex) && WeaponRotations.IsValidIndex(TurretData->AnimationIndex))
					{
						const FRotator ActorRotation = FRotator(GetOwner()->GetActorRotation().Pitch, 0.f, 0.f);
						FRotator WeaponDeltaRotation = RawWeaponRotations[TurretData->AnimationIndex] - ActorRotation;
						WeaponDeltaRotation.Normalize();	
						FRotator DesiredWeaponRotation = FMath::RInterpTo(WeaponDeltaRotation, WeaponTargetRotation, GetWorld()->GetDeltaSeconds(), TurretData->GetPitchSpeed());

						FRotator OffsetRotation = WeaponRotation;
						float AdjustedPitch = 0.f;
						if(TurretData->ShouldPerformCollisionCheck())
						{
							// Check current location collision						
							while (WeaponBodyCollisionCheck(WeaponLocation, OffsetRotation, false))
							{
								OffsetRotation.Pitch += 0.1f;
								AdjustedPitch += 0.1f; 
							}						

							// Add adjusted pitch
							DesiredWeaponRotation.Pitch += AdjustedPitch;

							// Check predicted rotation collision with foreign bodies,
							// predicted rotation is current turret rotation plus the desired weapon rotation (including adjusted pitch)
							FRotator PredictedRotation = FRotator(FQuat(CurrentTurretRotation) * FQuat(DesiredWeaponRotation));
							if(WeaponBodyCollisionCheck(WeaponLocation, PredictedRotation, true))
							{
								// If collision with body or other prevent rotation change
								DesiredWeaponRotation = WeaponDeltaRotation;
							}
						}
					
						FRotator RangeClampedWeaponRotation = FRotator(FMath::Clamp(DesiredWeaponRotation.Pitch, TurretData->GetVerticalRange().X, TurretData->GetVerticalRange().Y), 0.f, 0.f);
					
						// Assign rotation for animation
						RawWeaponRotations[TurretData->AnimationIndex] = FRotator(FQuat(ActorRotation) * FQuat(RangeClampedWeaponRotation));					
						WeaponRotations[TurretData->AnimationIndex] = RangeClampedWeaponRotation;
					}
					// Rotation SFX
					if(TurretData->RotationSound != nullptr)
					{
						const FRotator CombinedTurretRotation = FRotator(FQuat(CurrentTurretRotation) * FQuat(ParentBoneRotation));
						EquipmentSlot.bIsTurretRotating = !EquipmentSlot.PreviousRotation.Equals(CombinedTurretRotation, 0.1f);
						EquipmentSlot.PreviousRotation = CombinedTurretRotation;
					}
				}
			}
		}
	}
}

FVector URTSEquipment_InventoryComponent::GetTurretAimPoint(const FVector& TurretLocation, const AActor* Target) const
{
	// Check we have a valid owner and world
	if(!GetOwner() || !GetWorld())
	{
		return FVector::ZeroVector;
	}

	// If we have a target return its position as an aimpoint @TODO could add specific aim bones here
	if(Target != nullptr)
	{
		FVector ViewPoint;
		FRotator ViewRotation;
		Target->GetActorEyesViewPoint(ViewPoint, ViewRotation);
		return ViewPoint;
	}
	
	const FVector End = TurretLocation + (GetOwner()->GetActorForwardVector() * 10000.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;
	if(GetWorld()->LineTraceSingleByChannel(HitResult, TurretLocation, End, ECC_Visibility, QueryParams))
	{
		return HitResult.Location;
	}

	return HitResult.TraceEnd;	
}

bool URTSEquipment_InventoryComponent::WeaponBodyCollisionCheck(const FVector& WeaponLocation, const FRotator& WeaponRotation, const bool bIgnoreSelf) const
{
	// Calculate the start location
	const FVector End = WeaponLocation - FVector(0.f,0.f, 25.f);	
	const FVector Start = End + (WeaponRotation.Vector().GetSafeNormal() * 500.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	if(bIgnoreSelf)
	{
		QueryParams.AddIgnoredActor(GetOwner());
	}
	
	FHitResult BodyHitResult;
	if (GetWorld()->LineTraceSingleByChannel(BodyHitResult, Start, End, ECC_Visibility, QueryParams))
	{
		if(bIgnoreSelf)
		{
			return true;
		}
		
		// Check if the hit bone is "body"
		if(BodyHitResult.BoneName == FName(TEXT("body")))
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 2.f);
			return true;
		}
	}
	
	// No collision, break out of the loop			
	return false;
}

void URTSEquipment_InventoryComponent::CalculateBallistics(FVector& SocketToTargetDir) const
{
	
}

bool URTSEquipment_InventoryComponent::HasWeaponEquipped() const
{
	return Inventory.IsValidIndex(EquippedSlot) && Inventory[EquippedSlot].Type == ERTSEquipment_Type::Weapon;
}

void URTSEquipment_InventoryComponent::FireWeapon()
{
	if(IsWeapon() && GetCurrentWeaponMagazineAmmoCount() > 0)
	{
		TArray<FRTSEquipment_SlotItem> EquippedSpawnedActors = Inventory[EquippedSlot].EquippedSpawnedActors;
		for (FRTSEquipment_SlotItem SlotItem : EquippedSpawnedActors)
		{
			if(ARTSEquipment_WeaponItem* Weapon = Cast<ARTSEquipment_WeaponItem>(SlotItem.Actor))
			{
				Weapon->Fire();

				// Update ammo and weapon inventories
				OnEquippedWeaponFired();								
			}
		}		
	}
}

void URTSEquipment_InventoryComponent::ReloadWeapon()
{
	// Check current equipped item is a weapon
	if(IsWeapon())
	{
		// Get ammunition
		const int32 AmmoSlot = GetAmmunitionSlot(Inventory[EquippedSlot].AmmunitionType);
		if(Inventory.IsValidIndex(AmmoSlot) && Inventory[AmmoSlot].StoredMagazineCount > 0)
		{
			// Get the spawn actors for this weapon
			TArray<FRTSEquipment_SlotItem> EquippedSpawnedActors = Inventory[EquippedSlot].EquippedSpawnedActors;
			for (FRTSEquipment_SlotItem SlotItem : EquippedSpawnedActors)
			{
				// If the slot item is a weapon, reload it
				if(ARTSEquipment_WeaponItem* Weapon = Cast<ARTSEquipment_WeaponItem>(SlotItem.Actor))
				{
					Weapon->Reload();

					// Update ammo and weapon inventories
					OnEquippedWeaponReload();						
				}
			}
		}		
	}
}

float URTSEquipment_InventoryComponent::GetCurrentWeaponRange()
{
	if(IsWeapon())
	{
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(Inventory[EquippedSlot].AssetId))
		{
			return ItemData->GetRange();
		}
	}

	return 0.f;
}

int32 URTSEquipment_InventoryComponent::GetCurrentWeaponMagazineAmmoCount()
{
	if(IsWeapon())
	{
		return Inventory[EquippedSlot].MagazineAmmunitionCount;
	}

	return 0;
}

float URTSEquipment_InventoryComponent::GetCurrentWeaponFireRate()
{
	if(IsWeapon())
	{
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(Inventory[EquippedSlot].AssetId))
		{
			return ItemData->GetFireRate();
		}
	}

	return -1.f;
}

float URTSEquipment_InventoryComponent::GetCurrentWeaponReloadRate()
{
	if(IsWeapon())
	{
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(Inventory[EquippedSlot].AssetId))
		{
			return ItemData->GetReloadRate();
		}
	}

	return -1.f;
}

bool URTSEquipment_InventoryComponent::HasInventoryAmmoForCurrentWeapon()
{
	// Check current equipped item is a weapon
	if(IsWeapon())
	{
		// Get weapon data
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(Inventory[EquippedSlot].AssetId))
		{
			// Get ammunition
			const int32 AmmoSlot = GetAmmunitionSlot(ItemData->GetAmmunitionType());
			return Inventory.IsValidIndex(AmmoSlot) && Inventory[AmmoSlot].StoredMagazineCount > 0;
		}
	}

	return false;
}

float URTSEquipment_InventoryComponent::GetCurrentWeaponAccuracy()
{
	// Check current equipped item is a weapon
	if(IsWeapon())
	{
		// Get weapon data
		if(const URTSEquipment_ItemWeaponBaseDataAsset* ItemData = GetData<URTSEquipment_ItemWeaponBaseDataAsset>(Inventory[EquippedSlot].AssetId))
		{
			// Get ammunition
			return ItemData->GetAccuracy();
		}
	}

	return false;
}

APawn* URTSEquipment_InventoryComponent::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}
