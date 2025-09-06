// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define CHARACTER_HITBOX ECollisionChannel::ECC_GameTraceChannel2
#define WEAPON_HITBOX ECollisionChannel::ECC_GameTraceChannel3
#define SKELETALMESH ECollisionChannel::ECC_GameTraceChannel4

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogMakeAMove, Log, All);