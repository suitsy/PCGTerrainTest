// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/RTSPlayer_Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Framework/Data/RTSCore_StaticGameData.h"
#include "Framework/Data/RTSPlayer_InputDataAsset.h"
#include "Framework/Interfaces/RTSCore_InputInterface.h"
#include "GameFramework/SpringArmComponent.h"
#if WITH_EDITOR
#include "Framework/Settings/RTSCore_DeveloperSettings.h"
#include "Framework/Debug/RTSPlayer_Debug.h"
#endif

ARTSPlayer_Pawn::ARTSPlayer_Pawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 3000.0f;
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	bIsInitialised = false;
}

void ARTSPlayer_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(SpringArmComponent && bIsInitialised)
	{
		UpdateCamera(DeltaTime);
	}	
}

void ARTSPlayer_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	ensureMsgf(InputDataAsset, TEXT("InputDataAsset is NULL!"));
	
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(Input) && IsValid(InputDataAsset))
	{
		InputData = Cast<URTSPlayer_InputDataAsset>(InputDataAsset);
		if(InputData != nullptr)
		{
			/** Default **/
			ERTSPlayer_InputActions::BindInput_TriggerOnly(Input, InputData->Move, this, &ARTSPlayer_Pawn::Input_Move);
			ERTSPlayer_InputActions::BindInput_TriggerOnly(Input, InputData->Look, this, &ARTSPlayer_Pawn::Input_Look);
			ERTSPlayer_InputActions::BindInput_TriggerOnly(Input, InputData->Zoom, this, &ARTSPlayer_Pawn::Input_Zoom);
			ERTSPlayer_InputActions::BindInput_TriggerOnly(Input, InputData->Rotate, this, &ARTSPlayer_Pawn::Input_Rotate);
			ERTSPlayer_InputActions::BindInput_StartTriggerComplete(Input, InputData->Select, this, &ARTSPlayer_Pawn::Input_Select, &ARTSPlayer_Pawn::Input_SelectHold, &ARTSPlayer_Pawn::Input_SelectEnd);
			ERTSPlayer_InputActions::BindInput_StartTriggerComplete(Input, InputData->Command, this, &ARTSPlayer_Pawn::Input_Command, &ARTSPlayer_Pawn::Input_CommandHold, &ARTSPlayer_Pawn::Input_CommandEnd);
			ERTSPlayer_InputActions::BindInput_TriggerOnly(Input, InputData->DoubleTap, this, &ARTSPlayer_Pawn::Input_DoubleTap);
			
			// Modifiers
			ERTSPlayer_InputActions::BindInput_TriggerComplete(Input, InputData->Shift, this, &ARTSPlayer_Pawn::Input_Modifier_Shift, &ARTSPlayer_Pawn::Input_Modifier_Shift);
			ERTSPlayer_InputActions::BindInput_TriggerComplete(Input, InputData->Alt, this, &ARTSPlayer_Pawn::Input_Modifier_Alt, &ARTSPlayer_Pawn::Input_Modifier_Alt);
			ERTSPlayer_InputActions::BindInput_TriggerComplete(Input, InputData->Ctrl, this, &ARTSPlayer_Pawn::Input_Modifier_Ctrl, &ARTSPlayer_Pawn::Input_Modifier_Ctrl);
			ERTSPlayer_InputActions::BindInput_TriggerComplete(Input, InputData->Space, this, &ARTSPlayer_Pawn::Input_Modifier_Space, &ARTSPlayer_Pawn::Input_Modifier_Space);
		}
	}

	SetPawnControlDefaults();
	SetPlayerInputMode();
	SetInputDefault();	
}

void ARTSPlayer_Pawn::BeginPlay()
{
	Super::BeginPlay();	
}

void ARTSPlayer_Pawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ARTSPlayer_Pawn::SetPawnControlDefaults()
{
	if(IsLocallyControlled() && SpringArmComponent != nullptr && InputData != nullptr)
	{
		// Set an initial rotation for the camera
		const FRotator Rotation = SpringArmComponent->GetRelativeRotation();
		TargetRotation = FRotator(Rotation.Pitch - InputData->StartPitch, Rotation.Yaw, 0.0f);				
		Pitch = TargetRotation.Pitch;
		Yaw = TargetRotation.Yaw;
				
		// Set the initial zoom
		TargetZoom = InputData->StartZoom;

		// Set start location
		TargetLocation = GetActorLocation();
	}

	ModifierKeyTypes = FModifierKeyType();

	bIsInitialised = true;
}

void ARTSPlayer_Pawn::SetPlayerInputMode()
{
	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			InputSubsystem->ClearAllMappings();
		
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void ARTSPlayer_Pawn::AddInputMapping(const UInputMappingContext* InputMapping, const int32 MappingPriority) const
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			ensure(InputMapping);
			if(!InputSubsystem->HasMappingContext(InputMapping))
			{
				InputSubsystem->AddMappingContext(InputMapping, MappingPriority);
			}
		}
	}
}

void ARTSPlayer_Pawn::RemoveInputMapping(const UInputMappingContext* InputMapping) const
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			ensure(InputMapping);
			InputSubsystem->RemoveMappingContext(InputMapping);
		}
	}
}

