// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Actors/RogueQueryActor.h"
#include "Framework/Actors/RogueQueryDebugActor.h"
#include "Selection.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/System/RogueQuerySubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/TransactionObjectEvent.h"


ARogueQueryActor::ARogueQueryActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
bReRunQueryOnlyOnFinishedMove(true), QueryingMode(ERogueQueryRunMode::AllMatching)
{
	// Default to no tick function, but if we set 'never ticks' to false (so there is a tick function) it is enabled by default
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	NavAgentProperties = FNavAgentProperties::DefaultProperties;
	
	static FName CollisionProfileName(TEXT("NoCollision"));
	GetCapsuleComponent()->SetCollisionProfileName(CollisionProfileName);
	
#if WITH_EDITORONLY_DATA	
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		UArrowComponent* ArrowComp = FindComponentByClass<UArrowComponent>();
		if (ArrowComp != nullptr)
		{
			ArrowComp->SetRelativeScale3D(FVector(2, 2, 2));
			ArrowComp->bIsScreenSizeScaled = true;
		}

		UBillboardComponent* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
		if (!IsRunningCommandlet() && (SpriteComponent != nullptr))
		{
			struct FConstructorStatics
			{
				ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
				FName ID_Misc;
				FText NAME_Misc;
				FConstructorStatics()
					: TextureObject(TEXT("/Engine/EditorResources/S_Pawn"))
					, ID_Misc(TEXT("Misc"))
					, NAME_Misc(NSLOCTEXT("SpriteCategory", "Misc", "Misc"))
				{
				}
			};
			static FConstructorStatics ConstructorStatics;

			SpriteComponent->Sprite = ConstructorStatics.TextureObject.Get();
			SpriteComponent->SetRelativeScale3D(FVector(1, 1, 1));
			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Misc;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Misc;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->bIsScreenSizeScaled = true;
		}
	}
#endif


#if WITH_EDITOR
	if (HasAnyFlags(RF_ClassDefaultObject) && GetClass() == StaticClass())
	{
		//USelection::SelectObjectEvent.AddStatic(&ARogueQueryActor::OnEditorSelectionChanged);
		//USelection::SelectionChangedEvent.AddStatic(&ARogueQueryActor::OnEditorSelectionChanged);
	}
#endif

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DefaultLandMovementMode = MOVE_None;
	}
}
void ARogueQueryActor::PostLoad()
{
	Super::PostLoad();

	if (UBillboardComponent* SpriteComponent = FindComponentByClass<UBillboardComponent>())
	{
		SpriteComponent->bHiddenInGame = !bShouldBeVisibleInGame;
	}

	if (PrimaryActorTick.bCanEverTick == false)
	{
		// Also disable components that may tick
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->PrimaryComponentTick.bCanEverTick = false;
		}
		if (GetMesh())
		{
			GetMesh()->PrimaryComponentTick.bCanEverTick = false;
		}
	}
}

#if WITH_EDITOR

void ARogueQueryActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		OnPropertyChanged(PropertyChangedEvent.MemberProperty->GetFName());
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ARogueQueryActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished || !bReRunQueryOnlyOnFinishedMove)
	{
		RunQuery();
	}
}

void ARogueQueryActor::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);

	if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
	{
		if (TransactionEvent.GetChangedProperties().Num() > 0)
		{
			// targeted update
			for (const FName PropertyName : TransactionEvent.GetChangedProperties())
			{
				OnPropertyChanged(PropertyName);
			}
		}
		else
		{
			// fallback - make sure the results are up to date
			RunQuery();
		}
	}
}

