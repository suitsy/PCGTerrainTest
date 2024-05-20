// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/Generators/RogueCover_GenCoverFMemory.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "Framework/CoverSystem/RogueCover_Subsystem.h"
#include "Framework/Query/RogueCover_EnvQueryCover.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator_CoverFMemory"


URogueCover_GenCoverFMemory::URogueCover_GenCoverFMemory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ItemType = URogueCover_EnvQueryCover::StaticClass();
	GenerateAround = UEnvQueryContext_Querier::StaticClass();
	SquareExtent.DefaultValue = 750.f;
	BoxHeight.DefaultValue = 200.f;
}

void URogueCover_GenCoverFMemory::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	const UObject* BindOwner = QueryInstance.Owner.Get();

	SquareExtent.BindData(BindOwner, QueryInstance.QueryID);
	const float SquareE = SquareExtent.GetValue();

	BoxHeight.BindData(BindOwner, QueryInstance.QueryID);
	const float BoxH = BoxHeight.GetValue();


	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(GenerateAround, ContextLocations);

	if(const URogueCover_Subsystem* CoverSubsystem = GetWorld()->GetSubsystem<URogueCover_Subsystem>())
	{
		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			TArray<URogueCover_Point*> Covers = CoverSubsystem->GetCoverWithinBounds(FBoxCenterAndExtent(ContextLocations[ContextIndex], FVector(SquareE, SquareE, BoxH)));
			QueryInstance.AddItemData<URogueCover_EnvQueryCover>(Covers);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Failed to retrieve CoverGenerator."), *GetNameSafe(this));
		return;
	}
}

FText URogueCover_GenCoverFMemory::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("DescriptionGenerateAroundContext", "{0}: generate around {1}"),
		Super::GetDescriptionTitle(), UEnvQueryTypes::DescribeContext(GenerateAround));
};

FText URogueCover_GenCoverFMemory::GetDescriptionDetails() const
{
	FText Desc = FText::Format(LOCTEXT("SimpleGridDescription", "Extent: {0}, {0}, {1}"),
		FText::FromString(SquareExtent.ToString()),
		FText::FromString(BoxHeight.ToString()));
	return Desc;
}

#undef LOCTEXT_NAMESPACE