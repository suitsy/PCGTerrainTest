// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/RTSEntities_GroupDisplay.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/Entities/Group/RTSEntities_Group.h"
#include "Framework/Data/RTSEntities_GroupDataAsset.h"

void URTSEntities_GroupDisplay::NativeOnInitialized()
{
	Super::NativeOnInitialized();	

	OwningGroup = nullptr;
}

void URTSEntities_GroupDisplay::Init(ARTSEntities_Group* NewOwningGroup)
{
	if(NewOwningGroup)
	{
		OwningGroup = NewOwningGroup;
	
		if(const URTSEntities_GroupDataAsset* GroupData = OwningGroup->GetData())
		{
			Icon->SetBrushFromTexture(GroupData->Icon.LoadSynchronous());
			Callsign->SetText(GroupData->Callsign);
			Callsign->SetRenderOpacity(0.75);
		}
	}
}
