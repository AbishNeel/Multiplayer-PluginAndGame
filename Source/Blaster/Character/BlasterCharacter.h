// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interfaces/CrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "../BlasterComponents/CombatComponent.h"
#include "BlasterCharacter.generated.h"

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_TurningLeft	UMETA("Turning Left"),
	ETIP_TurningRight	UMETA("Turning Right"),
	ETIP_NotTurning		UMETA("Not Turning"),

	ETIP_MAX			UMETA("Default Max")
};

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public ICrosshairInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	void SetOverlappingWeapon(class AWeapon* Weapon);
	
	
	FORCEINLINE float GetAO_Yaw() const{ return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const{ return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetIsElimmed() const { return bIsElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	
	AWeapon* GetEquippedWeapon() const;
	void PlayFireMontage(bool bIsAiming);
	FVector GetAimHitLocation() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Jump() override;
	void AimOffset(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void TurnInPlace(float DeltaTime);
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

public:	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	void AimButtonPressed();
	void AimButtonReleased();
	void ReloadButtonPressed();
	bool GetWeaponEquipped();
	bool GetIsAiming();
	void Elim();
	void PlayReloadMontage();
	ECombatState GetCombatState() const;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere, Category = "Follow Camera")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(EditAnywhere, Category = "Follow Camera")
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadDisplay;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Follow Camera")
	float CameraThreshold = 225.f;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	float RespawnDelay = 3.f;

	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartAimRotation;
	ETurningInPlace TurningInPlace;
	FTimerHandle RespawnTimer;
	bool bIsElimmed;

	/**
	* Health Variables and Functions
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	void UpdateHealth();

	/**
	* Dissolve Functionality
	*/
	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = "Elim")
	UCurveFloat* DissolveCurve;

	UPROPERTY(EditAnywhere, Category = "Elim")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	
	void StartDissolve();

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	/**
	* Elim Bot
	*/
	UPROPERTY(EditAnywhere, Category = "Elim")
	UParticleSystem* ElimBotParticle;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UParticleSystemComponent* ElimBotParticleComponent;

	UPROPERTY(EditAnywhere, Category = "Elim")
	class USoundCue* ElimBotSound;
	

	/**
	*Functions
	*/
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	void HideCharacter();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void TryRespawnPlayer();
	void PollInit(); // Polls for relevant data in Tick()
};
