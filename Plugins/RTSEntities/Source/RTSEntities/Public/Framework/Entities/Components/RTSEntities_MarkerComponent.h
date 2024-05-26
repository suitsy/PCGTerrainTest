// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "RTSEntities_MarkerComponent.generated.h"

class URTSEntities_Entity;

UENUM()
enum class ERTSEntities_MarkerType
{	
	Selection,
	Destination,
	Preview,
	NoMarker
};

template<typename T>
static FORCEINLINE TArray<T> GetEnumValues()
{
	TArray<T> EnumValues;
	for (int32 i = 0; i < static_cast<int32>(T::NoMarker); ++i)
	{
		EnumValues.Add(static_cast<T>(i));
	}
	return EnumValues;
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RTSENTITIES_API URTSEntities_MarkerComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	explicit URTSEntities_MarkerComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	static URTSEntities_MarkerComponent* FindMarkerComponent(const AActor* Entity, const ERTSEntities_MarkerType& Type);
	void SetMarkerType(const ERTSEntities_MarkerType NewType);
	ERTSEntities_MarkerType GetMarkerType() const { return Type; }
	void SetMarkerVisibility(const bool NewVisibility);
	void UpdatePreview(const uint8 PreviewState);
	void DestinationUpdate();
	
protected:
	virtual void OnRegister() override;
	void SetSelectedDelegate();
	void StartPositionTimer();
	void StopPositionTimer();
	void PositionTimer();

	UPROPERTY()
	uint8 Visibility:1;

	UPROPERTY()
	uint8 Preview:1;

	UPROPERTY()
	ERTSEntities_MarkerType Type;

	UPROPERTY()
	URTSEntities_Entity* EntityComponent;

private:
	FDelegateHandle DelegateHandleSelected;

	UPROPERTY()
	FTimerHandle TimerHandle_Position;
};
