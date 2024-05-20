// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/EQS/Data/RTSAi_QueryLocation.h"

URTSAi_QueryLocation::URTSAi_QueryLocation() : Location(FVector::ZeroVector)
{
}

URTSAi_QueryLocation::URTSAi_QueryLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), Location(FVector::ZeroVector)
{
}

URTSAi_QueryLocation::URTSAi_QueryLocation(const FVector& InLocation) : Location(InLocation)
{
}
