// Fill out your copyright notice in the Description page of Project Settings.


#include "Gridcell.h"
#include "Engine/World.h"
#include "Tower.h"  
#include "Unit.h"
#include "Brawler.h"
#include "Sniper.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridcell::AGridcell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGridcell::BeginPlay()
{
	Super::BeginPlay();


	RandomSeed = FMath::FRandRange(0.f, 10000.f);
	GenerateGrid();
	SpawnTowers();
	DisplayUnitStats();
	GetWorld()->GetFirstPlayerController()->bEnableClickEvents = true;
	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;


	bPlayerStarts = FMath::RandBool();
	bIsPlayerTurn = bPlayerStarts;
	DeploymentTurn = 0;

	// Find all towers in the level
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* FoundTower = *It;
		Towers.Add(FoundTower);
	}

	if (bPlayerStarts)
	{
		UE_LOG(LogTemp, Warning, TEXT("Coin toss: PLAYER starts"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Coin toss: AI starts"));

		SpawnAIUnits();
		DeploymentTurn++;

		bIsPlayerTurn = true;
	}

	EnableInput(GetWorld()->GetFirstPlayerController());

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC)
	{
		PC->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
	}

	// Create and display the HUD
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UGameHUDWidget>(GetWorld(), HUDWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();

			HUDWidget->UpdateHUD(
				TEXT("Turn: Player"),
				TEXT("Player HP"),
				TEXT("AI HP"),
				TEXT("Towers 0 - 0"));  
		}
	}
	//SpawnUnits();
}

// Called every frame
void AGridcell::GenerateGrid()
{
	if (!CellClass) return;

	Grid.SetNum(GridSize);

	for (int32 X = 0; X < GridSize; X++)
	{
		Grid[X].SetNum(GridSize);

		for (int32 Y = 0; Y < GridSize; Y++)
		{
			FVector Location = FVector(
				X * CellSpacing - (GridSize * CellSpacing / 2),
				Y * CellSpacing - (GridSize * CellSpacing / 2),
				0.f
			);

			AMyActor* NewCell = GetWorld()->SpawnActor<AMyActor>(
				CellClass,
				Location,
				FRotator::ZeroRotator
			);

			if (NewCell)
			{
				float NoiseValue = FMath::PerlinNoise2D(
					FVector2D(
						(X + RandomSeed) * NoiseScale,
						(Y + RandomSeed) * NoiseScale
					)
				);

				NoiseValue = (NoiseValue + 1.f) / 2.f;

				int32 Height;

				if (NoiseValue < 0.35f)
					Height = 0;
				else if (NoiseValue < 0.50f)
					Height = 1;
				else if (NoiseValue < 0.60f)
					Height = 2;
				else if (NoiseValue < 0.69f)
					Height = 3;
				else
					Height = 4;

				NewCell->GridManager = this;
				NewCell->InitializeCell(X, Y, Height);

				Grid[X][Y] = NewCell;     // IMPORTANT
				GridCells.Add(NewCell);
			}
		}
	}
}



void AGridcell::SpawnTowers()
{
	if (!TowerClass) return;

	TArray<FIntPoint> IdealPositions;

	IdealPositions.Add(FIntPoint(12, 12));
	IdealPositions.Add(FIntPoint(5, 12));
	IdealPositions.Add(FIntPoint(19, 12));

	for (FIntPoint Pos : IdealPositions)
	{
		AMyActor* Cell = FindNearestValidCell(Pos.X, Pos.Y);

		if (!Cell) continue;

		FVector Location = Cell->GetActorLocation() + FVector(0, 0, 100);

		ATower* NewTower = GetWorld()->SpawnActor<ATower>(TowerClass, Location, FRotator::ZeroRotator);

		if (NewTower)
		{
			// Save tower coordinates for capture logic
			NewTower->GridX = Cell->X;
			NewTower->GridY = Cell->Y;

			// Mark the cell as tower
			Cell->bIsWalkable = false;
			Cell->bIsTower = true;
		}
	}
}

AMyActor* AGridcell::FindNearestValidCell(int32 TargetX, int32 TargetY)
{
	AMyActor* BestCell = nullptr;
	float BestDistance = FLT_MAX;

	for (AMyActor* Cell : GridCells)
	{
		if (!Cell)
		{
			continue;
		}
		if (Cell->Height == 0) continue; // treat water as non-walkable
		if (!Cell->bIsWalkable) continue;

		float Distance = FVector2D(Cell->X - TargetX, Cell->Y - TargetY).Size();

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestCell = Cell;
		}
	}

	return BestCell;
}

