// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../HUD/BlasterHUD.h"
#include "../Weapons/Weapon.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied		UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading		UMETA(DisplayName = "Reloading"),

	ECS_DefaultMax		UMETA(DisplayName = "MAX")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void EquipWeapon(class AWeapon* WeaponToEquip);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetFireButtonPressed(bool bPressed);

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;


	UFUNCTION(BlueprintCallable)
	void ReloadFinished();

protected:
	virtual void BeginPlay() override;
	void SetIsAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bAiming);

	UFUNCTION()
	void OnRep_WeaponEquipped();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitLocation);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* BlasterController;

	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquipped)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	bool bFireButtonPressed;
	float CrosshairVelocityFactor, CrosshairInAirFactor, CrosshairShootingFactor, CrosshairAimFactor, AimingAtEnemyFactor;
	FHitResult AimHitResult;
	FHUDPackage HUDPackage;
	float CurrentFOV;
	float DefaultFOV = 45.f;
	FTimerHandle FireTimer;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Carried Ammo")
	int32 CarriedARAmmo = 60;

	/**
	*Functions
	*/
	void SetHUDCrosshairs(float DeltaTime);
	void InterpFOV(float DeltaTime);
	void Fire();
	void StartFireButtonDelay();
	void FireButtonDelayComplete();
	bool CanFire();
	void InitializeCarriedAmmo();
	void Reload();
	int32 AmmoToAdd();
	void AddAmmoToGun(int32 AmmoAmount);

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION(Server, Reliable)
	void ServerReload();
};
