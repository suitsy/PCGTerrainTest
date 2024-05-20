// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "RTSEntities_Entity.generated.h"

class ARTSEntities_Decal;
class ARTSEntities_PlayerController;
class IRTSCore_TeamManagerInterface;
class URTSEntities_FormationDataAsset;
class ARTSEntities_AiControllerCommand;
class URTSEntities_GroupDataAsset;
class URTSEntities_EntityDataAsset;
class ARTSEntities_Group;


DECLARE_MULTICAST_DELEGATE(FOnFormationChangeDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectedChangeDelegate, const bool);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSENTITIES_API URTSEntities_Entity : public UActorComponent, public IRTSCore_TeamInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	explicit URTSEntities_Entity(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Entity")
	static URTSEntities_Entity* FindEntityComponent(const AActor* Entity) { return (Entity ? Entity->FindComponentByClass<URTSEntities_Entity>() : nullptr); }

	// Initialise entity attributes
	void Initialise(APlayerController* NewOwningPlayer, const FRTSEntities_InitialisationData& EntityInitData);
	void InitDefaultSpacing();

	// AActor Interface
	AActor* GetActor() const { return GetOwner(); }
	ARTSEntities_AiControllerCommand* GetAiController() const;
	FVector GetActorLocation() const { return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector; }
	FRotator GetActorRotation() const { return GetOwner() ? GetOwner()->GetActorRotation() : FRotator::ZeroRotator; }
	virtual bool IsOwningPlayer(AController* QueryPlayer) const { return QueryPlayer == OwningPlayer; }
	virtual AController* GetOwningPlayer() const { return OwningPlayer; }
	float GetHalfHeight() const;
	// End AActor Interface

	// Group Interface
	virtual ARTSEntities_Group* GetGroup() const { return Group; }
	virtual URTSEntities_GroupDataAsset* GetGroupData() const;
	virtual int32 GetIndex() const { return Index; }
	virtual int32 GetSubGroupId() const { return SubGroupId; }	
	virtual AActor* GetGroupLead(const TArray<AActor*>& Members);
	virtual void GetSubGroupMembers(TArray<AActor*>& ReturnMembers) const;
	virtual float GetMaxWaypointLength() const;
	// End Group Interface

	// Selection Interface
	virtual uint8 IsEntitySelected() const;
	virtual void Highlight(APlayerController* RequestingPlayer, const bool bHighlight = true);
	virtual void Select(APlayerController* RequestingPlayer, const bool bSelect = true);
	virtual FRTSEntities_PlayerSelection CreatePlayerSelection(const TArray<AActor*>& SelectionMembers);
	virtual UMaterialInstance* GetHighlightMaterial() const;
	virtual UMaterialInstance* GetSelectMaterial() const;
	virtual UMaterialInstanceDynamic* GetCommandDestinationMaterial();
	virtual void CreateDecalComponent();
	virtual float GetSelectionDecalSize() const;
	virtual void HandleDestinationDecal(uint8 Display, const uint8 Preview, const FVector& NewLocation = FVector::ZeroVector, const FRotator& NewRotation = FRotator::ZeroRotator,
		const ERTSEntities_CommandStatus& Status = ERTSEntities_CommandStatus::NoStatus);
	
	FOnSelectedChangeDelegate OnSelectedChange;

protected:
	UFUNCTION(Client, Unreliable)
	void Client_HandleDestinationDecal(uint8 Display, const uint8 Preview, const FVector& NewLocation, const FRotator& NewRotation,	const ERTSEntities_CommandStatus& Status);
	
	virtual ARTSEntities_Decal* CreateDecalActor();
	virtual void CreateOrUpdateDecalActor(const ARTSEntities_Decal* DecalActor, const FVector& NewLocation, const FRotator& NewRotation, const ERTSEntities_CommandStatus Status) const;

	UFUNCTION(Client, Reliable)
	void Client_CreateDecalComponent();
	
	UPROPERTY()
	ARTSEntities_Decal* DestinationMarker;
	
	UPROPERTY()
	ARTSEntities_Decal* DestinationMarkerPreview;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicHighlightMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDestinationMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicPreviewDestinationMaterial;
	// End Selection Interface

	// Entity Data
public:
	virtual URTSEntities_EntityDataAsset* GetEntityData() const;
	virtual ERTSCore_EntityType GetType() const;
	virtual URTSEntities_GroupDataAsset* GetEntityGroupData() const;
	virtual TSubclassOf<APawn> GetEntityClass() const;
	// End Entity Data

	// Team
	
	
	
	/** IRTSCore_TeamInterface **/
public:
	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override { return &OnTeamChangedDelegate; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	
protected:	
	virtual void SetDecalTeam();
	virtual void SetMaterialTeam(UMaterialInstanceDynamic* DynamicMaterial);
	virtual bool HasTeamsManagerInterface();	 
	
	UPROPERTY()
	FOnTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	FGenericTeamId TeamId;

	UPROPERTY()
	TScriptInterface<IRTSCore_TeamManagerInterface> TeamsManagerInterface;

private:
	UFUNCTION()
	void OnRep_TeamId(FGenericTeamId OldTeamId);  
	/** End IRTSCore_TeamInterface **/

	// Navigation
public:
	virtual bool IsNavigating() const;
	virtual float GetSpacing() const { return Spacing; }
	virtual float GetMaxSpeed() const;
	virtual float GetDefaultMaxSpeed() const;
	float GetSpeed() const { return GetActor() && GetActor()->GetVelocity().Length() > 1.f ? FMath::Abs(GetActor()->GetVelocity().Length() * 0.036) : 0.f; }
	virtual FVector GetExtent() const;
	virtual float GetStandOffRadius();
	virtual bool IsApproachingWaypoint(const float DistanceToWaypoint);
	virtual bool HasReachedDestination(const float DistanceToWaypoint);
	virtual float GetArrivalDistance() const;
	virtual float GetAcceptanceRadius() const;	
	virtual float GetAcceptanceSpeed() const;
	virtual float GetMaxTurnRate() const;
	virtual float GetMaxDestinationBrake() const;
	virtual float GetSpeedChangeFactor(const float Value) const;
	virtual void SetMaxSpeed(const float NewMaxSpeed);
	virtual void ResetMaxSpeed();
	virtual void RequestSpeedChange(const float SpeedChangeFactor);
	virtual void HandleSpeedChange();
	virtual float GetFormationThreshold() const;
	
	FOnFormationChangeDelegate OnFormationChange;
	// End Navigation
	

	// Formation
public:
	virtual void SetFormation(const FPrimaryAssetId NewFormation);
	virtual FPrimaryAssetId GetFormation() const { return Formation; }	
	virtual URTSEntities_FormationDataAsset* GetFormationData() const;
	virtual float GetFormationSpacingTolerance() const;
	virtual FVector2D GetFormationSubordinateRearAngle() const;
	virtual EFormationType GetFormationType() const;
	virtual FVector GetFormationOffset() const;
	virtual float GetEntitySpacing() const { return EntitySpacing; }
	virtual float GetGroupSpacing() const { return GroupSpacing; }
	virtual void SetEntitySpacing(const float InSpacing) { EntitySpacing = InSpacing; } 
	virtual void SetGroupSpacing(const float InSpacing) { GroupSpacing = InSpacing; }  
	
protected:	
	UPROPERTY(ReplicatedUsing = OnRep_Formation)
	FPrimaryAssetId Formation;
	
private:	
	UFUNCTION()
	void OnRep_Formation();
	
	// End Formation
	

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EntityDataAssetId();
	
	UPROPERTY(Replicated)
	APlayerController* OwningPlayer;

	UPROPERTY(ReplicatedUsing = OnRep_EntityDataAssetId)
	FPrimaryAssetId EntityDataAssetId;

	UPROPERTY(Replicated)
	int32 Index;

	UPROPERTY(Replicated)
	int32 SubGroupId;
	
	UPROPERTY(Replicated)
	ARTSEntities_Group* Group;
	
	UPROPERTY(Replicated)
	float Spacing;

	/** Modifiable Data **/	
	UPROPERTY(Replicated)
	float EntitySpacing;
	
	UPROPERTY(Replicated)
	float GroupSpacing;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Replicated)
	float MaxSpeed;

	UPROPERTY()
	float RequestedSpeedChangeFactor = 1.f;

private:	
	bool HasAuthority() const;
	bool IsLocalController() const;
};
