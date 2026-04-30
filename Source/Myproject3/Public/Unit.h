
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "EAttackType.h"
#include "MyActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Unit.generated.h"


class AMyActor;



UENUM(BlueprintType)
enum class EUnitType : uint8
{
    PlayerBrawler,
    PlayerSniper,
    AIBrawler,
    AISniper
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
    None,
    Player,
    AI,
    Neutral
};

UCLASS()
class MYPROJECT3_API AUnit : public AActor
{
    GENERATED_BODY()

public:
    AUnit();

protected:

    virtual void BeginPlay() override;

    // Color assigned to the team (Human or AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Visual")
    FLinearColor TeamColor;

    // Dynamic material instance used to color this unit
    UPROPERTY()
    UMaterialInstanceDynamic* UnitMaterialInstance;

public:

    // -------------------- METODI --------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor UnitColor;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUnitType UnitType;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    //UPROPERTY()
    //UMaterialInstanceDynamic* UnitMaterialInstance;

    UStaticMeshComponent* GetMesh() const { return Mesh; }

    // Materials for player and AI teams
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* PlayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* AIMaterial;

    // Team of this unit
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETeam Team;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MovementRange = 1;

    void SetUnitColor(const FLinearColor& NewColor);

    FVector TargetLocation;
    bool bIsMoving = false;
    float MoveSpeed = 200.f;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentHP; // Current health points of the unit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHP; // Maximum health points of the unit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttackRange; // Range at which the unit can attack

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinDamage; // Minimum damage the unit can inflict

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDamage; // Maximum damage the unit can inflict

	void Attack(AUnit* Target); // Method to perform an attack on another unit

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnitName;

    void CounterAttack(AUnit* Attacker);

    UPROPERTY()
    AMyActor* CurrentCell;

    // Stores the initial spawn cell of the unit
    UPROPERTY()
    AMyActor* SpawnCell;

    // Indicates whether this unit has already acted during the current turn
    UPROPERTY()
    bool bHasActed = false;
};
