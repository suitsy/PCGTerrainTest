// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Actors/RogueQueryDebugActor.h"
#include "Framework/Data/RogueQuery_DataTypes.h"


ARogueQueryDebugActor::ARogueQueryDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Set up the root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	// Create and attach the static mesh component
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetCanEverAffectNavigation(false);
	StaticMeshComponent->SetCastShadow(false);
	
	// Load and assign the static mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetFinder(TEXT("/Script/Engine.StaticMesh'/RogueQuery/Framework/Actors/Materials/Sphere.Sphere'"));
	if (MeshAssetFinder.Succeeded())
	{
		MeshAsset = MeshAssetFinder.Object;
		StaticMeshComponent->SetStaticMesh(MeshAsset);
	}

	// Load and assign the material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAssetFinder(TEXT("/Script/Engine.MaterialInstanceConstant'/RogueQuery/Framework/Actors/Materials/MI_TestMaterial.MI_TestMaterial'"));
	if (MaterialAssetFinder.Succeeded())
	{
		MaterialAsset = MaterialAssetFinder.Object;
		StaticMeshComponent->SetMaterial(0, MaterialAsset);
	}
}

void ARogueQueryDebugActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UE_LOG(LogRogueQuery, Warning, TEXT("TEST"));
}

void ARogueQueryDebugActor::UpdateMaterialInstance()
{
	if (MaterialAsset != nullptr)
	{
		//UMaterialInterface* CurrentMaterial = Material.LoadSynchronous();
	
		// If we already set a MID, then we need to create based on its parent.
		if (const UMaterialInstanceDynamic* CurrentMaterialMID = Cast<UMaterialInstanceDynamic>(MaterialAsset))
		{
			MaterialAsset = CurrentMaterialMID->Parent;
		}

		// Create the MID
		DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialAsset, this);

		// Assign the dynamic material instance to the static mesh component
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}
}

void ARogueQueryDebugActor::SetColor(const FLinearColor NewColor)
{
	UpdateMaterialInstance();
	
	if(DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue("Color", NewColor);
	}
}

void ARogueQueryDebugActor::SetScale(const float Scale)
{
	SetActorScale3D(FVector(Scale));
}

void ARogueQueryDebugActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();	

	UpdateMaterialInstance();
}

void ARogueQueryDebugActor::BeginPlay()
{
	Super::BeginPlay();
}	
