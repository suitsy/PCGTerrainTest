// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "Framework/Data/RTSEntities_DataTypes.h"
#include "Framework/Interfaces/RTSCore_InputInterface.h"
#include "Framework/Player/RTSCore_PlayerController.h"
#include "RTSEntities_PlayerController.generated.h"

class URTSEntities_FormationDataAsset;
struct FEnvQueryResult;
class IRTSCore_EquipmentManagerInterface;
class URTSEntities_GroupDataAsset;
class URTSEntities_EntityDataAsset;
class IRTSCore_AiInterface;
class URTSEntities_BoxSelectWidget;
class IRTSCore_TeamManagerInterface;
class ARTSEntities_PlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedUpdatedDelegate, FRTSEntities_PlayerSelections, Selected);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerEntitiesLoadedDelegate, const ARTSEntities_PlayerController*);


UCLASS(Abstract)
class RTSENTITIES_API ARTSEntities_PlayerController : public ARTSCore_PlayerController, public IRTSCore_InputInterface
{
	GENERATED_BODY()

public:
	ARTSEntities_PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaSeconds) override;
	
	/** Input Functions **/
	virtual void Select() override;
	virtual void SelectHold() override;
	void AssignSelectionLead();
	virtual void SelectEnd() override;
	virtual void Command() override;
	virtual void CommandHold() override;
	virtual void CommandEnd() override;
	virtual void DoubleTap() override;
	virtual void SetModifier(const EInputModifierKey NewModifier) override;
	virtual void WheelMouse(const float Input) override;
	virtual void SetPreviewCommand() override;
	virtual TArray<AActor*> GetAllSelectedEntities();
	virtual bool IsEntitySelected(AActor* Entity);

	/** Delegates **/
	FOnSelectedUpdatedDelegate OnSelectedUpdated;
	FOnPlayerEntitiesLoadedDelegate OnPlayerEntitiesLoaded;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

	/** Selection Functions  **/
	static AActor* GetSelectionLead(TArray<AActor*> Members);
	AActor* GetHitSelectable(const FVector& Location) const;
	virtual bool HasSelectedEntities() const { return Selected.Selections.Num() > 0; } 
	virtual void CreateBoxSelectionWidget();
	virtual void UpdateBoxSelected();
	virtual void CreateBoxSelectionPlanes(TArray<FPlane>& Planes) const;
	virtual void UpdatePendingBoxSelectionEntities(TArray<FPlane>& Planes, TArray<AActor*>& Client_PendingBoxSelectedEntities) const;
	virtual void CreateGroupSelections(TArray<AActor*>& Client_PendingBoxSelectedEntities, TArray<FRTSEntities_PlayerSelection>& NewPendingSelections);
	virtual void UpdatePendingBoxSelection(TArray<FRTSEntities_PlayerSelection>& NewPendingSelections);
	virtual void HandlePendingSelectionHighlight(const bool bHighlight);
	virtual void HighlightGroupSelection(const FRTSEntities_PlayerSelection& Selection, const bool bHighlight);
	virtual void CheckForDuplicateSelections();
	virtual void ClearSelections();
	TArray<AActor*> GetAllPlayerEntities() const { return PlayerEntities; }
	virtual void DrawSelectionBox();
	virtual void EndDrawSelectionBox();
	virtual void HandleSingleSelection();
	virtual void HandleLocalSelection();
	virtual void ClearSelectedEffects();
	virtual void GetLeadSelectionIndex(const FRTSEntities_PlayerSelections& CommandSelections, int32& LeadSelectionIndex) const;
	//virtual ARTSEntities_Group* AssignMultiGroupLead(const TArray<FRTSEntities_PlayerSelection>& CommandSelections);
	static FVector GetSelectionCenterPosition(const FRTSEntities_PlayerSelections& Selected);
	virtual void CycleFormation(const float Input);
	
	UFUNCTION(Server, Reliable)
	void Server_ClearSelections();
	
	UFUNCTION(Server, Reliable)
	void Server_HandleSelection(const FRTSEntities_PlayerSelections& ReceivedSelections);

	UPROPERTY(ReplicatedUsing = OnRep_Selections)
	FRTSEntities_PlayerSelections Selected;

	UPROPERTY()
	EInputModifierKey InputModifierKey;

	UPROPERTY()
	uint8 bPreviewCommand:1;

	UPROPERTY()
	FRTSEntities_Navigation Client_PreviewNavigation;

	UPROPERTY()
	FVector PreviousPreviewLocation;

	UPROPERTY()
	FRotator PreviousPreviewRotation;

