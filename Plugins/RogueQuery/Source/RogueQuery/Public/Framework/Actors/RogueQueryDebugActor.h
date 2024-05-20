// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueQueryDebugActor.generated.h"

UCLASS()
class ROGUEQUERY_API ARogueQueryDebugActor : public AActor
{
	GENERATED_BODY()

public:
	ARogueQueryDebugActor();
	virtual void Tick(float DeltaSeconds) override;
	void UpdateMaterialInstance();
	void SetColor(const FLinearColor NewColor);
	void SetScale(const float Scale);
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Rendering)
	TSoftObjectPtr<UMaterialInstance> Material;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY(EditAnywhere)
	UStaticMesh* MeshAsset; 

	UPROPERTY(EditAnywhere)
	UMaterialInterface* MaterialAsset;
};
