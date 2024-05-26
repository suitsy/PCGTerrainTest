#pragma once

#include "GameplayTagContainer.h"
#include "RTSCore_DataTypes.generated.h"

class UCommonActivatableWidget;


UENUM()
enum class ERTSCore_GamePhaseState : uint8
{
	Loading,		// Generic default phase
	NewGame,		// Phase indicating a new game and prior to the phase system loading
	LoadGameData,	// Initial phase of loaded game, load menu, preload general required data
	Initiate,		// Phase for finding, joining or hosting sessions
	Setup,			// Phase for choosing settings, such as a lobby or setup ui
	PreLoad,		// Phase for preloading assets and data required for game
	PreGameplay,	// Phase for any configuration that needs to occur after PreGameplay asset loading
	Gameplay,		// Phase actual gameplay takes place
	PostGameplay,	// Phase for displaying post game statistics and menus
	Abort			// Phase to handle aborting the current game
};

UENUM(BlueprintType)
enum class ERTSCore_EntityClass
{
	NoClass,
	EntityA,
	EntityB,
	EntityC,
	EntityD,
	EntityE,
	EntityF
};

UENUM(BlueprintType)
enum class ERTSCore_EntityType
{
	NonEntity,
	Character,
	Vehicle
};

USTRUCT()
struct FPhaseRegistrationsData
{
	GENERATED_BODY()

public:
	FPhaseRegistrationsData(): GamePhase(ERTSCore_GamePhaseState::LoadGameData), DebugText(FString()), bIsCompleted(false) {}
	FPhaseRegistrationsData(const ERTSCore_GamePhaseState InGamePhase, const FString& InText):
	GamePhase(InGamePhase), DebugText(InText), bIsCompleted(false) {}

	UPROPERTY()
	ERTSCore_GamePhaseState GamePhase;

	UPROPERTY()
	FString DebugText;

	UPROPERTY()
	bool bIsCompleted;
};

UENUM(BlueprintType)
enum class EInputModifierKey
{
	NoModifier,
	Ctrl,
	Shift,
	Alt,
	Space,
	ShiftCtrl,
	ShiftAlt,
	ShiftSpace,
	CtrlAlt,
	CtrlSpace,
	AltSpace,
	ShiftCtrlAlt,
	ShiftCtrlSpace,
	ShiftAltSpace,
	CtrlAltSpace
};



UENUM()
enum ERTSCore_GameSetting_Difficulty : uint8
{
	Easy,
	Medium,
	Hard
};

#define RTS_GAMESETTING_DIFFICULTY_COUNT 3

UENUM()
enum ERTSCore_GameSetting_VictoryCondition : uint8
{
	Condition1,
	Condition2,
	Condition3,
	Condition4,
};

#define RTS_GAMESETTING_VICTORYCOND_COUNT 4



UENUM(BlueprintType)
enum class ERTSCore_StateCategory : uint8
{
	Active,
	Navigation,
	Behaviour,
	Speed,
	Stance,
	Condition,
	Posture,
	Ammunition
};

UENUM(BlueprintType)
enum class ERTSCore_ActiveState : uint8
{
	InActive,
	Active
};

UENUM(BlueprintType)
enum class ERTSCore_NavigationState : uint8
{
	NavOff,
	Idle,
	Navigating
};

UENUM(BlueprintType)
enum class ERTSCore_BehaviourState : uint8
{
	NoBehaviour,
	Stealth,
	Safe,
	Cautious,
	Combat
};

UENUM(BlueprintType)
enum class ERTSCore_SpeedState : uint8
{
	NoSpeed,
	Prone,
	Crouch,
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class ERTSCore_ConditionState : uint8
{
	NoCondition,
	Normal,
	Injured,
	Restricted,
	Incapacitated,
	Dead
};

UENUM(BlueprintType)
enum class ERTSCore_StanceState : uint8
{
	Standing,
	Crouch,
	Prone
};

UENUM(BlueprintType)
enum class ERTSCore_PostureState : uint8
{
	NoPosture,
	Withdrawing,
	Defensive,
	Aggressive,
	Offensive
};

UENUM(BlueprintType)
enum class ERTSCore_AmmoState : uint8
{
	NoAmmo,
	HasAmmo,
	Reloading
};

UENUM()
enum class ERTSCore_ThreatLevel
{
	NoThreat,
	Low,
	Medium,
	High,
	Extreme,
	PlayerSet
};

USTRUCT()
struct FModifierKeyType
{
	GENERATED_BODY()

public:
	FModifierKeyType(): bModifierCtrl(false), bModifierAlt(false), bModifierShift(false), bModifierSpace(false)	{}

	UPROPERTY()
	bool bModifierCtrl;

	UPROPERTY()
	bool bModifierAlt;

	UPROPERTY()
	bool bModifierShift;

	UPROPERTY()
	bool bModifierSpace;
};

USTRUCT(BlueprintType)
struct FRTSCore_UILayoutData
{
	GENERATED_BODY()
	
	// The layout widget to spawn
	UPROPERTY(EditAnywhere, Category=UI)
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	// The layer to insert the widget in
	UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
	FGameplayTag LayerID;
};

USTRUCT(BlueprintType)
struct FRTSCore_UIWidgetData
{
	GENERATED_BODY()

	// The widget to spawn
	UPROPERTY(EditAnywhere, Category=UI)
	TSoftClassPtr<UUserWidget> WidgetClass;

	// The slot ID where we should place this widget
	UPROPERTY(EditAnywhere, Category = UI)
	FGameplayTag SlotID;
};

USTRUCT(BlueprintType)
struct FRTSCore_UIConfiguration
{
	GENERATED_BODY()
	
	// The game phase to include the configuration in
	UPROPERTY(EditAnywhere, Category=UI)
	ERTSCore_GamePhaseState GamePhase = ERTSCore_GamePhaseState::LoadGameData;

	// The layout data for the configuration
	UPROPERTY(EditAnywhere, Category=UI)
	FRTSCore_UILayoutData LayoutData;

	// Widgets to load for the configuration
	UPROPERTY(EditAnywhere, Category=UI)
	TArray<FRTSCore_UIWidgetData> Widgets;

	UPROPERTY()
	FGuid TaskId;

	bool IsValid() const { return LayoutData.LayoutClass != nullptr; }
};

USTRUCT()
struct FRTSCore_VehicleInput
{
	GENERATED_BODY()

public:
	FRTSCore_VehicleInput(): Steering(0.f), Throttle(0.f), Braking(0.f) {}
	FRTSCore_VehicleInput(const float InSteering, const float InThrottle, const float InBraking):
	Steering(InSteering), Throttle(InThrottle), Braking(InBraking) {}

	UPROPERTY()
	float Steering;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float Braking;	
};