// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Interfaces/RTSCore_TeamInterface.h"
#include "RTSEntities_Entity.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class ARTSEntities_MarkerActor;
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
	virtual ERTSEntities_SelectionMarkerType GetMarkerType();
	virtual void CreateMarkerComponents();
	virtual float GetSelectionMarkerSize() const;
	virtual float GetMarkerHeight() const;
	virtual UMaterialInstance* GetSelectionMaterial() const;
	virtual UMaterialInstance* GetDestinationMaterial() const;
	virtual UNiagaraSystem* GetSelectionNiagaraSystem() const;
	virtual UNiagaraSystem* GetDestinationNiagaraSystem() const;
	virtual void HandleDestinationMarker(uint8 Display, const FRTSEntities_EntityPosition& EntityPosition = FRTSEntities_EntityPosition(), const ERTSEntities_CommandStatus& Status = ERTSEntities_CommandStatus::NoStatus);
	
	FOnSelectedChangeDelegate OnSelectedChange;

protected:
	UFUNCTION(Client, Unreliable)
	void Client_HandleDestinationMarker(uint8 Display, const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus& Status);
	
	virtual ARTSEntities_MarkerActor* CreateMarkerActor();
	virtual void UpdateDestinationMarker(const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus Status);
	virtual void CreateDestinationEffect(uint8 Display, const FRTSEntities_EntityPosition& EntityPosition, const ERTSEntities_CommandStatus& Status);

	UFUNCTION(Client, Reliable)
	void Client_CreateDecalComponent();
	
	UPROPERTY()
	ARTSEntities_MarkerActor* DestinationMarkerDecal;
	
	UPROPERTY()
	UNiagaraComponent* DestinationMarkerEffect;

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
	virtual float GetMinSpacing() const;
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
	
	virtual FVector GetDestination() const { return NavigationDestination; }
	virtual void SetNavigationDestination(const FVector& Location);
	virtual bool HasDestination() const { return (NavigationDestination - GetActorLocation()).Length() > 25.f; }
	virtual FVector GetPreviewLocation() const { return Client_PreviewEntityPosition.Destination; }	
	virtual void PreviewNavigation(const FRTSEntities_EntityPosition& EntityPosition, const uint8 ShowPreview);
	virtual bool HasPreviewData() const { return Client_PreviewEntityPosition.IsValid(); }

	/** Speed State **/	
	virtual void SetSpeedState(const ERTSCore_SpeedState& NewSpeedState) const;
	virtual void SetStanceState(const ERTSCore_StanceState& NewStanceState) const;
	
	FOnFormationChangeDelegate OnFormationChange;

	UPROPERTY(ReplicatedUsing = OnRep_NavigationDestination)
	FVector NavigationDestination;

	UPROPERTY()
	FRTSEntities_EntityPosition Client_PreviewEntityPosition;

private:
	UFUNCTION()
	void OnRep_NavigationDestination();
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

	/** Behaviour Manager **/
public:
	void OnBehaviourStateChange(const ERTSCore_BehaviourState& NewState);

	/** End Behaviour Manager **/
	

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
