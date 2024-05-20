// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Engine/DataAsset.h"
#include "RTSPlayer_InputDataAsset.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS(BlueprintType)
class RTSPLAYER_API URTSPlayer_InputDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Input Control Parameters **/	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float MoveSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float EdgeScrollSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float RotateSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float PitchMin;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float PitchMax;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float StartPitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float MinZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float StartZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float MaxZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float ZoomSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RTSPlayer Settings|Control Parameters")
	float Smoothing;
	
	/** Default Player Mapping Context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputMappingContext* MappingContextDefault;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 MapPriorityDefault;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Move;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Look;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Zoom;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Select;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Rotate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* Command;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	UInputAction* DoubleTap;
	

	/** Modifier Keys **/		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Modifier")
	UInputAction* Shift;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Modifier")
	UInputAction* Alt;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Modifier")
	UInputAction* Ctrl;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Modifier")
	UInputAction* Space;
};

namespace ERTSPlayer_InputActions
{	
	template<class T, class FuncType>
	void BindInput_TriggerOnly(UEnhancedInputComponent* Input, const UInputAction* Action, T* Obj, FuncType TriggerFunc)
	{
		if(TriggerFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Triggered, Obj, TriggerFunc);
		}
	}	

	template<class T, class FuncType>
	void BindInput_StartTriggerComplete(UEnhancedInputComponent* Input, const UInputAction* Action, T* Obj, FuncType StartFunc, FuncType TriggerFunc, FuncType CompleteFunc)
	{
		if(StartFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Started, Obj, StartFunc);
		}
		
		if(TriggerFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Triggered, Obj, TriggerFunc);
		}
			
		if(CompleteFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Completed, Obj, CompleteFunc);
		}
	}	

	template<class T, class FuncType>
	void BindInput_TriggerComplete(UEnhancedInputComponent* Input, const UInputAction* Action, T* Obj, FuncType TriggerFunc, FuncType CompleteFunc)
	{
		if(TriggerFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Started, Obj, TriggerFunc);
		}
			
		if(CompleteFunc != nullptr)
		{
			Input->BindAction(Action, ETriggerEvent::Completed, Obj, CompleteFunc);
		}
	}
}