// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MakeAMove/Weapon/Weapon.h"
#include "MakeAMove/MakeAMoveCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (MakeAMoveCharacter == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = MakeAMoveCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, MakeAMoveCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(MakeAMoveCharacter);
}

void UCombatComponent::AttackButtonPressed(bool bAttackButtonPressed)
{
	/*bCombatComponent_AttackButtonPressed = bAttackButtonPressed;
	

	MakeAMoveCharacter->PlayAttackMontage();*/

	if (MakeAMoveCharacter == nullptr) return;
	if (MakeAMoveCharacter->IsLocallyControlled())
	{
		ServerAttackButtonPressed();
	}
}

void UCombatComponent::ServerAttackButtonPressed_Implementation()
{
	MulticastAttackButtonPressed();
}

void UCombatComponent::MulticastAttackButtonPressed_Implementation()
{
	if (MakeAMoveCharacter)
	{
		MakeAMoveCharacter->PlayAttackMontage();
	}
}

void UCombatComponent::EnableWeaponHitBox()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->EnableHitBox();
	}
}

void UCombatComponent::DisableWeaponHitBox()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->DisableHitBox();
	}
}

