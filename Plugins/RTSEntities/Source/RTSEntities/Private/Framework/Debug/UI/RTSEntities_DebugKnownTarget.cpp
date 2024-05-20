// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Debug/UI/RTSEntities_DebugKnownTarget.h"
#include "CommonTextBlock.h"

void URTSEntities_DebugKnownTarget::SetData(const FRTSEntities_TargetData& TargetData) const
{
	if(TargetName && LastSeen && Location)
	{
		TargetName->SetText(FText::FromString(TargetData.TargetActor->GetName()));
		LastSeen->SetText(FText::AsNumber(TargetData.LastSeenEnemyTime));
		Location->SetText(FText::FromString(TargetData.LastSeenEnemyLocation.ToString()));
	}
}
