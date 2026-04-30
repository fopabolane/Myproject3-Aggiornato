// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Unit.h"
#include "Tower.generated.h"


class AMyActor;


// Tower state according to capture rules
UENUM(BlueprintType)
enum class ETowerState : uint8
{
	Neutral,
	ControlledByPlayer,
	ControlledByAI,
	Contested
};


UCLASS()
class MYPROJECT3_API ATower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATower();

	void InitializeTower(int32 InX, int32 InY);

	int32 GridX;
	int32 GridY;

	// Current control state of the tower
	UPROPERTY()
	ETowerState TowerState = ETowerState::Neutral;

	// Team controlling the tower
	UPROPERTY()
	ETeam ControllingTeam;

	// Number of consecutive turns under control
	UPROPERTY()
	int32 ControlTurns = 0;

	// Updates tower ownership based on nearby units
	void UpdateTowerControl(const TArray<AMyActor*>& GridCells);

	// Original material of the tower (neutral state)
	UPROPERTY()
	UMaterialInterface* NeutralMaterial;

	// Updates tower visual color based on controlling team
	void UpdateTowerColor();

	UPROPERTY()
	UMaterialInstanceDynamic* DynMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BaseTowerMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	// Material used when both teams are contesting the tower
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* ContestedMaterial;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

//public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
