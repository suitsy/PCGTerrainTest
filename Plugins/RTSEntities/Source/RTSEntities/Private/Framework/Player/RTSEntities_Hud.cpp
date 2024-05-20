// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/RTSEntities_Hud.h"
#include "Framework/UI/UIExtensionSubsystem.h"
#include "Framework/Debug/UI/RTSEntities_DebugSelected.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(DebugSelectedPoint, "UI.Point.Debug.Selected");


ARTSEntities_Hud::ARTSEntities_Hud(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ARTSEntities_Hud::ShowSelectedDebug(const TSoftClassPtr<URTSEntities_DebugSelected>& UIClass)
{
	if(GetWorld())
	{
		if(UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
		{
			ExtensionSubsystem->RegisterExtensionAsWidgetForContext(DebugSelectedPoint, GetOwningPlayerController()->GetLocalPlayer(), UIClass.LoadSynchronous(), -1);
		}
	}
}
