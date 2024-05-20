// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameStateBase.h"
#include "RTSCore_GameStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSCORE_API URTSCore_GameStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	explicit URTSCore_GameStateComponent(const FObjectInitializer& ObjectInitializer);
	
	/** Checks that the owner has **/
	bool HasAuthority() const;

	/** Gets the game state that owns the component, this will always be valid during gameplay but can return null in the editor */
	template <class T>
	T* GetGameState() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AGameStateBase>::Value, "'T' template parameter to GetGameState must be derived from AGameStateBase");
		return Cast<T>(GetOwner());
	}

	template <class T>
	T* GetGameStateChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AGameStateBase>::Value, "'T' template parameter to GetGameStateChecked must be derived from AGameStateBase");
		return CastChecked<T>(GetOwner());
	}

	/** Gets the game mode that owns this component, this will always return null on the client */
	template <class T>
	T* GetGameMode() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AGameModeBase>::Value, "'T' template parameter to GetGameMode must be derived from AGameModeBase");
		return Cast<T>(GetGameStateChecked<AGameStateBase>()->AuthorityGameMode);
	}
};
