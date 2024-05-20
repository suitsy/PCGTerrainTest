// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Interfaces/RTSCore_GroupInterface.h"
#include "GameFramework/Actor.h"
#include "RTSEntities_Group.generated.h"


class ARTSEntities_AiControllerCommand;
class URTSEntities_GroupDisplay;
class URTSEntities_GroupStateManager;
class UWidgetComponent;
class URTSEntities_GroupDataAsset;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateUpdateDelegate, const FRTSEntities_GroupState);


UCLASS()
class RTSENTITIES_API ARTSEntities_Group : public AActor, public IRTSCore_GroupInterface
{
	GENERATED_BODY()

public:
	explicit ARTSEntities_Group(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	void Initialise(APlayerController* NewOwningPlayer, const FPrimaryAssetId& GroupDataAsset);
	void AssignEntities(const TArray<AActor*>& GroupEntities);
	
	// Public Get Functions
	URTSEntities_GroupDataAsset* GetData() const;
	void GetEntities(TArray<AActor*>& ReturnMembers) const { ReturnMembers = Entities; }
	//FVector GetCurrentFormationOffset() const;
	FVector GetCenterPosition() const;	
	virtual AActor* GetLead(const TArray<AActor*>& GroupMembers);
	FRotator GetOrientation() const { return Orientation; }
	
protected:
	virtual void BeginPlay() override;
	void SetOrientation(const FRotator& NewOrientation) { Orientation = NewOrientation; }	
	
private:	
	UPROPERTY(Replicated)
	FPrimaryAssetId DataAssetId;
	
	UPROPERTY(Replicated)
	TArray<AActor*> Entities;
	
	UPROPERTY(Replicated)
	APlayerController* OwningPlayer;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* DisplayComponent;

	UPROPERTY()
	FRotator Orientation;
	
	
	/** IRTSCore_GroupCombatInterface **/
public:
	virtual void ReportHostiles(AAIController* Reporter, TArray<AActor*> Hostiles) override;
	void SetKnownTargetsUpdating(const bool State = true) { bUpdatingKnownTargets = State; }
	bool IsKnownTargetsUpdating() const { return bUpdatingKnownTargets; }
	
	UPROPERTY(Replicated)
	TArray<FRTSEntities_TargetData> KnownTargets;

protected:
	void AssignThreatLevel(FRTSEntities_TargetData& TargetData) const;
	void UpdateKnownTargetsData();

	UPROPERTY()
	bool bUpdatingKnownTargets;

	UPROPERTY()
	TArray<FRTSEntities_TargetData> PendingHostileTargets;
	
	/** Group State Manager **/
public:
	URTSEntities_GroupStateManager* GetGroupStateManager() const { return GroupStateManager; }
	int32 GetState(const ERTSCore_StateCategory Category);
	void SetState(const ERTSCore_StateCategory Category, const int32 NewState);
	void Register_OnStateUpdate(FOnStateUpdateDelegate::FDelegate&& Delegate);	
	
protected:	
	void CreateGroupStateManager();
	void StopGroupStateManager() const;
	
	UPROPERTY()
	URTSEntities_GroupStateManager* GroupStateManager;

	UPROPERTY(ReplicatedUsing = OnRep_GroupState)
	FRTSEntities_GroupState GroupState;

	FOnStateUpdateDelegate OnStateUpdate;
	
private:
	UFUNCTION()
	void OnRep_GroupState();
	
	
	/** Group Display **/
public:
	void CreateGroupDisplay();

protected:
	UPROPERTY()
	URTSEntities_GroupDisplay* GroupDisplayWidget;
	
	UPROPERTY()
	float DisplayWidgetHeight;		
	
	UPROPERTY()
	float DisplayWidgetDistance;

	UPROPERTY()
	APlayerCameraManager* CameraManager;
	

	/** Formation Data **/
public:
	void SetInitialFormation();
	void SetAllEntitiesFormation(const FPrimaryAssetId& NewFormation);
	void SetEntitiesFormation(const TArray<AActor*>& InEntities, const FPrimaryAssetId& NewFormation) const;
	TArray<FPrimaryAssetId> GetFormationData() const;
	float GetDefaultEntitySpacing() const;
	float GetDefaultGroupSpacing() const;
protected:
	
	/** End Formation Data **/

	
	
	/** Group Command System **/
public:
	void ExecuteNavigationCommand(URTSEntities_Command* Command);
	bool HasActiveCommandFor(const TArray<AActor*>& CommandMembers) const;
	bool AssignedCommand(const FGuid Id);
	bool HasCompletedCommand(const FGuid Id);
	void OnEntityCommandComplete(const FGuid Id, const ERTSEntities_CommandStatus EntityStatus, const FVector& Location); 

	// Get
	float GetMaxFormUpDistance() const;
	float GetRequiredSpacing() const;
	
protected:
	void AssignCommandToGroupEntities(URTSEntities_Command* Command) const;
	FRTSEntities_PlayerSelection GetGroupSelection(URTSEntities_Command* Command) const;
	void RemoveCommand(const FGuid Id);
	URTSEntities_Command* GetCommand(const FGuid Id);
	void CommandComplete(const FGuid Id, const ERTSEntities_CommandStatus Status, const FVector& Location);
	ERTSEntities_CommandStatus CheckCommandStatus(const FGuid Id) const;
	bool IsActiveCommand(const FGuid Id, const TArray<AActor*>& CommandMembers) const;
	
	UPROPERTY()
	TArray<URTSEntities_Command*> Commands;
	
	UPROPERTY()
	TArray<URTSEntities_Command*> CommandHistory;
	
	/** End Group Command System **/
};
