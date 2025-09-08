// Copyright Epic Games, Inc. All Rights Reserved.

#include "MakeAMoveCharacter.h"
#include "MakeAMove.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MakeAMove.h"
#include "Components/WidgetComponent.h"
#include <Net/UnrealNetwork.h>
#include "MakeAMove/Weapon/Weapon.h"
#include "MakeAMove/MakeAMoveComponents/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MakeAMove/MakeAMovePlayerController.h"
#include "Components/SkinnedMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"


AMakeAMoveCharacter::AMakeAMoveCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);

	GetMesh()->SetCollisionObjectType(SKELETALMESH);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	ConstructLimbHitboxes();
}

void AMakeAMoveCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddUniqueDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void AMakeAMoveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMakeAMoveCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMakeAMoveCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMakeAMoveCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMakeAMoveCharacter::Look);

		// Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::Equip);

		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::Attack);
	}
	else
	{
		UE_LOG(LogMakeAMove, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMakeAMoveCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->MakeAMoveCharacter = this;
	}
}

void AMakeAMoveCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMakeAMoveCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMakeAMoveCharacter, Health);
}

void AMakeAMoveCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	DoMove(MovementVector.X, MovementVector.Y);
}

void AMakeAMoveCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMakeAMoveCharacter::Equip()
{
	if (CombatComponent)
	{
		// if we're on the server, we call the CombatComponent's EquipWeapon function
		if (HasAuthority()) CombatComponent->EquipWeapon(OverlappingWeapon);

		// if we're on a client, we send the RPC to the server
		else ServerEquip();
	}
}

void AMakeAMoveCharacter::ServerEquip_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AMakeAMoveCharacter::Attack()
{
	if (CombatComponent)
	{
		CombatComponent->AttackButtonPressed(true);
	}
}

void AMakeAMoveCharacter::PlayAttackMontage()
{
	if (AttackMontage == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage && !AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		AnimInstance->Montage_Play(AttackMontage);
	}
}

void AMakeAMoveCharacter::Multicast_PlayAttackMontage_Implementation()
{
	if (!IsLocallyControlled())
	{
		if (AttackMontage)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackMontage))
			{
				AnimInstance->Montage_Play(AttackMontage);
			}
		}
	}
}

void AMakeAMoveCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMakeAMoveCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMakeAMoveCharacter::DoJumpStart()
{
	Jump();
}

void AMakeAMoveCharacter::DoJumpEnd()
{
	StopJumping();
}

void AMakeAMoveCharacter::OnRep_Health()
{
	// when the value of Health changes (with ReceiveDamage()), this function will be automatically called, and call the functions below on clients
	UpdateHUDHealth();
	// TO DO: PlayHitReactMontage();
}