void AGridcell::SpawnAIUnits()
{
	if (bAIBrawlerPlaced && bAISniperPlaced)
	{
		return;
	}

	TArray<AMyActor*> ValidCells;

	for (AMyActor* Cell : GridCells)
	{
		if (Cell &&
			Cell->Y >= 22 &&
			Cell->bIsWalkable &&
			Cell->OccupyingUnit == nullptr)
		{
			ValidCells.Add(Cell);
		}
	}

	if (ValidCells.Num() == 0) return;

	AMyActor* ChosenCell = ValidCells[FMath::RandRange(0, ValidCells.Num() - 1)];

	bool bChooseBrawler = false;

	if (!bAIBrawlerPlaced && !bAISniperPlaced)
	{
		bChooseBrawler = FMath::RandBool();
	}
	else if (!bAIBrawlerPlaced)
	{
		bChooseBrawler = true;
	}
	else
	{
		bChooseBrawler = false;
	}

	if (bChooseBrawler)
	{
		SpawnUnitAt(AI_BrawlerClass, ChosenCell, ETeam::AI);
		bAIBrawlerPlaced = true;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Yellow,
				TEXT("AI placed: Brawler")
			);
		}
	}
	else
	{
		SpawnUnitAt(AI_SniperClass, ChosenCell, ETeam::AI);
		bAISniperPlaced = true;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Yellow,
				TEXT("AI placed: Sniper")
			);
		}
	}

	AIUnitsPlaced++;
	bIsPlayerTurn = true;
}

void AGridcell::SpawnUnitAt(TSubclassOf<AUnit> InUnitClass, AMyActor* Cell, ETeam Team)
{
	// Stop if the unit class or target cell is invalid
	if (!InUnitClass || !Cell)
		return;

	// Spawn the unit slightly above the cell
	FVector Location = Cell->GetActorLocation();

	AUnit* SpawnedUnit = GetWorld()->SpawnActor<AUnit>(
		InUnitClass,
		Location,
		FRotator::ZeroRotator
	);

	if (SpawnedUnit)
	{
		// Get half of the cell height
		float CellHalfHeight = Cell->Mesh->Bounds.BoxExtent.Z;

		// Get half of the unit height
		float UnitHalfHeight = SpawnedUnit->Mesh->Bounds.BoxExtent.Z;

		// Calculate the final position so the unit sits exactly on the cell
		FVector FinalLocation = Cell->GetActorLocation() + FVector(0.f, 0.f, CellHalfHeight + UnitHalfHeight);

		// Move the unit to the corrected position
		SpawnedUnit->SetActorLocation(FinalLocation);

		// Assign unit type
		if (InUnitClass == AI_BrawlerClass)
		{
			SpawnedUnit->UnitType = EUnitType::AIBrawler;
			

			if (AIBrawlerStartCell == nullptr)
			{
				AIBrawlerStartCell = Cell;
			}
		}
		else if (InUnitClass == AI_SniperClass)
		{
			SpawnedUnit->UnitType = EUnitType::AISniper;
			

			if (AISniperStartCell == nullptr)
			{
				AISniperStartCell = Cell;
			}

		}
		else if (InUnitClass == BrawlerClass)
		{
			SpawnedUnit->UnitType = EUnitType::PlayerBrawler;
			

			// Save initial spawn cell only once
			if (PlayerBrawlerStartCell == nullptr)
			{
				PlayerBrawlerStartCell = Cell;
			}
		}
		else if (InUnitClass == SniperClass)
		{
			SpawnedUnit->UnitType = EUnitType::PlayerSniper;
			

			if (PlayerSniperStartCell == nullptr)
			{
				PlayerSniperStartCell = Cell;
			}
		}

		SpawnedUnit->Team = Team;

		// Reset unit action state
		SpawnedUnit->bHasActed = false;

		// Assign movement range based on unit type
		if (InUnitClass == BrawlerClass || InUnitClass == AI_BrawlerClass)
		{
			SpawnedUnit->MovementRange = 6;
		}
		else if (InUnitClass == SniperClass || InUnitClass == AI_SniperClass)
		{
			SpawnedUnit->MovementRange = 4;
		}

		// Assign material based on team
		if (Team == ETeam::Player)
		{
			SpawnedUnit->Mesh->SetMaterial(0, SpawnedUnit->PlayerMaterial);
		}
		else
		{
			SpawnedUnit->Mesh->SetMaterial(0, SpawnedUnit->AIMaterial);
		}

		// Mark the cell as occupied
		Cell->OccupyingUnit = SpawnedUnit;

		SpawnedUnit->CurrentCell = Cell;

		// Save original spawn cell for reset logic
		SpawnedUnit->SpawnCell = Cell;

		if (Team == ETeam::Player)
		{
			if (InUnitClass == BrawlerClass)
				PlayerBrawlerSpawnCell = Cell;

			else if (InUnitClass == SniperClass)
				PlayerSniperSpawnCell = Cell;
		}
		else if (Team == ETeam::AI)
		{
			if (InUnitClass == AI_BrawlerClass)
				AIBrawlerSpawnCell = Cell;

			else if (InUnitClass == AI_SniperClass)
				AISniperSpawnCell = Cell;
		}
	}
}

