// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Tests/RogueQueryTest_Distance.h"

URogueQueryTest_Distance::URogueQueryTest_Distance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TestMode = ERogueTestDistance::Type::Distance3D;
}

void URogueQueryTest_Distance::RunTest(FRogueQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryId);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryId);
	float MaxThresholdValue = FloatValueMax.GetValue();

	switch (TestMode)
	{
		case ERogueTestDistance::Distance3D:	
			for (FRogueQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				const float Distance = CalcDistance3D(ItemLocation, QueryInstance.SourceLocation);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
			break;

		case ERogueTestDistance::Distance2D:	
			for (FRogueQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				const float Distance = CalcDistance2D(ItemLocation, QueryInstance.SourceLocation);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
			break;

		case ERogueTestDistance::DistanceZ:	
			for (FRogueQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				const float Distance = CalcDistanceZ(ItemLocation, QueryInstance.SourceLocation);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
			break;

		case ERogueTestDistance::DistanceAbsoluteZ:
			for (FRogueQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				const float Distance = CalcDistanceAbsoluteZ(ItemLocation, QueryInstance.SourceLocation);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
			break;

		default:
			checkNoEntry();
			return;
	}
}
