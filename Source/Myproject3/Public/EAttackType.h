#pragma once

#include "EAttackType.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Melee UMETA(DisplayName="Melee"),
	Ranged UMETA(DisplayName="Ranged")
};