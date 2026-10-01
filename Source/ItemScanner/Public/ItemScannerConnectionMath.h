#pragma once
#include "CoreMinimal.h"

/** Conservative geometry test. A candidate is a facing, compatible pair, never a lone unused port. */
struct FItemScannerConnectionMath
{
    static bool IsFacingGap(const FVector& A, const FVector& ANormal, const FVector& B, const FVector& BNormal, float MaxGap)
    {
        const FVector Delta = B - A;
        const double DistanceSquared = Delta.SizeSquared();
        if (!FMath::IsFinite(DistanceSquared) || DistanceSquared > FMath::Square(MaxGap) ||
            FVector::DotProduct(ANormal, BNormal) > -0.95) return false;
        if (DistanceSquared < 0.25) return true;
        const double Distance = FMath::Sqrt(DistanceSquared);
        const FVector Direction = Delta / Distance;
        return FVector::DotProduct(Direction, ANormal) > 0.95 && FVector::DotProduct(-Direction, BNormal) > 0.95;
    }
};
