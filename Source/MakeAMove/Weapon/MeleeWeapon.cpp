// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "MakeAMove/MakeAMoveCharacter.h"
#include "Components/BoxComponent.h"

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
	AlreadyHitActors.Empty();
	if (HasAuthority() && DamageHitbox)
	{
		DamageHitbox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AMeleeWeapon::OnHitboxBeginOverlap);
	}
}

void AMeleeWeapon::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bDamageBoxActive) return;
	if (!OtherActor || OtherActor == GetOwner()) return;

	if (AMakeAMoveCharacter* HitCharacter = Cast<AMakeAMoveCharacter>(OtherActor))
	{
		if (AlreadyHitActors.Contains(HitCharacter)) return;
		AlreadyHitActors.Add(HitCharacter);

		UGameplayStatics::ApplyDamage(HitCharacter, Damage, GetInstigatorController(), this, UDamageType::StaticClass()); // TO DO: create custom damage types to replace StaticClass

		if (UBoxComponent* LimbHitBox = Cast<UBoxComponent>(OtherComp))
		{
			if (FName* HitBone = HitCharacter->HitboxLimbNameMap.Find(LimbHitBox->GetFName()))
			{
				HitCharacter->Server_ProcessHit(*HitBone);
			}
		}
	}	
}

void AMeleeWeapon::ResetHitActors()
{
	AlreadyHitActors.Empty();
}

void AMeleeWeapon::EnableHitBox()
{
	Super::EnableHitBox();
	ResetHitActors();
}

void AMeleeWeapon::DisableHitBox()
{
	Super::DisableHitBox();
}
