// Fill out your copyright notice in the Description page of Project Settings.


#include "Tower.h"
#include "UObject/ConstructorHelpers.h"
#include "MyActor.h"

// Sets default values
ATower::ATower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	 ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
		Mesh->SetWorldScale3D(FVector(2.f, 2.f, 3.f));
	}

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
        TEXT("/Game/Materials/M_TowerColor.M_TowerColor")
    );

    if (MaterialFinder.Succeeded())
    {
        BaseTowerMaterial = MaterialFinder.Object;
    }

    // Load contested material from content folder
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ContestedMatObj(
        TEXT("/Game/Materials/M_TowerContested")
    );

    if (ContestedMatObj.Succeeded())
    {
        ContestedMaterial = ContestedMatObj.Object;
    }
}

// Called when the game starts or when spawned
void ATower::BeginPlay()
{
	Super::BeginPlay();

    // Save original material for neutral state
    NeutralMaterial = Mesh->GetMaterial(0);

    if (Mesh && BaseTowerMaterial)
    {
        // Assign base material to tower mesh
        Mesh->SetMaterial(0, BaseTowerMaterial);

        // Create dynamic material instance for runtime color changes
        DynMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);

    }

    TowerState = ETowerState::Neutral;

    // Set initial neutral tower color
    UpdateTowerColor();
	
}

void ATower::InitializeTower(int32 InX, int32 InY)
{
	GridX = InX;
	GridY = InY;
}

void ATower::UpdateTowerControl(const TArray<AMyActor*>& GridCells)
{
    bool bPlayerNearby = false;
    bool bAINearby = false;

    // Check all cells in capture range
    for (AMyActor* Cell : GridCells)
    {
        if (!Cell || !Cell->OccupyingUnit) continue;

        int32 dx = FMath::Abs(Cell->X - GridX);
        int32 dy = FMath::Abs(Cell->Y - GridY);

        if (dx <= 2 && dy <= 2)
        {
            if (Cell->OccupyingUnit->Team == ETeam::Player)
                bPlayerNearby = true;
            else if (Cell->OccupyingUnit->Team == ETeam::AI)
                bAINearby = true;
        }
    }

    // Determine tower owner
    if (bPlayerNearby && bAINearby)
    {
        TowerState = ETowerState::Contested;
        ControllingTeam = ETeam::Neutral;
    }
    else if (bPlayerNearby)
    {
        TowerState = ETowerState::ControlledByPlayer;
        ControllingTeam = ETeam::Player;
    }
    else if (bAINearby)
    {
        TowerState = ETowerState::ControlledByAI;
        ControllingTeam = ETeam::AI;
    }
    else
    {
        TowerState = ETowerState::Neutral;
        ControllingTeam = ETeam::Neutral;
    }

    UpdateTowerColor();
}

void ATower::UpdateTowerColor()
{
    if (!DynMaterial) return;

    // Neutral tower = gray
    if (TowerState == ETowerState::Neutral)
    {
        DynMaterial->SetVectorParameterValue("Color", FLinearColor::Black);
    }
    // Controlled by Player = blue
    else if (ControllingTeam == ETeam::Player)
    {
        DynMaterial->SetVectorParameterValue("Color", FLinearColor(1.0f, 0.0f, 1.0f, 1.0f));
    }
    // Controlled by AI = white
    else if (ControllingTeam == ETeam::AI)
    {
        DynMaterial->SetVectorParameterValue("Color", FLinearColor::White);
    }

    // Tower contested by both teams = purple + white mix
    else if (TowerState == ETowerState::Contested)
    {
        DynMaterial->SetVectorParameterValue("Color", FLinearColor(1.0f, 0.5f, 1.0f, 1.0f));
    }
}

// Called every frame
//void ATower::Tick(float DeltaTime)
//{
	//Super::Tick(DeltaTime);

//}

