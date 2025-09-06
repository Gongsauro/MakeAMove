// Copyright Epic Games, Inc. All Rights Reserved.


#include "MakeAMovePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "MakeAMove.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "MakeAMove/HUD/MakeAMoveHUD.h"
#include "MakeAMove/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AMakeAMovePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogMakeAMove, Error, TEXT("Could not spawn mobile controls widget."));

		}
	}

	MakeAMoveHUD = Cast<AMakeAMoveHUD>(GetHUD());
}

void AMakeAMovePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AMakeAMovePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MakeAMoveHUD = MakeAMoveHUD == nullptr ? Cast<AMakeAMoveHUD>(GetHUD()) : MakeAMoveHUD;

	bool bHUDValid = MakeAMoveHUD &&
		MakeAMoveHUD->CharacterOverlay &&
		MakeAMoveHUD->CharacterOverlay->HealthBar &&
		MakeAMoveHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		MakeAMoveHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MakeAMoveHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}