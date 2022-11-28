// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_InitialState	UMETA(DisplayName = "Initial State"),
	EWS_Equipped		UMETA(DisplayName = "Equipped"),
	EWS_Dropped			UMETA(DisplayName = "Dropped"),

	EWS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle	UMETA(DisplayName = "Assault Rifle"),

	EWT_MAX				UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	void Drop();
	
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	UPROPERTY(EditAnywhere, Category = "Fire Mechanics")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Fire Mechanics")
	bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	class USoundBase* EquipSound;

	UPROPERTY(EditAnywhere)
	FLinearColor DisplayColor;

	FString DisplayName;
	bool bCanFire = true;

	/**
	* Crosshair Texture Variables
	*/
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	class UTexture2D* CrosshairCenter;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairLeft;


	FORCEINLINE class USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE int32 GetMaxClipSize() const { return MaxClipSize; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	
	bool IsMagEmpty();
	void AddAmmo(int32 AmmoAmount);
	void SetHUDAmmo();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;

private:
	//Variables
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere)
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, Category = "Ammo")
	int32 Ammo = 30;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxClipSize = 30;

	UPROPERTY()
	class ABlasterCharacter* OwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* OwnerController;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;

	/**
	* FOV Variables
	*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 45.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 25.f;


	/**
	* Functions
	*/
	UFUNCTION()
	void OnRep_WeaponState();

	UFUNCTION()
	void OnRep_Ammo();

	void SpendAmmo();
	void InitializeWeaponType();
	void SetOwnerController();
};