void ARogueQueryActor::OnPropertyChanged(const FName PropName)
{
	static const FName NAME_Query = GET_MEMBER_NAME_CHECKED(ARogueQueryActor, Query);
	static const FName NAME_ShouldBeVisibleInGame = GET_MEMBER_NAME_CHECKED(ARogueQueryActor, bShouldBeVisibleInGame);
	static const FName NAME_QueryingMode = GET_MEMBER_NAME_CHECKED(ARogueQueryActor, QueryingMode);
	
	if (PropName == NAME_Query)
	{
		RunQuery();
	}
	else if (PropName == NAME_ShouldBeVisibleInGame)
	{		
		if (UBillboardComponent* SpriteComponent = FindComponentByClass<UBillboardComponent>())
		{
			SpriteComponent->bHiddenInGame = !bShouldBeVisibleInGame;
		}
	}
	else if (PropName == NAME_QueryingMode)
	{
		RunQuery();
	}
	
}
#endif

void ARogueQueryActor::BeginPlay()
{
	Super::BeginPlay();

	
}

void ARogueQueryActor::RunQuery()
{
	UE_LOG(LogRogueQuery, Log, TEXT("[%s] Query Update"), *GetClass()->GetName());	
	
	if(Query != nullptr)
	{
		if(URogueQuerySubsystem* RogueQuerySubsystem = GetWorld()->GetSubsystem<URogueQuerySubsystem>())
		{
			RogueQuerySubsystem->Reset();
			ResetDebug();
			
			const FRogueQueryResultFilter ResultFilter = FRogueQueryResultFilter(false, 15, 0.25f, 1.f);
			const FRogueQueryRequest QueryRequest = FRogueQueryRequest(Query, GetActorLocation(), FRogueQueryCustomData(), this, ResultFilter);
			RogueQuerySubsystem->Query(QueryRequest, FRogueQueryFinishedSignature::CreateUObject(this, &ThisClass::QueryComplete));
		}
	}
}

void ARogueQueryActor::QueryComplete(const FRogueQueryInstance& Result)
{
	QueryResult = static_cast<FRogueQueryResult>(Result);
	ShowQueryDebug();
}

void ARogueQueryActor::ShowQueryDebug()
{
	if(bShowDebugGrid && QueryResult.IsValid())
	{	
		if(QueryResult.IsSuccessful() && QueryResult.ShowDebugGrid() && !QueryResult.IsDebugGridShown())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.bTemporaryEditorActor = true;
			
			for (int j = 0; j < QueryResult.Items.Num(); ++j)
			{
				if(AActor* NewActor = GetWorld()->SpawnActor<ARogueQueryDebugActor>(ARogueQueryDebugActor::StaticClass(), QueryResult.Items[j].Location, FRotator::ZeroRotator, SpawnParams))
				{
					DebugActors.Add(NewActor);

					if(ARogueQueryDebugActor* QueryActor = Cast<ARogueQueryDebugActor>(NewActor))
					{
						
						UE_LOG(LogRogueQuery, Log, TEXT("[%s] Point [%d] Score: %f"), *GetClass()->GetName(), j, QueryResult.Items[j].Score);
						QueryActor->SetColor(FMath::Lerp(FLinearColor::Red, FLinearColor::Green, QueryResult.Items[j].Score));
					}
				}
			}

			QueryResult.SetDebugGridShown();
		}
	}
}

void ARogueQueryActor::ResetDebug()
{
	for (int i = 0; i < DebugActors.Num(); ++i)
	{
		DebugActors[i]->Destroy();
	}

	DebugActors.Empty();
}

#if WITH_EDITOR
void ARogueQueryActor::OnEditorSelectionChanged(UObject* NewSelection)
{
	TArray<ARogueQueryActor*> SelectedPawns;
	ARogueQueryActor* SelectedPawn = Cast<ARogueQueryActor>(NewSelection);
	if (SelectedPawn)
	{
		SelectedPawns.Add(Cast<ARogueQueryActor>(NewSelection));
	}
	else 
	{
		USelection* Selection = Cast<USelection>(NewSelection);
		if (Selection != NULL)
		{
			Selection->GetSelectedObjects<ARogueQueryActor>(SelectedPawns);
		}
	}

	for (ARogueQueryActor* RogueQuery : SelectedPawns)
	{
		if (RogueQuery->Query != nullptr && RogueQuery->QueryResult.IsValid() == false)
		{
			RogueQuery->RunQuery();
		}
	}
}

#endif