void AGridcell::HandleCellClicked(AMyActor* ClickedCell)
{

	// Block input during AI turn
	if (!bIsPlayerTurn)
	{
		return;
	}

	if (bIsUnitMoving)
	{
		return;
	}

	if (PlayerUnitsPlaced < 2 || AIUnitsPlaced < 2)
	{
		// PLAYER TURN
		if (bIsPlayerTurn)
		{
			if (ClickedCell->Y > 2)
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid player deployment zone"));
				return;
			}

			if (!ClickedCell->bIsWalkable || ClickedCell->OccupyingUnit != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid spawn cell"));
				return;
			}

			if (!SelectedDeployUnit)
			{
				UE_LOG(LogTemp, Warning, TEXT("No unit selected for deployment"));
				return;
			}

			SpawnUnitAt(SelectedDeployUnit, ClickedCell, ETeam::Player);

			if (SelectedDeployUnit == BrawlerClass)
				bBrawlerPlaced = true;
			else if (SelectedDeployUnit == SniperClass)
				bSniperPlaced = true;

			SelectedDeployUnit = nullptr;
			PlayerUnitsPlaced++;

			bIsPlayerTurn = false;

			SpawnAIUnits();

			// If deployment phase is finished, first turn goes to coin toss winner
			if (PlayerUnitsPlaced >= 2 && AIUnitsPlaced >= 2)
			{
				bIsPlayerTurn = bPlayerStarts;

				if (!bIsPlayerTurn)
				{
					UE_LOG(LogTemp, Warning, TEXT("AI starts first turn"));
					ExecuteAITurn();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Player starts first turn"));
				}
			}

			return;
		}
	}

	
     // PHASE 2: PLAYER TURN


   // Select a unit only if it belongs to the player
	if (!SelectedUnit &&ClickedCell->OccupyingUnit &&ClickedCell->OccupyingUnit->CurrentHP > 0)
	{
		if (ClickedCell->OccupyingUnit->Team != ETeam::Player)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot select AI units"));
			return;
		}

		SelectedUnit = ClickedCell->OccupyingUnit;

		HighlightReachableCells(SelectedUnit);

		UE_LOG(LogTemp, Warning, TEXT("Player unit selected"));

		return;
	}

	// If a unit is selected
	if (SelectedUnit)
	{

		// ATTACK ENEMY UNIT

		if (SelectedUnit &&
			ClickedCell->OccupyingUnit &&
			ClickedCell->OccupyingUnit->Team != SelectedUnit->Team)
		{
			// Find the current cell of the selected unit
			AMyActor* CurrentCell = nullptr;

			for (AMyActor* Cell : GridCells)
			{
				if (Cell && Cell->OccupyingUnit == SelectedUnit)
				{
					CurrentCell = Cell;
					break;
				}
			}

			if (!CurrentCell) return;

			// Calculate Manhattan distance
			int32 Distance =
				FMath::Abs(CurrentCell->X - ClickedCell->X) +
				FMath::Abs(CurrentCell->Y - ClickedCell->Y);

			// Check if enemy is within attack range
			if (Distance <= SelectedUnit->AttackRange)
			{
				SelectedUnit->Attack(ClickedCell->OccupyingUnit);

				// Recalculate towers after attack
				UpdateAllTowers(); 

				 

				// Mark unit as used
				SelectedUnit->bHasActed = true;

				// Count action
				PlayerActionsThisTurn++;

				ClearHighlightedCells();
				SelectedUnit = nullptr;

				UE_LOG(LogTemp, Warning, TEXT("Attack executed"));

				// End turn only after 2 actions
				if (PlayerActionsThisTurn >= 2)
				{
					bIsPlayerTurn = false;

					FTimerHandle AITurnDelayHandle;
					GetWorld()->GetTimerManager().SetTimer(
						AITurnDelayHandle,
						this,
						&AGridcell::ExecuteAITurn,
						1.0f,
						false
					);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Target out of range"));
			}

			return;
		}

		
		// MOVE TO VALID CELL
		
		if (HighlightedCells.Contains(ClickedCell))
		{
			if (ClickedCell->bIsTower)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot move onto tower"));
				return;
			}

			MoveUnitToCell(SelectedUnit, ClickedCell, true);

			ClearHighlightedCells();
			SelectedUnit = nullptr;

			bIsPlayerTurn = false;

			UE_LOG(LogTemp, Warning, TEXT("Unit moved"));

			

			return;
		}

		
		// INVALID CLICK
		
		ClearHighlightedCells();
		SelectedUnit = nullptr;

		UE_LOG(LogTemp, Warning, TEXT("Invalid movement or attack"));

		return;
	}
}

AUnit* AGridcell::FindClosestEnemy(AUnit* AIUnit)
{
	if (!AIUnit) return nullptr;

	AUnit* ClosestEnemy = nullptr;
	float BestDistance = FLT_MAX;

	for (AMyActor* Cell : GridCells)
	{
		if (!Cell || !Cell->OccupyingUnit) continue;

		AUnit* OtherUnit = Cell->OccupyingUnit;

		bool bIsEnemy =
			OtherUnit->UnitType == EUnitType::PlayerBrawler ||
			OtherUnit->UnitType == EUnitType::PlayerSniper;

		if (!bIsEnemy) continue;

		float Distance = FVector::Dist(
			AIUnit->GetActorLocation(),
			OtherUnit->GetActorLocation()
		);

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			ClosestEnemy = OtherUnit;
		}
	}

	return ClosestEnemy;
}


