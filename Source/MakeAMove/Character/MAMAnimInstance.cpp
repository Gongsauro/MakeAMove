// Fill out your copyright notice in the Description page of Project Settings.


#include "MAMAnimInstance.h"
#include "MakeAMove/MakeAMoveCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMAMAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AMakeAMoveCharacter>(TryGetPawnOwner());
}

void UMAMAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Character == nullptr)
	{
		Character = Cast<AMakeAMoveCharacter>(TryGetPawnOwner());
	}

	if (Character == nullptr) return;

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
