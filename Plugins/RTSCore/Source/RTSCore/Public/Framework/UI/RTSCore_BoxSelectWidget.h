// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RTSCore_BoxSelectWidget.generated.h"

class UBorder;


UCLASS(Abstract)
class RTSCORE_API URTSCore_BoxSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void DrawSelectionBox(const bool bShouldDraw, const FVector2D Start, const FVector2D End) const;

protected:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* Box;
};
