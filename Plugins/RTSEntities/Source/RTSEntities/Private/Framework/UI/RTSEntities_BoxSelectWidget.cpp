// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/RTSEntities_BoxSelectWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

void URTSEntities_BoxSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Set box visibility on start
	Box->SetVisibility(ESlateVisibility::Hidden);
}

void URTSEntities_BoxSelectWidget::DrawSelectionBox(const bool bShouldDraw, const FVector2D Start, const FVector2D End) const
{
	if(!Box)
	{
		return;
	}

	// Set box visibility on viewport
	Box->SetVisibility(bShouldDraw ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
	if(bShouldDraw)
	{
		if(UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Box))
		{
			const FVector2D BoxPosition = FVector2D(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y));
			const FVector2D BoxSize = FVector2D(FMath::Abs(Start.X - End.X), FMath::Abs(Start.Y - End.Y));
			CanvasSlot->SetPosition(BoxPosition);
			CanvasSlot->SetSize(BoxSize);
		}
	}
}

UMaterialInstanceDynamic* URTSEntities_BoxSelectWidget::GetDynamicMaterial() const
{
	if(Box != nullptr)
	{
		if(UMaterialInstanceDynamic* MaterialInstance = Box->GetDynamicMaterial())
		{
			return MaterialInstance;
		}
	}
	
	return nullptr;
}
