// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Query/RogueCover_EnvQueryCover.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Framework/CoverSystem/RogueCover_Point.h"

URogueCover_EnvQueryCover::URogueCover_EnvQueryCover(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ValueSize = sizeof(FWeakObjectPtr);
}

URogueCover_Point* URogueCover_EnvQueryCover::GetValue(const uint8* RawData)
{
	const FWeakObjectPtr WeakObjPtr = GetValueFromMemory<FWeakObjectPtr>(RawData);
	return (URogueCover_Point*)(WeakObjPtr.Get());
}

void URogueCover_EnvQueryCover::SetValue(uint8* RawData, const URogueCover_Point* Value)
{
	const FWeakObjectPtr WeakObjPtr(Value);
	SetValueInMemory<FWeakObjectPtr>(RawData, WeakObjPtr);
}

FVector URogueCover_EnvQueryCover::GetItemLocation(const uint8* RawData) const
{
	const URogueCover_Point* Cover = URogueCover_EnvQueryCover::GetValue(RawData);
	return Cover ? Cover->Location : FVector::ZeroVector;
}

void URogueCover_EnvQueryCover::AddBlackboardFilters(FBlackboardKeySelector& KeySelector, UObject* FilterOwner) const
{
	Super::AddBlackboardFilters(KeySelector, FilterOwner);
	KeySelector.AddObjectFilter(FilterOwner, GetClass()->GetFName(), AActor::StaticClass());
}

bool URogueCover_EnvQueryCover::StoreInBlackboard(FBlackboardKeySelector& KeySelector, UBlackboardComponent* Blackboard,
	const uint8* RawData) const
{
	bool bStored = Super::StoreInBlackboard(KeySelector, Blackboard, RawData);
	if (!bStored && KeySelector.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		UObject* MyObject = GetValue(RawData);
		Blackboard->SetValue<UBlackboardKeyType_Object>(KeySelector.GetSelectedKeyID(), MyObject);

		bStored = true;
	}

	return bStored;
}

FString URogueCover_EnvQueryCover::GetDescription(const uint8* RawData) const
{
	const URogueCover_Point* Cover = GetValue(RawData);
	return GetNameSafe(Cover);
}
