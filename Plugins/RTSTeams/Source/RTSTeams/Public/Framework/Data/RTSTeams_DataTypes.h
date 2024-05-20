#pragma once


#include "RTSTeams_DataTypes.generated.h"

class ARTSTeams_Info;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamAssetChangedDelegate, const ARTSTeams_Info*, DisplayAsset);

UENUM()
enum class EGameTeam : uint8
{	
	Team0,
	Team1,
	Team2,
	Team3,
	Team4,
	Team5,
	Team6,
	Team7,
	Team8,
	Team9,
	Neutral
};

USTRUCT()
struct FTeamTrackingInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ARTSTeams_Info> TeamInfo = nullptr;

	UPROPERTY()
	FOnTeamAssetChangedDelegate OnTeamAssetChanged;

public:
	void SetTeamInfo(ARTSTeams_Info* Info);
	void RemoveTeamInfo(ARTSTeams_Info* Info);
};

inline void FTeamTrackingInfo::SetTeamInfo(ARTSTeams_Info* Info)
{
	if(!Info)
	{
		return;
	}

	const ARTSTeams_Info* OldAsset = TeamInfo;	
	TeamInfo = Info;

	if (OldAsset != TeamInfo)
	{
		OnTeamAssetChanged.Broadcast(TeamInfo);
	}
}

inline void FTeamTrackingInfo::RemoveTeamInfo(ARTSTeams_Info* Info)
{
	if (TeamInfo == Info)
	{
		TeamInfo = nullptr;
	}
}
