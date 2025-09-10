// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "MakeAMove.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "MakeAMove/MakeAMoveCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	//SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(CHARACTER_HITBOX, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(WEAPON_HITBOX, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	DamageHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageHitbox"));
	DamageHitbox->SetupAttachment(RootComponent);
	DamageHitbox->SetCollisionObjectType(WEAPON_HITBOX);
	DamageHitbox->IgnoreActorWhenMoving(GetOwner(), true);
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageHitbox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DamageHitbox->SetCollisionResponseToChannel(CHARACTER_HITBOX, ECR_Overlap);
	DamageHitbox->SetGenerateOverlapEvents(true);
	DamageHitbox->SetIsReplicated(true);
	//DamageHitbox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnHitboxBeginOverlap);

	DamageBoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("DamageBox Trace Start"));
	DamageBoxTraceStart->SetupAttachment(RootComponent);
	DamageBoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("DamageBox Trace End"));
	DamageBoxTraceEnd->SetupAttachment(RootComponent);
	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMakeAMoveCharacter* MakeAMoveCharacter = Cast<AMakeAMoveCharacter>(OtherActor);
	if (MakeAMoveCharacter)
	{
		// SetOverlappingWeapon only happens on the server
		// as soon as we call it from the MakeAMoveCharacter class, the server will replicate it to the owner client (DOREPLIFETIME_CONDITION)
		MakeAMoveCharacter->SetOverlappingWeapon(this); 
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMakeAMoveCharacter* MakeAMoveCharacter = Cast<AMakeAMoveCharacter>(OtherActor);
	if (MakeAMoveCharacter)
	{
		MakeAMoveCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState StateToSet)
{
	WeaponState = StateToSet;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(true);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(true);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AWeapon::EnableHitBox()
{
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	bDamageBoxActive = true;
	bHasHitSomething = false;
	
}

void AWeapon::DisableHitBox()
{
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bDamageBoxActive = false;
}



