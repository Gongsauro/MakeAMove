// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MakeAMoveTypes/LimbTypes.h"
#include "MakeAMoveCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UBoxComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);


UCLASS(abstract)
class AMakeAMoveCharacter : public ACharacter
{
	GENERATED_BODY()


public:

	AMakeAMoveCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayAttackMontage();

	/*
	* Input
	*/
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();
	/* Input */


	/*
	* Montages
	*/
	UPROPERTY(EditAnywhere, Category = "Montages")
	class UAnimMontage* AttackMontage;

	/*
	* Components
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCombatComponent* CombatComponent;
	/* Components */

	/*
	* HitBoxes
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftShoulderBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightShoulderBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftElbowBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightElbowBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftHandBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightHandBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftHipBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightHipBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftKneeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightKneeBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* LeftAnkleBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBoxes")
	UBoxComponent* RightAnkleBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitBoxes")
	TMap<UBoxComponent*, FName> HitboxLimbMap;

	UPROPERTY()
	TMap<FName, FName> HitboxLimbNameMap;
	/* HitBoxes */


	/*
	* Dismemberment
	*/
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Dismember(FName BoneName);

	UFUNCTION(Server, Reliable)
	void Server_ProcessHit(FName HitBoneName);
	
protected:

	virtual void BeginPlay() override;

	void UpdateHUDHealth();

	/*
	* Input
	*/
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;


	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip();
	void Attack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();
	/* Input */

	void ConstructLimbHitboxes();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:

	class AMakeAMovePlayerController* MAMPlayerController;


	/*
	* Camera
	*/
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;
	/* Camera */


	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	/*
	* Weapon
	*/
	// Replication only works when a value is changed
	// When the value of the Overlapping Weapon changes on the server, it will be replicated and set to all clients
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; 

	//OnRep_ functions get called automatically as soon as the variable gets replicated, and DO NOT get called on the server
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/* Weapon */

	/*
	* Input
	*/
	UFUNCTION(Server, Reliable)
	void ServerEquip();

	/*
	* Player Stats
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();



public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void SetOverlappingWeapon(AWeapon* Weapon);
};

