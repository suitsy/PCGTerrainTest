// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_DataTypes.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_GroupDataAsset.generated.h"

class URTSEntities_FormationDataAsset;
class UEnvQuery;
class URTSEntities_GroupDisplay;
class URTSEntities_Command;
struct FSubGroupData;


UCLASS()
class RTSENTITIES_API URTSEntities_GroupDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Type of this item, set in native parent class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText Callsign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TSoftClassPtr<URTSEntities_GroupDisplay> DisplayWidgetClass;

	/** The height of the group icon from the terrain **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	float DisplayWidgetHeight = 500.f;

	/** The distance at which the group icon begins to become transparent **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	float DisplayWidgetDistance = 3500.f;

	/** List of members of this group **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	TArray<FRTSEntities_EntityConfigData> Members;

	/** List of commands this group is capable of **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command System")
	TMap<ERTSEntities_CommandType, TSoftClassPtr<URTSEntities_Command>> Commands;

	/** The maximum time the group will remain in combat after not spotting an enemy. **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|States")
	float CombatTimeout;

	/** The maximum time the group will remain Cautious after not spotting an enemy. **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|States")
	float CautiousTimeout;

	/** The maximum time the group will remember a target for **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|States")
	float TargetRecallTime;

	/** Navigation formation threshold is the distance the final destination needs to be over in order for this group
	 *  to consider that forming up into formation should be considered. Otherwise they will move to the destination
	 *  individually under this threshold distance. This is to prevent lengthy forming up when the destination is close **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Navigation")
	float NavigationFormationThreshold;

	/** The maximum distance the group will travel individually before they form into a set formation
	 *  this is effectively the easing distance to a set formation. **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Navigation")
	float MaxFormUpDistance;

	/** The maximum distance the navigation system will set a waypoint on the path at before breaking
	 * the path into another waypoint. Prevents long straight lines from having no waypoints. **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Navigation")
	float MaxWaypointLength;

	UPROPERTY(EditAnywhere, Category = "Configuration|Navigation")
	UCurveFloat* MemberSpeedReductionCurve;

	/** This is the default formation that the group will use prior to player changing the formation
	 *  or any other outside factors changing the current formation **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Navigation|Formation", meta = (AllowedTypes = "FormationData"))
	FPrimaryAssetId DefaultFormation;

	/** A factor used to calculate the maximum (or minimum) spacing of formation members before speed adjustments are made
	 *  to maintain formation **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation|Formation", meta=(ClampMin=0, ClampMax=1))
	float SpacingTolerance = 0.5f;

	/** The default spacing between this group and other groups **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation|Formation")
	float DefaultGroupSpacing = 250.f;

	/** The default spacing between entities in this group **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Navigation|Formation")
	float DefaultEntitySpacing = 100.f;

	/** The formation data asset list determines what formations this group has available **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Navigation|Formation", meta = (AllowedTypes = "FormationData"))
	TArray<FPrimaryAssetId> Formations;

	UPROPERTY(EditAnywhere, Category = "Behaviour|Queries")
	UEnvQuery* SafeLocationsQuery;

	bool HasFormationType(const EFormationType FormationType);
	FPrimaryAssetId GetFormationData(const EFormationType FormationType);
	TArray<FPrimaryAssetId> GetGroupFormations() const { return Formations; }
	void GetGroupEntityData(TArray<FPrimaryAssetId>& GroupEntityData);
	URTSEntities_FormationDataAsset* GetDefaultFormationData() const;
};
