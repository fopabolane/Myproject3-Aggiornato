// Fill out your copyright notice in the Description page of Project Settings.


#include "Brawler.h"

ABrawler::ABrawler()
{
	// Brawler stats
	MaxHP = 40;
	CurrentHP = MaxHP;

	AttackRange = 1;

	MinDamage = 1;
	MaxDamage = 6;

	UnitName = "Brawler";
}