void AGridcell::ExecuteAITurn()
{

	// Stop AI turn after 2 actions
	if (AIActionsThisTurn >= 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("FULL TURN ENDED"));

		// Update towers at end of full round
		UpdateAllTowers();

		// Check victory only now
		CheckVictoryCondition();

		if (bGameEnded)
			return;

		// Reset units for next turn
		for (AMyActor* Cell : GridCells)
		{
			if (Cell && Cell->OccupyingUnit)
			{
				Cell->OccupyingUnit->bHasActed = false;
			}
		}

		PlayerActionsThisTurn = 0;
		AIActionsThisTurn = 0;

		bIsPlayerTurn = true;

		UE_LOG(LogTemp, Warning, TEXT("AI turn ended, player turn starts"));

		return;
	}

	TArray<AUnit*> AIUnits;

	// Collect all AI units
	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit &&
			Cell->OccupyingUnit->Team == ETeam::AI)
		{
			AIUnits.Add(Cell->OccupyingUnit);
		}
	}

	if (AIUnits.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FULL TURN ENDED"));

		UpdateAllTowers();

		

		if (bGameEnded)
			return;

		ResetTurn();

		PlayerActionsThisTurn = 0;
		AIActionsThisTurn = 0;

		bIsPlayerTurn = true;

		return;
	}

	// Alternate AI units turn by turn
	LastAIUnitIndex = (LastAIUnitIndex + 1) % AIUnits.Num();

	AUnit* AIUnit = AIUnits[LastAIUnitIndex];

	AMyActor* AICell = nullptr;

	// Find AI unit current cell
	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit == AIUnit)
		{
			AICell = Cell;
			break;
		}
	}

	if (!AICell)
	{
		bIsPlayerTurn = true;
		return;
	}

	
	// ATTACK IF PLAYER IS IN RANGE
	
	for (AMyActor* PlayerCell : GridCells)
	{
		if (!PlayerCell || !PlayerCell->OccupyingUnit) continue;
		if (PlayerCell->OccupyingUnit->Team != ETeam::Player) continue;

		int32 Distance =
			FMath::Abs(AICell->X - PlayerCell->X) +
			FMath::Abs(AICell->Y - PlayerCell->Y);

		if (Distance <= AIUnit->AttackRange)
		{
			AIUnit->Attack(PlayerCell->OccupyingUnit);

			// Recalculate towers after attack
			UpdateAllTowers();

			

			// Mark AI unit as used
			AIUnit->bHasActed = true;
			AIActionsThisTurn++;

			UE_LOG(LogTemp, Warning, TEXT("AI attacked player"));

			DisplayUnitStats();

			// Continue AI turn with next unit
			ExecuteAITurn();
			return;
		}
	}

	// OTHERWISE MOVE
	
	AMyActor* BestMoveCell = nullptr;
	float BestImprovement = -1.f;

	ATower* TargetTower = nullptr;
	float BestTowerDistance = FLT_MAX;

	// Find nearest tower not controlled by AI
	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		if (Tower->TowerState == ETowerState::ControlledByAI)
			continue;

		float Dist = FVector::Dist(
			AICell->GetActorLocation(),
			Tower->GetActorLocation()
		);

		if (Dist < BestTowerDistance)
		{
			BestTowerDistance = Dist;
			TargetTower = Tower;
		}
	}

	// PRIORITY 1: move toward tower
	if (TargetTower)
	{
		float CurrentDistance = FVector::Dist(
			AICell->GetActorLocation(),
			TargetTower->GetActorLocation()
		);

		for (AMyActor* Candidate : GridCells)
		{
			if (!Candidate) continue;
			if (Candidate->OccupyingUnit != nullptr) continue;
			if (Candidate->Height == 0) continue;
			if (Candidate->bIsTower) continue;

			TArray<AMyActor*> Path = FindPath(AICell, Candidate);

			if (Path.Num() <= 1 || Path.Num() > AIUnit->MovementRange + 1)
				continue;

			// Evaluate candidate cell using heuristic scoring
			float Score = EvaluateCellHeuristic(Candidate, AIUnit);

			// Select the cell with the best heuristic score
			if (Score > BestImprovement)
			{
				BestImprovement = Score;
				BestMoveCell = Candidate;
			}
		}
	}

	// PRIORITY 2: if no useful tower move, chase player
	if (!BestMoveCell)
	{
		for (AMyActor* PlayerCell : GridCells)
		{
			if (!PlayerCell || !PlayerCell->OccupyingUnit) continue;
			if (PlayerCell->OccupyingUnit->Team != ETeam::Player) continue;

			float CurrentDistance = FVector::Dist(
				AICell->GetActorLocation(),
				PlayerCell->GetActorLocation()
			);

			for (AMyActor* Candidate : GridCells)
			{
				if (!Candidate) continue;
				if (Candidate->OccupyingUnit != nullptr) continue;
				if (Candidate->Height == 0) continue;
				if (Candidate->bIsTower) continue;

				TArray<AMyActor*> Path = FindPath(AICell, Candidate);

				if (Path.Num() <= 1 || Path.Num() > AIUnit->MovementRange + 1)
					continue;

				float NewDistance = FVector::Dist(
					Candidate->GetActorLocation(),
					PlayerCell->GetActorLocation()
				);

				float Improvement = CurrentDistance - NewDistance;

				if (Improvement > BestImprovement)
				{
					BestImprovement = Improvement;
					BestMoveCell = Candidate;
				}
			}
		}
	}

	if (BestMoveCell)
	{
		// Move AI unit, turn continuation handled inside MoveUnitToCell
		MoveUnitToCell(AIUnit, BestMoveCell, false);

		UE_LOG(LogTemp, Warning, TEXT("AI moved"));

		return;
	}
	else
	{
		// No movement possible, count this as AI action
		AIUnit->bHasActed = true;
		AIActionsThisTurn++;

		UE_LOG(LogTemp, Warning, TEXT("AI could not move"));

		// Continue AI turn if actions remain
		if (AIActionsThisTurn < 2)
		{
			ExecuteAITurn();
		}
		else
		{
			// Reset units for next player turn
			for (AMyActor* Cell : GridCells)
			{
				if (Cell && Cell->OccupyingUnit)
				{
					Cell->OccupyingUnit->bHasActed = false;
				}
			}

			PlayerActionsThisTurn = 0;
			AIActionsThisTurn = 0;

			bIsPlayerTurn = true;

			UE_LOG(LogTemp, Warning, TEXT("AI turn ended"));
		}

		return;
	}
}




