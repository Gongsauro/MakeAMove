// Fill out your copyright notice in the Description page of Project Settings.


#include "MAMAnimInstance.h"
#include "MakeAMove/MakeAMoveCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMAMAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MakeAMoveCharacter = Cast<AMakeAMoveCharacter>(TryGetPawnOwner());
}

void UMAMAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MakeAMoveCharacter == nullptr)
	{
		MakeAMoveCharacter = Cast<AMakeAMoveCharacter>(TryGetPawnOwner());
	}

	if (MakeAMoveCharacter == nullptr) return;

	FVector Velocity = MakeAMoveCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MakeAMoveCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MakeAMoveCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MakeAMoveCharacter->IsWeaponEquipped();
}
