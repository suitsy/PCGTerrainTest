// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Input/RTSPlayer_InputTriggerDoubleTap.h"
#include "EnhancedPlayerInput.h"

ETriggerState URTSPlayer_InputTriggerDoubleTap::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput,
                                                                            FInputActionValue ModifiedValue, float DeltaTime)
{
	if(PlayerInput && PlayerInput->GetOuterAPlayerController() && PlayerInput->GetOuterAPlayerController()->GetWorld())
	{
		if (IsActuated(ModifiedValue) && !IsActuated(LastValue))
		{
			const float CurrentTime = PlayerInput->GetOuterAPlayerController()->GetWorld()->GetRealTimeSeconds();
			if (CurrentTime - LastTappedTime < Delay)
			{
				LastTappedTime = 0;
				return ETriggerState::Triggered;
			}
			LastTappedTime = CurrentTime;
		}
	}
	
	return ETriggerState::None;
}
