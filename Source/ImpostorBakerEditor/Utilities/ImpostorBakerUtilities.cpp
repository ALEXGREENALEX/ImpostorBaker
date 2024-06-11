#include "ImpostorBakerUtilities.h"
#include "ImpostorData/ImpostorData.h"
#include "Managers/ImpostorProceduralMeshManager.h"

FVector FImpostorBakerUtilities::OctahedronToUnitVector(const FVector2D& Octahedron)
{
	const float Z = 1.f - Octahedron.GetAbs().Dot(FVector2D::One());

	if (Z < 0.f)
	{
		const float X = Octahedron.X >= 0.f ? 1.f : -1.f;
		const float Y = Octahedron.Y >= 0.f ? 1.f : -1.f;

		return FVector(X * (1.f - FMath::Abs(Octahedron.Y)), Y * (1.f - FMath::Abs(Octahedron.X)), Z).GetSafeNormal();
	}

	return FVector(Octahedron.X, Octahedron.Y, Z).GetSafeNormal();
}

FVector FImpostorBakerUtilities::HemiOctahedronToUnitVector(const FVector2D& HemiOctahedron)
{
	FVector2D Octahedron(HemiOctahedron.X + HemiOctahedron.Y, HemiOctahedron.X - HemiOctahedron.Y);
	Octahedron *= 0.5f;

	return FVector(Octahedron.X, Octahedron.Y, 1.f - Octahedron.GetAbs().Dot(FVector2D::One())).GetSafeNormal();
}

FVector FImpostorBakerUtilities::GetGridVector(const int32 X, const int32 Y, const int32 Size, const EImpostorLayoutType Type)
{
	FVector2D Octahedron(float(X) / FMath::Max(1.f, float(Size - 1)), float(Y) / FMath::Max(1.f, float(Size - 1)));
	Octahedron *= 2.f;
	Octahedron = Octahedron - 1.f;

	switch (Type)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView: return OctahedronToUnitVector(Octahedron);
	case EImpostorLayoutType::UpperHemisphereOnly:
	case EImpostorLayoutType::TraditionalBillboards: return HemiOctahedronToUnitVector(Octahedron);
	}
}

int32 FImpostorBakerUtilities::GetImpostorTypeResolution(const EImpostorLayoutType Type)
{
	switch (Type)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView: return 16;
	case EImpostorLayoutType::UpperHemisphereOnly: return 16;
	case EImpostorLayoutType::TraditionalBillboards: return 48;
	}
}

bool FImpostorBakerUtilities::FindIntersection(const int32 CornerPass, const FImpostorTextureData& TextureData, const EImpostorLayoutType Type, const int32 StartX, int32& OutX, int32& OutY)
{
	OutX = StartX;
	OutY = 0;

	for (int32 Index = 0; Index < 16; Index++)
	{
		FVector2D Z = GetRotatedCoordsByCorner(FVector2D(OutX, OutY), 16, false, CornerPass);
		if (!FMath::IsNearlyZero(TextureData.GetAlpha(Z)))
		{
			return true;
		}

		OutY++;
	}

	return false;
}

FVector2D FImpostorBakerUtilities::GetRotatedCoordsByCorner(const FVector2D& XY, int32 Size, bool bVector, int32 Corner)
{
	FVector2D Degrees0;
	FVector2D Degrees90;
	FVector2D Degrees180;
	FVector2D Degrees270;
	GetRotatedCoords(XY, Size, bVector, Degrees0, Degrees90, Degrees180, Degrees270);

	switch (Corner)
	{
	default: check(false);
	case 0: return Degrees0;
	case 1: return Degrees90;
	case 2: return Degrees180;
	case 3: return Degrees270;
	}
}

void FImpostorBakerUtilities::GetRotatedCoords(const FVector2D& XY, const int32 Size, const bool bVector, FVector2D& Out0, FVector2D& Out90, FVector2D& Out180, FVector2D& Out270)
{
	const float X = bVector ? (XY.X * -1.f) : (float(Size - 1) - XY.X);
	const float Y = bVector ? (XY.Y * -1.f) : (float(Size - 1) - XY.Y);

	Out0 = XY;
	Out90 = FVector2D(Y, XY.X);
	Out180 = FVector2D(X, Y);
	Out270 = FVector2D(XY.Y, X);
}

void FImpostorBakerUtilities::DeriveAxes(const FVector& Vector, FVector& OutX, FVector& OutY, FVector& OutZ)
{
	const FRotationMatrix Rotator((Vector * -1.f).ToOrientationRotator());
	FVector X, Y, Z;
	Rotator.GetScaledAxes(X, Y, Z);
	OutX = Y;
	OutY = Z * -1.f;
	OutZ = X;
}

FVector FImpostorBakerUtilities::DeriveAxis(const FVector& Vector, const EAxis::Type Axis)
{
	const FRotationMatrix Rotator((Vector * -1.f).ToOrientationRotator());

	switch (Axis)
	{
	default: check(false);
	case EAxis::X: return Rotator.GetScaledAxis(EAxis::Y);
	case EAxis::Y: return Rotator.GetScaledAxis(EAxis::Z) * -1.f;
	case EAxis::Z: return Rotator.GetScaledAxis(EAxis::X);
	}
}