void AMakeAMoveCharacter::UpdateHUDHealth()
{
	MAMPlayerController = MAMPlayerController == nullptr ? Cast<AMakeAMovePlayerController>(Controller) : MAMPlayerController;
	if (MAMPlayerController)
	{
		MAMPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMakeAMoveCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	// handles showing the widget if we're the server
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AMakeAMoveCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMakeAMoveCharacter::ConstructLimbHitboxes()
{
	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	HeadBox->SetupAttachment(GetMesh(), TEXT("head"));
	HeadBox->SetCollisionObjectType(CHARACTER_HITBOX);
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HeadBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	HeadBox->SetGenerateOverlapEvents(true);
	HeadBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(HeadBox, TEXT("head"));
	HitboxLimbNameMap.Add(HeadBox->GetFName(), TEXT("head"));

	LeftShoulderBox = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	LeftShoulderBox->SetupAttachment(GetMesh(), TEXT("upperarm_l"));
	LeftShoulderBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftShoulderBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftShoulderBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftShoulderBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftShoulderBox->SetGenerateOverlapEvents(true);
	LeftShoulderBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftShoulderBox, TEXT("upperarm_l"));
	HitboxLimbNameMap.Add(LeftShoulderBox->GetFName(), TEXT("upperarm_l"));

	RightShoulderBox = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	RightShoulderBox->SetupAttachment(GetMesh(), TEXT("upperarm_r"));
	RightShoulderBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightShoulderBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightShoulderBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightShoulderBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightShoulderBox->SetGenerateOverlapEvents(true);
	RightShoulderBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightShoulderBox, TEXT("upperarm_r"));
	HitboxLimbNameMap.Add(RightShoulderBox->GetFName(), TEXT("upperarm_r"));

	LeftElbowBox = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	LeftElbowBox->SetupAttachment(GetMesh(), TEXT("lowerarm_l"));
	LeftElbowBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftElbowBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftElbowBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftElbowBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftElbowBox->SetGenerateOverlapEvents(true);
	LeftElbowBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftElbowBox, TEXT("lowerarm_l"));
	HitboxLimbNameMap.Add(LeftElbowBox->GetFName(), TEXT("lowerarm_l"));

	RightElbowBox = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	RightElbowBox->SetupAttachment(GetMesh(), TEXT("lowerarm_r"));
	RightElbowBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightElbowBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightElbowBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightElbowBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightElbowBox->SetGenerateOverlapEvents(true);
	RightElbowBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightElbowBox, TEXT("lowerarm_r"));
	HitboxLimbNameMap.Add(RightElbowBox->GetFName(), TEXT("lowerarm_r"));

	LeftHandBox = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	LeftHandBox->SetupAttachment(GetMesh(), TEXT("hand_l"));
	LeftHandBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftHandBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftHandBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftHandBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftHandBox->SetGenerateOverlapEvents(true);
	LeftHandBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftHandBox, TEXT("hand_l"));
	HitboxLimbNameMap.Add(LeftHandBox->GetFName(), TEXT("hand_l"));

	RightHandBox = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	RightHandBox->SetupAttachment(GetMesh(), TEXT("hand_r"));
	RightHandBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightHandBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightHandBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightHandBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightHandBox->SetGenerateOverlapEvents(true);
	RightHandBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightHandBox, TEXT("hand_r"));
	HitboxLimbNameMap.Add(RightHandBox->GetFName(), TEXT("hand_r"));

	PelvisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	PelvisBox->SetupAttachment(GetMesh(), TEXT("pelvis"));
	PelvisBox->SetCollisionObjectType(CHARACTER_HITBOX);
	PelvisBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PelvisBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PelvisBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	PelvisBox->SetGenerateOverlapEvents(true);
	PelvisBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(PelvisBox, TEXT("pelvis"));
	HitboxLimbNameMap.Add(PelvisBox->GetFName(), TEXT("pelvis"));

	LeftHipBox = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	LeftHipBox->SetupAttachment(GetMesh(), TEXT("thigh_l"));
	LeftHipBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftHipBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftHipBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftHipBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftHipBox->SetGenerateOverlapEvents(true);
	LeftHipBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftHipBox, TEXT("thigh_l"));
	HitboxLimbNameMap.Add(LeftHipBox->GetFName(), TEXT("thigh_l"));

	RightHipBox = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	RightHipBox->SetupAttachment(GetMesh(), TEXT("thigh_r"));
	RightHipBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightHipBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightHipBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightHipBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightHipBox->SetGenerateOverlapEvents(true);
	RightHandBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightHipBox, TEXT("thigh_r"));
	HitboxLimbNameMap.Add(RightHipBox->GetFName(), TEXT("thigh_r"));

	LeftKneeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	LeftKneeBox->SetupAttachment(GetMesh(), TEXT("calf_l"));
	LeftKneeBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftKneeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftKneeBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftKneeBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftKneeBox->SetGenerateOverlapEvents(true);
	LeftKneeBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftKneeBox, TEXT("calf_l"));
	HitboxLimbNameMap.Add(LeftKneeBox->GetFName(), TEXT("calf_l"));

	RightKneeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	RightKneeBox->SetupAttachment(GetMesh(), TEXT("calf_r"));
	RightKneeBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightKneeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightKneeBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightKneeBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightKneeBox->SetGenerateOverlapEvents(true);
	RightKneeBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightKneeBox, TEXT("calf_r"));
	HitboxLimbNameMap.Add(RightKneeBox->GetFName(), TEXT("calf_r"));

	LeftAnkleBox = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	LeftAnkleBox->SetupAttachment(GetMesh(), TEXT("foot_l"));
	LeftAnkleBox->SetCollisionObjectType(CHARACTER_HITBOX);
	LeftAnkleBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftAnkleBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftAnkleBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	LeftAnkleBox->SetGenerateOverlapEvents(true);
	LeftAnkleBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(LeftAnkleBox, TEXT("foot_l"));
	HitboxLimbNameMap.Add(LeftAnkleBox->GetFName(), TEXT("foot_l"));

	RightAnkleBox = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	RightAnkleBox->SetupAttachment(GetMesh(), TEXT("foot_r"));
	RightAnkleBox->SetCollisionObjectType(CHARACTER_HITBOX);
	RightAnkleBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightAnkleBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightAnkleBox->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Overlap);
	RightAnkleBox->SetGenerateOverlapEvents(true);
	RightAnkleBox->SetIsReplicated(true);
	//HitboxLimbMap.Add(RightAnkleBox, TEXT("foot_r"));
	HitboxLimbNameMap.Add(RightAnkleBox->GetFName(), TEXT("foot_r"));

	for (auto& Pair : HitboxLimbNameMap)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s mapped to bone %s"), *Pair.Key->GetName(), *Pair.Value.ToString());
	}
}

void AMakeAMoveCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	// TO DO: PlayHitReactMontage();
}

void AMakeAMoveCharacter::Multicast_Dismember_Implementation(FName BoneName)
{
	//USkeletalMeshComponent* SkelMeshComp = GetMesh();
	//if (SkelMeshComp == nullptr) return;

	//// Hide the main bone
	//SkelMeshComp->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);

	//// Get child bones recursively
	//const FReferenceSkeleton& RefSkeleton = SkelMeshComp->SkeletalMesh->GetRefSkeleton();
	//int32 HitBoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	//if (HitBoneIndex == INDEX_NONE) return;

	//FTimerHandle TimerHandle;

	//TArray<int32> ChildBoneIndices;
	//RefSkeleton.GetDirectChildBones(HitBoneIndex, ChildBoneIndices);

	//for (int32 ChildIndex : ChildBoneIndices)
	//{
	//	FName ChildBoneName = RefSkeleton.GetBoneName(ChildIndex);
	//	SkelMeshComp->HideBoneByName(ChildBoneName, EPhysBodyOp::PBO_Term);
	//}

	HideBoneAndChildren(BoneName);

	UE_LOG(LogTemp, Warning, TEXT("Dismembering bone: %s"), *BoneName.ToString());
}

void AMakeAMoveCharacter::Multicast_DismemberPhysics_Implementation(FName BoneName)
{
	if (!GetMesh()) return;

	HideBoneAndChildren(BoneName);

	// 2. Determine limb class
	TSubclassOf<AActor> LimbClass = nullptr;
	if (BoneToLimbMeshMap.Contains(BoneName))
	{
		LimbClass = BoneToLimbMeshMap[BoneName];
	}
	if (!LimbClass) return;

	// 3. Spawn limb at bone socket
	FTransform BoneTransform = GetMesh()->GetSocketTransform(BoneName);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SeveredLimb = GetWorld()->SpawnActor<AActor>(LimbClass, BoneTransform, SpawnParams);
	if (!SeveredLimb) return;

	// 4. Enable physics
	if (USkeletalMeshComponent* LimbMesh = Cast<USkeletalMeshComponent>(SeveredLimb->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
	{
		LimbMesh->SetSimulatePhysics(true);
		LimbMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		LimbMesh->SetPhysicsLinearVelocity(GetVelocity());
		FVector Impulse = FVector(FMath::RandRange(-100.f, 100.f), FMath::RandRange(-100.f, 100.f), -200.f);
		LimbMesh->AddImpulse(Impulse, NAME_None, true);
	}

	// 5. Spawn BloodFX
	if (BloodSprayFX)
	{
		FTransform SeveredSocket = GetMesh()->GetSocketTransform(BoneName);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BloodSprayFX,
			SeveredSocket.GetLocation(),
			SeveredSocket.GetRotation().Rotator(),
			FVector(1.0f) // scale (adjust if you want bigger/smaller spray)
		);
	}

}

void AMakeAMoveCharacter::HideBoneAndChildren(FName BoneName)
{
	if (!GetMesh()) return;

	USkeletalMesh* SkelMesh = GetMesh()->GetSkeletalMeshAsset();
	if (!SkelMesh) return;

	const FReferenceSkeleton& RefSkeleton = SkelMesh->GetRefSkeleton();

	// Hide main bone
	GetMesh()->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);

	int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return;

	TArray<int32> ChildBones;
	RefSkeleton.GetDirectChildBones(BoneIndex, ChildBones);

	while (ChildBones.Num() > 0)
	{
		int32 ChildIndex = ChildBones.Pop();
		FName ChildBoneName = RefSkeleton.GetBoneName(ChildIndex);
		GetMesh()->HideBoneByName(ChildBoneName, EPhysBodyOp::PBO_None);

		RefSkeleton.GetDirectChildBones(ChildIndex, ChildBones);
	}
}

void AMakeAMoveCharacter::Server_ProcessHit_Implementation(FName HitBoneName)
{
	if (Health <= 0.f)
	{
		Multicast_DismemberPhysics(HitBoneName);
	}
}

bool AMakeAMoveCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}
