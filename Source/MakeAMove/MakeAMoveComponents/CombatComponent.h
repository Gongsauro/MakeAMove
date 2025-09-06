// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MakeAMove/MakeAMoveTypes/LimbTypes.h"
#include "CombatComponent.generated.h"


class AWeapon;
class AMakeAMoveCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAKEAMOVE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	friend class AMakeAMoveCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AWeapon* EquippedWeapon;

	UFUNCTION(BlueprintCallable)
	void EnableWeaponHitBox();
	UFUNCTION(BlueprintCallable)
	void DisableWeaponHitBox();


	void AttackButtonPressed(bool bAttackButtonPressed);

protected:

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerAttackButtonPressed();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttackButtonPressed();

	/*
	* Montages
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages")
	class UAnimMontage* AttackMontage;

private:

	class AMakeAMoveCharacter* MakeAMoveCharacter;
	class AMakeAMovePlayerController* PlayerController;
	class AMakeAMoveHUD* MakeAMoveHUD;

	bool bCombatComponent_AttackButtonPressed;

public:	

	

		
};