void ARTSPlayer_Pawn::SetInputDefault(const bool Enabled) const
{
	if(IsValid(InputDataAsset))
	{
		if(InputData != nullptr)
		{
			if(InputData->MappingContextDefault)
			{
				if(Enabled)
				{			
					AddInputMapping(InputData->MappingContextDefault, InputData->MapPriorityDefault);
				}
				else
				{
					RemoveInputMapping(InputData->MappingContextDefault);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[ARTSP_Pawn::SetInputDefault] Failed to assign mapping context, Mapping Context Null!"));
			}
		}
	}
}

void ARTSPlayer_Pawn::UpdateCamera(float DeltaTime)
{
	if(SpringArmComponent != nullptr && InputData != nullptr)
	{
		// Move the pawn in the desired direction
		const FVector InterpolatedLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, InputData->Smoothing);	
		SetActorLocation(InterpolatedLocation);

		// Zoom the camera in the desired direction
		const float InterpolatedZoom = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetZoom, DeltaTime, InputData->Smoothing);
		SpringArmComponent->TargetArmLength = InterpolatedZoom;

		// Rotate the camera in the desired direction
		const FRotator InterpolatedRotation = FMath::RInterpTo(SpringArmComponent->GetRelativeRotation(), TargetRotation, DeltaTime, InputData->Smoothing);
		SpringArmComponent->SetRelativeRotation(InterpolatedRotation);
	}
}

void ARTSPlayer_Pawn::Input_Move(const FInputActionValue& InputActionValue)
{
	if(SpringArmComponent != nullptr && InputData != nullptr)
	{
		if(ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis2D))
		{
			const FVector2D Value = InputActionValue.Get<FVector2D>();
			const float Speed = Modifier == EInputModifierKey::Shift ? InputData->MoveSpeed * 2 : InputData->MoveSpeed;
			TargetLocation += SpringArmComponent->GetTargetRotation().RotateVector(FVector(Value.X * Speed, Value.Y * Speed, 0.0f));
			GetTerrainPosition(TargetLocation);	
		}
	}
}

void ARTSPlayer_Pawn::Input_Look(const FInputActionValue& InputActionValue)
{
	if(InputData != nullptr)
	{
		if(ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
		{
			const float Speed = Modifier == EInputModifierKey::Shift ? InputData->RotateSpeed * 2 : InputData->RotateSpeed;
			const float NewPitch = InputActionValue.Get<float>() * Speed * 0.25;
			Pitch = FMath::Clamp(Pitch + NewPitch, InputData->PitchMax, InputData->PitchMin);
			const FQuat YawQuat(FVector::UpVector, FMath::DegreesToRadians(Yaw));
			const FQuat PitchQuat(-FVector::RightVector, FMath::DegreesToRadians(Pitch));
			FRotator NewRot = (YawQuat * PitchQuat).Rotator();
			NewRot.Roll = 0.f;
			TargetRotation = NewRot;
		}
	}
}

void ARTSPlayer_Pawn::Input_Zoom(const FInputActionValue& InputActionValue)
{
	if(InputData != nullptr)
	{
		if(ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
		{
			if(Modifier == EInputModifierKey::NoModifier)
			{
				const float Speed = Modifier == EInputModifierKey::Shift ? InputData->ZoomSpeed * 2 : InputData->ZoomSpeed;
				TargetZoom = FMath::Clamp(TargetZoom + (InputActionValue.Get<float>() * Speed), InputData->MinZoom, InputData->MaxZoom);
			}
		
			if(GetController())
			{
				if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
				{
					InputInterface->WheelMouse(InputActionValue.Get<float>());	
				}
			}		
		}
	}
}

void ARTSPlayer_Pawn::Input_Rotate(const FInputActionValue& InputActionValue)
{
	if(InputData != nullptr)
	{
		if(ensure(InputActionValue.GetValueType() == EInputActionValueType::Axis1D))
		{
			const float Speed = Modifier == EInputModifierKey::Shift ? InputData->RotateSpeed * 2 : InputData->RotateSpeed;
			const float NewRot = InputActionValue.Get<float>() * Speed * 0.25;
			Yaw += NewRot;
			TargetRotation = FRotator(TargetRotation.Pitch, TargetRotation.Yaw + NewRot, 0.f);
		}
	}
}

void ARTSPlayer_Pawn::Input_Select(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->Select();	
		}
	}
}

void ARTSPlayer_Pawn::Input_SelectHold(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->SelectHold();	
		}
	}
}

void ARTSPlayer_Pawn::Input_SelectEnd(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->SelectEnd();	
		}
	}
}

void ARTSPlayer_Pawn::Input_Command(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->Command();	
		}
	}
}

void ARTSPlayer_Pawn::Input_CommandHold(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->CommandHold();	
		}
	}
}

void ARTSPlayer_Pawn::Input_CommandEnd(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->CommandEnd();	
		}
	}
}

void ARTSPlayer_Pawn::Input_DoubleTap(const FInputActionValue& InputActionValue)
{
	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->DoubleTap();	
		}
	}
}

