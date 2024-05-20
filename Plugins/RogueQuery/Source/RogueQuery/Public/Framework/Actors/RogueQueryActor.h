// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Framework/Data/RogueQuery_DataTypes.h"
#include "GameFramework/Character.h"
#include "RogueQueryActor.generated.h"

class UEQSRenderingComponent;
class URogueQuery;
class ARogueQueryDebugActor;

UENUM()
enum class ERogueQueryHighlightMode : uint8
{
	All,
	Best5Pct UMETA(DisplayName = "Best 5%"),
	Best25Pct UMETA(DisplayName = "Best 25%"),
};

UCLASS(HideCategories=(Advanced, Attachment, Mesh, Animation, Clothing, Physics, Rendering, Lighting, Activation, CharacterMovement, AgentPhysics, Avoidance, MovementComponent, Velocity, Shape, Camera, Input, Layers, SkeletalMesh, Optimization, Pawn, Replication, Actor), MinimalAPI)
class ARogueQueryActor : public ACharacter
{
	GENERATED_BODY()

public:
	ARogueQueryActor(const FObjectInitializer& ObjectInitializer);
	virtual void PostLoad() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	void OnPropertyChanged(const FName PropName);
#endif

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=RogueQuery)
	URogueQuery* Query;
	
	UPROPERTY(Category=RogueQuery, EditAnywhere)
	TSoftClassPtr<ARogueQueryDebugActor> DebugActor;

	UPROPERTY(Category=RogueQuery, EditAnywhere)
	uint32 bReRunQueryOnlyOnFinishedMove:1;

	UPROPERTY(Category=RogueQuery, EditAnywhere)
	uint32 bShouldBeVisibleInGame:1;

	UPROPERTY(Category = RogueQuery, EditAnywhere)
	uint32 bShowDebugGrid : 1;

	UPROPERTY(Category = RogueQuery, EditAnywhere)
	ERogueQueryHighlightMode HighlightMode;

	UPROPERTY(Category=RogueQuery, EditAnywhere)
	TEnumAsByte<ERogueQueryRunMode::Type> QueryingMode;

	UPROPERTY(Category = RogueQuery, EditAnywhere)
	FNavAgentProperties NavAgentProperties;

protected:
	virtual void BeginPlay() override;
	void RunQuery();
	void QueryComplete(const FRogueQueryInstance& Result);
	void ShowQueryDebug();
	void ResetDebug();

#if WITH_EDITOR
	static void OnEditorSelectionChanged(UObject* NewSelection);
#endif

	FRogueQueryResult QueryResult;

	UPROPERTY()
	TArray<AActor*> DebugActors;
};
