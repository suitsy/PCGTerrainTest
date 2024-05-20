// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSEntities_Command.h"
#include "RTSEntities_NavigateTo.generated.h"

class ARTSEntities_Decal;
class URTSEntities_FormationDataAsset;
/**
 * 
 */
UCLASS(BlueprintType)
class RTSENTITIES_API URTSEntities_NavigateTo : public URTSEntities_Command
{
	GENERATED_BODY()

public:
	URTSEntities_NavigateTo();	
	
	virtual void Execute() override;
	virtual void Undo() override;
	virtual void Redo() override;
	virtual void Complete(const FGuid CompleteId, const ARTSEntities_Group* Group, const ERTSEntities_CommandStatus StatusUpdate, const FVector& Location) override;  
	//virtual void CreateNavigation(const uint8 PreviewOnly = false);
	//virtual void OnNavigationCreated(const uint8 PreviewOnly);
	virtual void ExecuteNavigation();
	virtual FRTSEntities_Navigation GetNavigation() const { return Data.Navigation; }
	virtual void SetNavigation(const FRTSEntities_Navigation& NewNavigation) { Data.Navigation = NewNavigation; }

protected:
	virtual bool Valid() const override;
	//virtual URTSEntities_FormationDataAsset* GetFormationData();
	//virtual FRTSEntities_CommandData GetCommandData(const FVector& Offset);
	//virtual void GenerateFormationReferencePaths();
	//virtual UNavigationPath* GeneratePath(const FVector& Start, const FVector& End, AActor* Context);
	virtual FVector GetAvgDestinationLocation();

private:
#if WITH_EDITOR
	void Debug_Formation() const;
#endif	
};
