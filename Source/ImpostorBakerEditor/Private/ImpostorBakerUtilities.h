// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>

struct FImpostorTextureData;
enum class EImpostorLayoutType;

class FImpostorBakerUtilities
{
public:
	static FVector OctahedronToUnitVector(const FVector2D& Octahedron);
	static FVector HemiOctahedronToUnitVector(const FVector2D& HemiOctahedron);

	static FVector GetGridVector(int32 X, int32 Y, int32 Size, EImpostorLayoutType Type);
	static int32 GetImpostorTypeResolution(EImpostorLayoutType Type);

	static bool FindIntersection(int32 CornerPass, const FImpostorTextureData& TextureData, EImpostorLayoutType Type, int32 StartX, int32& OutX, int32& OutY);
	static FVector2D GetRotatedCoordsByCorner(const FVector2D& XY, int32 Size, bool bVector, int32 Corner);
	static void GetRotatedCoords(const FVector2D& XY, int32 Size, bool bVector, FVector2D& Out0, FVector2D& Out90, FVector2D& Out180, FVector2D& Out270);

	static void DeriveAxes(const FVector& Vector, FVector& OutX, FVector& OutY, FVector& OutZ);
	static FVector DeriveAxis(const FVector& Vector, EAxis::Type Axis);
};
