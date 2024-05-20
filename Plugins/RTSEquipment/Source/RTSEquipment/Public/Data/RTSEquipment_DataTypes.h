#pragma once

#include "Engine/AssetManager.h"
#include "RTSEquipment_DataTypes.generated.h"

class ARTSEquipment_Item;



template<class T>
T* GetData(const FPrimaryAssetId& DataId)
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(DataId.IsValid())
		{
			return Cast<T>(AssetManager->GetPrimaryAssetObject(DataId));
		}
	}

	return nullptr;
}

template<class T>
bool IsItemType(const FPrimaryAssetId& EquipmentDataId)
{
	if (const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(EquipmentDataId.IsValid())
		{
			if(const UObject* Item = Cast<UObject>(AssetManager->GetPrimaryAssetObject(EquipmentDataId)))
			{
				if(Item->IsA(T::StaticClass()))
				{
					return true;
				}
			}
		}
	}

	return false;
}

UENUM()
enum class ERTSEquipment_Type
{
	NoInventory,
	Equipment,
	Ammunition,
	Weapon
};

UENUM()
enum class ERTSEquipment_TypeWeapon
{
	Rifle,
	Turret,
	MachineGun
};

UENUM()
enum class ERTSEquipment_WeaponSlot
{
	NoSlot,
	Primary,
	Secondary,
	Tertiary,
	AlternatePrimary,
	AlternateSecondary,
	AlternateTertiary
};

USTRUCT()
struct FRTSEquipment_SlotItem
{
	GENERATED_BODY()

public:
	FRTSEquipment_SlotItem(): Actor(nullptr), bRemainsDisplayedEquipped(false) {}
	FRTSEquipment_SlotItem(const TObjectPtr<AActor> InActor, const bool InRemainsOnEquipped): Actor(InActor), bRemainsDisplayedEquipped(InRemainsOnEquipped) {}

	UPROPERTY()
	TObjectPtr<AActor> Actor;
	
	UPROPERTY()
	bool bRemainsDisplayedEquipped;

	
};

USTRUCT()
struct FRTSEquipment_Slot
{
	GENERATED_BODY()

public:
	FRTSEquipment_Slot(): AssetId(FPrimaryAssetId()), UnEquippedSpawnedActors(TArray<FRTSEquipment_SlotItem>()), EquippedSpawnedActors(TArray<FRTSEquipment_SlotItem>()),
	Type(ERTSEquipment_Type::NoInventory), WeaponSlot(ERTSEquipment_WeaponSlot::NoSlot), MagazineAmmunitionCount(0), AmmunitionType(FPrimaryAssetId()), StoredMagazineCount(0), Target(nullptr), bIsTurretRotating(false) {}
	explicit FRTSEquipment_Slot(const FPrimaryAssetId& InAssetId): AssetId(InAssetId), UnEquippedSpawnedActors(TArray<FRTSEquipment_SlotItem>()), EquippedSpawnedActors(TArray<FRTSEquipment_SlotItem>()),
	Type(ERTSEquipment_Type::NoInventory), WeaponSlot(ERTSEquipment_WeaponSlot::NoSlot), MagazineAmmunitionCount(0), AmmunitionType(FPrimaryAssetId()), StoredMagazineCount(0), Target(nullptr), bIsTurretRotating(false) {}

	UPROPERTY()
	FPrimaryAssetId AssetId;

	UPROPERTY()
	TArray<FRTSEquipment_SlotItem> UnEquippedSpawnedActors;

	UPROPERTY()
	TArray<FRTSEquipment_SlotItem> EquippedSpawnedActors;

	UPROPERTY()
	ERTSEquipment_Type Type;

	UPROPERTY()
	ERTSEquipment_WeaponSlot WeaponSlot;

	UPROPERTY()
	int32 MagazineAmmunitionCount;

	UPROPERTY()
	FPrimaryAssetId AmmunitionType;

	UPROPERTY()
	int32 StoredMagazineCount;

	UPROPERTY()
	AActor* Target;

	UPROPERTY()
	FRotator TurretRotation;

	UPROPERTY()
	FRotator GunRotation;

	UPROPERTY()
	FRotator PreviousRotation;

	UPROPERTY()
	bool bIsTurretRotating;
	
	bool IsValid() const { return AssetId.IsValid(); }
};

USTRUCT()
struct FRTSEquipment_ItemActorData
{
	GENERATED_BODY()

public:
	FRTSEquipment_ItemActorData(): bDisplayUnEquipped(false),  bRemainsDisplayedEquipped(false), ItemToSpawn(nullptr), AttachSocket(FName(TEXT(""))), AttachTransform(FTransform::Identity) {}
	
	UPROPERTY()
	bool bDisplayUnEquipped;
	
	UPROPERTY()
	bool bRemainsDisplayedEquipped;
	
	UPROPERTY(EditAnywhere, Category=Equipment)
	TSoftClassPtr<ARTSEquipment_Item> ItemToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};

USTRUCT()
struct FRTSEquipment_ItemData
{
	GENERATED_BODY()

	FRTSEquipment_ItemData(): UnEquippedActorData(TArray<FRTSEquipment_ItemActorData>()), EquippedActorData(TArray<FRTSEquipment_ItemActorData>()) {}

	UPROPERTY(EditAnywhere, Category=Equipment)
	TArray<FRTSEquipment_ItemActorData> UnEquippedActorData;

	UPROPERTY(EditAnywhere, Category=Equipment)
	TArray<FRTSEquipment_ItemActorData> EquippedActorData;
};

USTRUCT(BlueprintType)
struct FRTSEquipment_Statistics
{
	GENERATED_BODY()

	
};

USTRUCT(BlueprintType)
struct FRTSEquipment_Statistics_WeaponBase : public FRTSEquipment_Statistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FireRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Accuracy;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ReloadRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ProjectileSocket;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinScatter;
};

USTRUCT(BlueprintType)
struct FRTSEquipment_Statistics_Rifle : public FRTSEquipment_Statistics_WeaponBase
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FRTSEquipment_Statistics_Turret : public FRTSEquipment_Statistics_WeaponBase
{
	GENERATED_BODY()	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RotationSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PitchSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D HorizontalRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D VerticalRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName TurretBone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName WeaponBone;
};

USTRUCT(BlueprintType)
struct FRTSEquipment_Statistics_MachineGun : public FRTSEquipment_Statistics_WeaponBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraversalSpeed;	
};
