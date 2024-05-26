// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandSystem/RTSEntities_NavigateSlow.h"
#include "Framework/Entities/Components/RTSEntities_Entity.h"


URTSEntities_NavigateSlow::URTSEntities_NavigateSlow()
{
}

void URTSEntities_NavigateSlow::ExecuteNavigation()
{
	if(!HasAuthority())
	{
		return;
	}
	
	
	
	Super::ExecuteNavigation();	
}
