// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RTSTeams_DataAsset.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

void URTSTeams_DataAsset::ApplyToMaterial(UMaterialInstanceDynamic* Material)
{
	if(!Material)
	{
		return;
	}

	for (const auto& KVP : ScalarParameters)
	{
		Material->SetScalarParameterValue(KVP.Key, KVP.Value);
	}

	for (const auto& KVP : ColorParameters)
	{
		Material->SetVectorParameterValue(KVP.Key, FVector(KVP.Value));
	}
}

void URTSTeams_DataAsset::ApplyToDecalComponent(UDecalComponent* DecalComponent)
{
	if(!DecalComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(DecalComponent->GetDecalMaterial());
	if(!DynamicMaterial)
	{
		DynamicMaterial = DecalComponent->CreateDynamicMaterialInstance();
	}

	if(DynamicMaterial)
	{
		for (const auto& KVP : ScalarParameters)
		{
			DynamicMaterial->SetScalarParameterValue(KVP.Key, KVP.Value);
		}

		for (const auto& KVP : ColorParameters)
		{
			DynamicMaterial->SetVectorParameterValue(KVP.Key, FVector(KVP.Value));
		}
	}
}

void URTSTeams_DataAsset::ApplyToMeshComponent(UMeshComponent* MeshComponent)
{
	if(!MeshComponent)
	{
		return;
	}

	for (const auto& KVP : ScalarParameters)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(KVP.Key, KVP.Value);
	}

	for (const auto& KVP : ColorParameters)
	{
		MeshComponent->SetVectorParameterValueOnMaterials(KVP.Key, FVector(KVP.Value));
	}
}

void URTSTeams_DataAsset::ApplyToNiagaraComponent(UNiagaraComponent* NiagaraComponent)
{
if (NiagaraComponent)
	{
		for (const auto& KVP : ScalarParameters)
		{
			NiagaraComponent->SetVariableFloat(KVP.Key, KVP.Value);
		}

		for (const auto& KVP : ColorParameters)
		{
			NiagaraComponent->SetVariableLinearColor(KVP.Key, KVP.Value);
		}

		for (const auto& KVP : TextureParameters)
		{
			UTexture* Texture = KVP.Value;
			NiagaraComponent->SetVariableTexture(KVP.Key, Texture);
		}
	}
}

void URTSTeams_DataAsset::ApplyToActor(AActor* TargetActor, bool bIncludeChildActors)
{
	if(!TargetActor)
	{
		return;
	}

	TargetActor->ForEachComponent(bIncludeChildActors, [this](UActorComponent* InComponent)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
		{
			ApplyToMeshComponent(MeshComponent);
		}
		else if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(InComponent))
		{
			ApplyToNiagaraComponent(NiagaraComponent);
		}
	});
}
