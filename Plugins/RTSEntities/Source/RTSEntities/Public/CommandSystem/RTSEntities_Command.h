// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "UObject/Object.h"
#include "RTSEntities_Command.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCommandCompleteDelegate, const FGuid CommandId, const uint8 Success);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCommandFailedDelegate, const FGuid CommandId /*Command*/);

/**
 *  Base class for all Commands
 */
UCLASS(Abstract)
class RTSENTITIES_API URTSEntities_Command : public UObject
{
	GENERATED_BODY()

public:
	URTSEntities_Command();
	virtual void Execute();
	virtual void Undo() PURE_VIRTUAL(URTSEntities_Command::Undo,;);
	virtual void Redo() PURE_VIRTUAL(URTSEntities_Command::Redo,;);
	virtual void Fail();
	virtual void Complete(const FGuid CompleteId, const ARTSEntities_Group* Group, const ERTSEntities_CommandStatus StatusUpdate, const FVector& Location);
		
	FOnCommandCompleteDelegate OnCommandComplete;
	FOnCommandFailedDelegate OnCommandFailed;

	// Set
	virtual void SetTransform(const FTransform& InTransform) { Data.TargetTransform = InTransform; }
	virtual void SetLocation(const FVector& Location) { Data.TargetTransform.SetLocation(Location); }
	virtual void SetRotation(const FRotator& Rotation) { Data.TargetTransform.SetRotation(Rotation.Quaternion()); }
	virtual void SetSourceLocation(const FVector& Location) { Data.SourceTransform.SetLocation(Location); }
	virtual void SetSourceRotation(const FRotator& Rotation) { Data.SourceTransform.SetRotation(Rotation.Quaternion()); }
	virtual void SetQueued() { Data.Status = ERTSEntities_CommandStatus::Queued; }
	virtual void SetActive() { Data.Status = ERTSEntities_CommandStatus::Active; }
	virtual void SetAborted() { Data.Status = ERTSEntities_CommandStatus::Aborted; }
	virtual void SetComplete() { Data.Status = ERTSEntities_CommandStatus::Completed; }
	virtual void SetShouldQueue(const uint8 ShouldQueue) { Data.Queue = ShouldQueue; }
	virtual void SetSelected(const FRTSEntities_PlayerSelections& InSelected);
	
	// Get
	virtual ERTSEntities_CommandType GetType() const { return Data.CommandType; }
	virtual FGuid GetId() const { return Data.Id; }
	virtual FTransform GetTargetTransform() const { return Data.TargetTransform; }
	virtual FTransform GetSourceTransform() const { return Data.SourceTransform; }
	virtual FVector GetTargetLocation() const { return Data.TargetTransform.GetLocation(); }
	virtual FRotator GetTargetRotation() const { return Data.TargetTransform.GetRotation().Rotator(); }
	virtual FVector GetSourceLocation() const { return Data.SourceTransform.GetLocation(); }
	virtual FRotator GetSourceRotation() const { return Data.SourceTransform.GetRotation().Rotator(); }
	virtual TArray<FRTSEntities_PlayerSelection>& GetSelections() { return Data.Selected.Selections; }
	virtual FRTSEntities_PlayerSelections& GetSelected() { return Data.Selected; }
	virtual AActor* GetTarget() const { return Data.TargetActor; }
	virtual bool IsActive() const { return Data.Status == ERTSEntities_CommandStatus::Active; }
	virtual int32 GetLeadSelectionIndex() { return Data.LeadSelectionIndex; }
	virtual FVector GetSelectionCenter(const FRTSEntities_PlayerSelection& Selection);
	virtual TArray<AActor*> GetGroupsSelectedEntities(const ARTSEntities_Group* Group);
	virtual ERTSEntities_CommandStatus GetStatus() const { return Data.Status; }
	virtual bool ShouldQueue() const { return Data.Queue; }
	virtual bool HasNavigation() const { return Data.Navigation.IsValid(); }
	
	
	UPROPERTY()
	FRTSEntities_CommandData Data;
	
protected:	
	virtual bool Valid() const;
	virtual void Activate();	
	virtual ERTSEntities_CommandStatus CheckStatus() const;
	virtual void AssignSelectionData();	
	virtual void GetCommandCenterLocation(FVector& CenterLocation) const;
	bool IsLocalController() const;
	bool HasAuthority() const;

	UPROPERTY()
	TMap<ARTSEntities_Group*, ERTSEntities_CommandStatus> GroupStatus;
};
