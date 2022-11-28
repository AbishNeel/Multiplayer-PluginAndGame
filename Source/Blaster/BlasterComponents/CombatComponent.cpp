// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "../Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../Character/BlasterPlayerController.h"
#include "Engine/Texture2D.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		BlasterController = Cast<ABlasterPlayerController>(Character->Controller);
		if (BlasterController)
		{
			BlasterController->SetHUDWeaponAmmo(0);
			BlasterController->SetHUDCarriedAmmo(0);
		}

		if (Character->HasAuthority())
			InitializeCarriedAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);
		TraceUnderCrosshairs(AimHitResult);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == NULL || WeaponToEquip == NULL || CombatState == ECombatState::ECS_Reloading)
		return;

	if (EquippedWeapon)
		EquippedWeapon->Drop();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if (EquippedWeapon->EquipSound)
		UGameplayStatics::SpawnSoundAttached(EquippedWeapon->EquipSound, Character->GetMesh());

	if (EquippedWeapon->IsMagEmpty())
		Reload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

	BlasterController = BlasterController == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterController;
	if (BlasterController)
	{
		if (EquippedWeapon)
		{
			BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
			BlasterController->SetWeaponTypeText(EquippedWeapon->DisplayColor, EquippedWeapon->DisplayName);
		}
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetIsAiming(bool bAiming)
{
	bIsAiming = bAiming;
	ServerSetIsAiming(bAiming);

	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ServerSetIsAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;

	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;

}

void UCombatComponent::OnRep_WeaponEquipped()
{
	if (Character && EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (RightHandSocket)
			RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}
void UCombatComponent::SetFireButtonPressed(bool bPressed)
{
	if (EquippedWeapon == NULL)
		return;
	
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
		Fire();
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		EquippedWeapon->bCanFire = false;
		ServerFire(AimHitResult.ImpactPoint);

		if (EquippedWeapon)
			CrosshairShootingFactor = 1.2f;

		StartFireButtonDelay();
	}
}

void UCombatComponent::StartFireButtonDelay()
{
	if (Character == NULL)
		return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireButtonDelayComplete, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireButtonDelayComplete()
{
	EquippedWeapon->bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bIsAutomatic)
		Fire();

	if (EquippedWeapon->IsMagEmpty())
		Reload();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitLocation)
{
	MulticastFire(TraceHitLocation);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitLocation)
{
	if (EquippedWeapon == NULL)
		return;

	if (Character && CombatState != ECombatState::ECS_Reloading)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitLocation);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && EquippedWeapon->GetAmmo() < 30 && CombatState != ECombatState::ECS_Reloading)
		ServerReload();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == NULL)
		return;

	CombatState = ECombatState::ECS_Reloading;
	Character->PlayReloadMontage();
}

void UCombatComponent::ReloadFinished()
{
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		AddAmmoToGun(AmmoToAdd());

		if (BlasterController)
			BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	if (bFireButtonPressed)
		Fire();
}

int32 UCombatComponent::AmmoToAdd()
{
	if (EquippedWeapon == NULL)
		return 0;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 Least = FMath::Min(EquippedWeapon->GetMaxClipSize() - EquippedWeapon->GetAmmo(), CarriedAmmoMap[EquippedWeapon->GetWeaponType()]);
		return FMath::Clamp(Least, 0, EquippedWeapon->GetMaxClipSize());
	}
	return 0;
}

void UCombatComponent::AddAmmoToGun(int32 AmmoAmount)
{
	if (EquippedWeapon == NULL)
		return;

	EquippedWeapon->AddAmmo(AmmoAmount);
	if (CarriedAmmoMap[EquippedWeapon->GetWeaponType()])
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= AmmoAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			Character->PlayReloadMontage();
			break;
		case ECombatState::ECS_Unoccupied:
			if (bFireButtonPressed)
				Fire();
			break;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	
	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - CrosshairWorldPosition).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 30.f);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
		else
		{
			if (TraceHitResult.GetActor())
			{
				ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(TraceHitResult.GetActor());
				if (HitCharacter == Character)
				{
					HUDPackage.CrosshairColor = FLinearColor::White;
					AimingAtEnemyFactor = 0.f;
				}
			}

			if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInterface>())
			{
				ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(TraceHitResult.GetActor());
				if (HitCharacter == Character)
				{
					HUDPackage.CrosshairColor = FLinearColor::White;
					AimingAtEnemyFactor = 0.f;
					TraceHitResult.ImpactPoint = End;
				}
				else
				{
					HUDPackage.CrosshairColor = FLinearColor::Red;
					AimingAtEnemyFactor = 0.15f;
				}
			}
			else
			{
				HUDPackage.CrosshairColor = FLinearColor::White;
				AimingAtEnemyFactor = 0.f;
			}
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == NULL || Character->Controller == NULL)
		return;

	BlasterController = BlasterController == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(BlasterController->GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
			}
			else
			{
				HUDPackage.CrosshairCenter = NULL;
				HUDPackage.CrosshairRight = NULL;
				HUDPackage.CrosshairLeft = NULL;
				HUDPackage.CrosshairBottom = NULL;
				HUDPackage.CrosshairTop = NULL;
			}

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector CharacterVelocity = Character->GetVelocity();
			CharacterVelocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, CharacterVelocity.Size());
			
			if (Character->GetCharacterMovement()->IsFalling())
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			else
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

			if (bIsAiming)
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			else
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f);

			HUDPackage.CrosshairSpread = 0.6f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor - CrosshairAimFactor - AimingAtEnemyFactor;
			BlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == NULL)
		return;
	if (Character && Character->GetFollowCamera())
	{
		if (bIsAiming)
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		else
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());

		Character->GetFollowCamera()->FieldOfView = CurrentFOV;
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == NULL)
		return false;
	return EquippedWeapon->bCanFire && !EquippedWeapon->IsMagEmpty() && CombatState != ECombatState::ECS_Reloading;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	BlasterController = BlasterController == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : BlasterController;
	if (BlasterController)
	{
		if(EquippedWeapon)
			BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, CarriedARAmmo);
}