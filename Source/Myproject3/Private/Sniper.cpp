	// Fill out your copyright notice in the Description page of Project Settings.


#include "Sniper.h"


ASniper::ASniper() 
{
	// Sniper stats
	MaxHP = 20;
	CurrentHP = MaxHP;

	AttackRange = 10;

	MinDamage = 4;
	MaxDamage = 8;

	UnitName = "Sniper";
}