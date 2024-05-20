// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Tests/RogueQueryTest_Collisions.h"



URogueQueryTest_Collisions::URogueQueryTest_Collisions(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TraceCount.DefaultValue = 6;
}

void URogueQueryTest_Collisions::PostLoad()
{
	Super::PostLoad();
	TraceData.OnPostLoad();
}

void URogueQueryTest_Collisions::RunTest(FRogueQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}
	
	InvertScore.BindData(QueryOwner, QueryInstance.QueryId);
	bool bInvertScore = InvertScore.GetValue();
	QueryInstance.bInvertScores = bInvertScore;
	
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryId);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryId);
	float MaxThresholdValue = FloatValueMax.GetValue();

	MaxTraceDistance.BindData(QueryOwner, QueryInstance.QueryId);
	float TraceDistance = MaxTraceDistance.GetValue();

	TraceCount.BindData(QueryOwner, QueryInstance.QueryId);
	float NumTraces = TraceCount.GetValue();

	// Assign height offset params	
	ItemHeightOffset.BindData(QueryOwner, QueryInstance.QueryId);
	float ItemZ = ItemHeightOffset.GetValue();
	
	SourceHeightOffset.BindData(QueryOwner, QueryInstance.QueryId);
	float SourceZ = SourceHeightOffset.GetValue();	

	// Adjust source location for height offset
	FVector SourceLocation = QueryInstance.SourceLocation;
	SourceLocation.Z += SourceZ;

	// Assign trace params
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(EnvQueryTrace), TraceData.bTraceComplex);
	ECollisionChannel TraceCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);	
	FVector TraceExtent(TraceData.ExtentX, TraceData.ExtentY, TraceData.ExtentZ);
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(QueryInstance.DebugActor);
	TraceParams.AddIgnoredActors(IgnoredActors);

	float AngleBetweenTraces = 360.0f / NumTraces;	
	
	for (FRogueQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{	
		for (int32 i = 0; i < NumTraces; ++i)
		{
			// Calculate direction based on angle
			FVector Direction = FVector::ForwardVector.RotateAngleAxis(AngleBetweenTraces * i, FVector::UpVector);

			// Perform line trace
			FHitResult HitResult;		
			FRunTraceSignature TraceFunc;
			switch (TraceData.TraceShape)
			{
				case ERogueQueryTraceShape::Line:
					TraceFunc.BindUObject(const_cast<URogueQueryTest_Collisions*>(this), &URogueQueryTest_Collisions::RunLineTrace);
					break;

				case ERogueQueryTraceShape::Box:
					TraceFunc.BindUObject(const_cast<URogueQueryTest_Collisions*>(this), &URogueQueryTest_Collisions::RunBoxTrace);
					break;

				case ERogueQueryTraceShape::Sphere:
					TraceFunc.BindUObject(const_cast<URogueQueryTest_Collisions*>(this), &URogueQueryTest_Collisions::RunSphereTrace);
					break;

				case ERogueQueryTraceShape::Capsule:
					TraceFunc.BindUObject(const_cast<URogueQueryTest_Collisions*>(this), &URogueQueryTest_Collisions::RunCapsuleTrace);
					break;

				default:
					return;
			}

			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex()) + FVector(0, 0, ItemZ);			
			const float HitDistance = TraceFunc.Execute(HitResult, ItemLocation, Direction, TraceDistance, QueryInstance.DebugActor, QueryInstance.World, TraceCollisionChannel, TraceParams, TraceExtent);
			UE_LOG(LogRogueQuery, Warning, TEXT("[%s] Distance: %f"), *GetClass()->GetName(), HitDistance);
			It.SetScore(TestPurpose, FilterType, HitDistance, MinThresholdValue, MaxThresholdValue);
		}
	}
}

float URogueQueryTest_Collisions::RunLineTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const FVector EndPos = ItemPos + Direction * Distance;
	
	if(World->LineTraceSingleByChannel(HitResult,ItemPos, EndPos, Channel, TraceParams))
	{
		if(HitResult.bBlockingHit)
		{
			//DrawDebugDirectionalArrow(World, ItemPos, HitResult.Location, 200.f, FColor::Red, false, 10.f, 0, 5.f);
			return static_cast<float>(FVector::Dist2D(ItemPos, HitResult.Location));
		}
	}
	return Distance;
}

float URogueQueryTest_Collisions::RunBoxTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const FVector EndPos = ItemPos + Direction * Distance;
	if(World->SweepSingleByChannel(HitResult, ItemPos, EndPos, FQuat((EndPos - ItemPos).Rotation()), Channel, FCollisionShape::MakeBox(Extent), TraceParams))
	{
		return static_cast<float>(FVector::Dist2D(ItemPos, HitResult.Location));
	}
	return Distance;
}

float URogueQueryTest_Collisions::RunSphereTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const FVector EndPos = ItemPos + Direction * Distance;
	if(World->SweepSingleByChannel(HitResult, ItemPos, EndPos, FQuat::Identity, Channel, FCollisionShape::MakeSphere(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision)), TraceParams))
	{
		return static_cast<float>(FVector::Dist2D(ItemPos, HitResult.Location));
	}
	return Distance;
}

float URogueQueryTest_Collisions::RunCapsuleTrace(FHitResult& HitResult, const FVector& ItemPos, const FVector& Direction, const float Distance, AActor* ItemActor, UWorld* World, enum ECollisionChannel Channel, const FCollisionQueryParams& Params, const FVector& Extent) const
{
	FCollisionQueryParams TraceParams(Params);
	TraceParams.AddIgnoredActor(ItemActor);

	const FVector EndPos = ItemPos + Direction * Distance;
	if(World->SweepSingleByChannel(HitResult, ItemPos, EndPos, FQuat::Identity, Channel, FCollisionShape::MakeCapsule(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision), FloatCastChecked<float>(Extent.Z, UE::LWC::DefaultFloatPrecision)), TraceParams))
	{
		return static_cast<float>(FVector::Dist2D(ItemPos, HitResult.Location));
	}
	return Distance;
}
