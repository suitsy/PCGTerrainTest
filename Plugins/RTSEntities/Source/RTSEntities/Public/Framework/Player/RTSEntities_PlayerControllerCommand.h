// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_PlayerController.h"
#include "CommandSystem/RTSEntities_Command.h"
#include "RTSEntities_PlayerControllerCommand.generated.h"

class UNiagaraComponent;
class URTSEntities_NavigateTo;

UENUM()
enum class ECommandState
{
	None,
	Initiated,
	Held
};

UCLASS()
class RTSENTITIES_API ARTSEntities_PlayerControllerCommand : public ARTSEntities_PlayerController
{
	GENERATED_BODY()

public:
	explicit ARTSEntities_PlayerControllerCommand(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Command System **/
	virtual void Command() override;
	virtual void CommandHold() override;
	virtual void CommandEnd() override;
	virtual void WheelMouse(const float Input) override;
	virtual void SetPreviewCommand() override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_IssueCommand(const FRTSEntities_ClientCommandData& ClientCommandData);

	// Create command
	virtual FRTSEntities_ClientCommandData CreateCommandClientData();
	virtual void AssignCommandModifierData(FRTSEntities_ClientCommandData& ClientCommandData);
	virtual void AssignCommandLocation();
	virtual URTSEntities_Command* CreateCommand(const FRTSEntities_ClientCommandData& ClientCommandData);
	virtual void GetCommandType(ERTSEntities_CommandType& CommandType, uint8& HasNavigation) const;
	virtual void UpdateCommandData(const FRTSEntities_ClientCommandData& ClientCommandData, FRTSEntities_CommandData& CommandData,
		const FRTSEntities_PlayerSelections& InSelected, const uint8 Preview = false) const;
	virtual void ValidateSelections(FRTSEntities_PlayerSelections& SelectedToValidate, const ERTSEntities_CommandType& Type) const;
	virtual void GetCommandCenterLocation(FVector& CenterLocation) const;
	virtual void GetCommandNavigationData(FRTSEntities_CommandData& CommandData, const FRTSEntities_PlayerSelections& InSelected) const;
	virtual void CreateCommandNavigation(const FGuid Id, FRTSEntities_CommandData& CommandData);
	virtual void OnNavigationCreated(const FGuid Id, const FRTSEntities_Navigation& Navigation);
	virtual bool ShouldQueueCommand();
	
	// Manage command queue
	virtual void Enqueue(URTSEntities_Command* NewCommand);
	virtual void Dequeue();
	virtual bool CanExecute(const FGuid Id, const TArray<FRTSEntities_PlayerSelection>& QueuedSelections);
	
	// On command completed
	virtual void CommandCompleted(const FGuid CommandId, const uint8 Success);
	virtual void CommandFailed(const FGuid CommandId);
	virtual void ArchiveCommand(const FGuid Id, const uint8 Success);

	// Command preview
	virtual void ForcePreview() { PreviewTimerStart(); }
	virtual void PreviewTimerStart();
	virtual void PreviewTimerStop();
	virtual void PreviewTimer();
	virtual void ResetCommandPreview(const FRTSEntities_Navigation& NavData);
	virtual void PreviewNavigation(const FRTSEntities_Navigation& Navigation, const uint8 ShowPreview = false);
	virtual void SpawnOrUpdatePreviewMarker(const uint8 ShowPreview, const FRTSEntities_EntityPosition& EntityPosition);

	// Command updates
	virtual void IssueSpacingChange(const ERTSEntities_SpacingType& ChangeType);
	
	UFUNCTION(Server, Reliable)
	void Server_IssueFormationChange(const float IndexChange);
	
	UFUNCTION(Server, Reliable)
	void Server_IssueSpacingChange(const ERTSEntities_SpacingType& ChangeType);

	UPROPERTY()
	TMap<AActor*, UNiagaraComponent*> Client_PreviewMarkers;

	UPROPERTY()
	TArray<URTSEntities_Command*> CommandQueue;

	UPROPERTY()
	TArray<URTSEntities_Command*> CommandQueueHistory;	

	UPROPERTY()
	FVector CommandLocation;

	UPROPERTY()
	ECommandState CommandState;

private:
	UPROPERTY()
	FTimerHandle TimerHandle_Preview;

	UPROPERTY()
	uint8 GeneratingPreview:1;
	
	/** End Command System **/
};
