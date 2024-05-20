
#include "Framework/Data/RogueRVO_DataTypes.h"
#include "AI/RVOAvoidanceInterface.h"
#include "Framework/RogueRVO_Subsystem.h"


FRogueRVO_NavAvoidanceData::FRogueRVO_NavAvoidanceData(const URogueRVO_Subsystem* Manager, IRVOAvoidanceInterface* AvoidanceComp)
{
	Init(Manager, AvoidanceComp);
}

void FRogueRVO_NavAvoidanceData::Init(const URogueRVO_Subsystem* RVO_Subsystem, IRVOAvoidanceInterface* AvoidanceComp)
{
	Center = AvoidanceComp->GetRVOAvoidanceOrigin();
	Radius = AvoidanceComp->GetRVOAvoidanceRadius() * RVO_Subsystem->ArtificialRadiusExpansion;
	HalfHeight = AvoidanceComp->GetRVOAvoidanceHeight();
	Velocity = AvoidanceComp->GetVelocityForRVOConsideration(),
	Weight = FMath::Clamp<float>(AvoidanceComp->GetRVOAvoidanceWeight(), 0.0f, 1.0f);
	GroupMask = AvoidanceComp->GetAvoidanceGroupMask();
	GroupsToAvoid = AvoidanceComp->GetGroupsToAvoidMask();
	GroupsToIgnore = AvoidanceComp->GetGroupsToIgnoreMask();
	TestRadius2D = AvoidanceComp->GetRVOAvoidanceConsiderationRadius();	
	OverrideWeightTime = 0.0f;
	RemainingTimeToLive = RVO_Subsystem->DefaultTimeToLive;
	Velocity.Z = 0.0f;
}