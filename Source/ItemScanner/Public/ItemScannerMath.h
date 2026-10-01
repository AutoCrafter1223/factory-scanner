#pragma once

#include "CoreMinimal.h"

struct ITEMSCANNER_API FItemScannerMath
{
    static float CalculateRelativeYawDegrees(const FVector& PlayerLocation, float ViewYawDegrees, const FVector& TargetLocation)
    {
        const FVector Delta = TargetLocation - PlayerLocation;
        if (FMath::IsNearlyZero(Delta.X) && FMath::IsNearlyZero(Delta.Y))
        {
            return 0.0f;
        }

        const float TargetYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
        return FMath::FindDeltaAngleDegrees(ViewYawDegrees, TargetYaw);
    }

    static float CentimetersToMeters(float Centimeters)
    {
        return Centimeters / 100.0f;
    }
};
