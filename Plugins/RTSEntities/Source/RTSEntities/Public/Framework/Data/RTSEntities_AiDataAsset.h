// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RTSEntities_AiDataAsset.generated.h"

class UBehaviorTree;
class ARTSEntities_AiControllerBase;
class UEnvQuery;
/**
 * 
 */
UCLASS()
class RTSENTITIES_API URTSEntities_AiDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Type of this item, set in native parent class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Asset Settings")
	FPrimaryAssetType DataType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(DataType, GetFName());
	}

	/** Maximum sight distance to notice a target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	float SightRadius = 1500.f;

	/** Maximum sight distance to see target that has been already seen. This the addition to the sight radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	float LoseSightRadius = 250.f;

	/** How far to the side AI can see, in degrees **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	float PeripheralVisionAngleDegrees = 85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	float SightAge = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	bool DetectEnemies = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	bool DetectFriendlies = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	bool DetectNeutrals = false;

	/** If not an InvalidRange (which is the default), we will always be able to see the target
	 *  that has already been seen if they are within this range of their last seen location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Perception)
	float AutoSuccessRangeFromLastSeenLocation = 700.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Configuration)
	TSoftObjectPtr<UBehaviorTree> BehaviourTreeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Configuration)
	TSoftClassPtr<ARTSEntities_AiControllerBase> AiControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collision)
	float CollisionDistance = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collision)
	float CollisionDetectionAngle = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collision)
	float ObstacleAvoidanceOffset = 400.f;

	UPROPERTY(EditAnywhere, Category = "Behaviour|Queries")
	UEnvQuery* QueryCollisionForward;
	
	UPROPERTY(EditAnywhere, Category = "Behaviour|Queries")
	UEnvQuery* QueryCollisionBackward;
};