void AGridcell::MoveUnitToCell(AUnit* Unit, AMyActor* TargetCell, bool bTriggerAITurn)
{
	if (!Unit || !TargetCell) return;

	bIsUnitMoving = true;

	AMyActor* CurrentCell = nullptr;

	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit == Unit)
		{
			CurrentCell = Cell;
			break;
		}
	}

	if (!CurrentCell) return;

	TArray<AMyActor*> Path = FindPath(CurrentCell, TargetCell);

	if (Path.Num() <= 1) return;

	LogMove(Unit, CurrentCell, TargetCell);

	CurrentCell->OccupyingUnit = nullptr;
	TargetCell->OccupyingUnit = Unit;
	Unit->CurrentCell = TargetCell;

	float Delay = 0.0f;
	int32 PathLength = Path.Num();

	for (int32 i = 0; i < PathLength; i++)
	{
		AMyActor* StepCell = Path[i];

		FVector NewLocation = StepCell->GetActorLocation();
		NewLocation.Z += 100.f;

		FTimerHandle MoveTimer;

		GetWorld()->GetTimerManager().SetTimer(
			MoveTimer,
			[this, Unit, NewLocation, i, PathLength, bTriggerAITurn]()
			{
				Unit->SetActorLocation(NewLocation);

				// If movement finished
				if (i == PathLength - 1)
				{
					UE_LOG(LogTemp, Warning, TEXT("Movement finished"));

					// Update stats after movement
					DisplayUnitStats();

					// Update tower ownership after movement
					UpdateAllTowers();

					CheckVictoryCondition();

					// Stop if game ended
					if (bGameEnded)
					{
						bIsUnitMoving = false;
						return;
					}

					bIsUnitMoving = false;

					if (bTriggerAITurn)
					{
						// Player movement completed
						Unit->bHasActed = true;
						PlayerActionsThisTurn++;

						UE_LOG(LogTemp, Warning, TEXT("PlayerActionsThisTurn = %d"), PlayerActionsThisTurn);

						// If player used both actions, AI starts
						if (PlayerActionsThisTurn >= 2)
						{
							bIsPlayerTurn = false;

							FTimerHandle AITurnDelayHandle;
							GetWorld()->GetTimerManager().SetTimer(
								AITurnDelayHandle,
								this,
								&AGridcell::ExecuteAITurn,
								1.0f,
								false
							);
						}
						else
						{
							bIsPlayerTurn = true;
						}
					}
					else
					{
						// AI movement completed
						AIActionsThisTurn++;

						UE_LOG(LogTemp, Warning, TEXT("AI Actions = %d"), AIActionsThisTurn);

						bIsUnitMoving = false;

						// Continue AI only if less than 2 actions
						if (AIActionsThisTurn < 2)
						{
							FTimerHandle NextAITurnHandle;
							GetWorld()->GetTimerManager().SetTimer(
								NextAITurnHandle,
								this,
								&AGridcell::ExecuteAITurn,
								1.0f,
								false
							);
						}
						else
						{
							// Reset for player turn
							for (AMyActor* Cell : GridCells)
							{
								if (Cell && Cell->OccupyingUnit)
								{
									Cell->OccupyingUnit->bHasActed = false;
								}
							}

							PlayerActionsThisTurn = 0;
							AIActionsThisTurn = 0;

							bIsPlayerTurn = true;

							UE_LOG(LogTemp, Warning, TEXT("AI turn ended"));
						}
					}
				}
			},
			Delay,
			false
		);

		Delay += 0.3f;
	}
}

