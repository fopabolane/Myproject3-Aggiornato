// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C:/Program Files/Epic Games/UE_5.6/Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h"
#include "Components/TextRenderComponent.h"
#include "MyActor.generated.h"


class AUnit;

UCLASS()
class MYPROJECT3_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();


//public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere) 
		UStaticMeshComponent* Mesh;

		UPROPERTY(BlueprintReadOnly)
		int32 X;

		UPROPERTY(BlueprintReadOnly)
		int32 Y;

		//UPROPERTY(BlueprintReadOnly)
		int32 Height;

		UPROPERTY(BlueprintReadOnly)
		FString CellID;

		UPROPERTY(VisibleAnywhere)
		UTextRenderComponent* Text;

		UPROPERTY()
		bool bIsOccupied = false;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int GridX;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int GridY;

		//UPROPERTY(EditAnywhere, BlueprintReadWrite)
		//bool bIsWalkable;

		UPROPERTY(EditAnywhere)
		int32 GridSize;

		UPROPERTY()
		class AUnit* OccupyingUnit;

		UPROPERTY()
		class AGridcell* GridManager;

		// Original color of the cell before highlighting
		UPROPERTY()
		FLinearColor OriginalColor;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsWalkable = true;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsWater = false;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsTower;

		
		void InitializeCell(int32 X, int32 Y, int32 Height);
	
		//}

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;
};
