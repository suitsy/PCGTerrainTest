// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Framework/Data/RTSCore_DataTypes.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "RTSPlayer_Pawn.generated.h"

class URTSPlayer_InputDataAsset;
class IRTSCore_InputInterface;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;

UCLASS(Abstract)
class RTSPLAYER_API ARTSPlayer_Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARTSPlayer_Pawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Public Get Functions
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	// Public Set Functions
	virtual void SetInputDefault(const bool Enabled = true) const;
	virtual void SetPawnControlDefaults();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetPlayerInputMode();
	void AddInputMapping(const UInputMappingContext* InputMapping, const int32 MappingPriority = 0) const;
	void RemoveInputMapping(const UInputMappingContext* InputMapping) const;	

	// Input Functions
	virtual void UpdateCamera(float DeltaTime);
	virtual void Input_Move(const FInputActionValue& InputActionValue);
	virtual void Input_Look(const FInputActionValue& InputActionValue);
	virtual void Input_Zoom(const FInputActionValue& InputActionValue);
	virtual void Input_Rotate(const FInputActionValue& InputActionValue);
	virtual void Input_Select(const FInputActionValue& InputActionValue);
	virtual void Input_SelectHold(const FInputActionValue& InputActionValue);
	virtual void Input_SelectEnd(const FInputActionValue& InputActionValue);
	virtual void Input_Command(const FInputActionValue& InputActionValue);
	virtual void Input_CommandHold(const FInputActionValue& InputActionValue);
	virtual void Input_CommandEnd(const FInputActionValue& InputActionValue);
	virtual void Input_DoubleTap(const FInputActionValue& InputActionValue);

	// Modifier Input
	virtual void Input_Modifier_Alt(const FInputActionValue& InputActionValue);
	virtual void Input_Modifier_Ctrl(const FInputActionValue& InputActionValue);
	virtual void Input_Modifier_Shift(const FInputActionValue& InputActionValue);
	virtual void Input_Modifier_Space(const FInputActionValue& InputActionValue);
	virtual void UpdateModifier();
	virtual void SetModifier();

	// Util Functions
	void GetTerrainPosition(FVector& TerrainPosition) const;

	/** Input Data **/
	UPROPERTY()
	URTSPlayer_InputDataAsset* InputData;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "RTSPlayer Settings")
	UDataAsset* InputDataAsset;

private:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	FRotator TargetRotation;

	UPROPERTY()
	float TargetZoom;

	UPROPERTY()
	float Pitch;

	UPROPERTY()
	float Yaw;

	UPROPERTY()
	EInputModifierKey Modifier;

	UPROPERTY()
	FModifierKeyType ModifierKeyTypes;

	UPROPERTY()
	bool bIsInitialised;

#if WITH_EDITOR
	void DebugModifierKey() const;
#endif

	

	template<typename TStruct, typename TVariable>
	void ProcessInputActionValue(const FInputActionValue& InputActionValue, TStruct& StructToUpdate, TVariable TStruct::*MemberVariable)
	{
		if (ensure(InputActionValue.GetValueType() == EInputActionValueType::Boolean))
		{
			StructToUpdate.*MemberVariable = InputActionValue.Get<bool>();
			UpdateModifier();
		}
	}
};
