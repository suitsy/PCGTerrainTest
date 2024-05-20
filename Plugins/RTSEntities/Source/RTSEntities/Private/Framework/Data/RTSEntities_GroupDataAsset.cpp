// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSEntities_GroupDataAsset.h"
#include "Engine/AssetManager.h"
#include "Framework/Data/RTSEntities_FormationDataAsset.h"

bool URTSEntities_GroupDataAsset::HasFormationType(const EFormationType FormationType)
{
	for (int i = 0; i < Formations.Num(); ++i)
	{
		if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSEntities_FormationDataAsset* FormationData = Cast<URTSEntities_FormationDataAsset>(AssetManager->GetPrimaryAssetObject(Formations[i])))
			{
				if(FormationData->Type == FormationType)
				{
					return true;
				}
			}
		}			
	}

	return false;
}

FPrimaryAssetId URTSEntities_GroupDataAsset::GetFormationData(const EFormationType FormationType)
{
	for (int i = 0; i < Formations.Num(); ++i)
	{
		if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			if(const URTSEntities_FormationDataAsset* FormationData = Cast<URTSEntities_FormationDataAsset>(AssetManager->GetPrimaryAssetObject(Formations[i])))
			{
				if(FormationData->Type == FormationType)
				{
					return Formations[i];
				}
			}
		}			
	}

	return FPrimaryAssetId();
}

void URTSEntities_GroupDataAsset::GetGroupEntityData(TArray<FPrimaryAssetId>& GroupEntityData)
{
	for (int j = 0; j < Members.Num(); ++j)
	{
		GroupEntityData.Add(Members[j].EntityData);
	}
}

URTSEntities_FormationDataAsset* URTSEntities_GroupDataAsset::GetDefaultFormationData() const
{
	if(const UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if(DefaultFormation.IsValid())
		{
			return Cast<URTSEntities_FormationDataAsset>(AssetManager->GetPrimaryAssetObject(DefaultFormation));
		}
	}

	return nullptr;
}