private:		
	UFUNCTION()
	void OnRep_Selections(); 
	
	UFUNCTION()
	void OnRep_PlayerGroups();
	
	UPROPERTY()
	FVector SelectStartWorldLocation;
	
	UPROPERTY()
	FVector2D SelectStartScreenLocation;
	
	UPROPERTY()
	FVector2D SelectStartViewportLocation;
	
	UPROPERTY()
	AActor* Client_HitSelectable;
	
	UPROPERTY()
	FRTSEntities_PlayerSelections Client_PendingSelections;

	UPROPERTY()
	bool bIsBoxSelection;

	UPROPERTY()
	URTSEntities_BoxSelectWidget* BoxSelectionWidget;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerGroups)
	TArray<ARTSEntities_Group*> PlayerGroups;
	
	UPROPERTY(Replicated)
	TArray<AActor*> PlayerEntities;

	/** End Selection Functions  **/
	
	/** Loadout Functions **/
public:
	FPrimaryAssetId GetPlayerLoadout() const { return Loadout; }
	virtual void CreateGroups(const TArray<FPrimaryAssetId>& InGroupAssetIds);
	virtual void CreateGroupsEQS(const TArray<FPrimaryAssetId>& InGroupAssetIds);
	
protected:	
	virtual void CreateGroupEQS(const FPrimaryAssetId& GroupAssetId, const int32 GroupSpawnIndex);
	virtual void CreateGroupEntitiesEQS(ARTSEntities_Group* Group, const int32 GroupSpawnIndex);
	virtual void OnQueryEntityPositions(const FRogueQueryInstance& Result);
	
	virtual void CreateGroup(const FPrimaryAssetId& GroupAssetId, const int32 GroupSpawnIndex);
	virtual void GenerateGroupSpawnLocations();
	virtual void CreateGroupEntities(const FPrimaryAssetId& GroupAssetId, ARTSEntities_Group* NewGroup, const int32 GroupSpawnIndex);
	virtual void GenerateEntitySpawnLocations();
	virtual void GenerateEntitySpawnLocation(const URTSEntities_GroupDataAsset* GroupData, const int32 GroupSpawnIndex);
	virtual void GetGroupSpawnSpaceRequirement(const URTSEntities_GroupDataAsset* GroupData, const TArray<FRTSEntities_EntityConfigData>& EntityConfigData, FVector2D& AreaSpacing) const;
	virtual void CreateEntityComponents(const FRTSEntities_InitialisationData& EntityInitData);
	virtual void CreateEquipmentComponent(const FRTSEntities_InitialisationData& EntityInitData);	
	virtual bool HasEquipmentManagerInterface();
	virtual void AssignEntityTeam(AActor* Entity);
	virtual void AssignAIController(AActor* Entity, const URTSEntities_EntityDataAsset* EntityData) const;

	UPROPERTY()
	TScriptInterface<IRTSCore_EquipmentManagerInterface> EquipmentManagerInterface;

	UPROPERTY(Replicated, BlueprintReadWrite, EditDefaultsOnly, Category = "RTS Entities Settings", meta = (AllowedTypes = LoadoutData))
	FPrimaryAssetId Loadout;

	UPROPERTY()
	TArray<FPrimaryAssetId> GroupAssetIds;
	
	/** End Loadout Functions **/


	/** IRTSCore_TeamManagerInterface **/
public:
	virtual bool HasTeamsManagerInterface();
	virtual bool HasTeamsInitiated() const;
	
protected:	
	UFUNCTION()
	void OnTeamChanged(UObject* ObjectChangingTeam, uint8 OldTeamId, uint8 NewTeamId);

	UPROPERTY()
	TScriptInterface<IRTSCore_TeamManagerInterface> TeamsManagerInterface;

	/** End IRTSCore_TeamManagerInterface **/
	

	/** Rogue Query System **/
protected:
	void InitRogueQueryData();
	
	UPROPERTY()
	TArray<FPrimaryAssetId> AllQueryAssets;
	/** End Rogue Query System **/

#if WITH_EDITOR
	void Debug_MousePosition(const FVector& MouseLocation) const;
	void Debug_CommandDestination() const;
	void Debug_CommandPreviewRotation(const FVector& MouseLocation) const;
	void Debug_ModifierKey() const;
	
	UFUNCTION(Client, Reliable)
	void Client_Debug_Selected() const;
#endif
};
