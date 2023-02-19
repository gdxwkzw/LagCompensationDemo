// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoHUD.h"

void ADemoHUD::DrawHUD()
{
	Super::DrawHUD();

	if(Texture == nullptr) return;
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	if(GEngine && GEngine->GameViewport)
	{
		FVector2D GameViewportSize;
		GEngine->GameViewport->GetViewportSize(GameViewportSize);
		const FVector2D ViewportCenter(GameViewportSize.X / 2, GameViewportSize.Y / 2);
		const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f)
		);
		
		DrawTexture(
			Texture,
			TextureDrawPoint.X,
			TextureDrawPoint.Y,
			TextureWidth,
			TextureHeight,
			0.f,
			0.f,
			1.f,
			1.f,
			FColor::Black
		);
	}
}