void AGridcell::HighlightReachableCells(AUnit* Unit)
{
	if (!Unit) return;

	ClearHighlightedCells();

	AMyActor* StartCell = nullptr;

	// Find the cell where the unit currently is
	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit == Unit)
		{
			StartCell = Cell;
			break;
		}
	}

	if (!StartCell) return;

	TMap<AMyActor*, int32> CostSoFar;
	TQueue<AMyActor*> Frontier;

	Frontier.Enqueue(StartCell);
	CostSoFar.Add(StartCell, 0);

	while (!Frontier.IsEmpty())
	{
		AMyActor* Current;
		Frontier.Dequeue(Current);

		TArray<AMyActor*> Neighbors = GetOrthogonalNeighbors(Current);

		for (AMyActor* Neighbor : Neighbors)
		{
			if (!Neighbor) continue;

			// Cannot move on water 
			if (Neighbor->Height == 0)
				continue;

			// Cannot move on towers
			if (Neighbor->bIsTower) continue;


			// Cannot move through occupied cells
			if (Neighbor->OccupyingUnit != nullptr) continue;

			// Movement cost
			int32 StepCost = (Neighbor->Height > Current->Height) ? 2 : 1;
			int32 NewCost = CostSoFar[Current] + StepCost;

			

			if (NewCost <= Unit->MovementRange &&
				(!CostSoFar.Contains(Neighbor) || NewCost < CostSoFar[Neighbor]))
			{
				CostSoFar.Add(Neighbor, NewCost);
				Frontier.Enqueue(Neighbor);

				Neighbor->Mesh->SetVectorParameterValueOnMaterials(
					TEXT("Color"),
					FVector(1.f, 1.f, 1.f)
				);

				HighlightedCells.Add(Neighbor);
			}
		}
	}
}

void AGridcell::ClearHighlightedCells()
{
	for (AMyActor* Cell : HighlightedCells)
	{
		if (!Cell) continue;

		Cell->Mesh->SetVectorParameterValueOnMaterials(
			TEXT("Color"),
			FVector(
				Cell->OriginalColor.R,
				Cell->OriginalColor.G,
				Cell->OriginalColor.B
			)
		);
	}

	HighlightedCells.Empty();
}

TArray<AMyActor*> AGridcell::GetOrthogonalNeighbors(AMyActor* Cell)
{
	TArray<AMyActor*> Result;

	for (AMyActor* Other : GridCells)
	{
		if (!Other) continue;

		int32 DX = FMath::Abs(Cell->X - Other->X);
		int32 DY = FMath::Abs(Cell->Y - Other->Y);

		if ((DX == 1 && DY == 0) || (DX == 0 && DY == 1))
		{
			Result.Add(Other);
		}
	}

	return Result;
}

TArray<AMyActor*> AGridcell::FindPath(AMyActor* StartCell, AMyActor* TargetCell)
{
	TMap<AMyActor*, AMyActor*> CameFrom;
	TQueue<AMyActor*> Frontier;

	Frontier.Enqueue(StartCell);
	CameFrom.Add(StartCell, nullptr);

	while (!Frontier.IsEmpty())
	{
		AMyActor* Current;
		Frontier.Dequeue(Current);

		if (Current == TargetCell)
			break;

		TArray<AMyActor*> Neighbors = GetOrthogonalNeighbors(Current);

		for (AMyActor* Neighbor : Neighbors)
		{
			if (!Neighbor) continue;
			if (Neighbor->Height == 0) continue;       // water
			if (Neighbor->bIsTower) continue;          // tower
			if (Neighbor->OccupyingUnit && Neighbor != TargetCell) continue;

			if (!CameFrom.Contains(Neighbor))
			{
				Frontier.Enqueue(Neighbor);
				CameFrom.Add(Neighbor, Current);
			}
		}
	}

	TArray<AMyActor*> Path;

	if (!CameFrom.Contains(TargetCell))
		return Path;

	AMyActor* Current = TargetCell;

	while (Current != nullptr)
	{
		Path.Insert(Current, 0);
		Current = CameFrom[Current];
	}

	return Path;
}

void AGridcell::SelectDeployUnit(bool bSelectBrawler)
{
	if (bSelectBrawler)
	{
		if (bBrawlerPlaced)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Red,
				TEXT("Brawler already placed")
			);
			return;
		}

		SelectedDeployUnit = BrawlerClass;

		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Green,
			TEXT("Selected: Brawler")
		);
	}
	else
	{
		if (bSniperPlaced)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.f,
				FColor::Red,
				TEXT("Sniper already placed")
			);
			return;
		}

		SelectedDeployUnit = SniperClass;

		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Green,
			TEXT("Selected: Sniper")
		);
	}
}

void AGridcell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerUnitsPlaced < 2)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;

		if (PC->WasInputKeyJustPressed(EKeys::One))
		{
			SelectDeployUnit(true);
		}

		if (PC->WasInputKeyJustPressed(EKeys::Two))
		{
			SelectDeployUnit(false);
		}
	}
}

void AGridcell::SelectUnitToDeploy(TSubclassOf<AUnit> InUnitClass)
{
	SelectedUnitToDeploy = InUnitClass;

	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("Selected unit: %s"), *InUnitClass->GetName());
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Green,
			Msg
		);
	}
}

