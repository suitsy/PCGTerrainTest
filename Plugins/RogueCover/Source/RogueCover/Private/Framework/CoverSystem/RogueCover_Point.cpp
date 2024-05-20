// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CoverSystem/RogueCover_Point.h"

URogueCover_Point::URogueCover_Point(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
DirectionToWall(FVector::ZeroVector), Location(FVector::ZeroVector)
{
}

URogueCover_Point::URogueCover_Point(const FVector& InLocation, const FVector& InDirectionToWall):
DirectionToWall(InDirectionToWall), Location(InLocation)
{
}
