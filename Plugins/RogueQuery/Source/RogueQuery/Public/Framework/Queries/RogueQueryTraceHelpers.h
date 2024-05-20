// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/RogueQuery_DataTypes.h"

class UNavigationSystemV1;
struct FRecastDebugPathfindingData;

namespace FRogueQueryHelpers
{
	enum class ERogueTraceMode : uint8
	{
		Keep,
		Discard,
	};

	struct FRogueQueryBatchTrace
	{
		UWorld* World;
		const FVector Extent;
		const FCollisionQueryParams QueryParams;
		FCollisionResponseParams ResponseParams;
		enum ECollisionChannel Channel;
		ERogueTraceMode TraceMode;
		TArray<uint8> TraceHits;

		FRogueQueryBatchTrace(UWorld* InWorld, enum ECollisionChannel InChannel, const FCollisionQueryParams& InParams,
			const FVector& InExtent, ERogueTraceMode InTraceMode)
			: World(InWorld), Extent(InExtent), QueryParams(InParams), Channel(InChannel), TraceMode(InTraceMode)
		{

		}

		FRogueQueryBatchTrace(UWorld* InWorld, const FRogueQueryTraceData& TraceData, const FCollisionQueryParams& InParams,
			const FVector& InExtent, ERogueTraceMode InTraceMode)
			: World(InWorld), Extent(InExtent), QueryParams(InParams), TraceMode(InTraceMode)
		{
			if (TraceData.TraceMode == ERogueQueryTrace::GeometryByProfile)
			{
				UCollisionProfile::GetChannelAndResponseParams(TraceData.TraceProfileName, Channel, ResponseParams);
			}
			else
			{
				Channel = UEngineTypes::ConvertToCollisionChannel(TraceData.TraceChannel);
			}
		}
		
		FORCEINLINE_DEBUGGABLE bool RunLineTrace(const FVector& StartPos, const FVector& EndPos, FVector& HitPos)
		{
			FHitResult OutHit;
			const bool bHit = World->LineTraceSingleByChannel(OutHit, StartPos, EndPos, Channel, QueryParams, ResponseParams);
			HitPos = OutHit.Location;
			return bHit;
		}

		FORCEINLINE_DEBUGGABLE bool RunSphereTrace(const FVector& StartPos, const FVector& EndPos, FVector& HitPos)
		{
			FHitResult OutHit;
			const bool bHit = World->SweepSingleByChannel(OutHit, StartPos, EndPos, FQuat::Identity, Channel, FCollisionShape::MakeSphere(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision)), QueryParams, ResponseParams);
			HitPos = OutHit.Location;
			return bHit;
		}

		FORCEINLINE_DEBUGGABLE bool RunCapsuleTrace(const FVector& StartPos, const FVector& EndPos, FVector& HitPos)
		{
			FHitResult OutHit;
			const bool bHit = World->SweepSingleByChannel(OutHit, StartPos, EndPos, FQuat::Identity, Channel, FCollisionShape::MakeCapsule(FloatCastChecked<float>(Extent.X, UE::LWC::DefaultFloatPrecision), FloatCastChecked<float>(Extent.Z, 1./16.)), QueryParams, ResponseParams);
			HitPos = OutHit.Location;
			return bHit;
		}

		FORCEINLINE_DEBUGGABLE bool RunBoxTrace(const FVector& StartPos, const FVector& EndPos, FVector& HitPos)
		{
			FHitResult OutHit;
			const bool bHit = World->SweepSingleByChannel(OutHit, StartPos, EndPos, FQuat((EndPos - StartPos).Rotation()), Channel, FCollisionShape::MakeBox(Extent), QueryParams, ResponseParams);
			HitPos = OutHit.Location;
			return bHit;
		}

		template<ERogueQueryTraceShape::Type TraceType>
		void DoSingleSourceMultiDestinations(const FVector& Source, TArray<FNavLocation>& Points)
		{
			UE_LOG(LogRogueQuery, Error, TEXT("FBatchTrace::DoSingleSourceMultiDestinations called with unhandled trace type: %d"), int32(TraceType));
		}

		/** note that this function works slightly different in terms of discarding items. 
		 *	"Accepted" items get added to the OutPoints array*/
		template<ERogueQueryTraceShape::Type TraceType>
		void DoMultiSourceMultiDestinations2D(const TArray<FRayStartEnd>& Rays, TArray<FNavLocation>& OutPoints)
		{
			UE_LOG(LogRogueQuery, Error, TEXT("FBatchTrace::DoMultiSourceMultiDestinations2D called with unhandled trace type: %d"), int32(TraceType));
		}

		template<ERogueQueryTraceShape::Type TraceType>
		void DoProject(TArray<FNavLocation>& Points, float StartOffsetZ, float EndOffsetZ, float HitOffsetZ)
		{
			UE_LOG(LogRogueQuery, Error, TEXT("FBatchTrace::DoSingleSourceMultiDestinations called with unhandled trace type: %d"), int32(TraceType));
		}	
	};

	ANavigationData* FindNavigationDataForQuery(const TObjectPtr<UObject> Owner, const UWorld* World);
	void ProjectToNavigation(const FRogueQueryInstance& QueryInstance, TArray<FNavLocation>& PointsToProject, const FRogueQueryTraceData& ProjectionData,
		const ERogueTraceMode Mode = ERogueTraceMode::Discard);
	void RunNavProjection(const ANavigationData& NavData, const UObject& Querier, const FRogueQueryTraceData& TraceData, TArray<FNavLocation>& PointsToProject, ERogueTraceMode TraceMode = ERogueTraceMode::Discard);
	void RunPhysProjection(UWorld* World, const FRogueQueryTraceData& TraceData, TArray<FNavLocation>& PointsToProject, const ERogueTraceMode TraceMode = ERogueTraceMode::Discard);
	float GetPathDistance(UObject* QueryOwner, UWorld* World, const FVector& Start, const FVector& End, TSubclassOf<UNavigationQueryFilter> FilterClass);
	ANavigationData* FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner);
	void ProjectAndFilterNavPoints(TArray<FNavLocation>& Points, const FRogueQueryInstance& QueryInstance,
	                               const FRogueQueryTraceData& ProjectionData, const float RangeMultiplierValue = 1.5f);

#if WITH_RECAST
	static bool HasPath(const FRecastDebugPathfindingData& NodePool, const NavNodeRef& NodeRef);
#endif
}