void AGridcell::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	

	PlayerInputComponent->BindAction("SelectBrawler", IE_Pressed, this, &AGridcell::SelectBrawler);
	PlayerInputComponent->BindAction("SelectSniper", IE_Pressed, this, &AGridcell::SelectSniper);
}

void AGridcell::SelectBrawler()
{
	SelectedDeployUnit = BrawlerClass;

	UE_LOG(LogTemp, Warning, TEXT("Brawler selected"));
}

void AGridcell::SelectSniper()
{
	SelectedDeployUnit = SniperClass;

	UE_LOG(LogTemp, Warning, TEXT("Sniper selected"));
}

void AGridcell::DisplayUnitStats()
{
	if (!GEngine) return;

	FString StatsText = TEXT("=== UNIT STATS ===\n\n");

	for (AMyActor* Cell : GridCells)
	{
		if (!Cell || !Cell->OccupyingUnit) continue;

		AUnit* Unit = Cell->OccupyingUnit;

		FString TeamName = (Unit->Team == ETeam::Player) ? TEXT("PLAYER") : TEXT("AI");

		FString UnitType = Unit->UnitName;

		// Create HP bar
		int32 MaxBars = 10;
		int32 FilledBars = FMath::RoundToInt((float(Unit->CurrentHP) / Unit->MaxHP) * MaxBars);

		FString HPBar = TEXT("[");

		for (int32 i = 0; i < MaxBars; i++)
		{
			if (i < FilledBars)
				HPBar += TEXT("|");
			else
				HPBar += TEXT("-");
		}

		HPBar += TEXT("]");

		StatsText += FString::Printf(
			TEXT("%s %s %s\n"),
			*TeamName,
			*UnitType,
			*HPBar
		);
	}

	GEngine->AddOnScreenDebugMessage(
		1,
		9999.f,
		FColor::Green,
		StatsText
	);
}

AMyActor* AGridcell::FindUnitCell(AUnit* Unit)
{
	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit == Unit)
		{
			return Cell;
		}
	}

	return nullptr;
}

FString AGridcell::GetCellCoordinate(AMyActor* Cell)
{
	if (!Cell) return "Invalid";

	TCHAR ColumnLetter = 'A' + Cell->X;
	int32 RowNumber = Cell->Y;

	return FString::Printf(TEXT("%c%d"), ColumnLetter, RowNumber);
}

void AGridcell::LogMove(AUnit* Unit, AMyActor* FromCell, AMyActor* ToCell)
{
	// Validate pointers
	if (!Unit || !FromCell || !ToCell) return;

	// Determine player identifier
	FString PlayerID = (Unit->Team == ETeam::Player) ? "HP" : "AI";

	// Determine unit identifier
	FString UnitID = (Unit->UnitName == "Brawler") ? "B" : "S";

	// Convert coordinates
	FString From = GetCellCoordinate(FromCell);
	FString To = GetCellCoordinate(ToCell);

	// Build move message
	FString LogMessage = FString::Printf(TEXT("%s: %s %s -> %s"),
		*PlayerID,
		*UnitID,
		*From,
		*To);

	// Append to move history
	MoveHistory += LogMessage + TEXT("\n");

	// Show on screen
	GEngine->AddOnScreenDebugMessage(
		10,
		10.f,
		FColor::White,
		MoveHistory
	);
}

void AGridcell::LogAttack(AUnit* Attacker, AMyActor* TargetCell, int32 Damage)
{
	// Validate pointers
	if (!Attacker || !TargetCell) return;

	// Determine player identifier
	FString PlayerID = (Attacker->Team == ETeam::Player) ? "HP" : "AI";

	// Determine unit identifier
	FString UnitID = (Attacker->UnitName == "Brawler") ? "B" : "S";

	// Get target coordinate
	FString TargetCoord = GetCellCoordinate(TargetCell);

	// Build attack message
	FString LogMessage = FString::Printf(TEXT("%s: %s attacks %s Damage:%d"),
		*PlayerID,
		*UnitID,
		*TargetCoord,
		Damage);

	// Append to move history
	MoveHistory += LogMessage + TEXT("\n");

	// Show on screen
	GEngine->AddOnScreenDebugMessage(
		10,
		10.f,
		FColor::Yellow,
		MoveHistory
	);
}

void AGridcell::UpdateAllTowers()
{
	for (ATower* Tower : Towers)
	{
		if (Tower)
		{
			Tower->UpdateTowerControl(GridCells);
		}
	}
}




