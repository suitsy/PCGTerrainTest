// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "RTSCore_TeamInterface.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, uint8, OldTeamId, uint8, NewTeamId);

inline int32 GenericTeamIdToInteger(const FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : static_cast<int32>(ID);
}

inline FGenericTeamId IntegerToGenericTeamId(const int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId(static_cast<uint8>(ID));
}

UINTERFACE()
class RTSCORE_API URTSCore_TeamInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};


class RTSCORE_API IRTSCore_TeamInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	virtual bool IsInitialised() const { return true; };
	virtual void SetInitialised() {}
	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	static void ConditionalBroadcastTeamChanged(TScriptInterface<IRTSCore_TeamInterface> This, FGenericTeamId OldTeamId, FGenericTeamId NewTeamId);	
	FOnTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};
