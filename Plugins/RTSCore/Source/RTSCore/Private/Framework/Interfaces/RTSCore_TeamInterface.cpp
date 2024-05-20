// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Interfaces/RTSCore_TeamInterface.h"


void IRTSCore_TeamInterface::ConditionalBroadcastTeamChanged(TScriptInterface<IRTSCore_TeamInterface> This,
                                                             FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
{
	if (OldTeamId != NewTeamId)
	{
		const uint8 OldTeamIndex = OldTeamId.GetId(); 
		const uint8 NewTeamIndex = NewTeamId.GetId();

		UObject* ThisObj = This.GetObject();
		UE_LOG(LogTemp, Log, TEXT("[TeamChanged] %s assigned team %d"), *GetPathNameSafe(ThisObj), NewTeamIndex);

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}