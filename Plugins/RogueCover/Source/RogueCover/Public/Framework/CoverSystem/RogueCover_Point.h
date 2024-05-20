// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RogueCover_Point.generated.h"

/**
 *  Instance of a cover position
 */
UCLASS(BlueprintType, Blueprintable)
class ROGUECOVER_API URogueCover_Point : public UObject
{
	GENERATED_BODY()	
	
public:
	explicit URogueCover_Point(const FObjectInitializer& ObjectInitializer);
	URogueCover_Point(const FVector& InLocation, const FVector& InDirectionToWall);	

	FVector GetDirectionToWall() const { return DirectionToWall; }
	void SetDirectionToWall(const FVector& Value) { 
		DirectionToWall = Value;
		// Set rotator from direction 
		RotatorXToWall = FRotationMatrix::MakeFromX(DirectionToWall).Rotator();
	}

	bool IsOccupied() const { return OccupiedBy != nullptr; }
	void Occupy(AController* Occupier) { OccupiedBy = Occupier; }
	void Vacate() { OccupiedBy = nullptr; Score = 0.f; }
	float GetScore() const { return Score; }
	void SetScore(const float NewScore) { Score = NewScore; }

	/** Direction to wall (Perpendicular to cover) */
	UPROPERTY()
	FVector DirectionToWall;

	/** Rotator from X of direction to wall */
	UPROPERTY()
	FRotator RotatorXToWall;

	/** Is it a Left cover (can lean on left) */
	UPROPERTY()
	bool bLeftCoverStanding = false;

	/** Is it a Right cover (can lean on Right) */
	UPROPERTY()
	bool bRightCoverStanding = false;

	/** Is it a Left cover (can lean on left) */
	UPROPERTY()
	bool bLeftCoverCrouched = false;

	/** Is it a Right cover (can lean on Right) */
	UPROPERTY()
	bool bRightCoverCrouched = false;

	/** Is it a Right cover (can lean on Right) */
	UPROPERTY()
	bool bFrontCoverCrouched = false;

	/** Is it a cover requiring crouch */
	UPROPERTY()
	bool bCrouchedCover = false;

	/** location of the cover */
	UPROPERTY()
	FVector Location;

protected:
	/** Controller to which this point has been given */
	UPROPERTY()
	AController* OccupiedBy = nullptr; 

	/** current score assigned to cover */
	UPROPERTY()
	float Score = 0.f;
};
