// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MAKEAMOVE_API AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:

	virtual void EnableHitBox() override;
	virtual void DisableHitBox() override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<FName> DismemberableBones;

protected:

	virtual void BeginPlay() override;

	virtual void OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

private:

	void ResetHitActors();


	

};
