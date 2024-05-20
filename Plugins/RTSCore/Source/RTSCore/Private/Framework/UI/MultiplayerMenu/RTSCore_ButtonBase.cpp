// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UI/MultiplayerMenu/RTSCore_ButtonBase.h"
#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "Framework/Data/RTSCore_StaticGameData.h"

void URTSCore_ButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	UpdateButtonText(ButtonText);
}

void URTSCore_ButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	InitButton();
}

void URTSCore_ButtonBase::InitButton()
{
	OnHovered().AddUObject(this, &URTSCore_ButtonBase::OnButtonHovered);
	OnUnhovered().AddUObject(this, &URTSCore_ButtonBase::OnButtonUnHovered);
	
	ApplyMaterial();
	UpdateButtonText(ButtonText);
	SetButtonSettings();
}

void URTSCore_ButtonBase::OnButtonHovered()
{
	if(ButtonBorder != nullptr)
	{
		if(UMaterialInstanceDynamic* MaterialInstance = ButtonBorder->GetDynamicMaterial())
		{
			MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_HOVER, 1.f);
		}
	}
}

void URTSCore_ButtonBase::OnButtonUnHovered()
{
	if(ButtonBorder != nullptr)
	{
		if(UMaterialInstanceDynamic* MaterialInstance = ButtonBorder->GetDynamicMaterial())
		{
			MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_HOVER, 0.f);
		}
	}
}

void URTSCore_ButtonBase::ApplyMaterial() const
{
	if(ButtonBorder != nullptr && Material != nullptr)
	{
		ButtonBorder->SetBrushFromMaterial(Material);
	}
}

void URTSCore_ButtonBase::UpdateButtonText(const FText& InText)
{
	if(ButtonTextBlock)
	{
		ButtonTextBlock->SetText(InText);
	}
}

void URTSCore_ButtonBase::SetButtonSettings() const
{
	if(ButtonBorder != nullptr)
	{
		if(UMaterialInstanceDynamic* MaterialInstance = ButtonBorder->GetDynamicMaterial())
		{
			if(bOverride_Texture_Alpha > 0)
			{
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_ALPHA, TextureAlpha);
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_ALPHA_HOVER, TextureHoverAlpha);
			}
			
			if(bOverride_Texture_Scale > 0)
			{
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_SCALE, TextureScale);
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_HOVERSCALE, TextureHoverScale);
			}

			if(bOverride_Texture_Shift > 0)
			{
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_SHIFTX, TextureShiftX);
				MaterialInstance->SetScalarParameterValue(RTS_MAT_PARAM_NAME_TEX_SHIFTY, TextureShiftY);
			}

			if(ButtonTexture != nullptr)
			{
				MaterialInstance->SetTextureParameterValue(RTS_MAT_PARAM_NAME_TEXTURE, ButtonTexture);
			}
		}
	}
}
