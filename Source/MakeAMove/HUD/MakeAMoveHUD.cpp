// Fill out your copyright notice in the Description page of Project Settings.


#include "MakeAMoveHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"

void AMakeAMoveHUD::DrawHUD()
{
	Super::DrawHUD();


}

void AMakeAMoveHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void AMakeAMoveHUD::AddCharacterOverlay()
{
		APlayerController* PlayerController = GetOwningPlayerController();
		if (PlayerController && CharacterOverlayWidget)
		{
			CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayWidget);
			CharacterOverlay->AddToViewport();
		}
}
