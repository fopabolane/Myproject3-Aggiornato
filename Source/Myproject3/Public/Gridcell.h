// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.h"
#include "Unit.h"
#include "GameHUDWidget.h"

#include "Gridcell.generated.h" // Questo include DEVE essere l'ULTIMO prima delle dichiarazioni UCLASS/USTRUCT/UENUM

UCLASS()
class MYPROJECT3_API AGridcell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridcell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere)
	int32 GridSize = 25;

	UPROPERTY(EditAnywhere)
	float CellSpacing = 220.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AMyActor> CellClass;
    
	UPROPERTY(EditAnywhere)
	float NoiseScale = 0.15f;

	UPROPERTY(EditAnywhere)
	int32 MaxHeight = 4;

	UPROPERTY(EditAnywhere, Category = "Towers")
	TSubclassOf<class ATower> TowerClass;

	UPROPERTY()
	TArray<AMyActor*> GridCells;
	//TArray<AMyActor*> Grid;
	struct FGridCell
	{
		int32 X;
		int32 Y;

		bool bIsWater = false;
		bool bIsWalkable = true;
	};

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AUnit> UnitClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AUnit> SniperClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AUnit> BrawlerClass;

	TArray<TArray<AMyActor*>> Grid;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AUnit> AI_BrawlerClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AUnit> AI_SniperClass;

	int PlayerUnitsPlaced = 0;
	int AIUnitsPlaced = 0;
	bool bIsPlayerTurn = true;

	//UPROPERTY()
	//class AUnit* OccupyingUnit;

	//UFUNCTION(BlueprintCallable)
	//void SpawnPlayerUnits();

	UFUNCTION(BlueprintCallable)
	void SpawnAIUnits();

	void SpawnUnitAt(TSubclassOf<AUnit> InUnitClass, AMyActor* Cell, ETeam Team);

	void SpawnTowers();

	AMyActor* FindNearestValidCell(int32 TargetX, int32 TargetY);

	float RandomSeed;

	void GenerateGrid();
	//void SpawnTowers(int32 NumTowers);

	//void SpawnUnits();

	void HandleCellClicked(AMyActor* ClickedCell);

	AUnit* FindClosestEnemy(AUnit* AIUnit);
	void ExecuteAITurn();

	void MoveUnitToCell(AUnit* Unit, AMyActor* TargetCell, bool bTriggerAITurn);

	AUnit* SelectedUnit = nullptr;

	void HighlightReachableCells(AUnit* Unit);
	void ClearHighlightedCells();

	// Stores currently highlighted cells
	UPROPERTY()
	TArray<AMyActor*> HighlightedCells;

	TArray<AMyActor*> FindPath(AMyActor* StartCell, AMyActor* TargetCell);

	TArray<AMyActor*> GetOrthogonalNeighbors(AMyActor* Cell);

	int32 CurrentAIUnitIndex = 0;

	UPROPERTY()
	bool bPlayerStarts = true;

	UPROPERTY()
	int32 DeploymentTurn = 0;

	TSubclassOf<AUnit> SelectedDeployUnit;
	bool bBrawlerPlaced = false;
	bool bSniperPlaced = false;

	void SelectDeployUnit(bool bSelectBrawler);

	virtual void Tick(float DeltaTime) override;

	bool bAIBrawlerPlaced = false;
	bool bAISniperPlaced = false;

	UPROPERTY()
	TSubclassOf<AUnit> SelectedUnitToDeploy;

	void SelectUnitToDeploy(TSubclassOf<AUnit> UnitClass);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void SelectBrawler();
	void SelectSniper();

	int32 LastAIUnitIndex = -1;

	// Shows all unit stats on screen
	void DisplayUnitStats();

	AMyActor* FindUnitCell(AUnit* Unit);

	FString GetCellCoordinate(AMyActor* Cell);

	void LogMove(AUnit* Unit, AMyActor* FromCell, AMyActor* ToCell);

	void LogAttack(AUnit* Attacker, AMyActor* TargetCell, int32 Damage);

	// Stores the history of actions performed during the game
	FString MoveHistory;

	// Updates all towers after each turn
   void UpdateAllTowers();

   // Checks if one team controls enough towers to win
   void CheckVictoryCondition();

   // List of all towers in the map
   UPROPERTY()
   TArray<class ATower*> Towers;

   // Prevent input while a unit is moving
   bool bIsUnitMoving = false;

   // Consecutive turns controlling at least 2 towers
   int32 PlayerVictoryTurns = 0;
   int32 AIVictoryTurns = 0;

   // End game flag
   bool bGameEnded = false;

   // Number of units that have already acted in current turn
   int32 PlayerActionsThisTurn = 0;

   int32 AIActionsThisTurn = 0;

   // Delay between turns
   UPROPERTY(EditAnywhere, Category = "Turn")
   float TurnDelay = 10.0f;

   int32 PlayerTurnWins = 0;
   int32 AITurnWins = 0;

   AMyActor* PlayerBrawlerStartCell;
   AMyActor* PlayerSniperStartCell;

   AMyActor* AIBrawlerStartCell;
   AMyActor* AISniperStartCell;

   void ResetTurn();

   bool bRoundJustEnded = false;

   AMyActor* PlayerBrawlerSpawnCell;
   AMyActor* PlayerSniperSpawnCell;
   AMyActor* AIBrawlerSpawnCell;
   AMyActor* AISniperSpawnCell;

   UPROPERTY(EditAnywhere, Category = "UI")
   TSubclassOf<UGameHUDWidget> HUDWidgetClass; // Reference to the HUD widget class

   UPROPERTY()
   UGameHUDWidget* HUDWidget; // Reference to the HUD widget instance

   // Evaluate the heuristic value of a reachable cell for the AI
   float EvaluateCellHeuristic(AMyActor* Cell, AUnit* AIUnit);
};
