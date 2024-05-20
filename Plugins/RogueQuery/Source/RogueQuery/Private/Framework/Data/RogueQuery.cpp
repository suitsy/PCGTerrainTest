// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Data/RogueQuery.h"

#include "Framework/Generators/RogueQueryGenerator_Formation.h"

URogueQuery::URogueQuery()
{
	
}

void URogueQuery::PostInitProperties()
{
	Super::PostInitProperties();

	QueryName = GetFName();
}

void URogueQuery::PostLoad()
{
	Super::PostLoad();
	
	if (QueryName == NAME_None || QueryName.IsValid() == false)
	{
		QueryName = GetFName();
	}
}

#if WITH_EDITOR
void URogueQuery::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	QueryName = GetFName();
}

void URogueQuery::PostDuplicate(bool bDuplicateForPIE)
{
	if (bDuplicateForPIE == false)
	{
		QueryName = GetFName();
	}

	Super::PostDuplicate(bDuplicateForPIE);
}
#endif // WITH_EDITOR