void ARTSPlayer_Pawn::Input_Modifier_Alt(const FInputActionValue& InputActionValue)
{
	ProcessInputActionValue(InputActionValue, ModifierKeyTypes, &FModifierKeyType::bModifierAlt);
}

void ARTSPlayer_Pawn::Input_Modifier_Ctrl(const FInputActionValue& InputActionValue)
{
	ProcessInputActionValue(InputActionValue, ModifierKeyTypes, &FModifierKeyType::bModifierCtrl);
}

void ARTSPlayer_Pawn::Input_Modifier_Shift(const FInputActionValue& InputActionValue)
{
	ProcessInputActionValue(InputActionValue, ModifierKeyTypes, &FModifierKeyType::bModifierShift);
}

void ARTSPlayer_Pawn::Input_Modifier_Space(const FInputActionValue& InputActionValue)
{
	ProcessInputActionValue(InputActionValue, ModifierKeyTypes, &FModifierKeyType::bModifierSpace);

	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->SetPreviewCommand();	
		}
	}
}

void ARTSPlayer_Pawn::UpdateModifier()
{
	SetModifier();

	if(GetController())
	{
		if(IRTSCore_InputInterface* InputInterface = Cast<IRTSCore_InputInterface>(GetController()))
		{
			InputInterface->SetModifier(Modifier);	
		}
	}

#if WITH_EDITOR	
	DebugModifierKey();	
#endif
}


void ARTSPlayer_Pawn::SetModifier()
{
	if(ModifierKeyTypes.bModifierShift)
	{
		Modifier = EInputModifierKey::Shift;		
		if(ModifierKeyTypes.bModifierCtrl) Modifier = EInputModifierKey::ShiftCtrl;
		if(ModifierKeyTypes.bModifierAlt) Modifier = EInputModifierKey::ShiftAlt;
		if(ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::ShiftSpace;
		if(ModifierKeyTypes.bModifierCtrl && ModifierKeyTypes.bModifierAlt) Modifier = EInputModifierKey::ShiftCtrlAlt;
		if(ModifierKeyTypes.bModifierCtrl && ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::ShiftCtrlSpace;
		if(ModifierKeyTypes.bModifierAlt && ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::ShiftAltSpace;

		return;
	}

	if(ModifierKeyTypes.bModifierCtrl)
	{
		Modifier = EInputModifierKey::Ctrl;
		
		if(ModifierKeyTypes.bModifierShift) Modifier = EInputModifierKey::ShiftCtrl;		
		if(ModifierKeyTypes.bModifierAlt) Modifier = EInputModifierKey::CtrlAlt;
		if(ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::CtrlSpace;
		if(ModifierKeyTypes.bModifierAlt && ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::CtrlAltSpace;
		
		return;
	}

	if(ModifierKeyTypes.bModifierAlt)
	{		
		Modifier = EInputModifierKey::Alt;
		
		if(ModifierKeyTypes.bModifierShift) Modifier = EInputModifierKey::ShiftAlt;
		if(ModifierKeyTypes.bModifierCtrl) Modifier = EInputModifierKey::CtrlAlt;
		if(ModifierKeyTypes.bModifierSpace) Modifier = EInputModifierKey::AltSpace;

		return;
	}

	if(ModifierKeyTypes.bModifierSpace)
	{		
		Modifier = EInputModifierKey::Space;
		
		if(ModifierKeyTypes.bModifierShift) Modifier = EInputModifierKey::ShiftSpace;
		if(ModifierKeyTypes.bModifierCtrl) Modifier = EInputModifierKey::CtrlSpace;
		if(ModifierKeyTypes.bModifierAlt) Modifier = EInputModifierKey::AltSpace;

		return;
	}

	Modifier = EInputModifierKey::NoModifier;
}

void ARTSPlayer_Pawn::GetTerrainPosition(FVector& TerrainPosition) const
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	FVector TraceOrigin = TerrainPosition;
	TraceOrigin.Z += 10000.f;
	FVector TraceEnd = TerrainPosition;
	TraceEnd.Z -= 10000.f;

	/**
	 * Ensure a Terrain trace channel is added to DefaultEngine.ini and the Terrain is set to block this channel
	 * +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Terrain")
	 */
	
	if(UWorld* WorldContext = GetWorld())
	{
		if(WorldContext->LineTraceSingleByChannel(Hit, TraceOrigin, TraceEnd, RTS_TRACE_CHANNEL_TERRAIN, CollisionParams))
		{
			TerrainPosition = Hit.ImpactPoint;
		}
	}
}

#if WITH_EDITOR

void ARTSPlayer_Pawn::DebugModifierKey() const
{
	if(const URTSCore_DeveloperSettings* CoreSettings = GetDefault<URTSCore_DeveloperSettings>())
	{
		if(CoreSettings->DebugRTSPlayer && CoreSettings->DebugPlayerPawn)
		{
			RTSPLAYER_PRINT_TICK(FColor::Cyan, FString::Printf(TEXT("Modifier: %s"), *UEnum::GetValueAsString(Modifier)));
		}
	}
}

#endif