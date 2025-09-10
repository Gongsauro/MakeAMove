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
	if (!bDamageBoxActive) return;
	if (!OtherActor || OtherActor == GetOwner()) return;

	if (AMakeAMoveCharacter* HitCharacter = Cast<AMakeAMoveCharacter>(OtherActor))
	{
		if (AlreadyHitActors.Contains(HitCharacter)) return;
		AlreadyHitActors.Add(HitCharacter);

        FName HitBoneName = NAME_None;
        if (UBoxComponent* LimbHitBox = Cast<UBoxComponent>(OtherComp))
        {
            if (FName* MappedBone = HitCharacter->HitboxLimbNameMap.Find(LimbHitBox->GetFName()))
            {
                HitBoneName = *MappedBone;

                if (!DismemberableBones.Contains(HitBoneName))
                {
                    UE_LOG(LogTemp, Log, TEXT("Hit non-dismember bone: %s (damage only)"), *HitBoneName.ToString());
                    HitBoneName = NAME_None; // ignore dismemberment
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Hitbox %s not found in HitboxLimbNameMap!"), *LimbHitBox->GetName());
            }
        }

        if (AMakeAMoveCharacter* OwnerCharacter = Cast<AMakeAMoveCharacter>(GetOwner()))
        {
            if (OwnerCharacter->IsLocallyControlled())
            {
                if (HitCharacter->BladeHitBodySound)
                {
                    // Play sound locally to avoid server-authoritative flow delay
                    UGameplayStatics::PlaySoundAtLocation(this, HitCharacter->BladeHitBodySound, SweepResult.ImpactPoint);
                    // TO DO: handle playing different sounds
                }
            }
        }

        // Store the hit bone in the character (only relevant if lethal)
        HitCharacter->LastHitBone = HitBoneName;

        // Apply damage; server will handle dismemberment if Health <= 0
        UGameplayStatics::ApplyDamage(HitCharacter, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
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