void AGridcell::CheckVictoryCondition()
{
	int32 PlayerControlled = 0;
	int32 AIControlled = 0;

	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		if (Tower->TowerState == ETowerState::ControlledByPlayer)
			PlayerControlled++;

		else if (Tower->TowerState == ETowerState::ControlledByAI)
			AIControlled++;
	}

	// Player wins the round
	if (PlayerControlled >= 2)
	{
		PlayerTurnWins++;
		AITurnWins = 0;

		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Purple,
			FString::Printf(TEXT("PLAYER WINS ROUND %d"), PlayerTurnWins)
		);

		if (PlayerTurnWins >= 2)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				8.f,
				FColor::Purple,
				TEXT("PLAYER WINS THE MATCH!")
			);

			UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
			return;
		}

		ResetTurn();
		return;
	}

	// AI wins the round
	if (AIControlled >= 2)
	{
		AITurnWins++;
		PlayerTurnWins = 0;

		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::White,
			FString::Printf(TEXT("AI WINS ROUND %d"), AITurnWins)
		);

		if (AITurnWins >= 2)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				8.f,
				FColor::White,
				TEXT("AI WINS THE MATCH!")
			);

			UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
			return;
		}

		ResetTurn();
		return;
	}
}


void AGridcell::ResetTurn()
{
	UE_LOG(LogTemp, Warning, TEXT("RESETTING TURN"));

	// ---------------------------------------------------
	// 1. Destroy all units and clear occupied cells
	// ---------------------------------------------------
	for (AMyActor* Cell : GridCells)
	{
		if (!Cell) continue;

		if (Cell->OccupyingUnit)
		{
			Cell->OccupyingUnit->Destroy();
			Cell->OccupyingUnit = nullptr;
		}
	}

	// ---------------------------------------------------
	// 2. Reset all towers to neutral state
	// ---------------------------------------------------
	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		Tower->TowerState = ETowerState::Neutral;
		Tower->ControllingTeam = ETeam::Neutral;
		Tower->ControlTurns = 0;

		// Reset tower visual color
		Tower->UpdateTowerColor();
	}

	// ---------------------------------------------------
	// 3. Respawn player units in original spawn cells
	// ---------------------------------------------------
	if (PlayerBrawlerSpawnCell)
	{
		SpawnUnitAt(BrawlerClass, PlayerBrawlerSpawnCell, ETeam::Player);
	}

	if (PlayerSniperSpawnCell)
	{
		SpawnUnitAt(SniperClass, PlayerSniperSpawnCell, ETeam::Player);
	}

	// ---------------------------------------------------
	// 4. Respawn AI units in original spawn cells
	// ---------------------------------------------------
	if (AIBrawlerSpawnCell)
	{
		SpawnUnitAt(AI_BrawlerClass, AIBrawlerSpawnCell, ETeam::AI);
	}

	if (AISniperSpawnCell)
	{
		SpawnUnitAt(AI_SniperClass, AISniperSpawnCell, ETeam::AI);
	}

	// ---------------------------------------------------
	// 5. Reset unit action states
	// ---------------------------------------------------
	for (AMyActor* Cell : GridCells)
	{
		if (Cell && Cell->OccupyingUnit)
		{
			Cell->OccupyingUnit->bHasActed = false;
		}
	}

	// ---------------------------------------------------
	// 6. Reset turn counters
	// ---------------------------------------------------
	PlayerActionsThisTurn = 0;
	AIActionsThisTurn = 0;

	// Reset to the original starting team
		bIsPlayerTurn = bPlayerStarts;

	// Unlock movement
	bIsUnitMoving = false;

	UE_LOG(LogTemp, Warning, TEXT("TURN RESET COMPLETE"));
}


float AGridcell::EvaluateCellHeuristic(AMyActor* Cell, AUnit* AIUnit)
{
	if (!Cell || !AIUnit) return -9999.f;

	float Score = 0.f;

	// 
	// 1. Reward cells near neutral or enemy towers
	// 
	for (ATower* Tower : Towers)
	{
		if (!Tower) continue;

		int32 dx = FMath::Abs(Cell->X - Tower->GridX);
		int32 dy = FMath::Abs(Cell->Y - Tower->GridY);

		// If cell is near tower, assign priority
		if (dx <= 2 && dy <= 2)
		{
			if (Tower->TowerState == ETowerState::Neutral)
			{
				Score += 100.f;
			}
			else if (Tower->ControllingTeam == ETeam::Player)
			{
				Score += 80.f;
			}
		}
	}

	// 2. Reward cells from which AI can attack player
	// 
	for (AMyActor* OtherCell : GridCells)
	{
		if (!OtherCell || !OtherCell->OccupyingUnit) continue;

		if (OtherCell->OccupyingUnit->Team == ETeam::Player)
		{
			int32 dx = FMath::Abs(Cell->X - OtherCell->X);
			int32 dy = FMath::Abs(Cell->Y - OtherCell->Y);

			// Attack range bonus
			if (dx + dy <= AIUnit->AttackRange)
			{
				Score += 50.f;
			}

			// Distance penalty to enemy
			Score -= (dx + dy);
		}
	}

	
	// 3. Penalize dangerous cells near enemies
	
	for (AMyActor* OtherCell : GridCells)
	{
		if (!OtherCell || !OtherCell->OccupyingUnit) continue;

		if (OtherCell->OccupyingUnit->Team == ETeam::Player)
		{
			int32 dx = FMath::Abs(Cell->X - OtherCell->X);
			int32 dy = FMath::Abs(Cell->Y - OtherCell->Y);

			// If too close to enemy, risk penalty
			if (dx + dy <= 1)
			{
				Score -= 30.f;
			}
		}
	}

	return Score;
}