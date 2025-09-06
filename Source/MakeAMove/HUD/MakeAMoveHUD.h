// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MakeAMoveHUD.generated.h"

/**
 * 
 */
UCLASS()
class MAKEAMOVE_API AMakeAMoveHUD : public AHUD
{
	GENERATED_BODY()
	

public:

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayWidget;

	class UCharacterOverlay* CharacterOverlay;

protected:

	virtual void BeginPlay() override;
	void AddCharacterOverlay();
};
