// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Gridcell.h"
#include "MyActor.h"


AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    CurrentCell = nullptr;

    // Load cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMesh.Object);
    }

    // Load base material for units
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnitMaterial(TEXT("/Game/Materials/M_Unit"));
   if (UnitMaterial.Succeeded())
   {
    // Assign base material to the mesh
   // Mesh->SetMaterial(0, UnitMaterial.Object);

    if (UnitMaterialInstance)
    {
        // Create a unique dynamic material instance for this unit
        UnitMaterialInstance = UMaterialInstanceDynamic::Create(UnitMaterial.Object, this);
        Mesh->SetMaterial(0, UnitMaterialInstance);
    }
   }

   static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMat(TEXT("/Game/Materials/M_PlayerUnit"));
   static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMat(TEXT("/Game/Materials/M_AIUnit"));

   if (PlayerMat.Succeeded())
   {
       PlayerMaterial = PlayerMat.Object;
   }

   if (AIMat.Succeeded())
   {
       AIMaterial = AIMat.Object;
   }

   CurrentHP = 20;
   MaxHP = 20;
   AttackRange = 1;
   MinDamage = 1;
   MaxDamage = 3;

}


	// Called when the game starts or when spawned
	void AUnit::BeginPlay()
	{
		Super::BeginPlay();
		//DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
        // Create a dynamic material instance for this unit
        DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);

	}

	//void AUnit::ReceiveDamage(int32 Amount)
	//{
		//int32 FinalDamage = FMath::Max(0, Amount - Defense);
		//CurrentHealth -= FinalDamage;

		//if (CurrentHealth < 0)
			//CurrentHealth = 0;
	//}

    void AUnit::SetUnitColor(const FLinearColor& NewColor)
    {
        if (UnitMaterialInstance)
        {
            UnitMaterialInstance->SetVectorParameterValue(TEXT("UnitColor"), NewColor);
        }
    }
	

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsMoving)
    {
        FVector NewLocation = FMath::VInterpConstantTo(
            GetActorLocation(),
            TargetLocation,
            DeltaTime,
            MoveSpeed
        );

        SetActorLocation(NewLocation);

        if (FVector::Dist(NewLocation, TargetLocation) < 5.f)
        {
            SetActorLocation(TargetLocation);
            bIsMoving = false;
        }
    }

}

void AUnit::Attack(AUnit* Target)
{
    if (!Target) return;

    // Main attack damage
    int32 Damage = FMath::RandRange(MinDamage, MaxDamage);

    Target->CurrentHP -= Damage;

    UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %d damage"),
        *UnitName,
        *Target->UnitName,
        Damage);

    if (Target->CurrentHP <= 0)
    {
        Target->CurrentHP = 0;

        if (Target->CurrentHP <= 0)
        {
            Target->CurrentHP = 0;

            if (Target->CurrentCell)
            {
                Target->CurrentCell->OccupyingUnit = nullptr;
            }

            // Respawn the defeated unit at its original spawn position
            Target->CurrentCell->OccupyingUnit = nullptr;

            Target->CurrentCell = Target->SpawnCell;
            Target->CurrentHP = Target->MaxHP;

            Target->SpawnCell->OccupyingUnit = Target;

            FVector RespawnLocation = Target->SpawnCell->GetActorLocation();
            RespawnLocation.Z += 100.f;

            Target->SetActorLocation(RespawnLocation);
            return;
        }
    }


    // SNIPER COUNTERATTACK RULE
    
    if (UnitName == "Sniper")
    {
        bool bCanCounter = false;

        // If target is Sniper => always counter
        if (Target->UnitName == "Sniper")
        {
            bCanCounter = true;
        }
        else if (Target->UnitName == "Brawler")
        {
            float Distance = FVector::Dist(
                GetActorLocation(),
                Target->GetActorLocation()
            );

            if (Distance <= 150.f)
            {
                bCanCounter = true;
            }
        }

        if (bCanCounter)
        {
            Target->CounterAttack(this);
        }
    }
}

void AUnit::CounterAttack(AUnit* Attacker)
{
    if (!Attacker) return;

    // Random counter damage between 1 and 3
    int32 CounterDamage = FMath::RandRange(1, 3);

    Attacker->CurrentHP -= CounterDamage;

    UE_LOG(LogTemp, Warning, TEXT("%s counterattacks for %d damage"),
        *UnitName,
        CounterDamage);

    if (Attacker->CurrentHP <= 0)
    {
        Attacker->CurrentHP = 0;

        if (Attacker->CurrentHP <= 0)
        {
            Attacker->CurrentHP = 0;

            if (Attacker->CurrentCell)
            {
                Attacker->CurrentCell->OccupyingUnit = nullptr;
            }

            Attacker->Destroy();
        }
    }

    // Stats will be refreshed elsewhere
}

