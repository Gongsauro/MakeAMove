// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MakeAMoveGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MAKEAMOVE_API AMakeAMoveGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	virtual void PlayerKilled(class AMakeAMoveCharacter* Victim, class AMakeAMovePlayerController* VictimController, class AMakeAMovePlayerController* AttackerController);
};
