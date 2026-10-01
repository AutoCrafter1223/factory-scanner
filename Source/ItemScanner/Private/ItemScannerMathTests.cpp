#include "ItemScannerMath.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FItemScannerRelativeYawTest,
    "ItemScanner.Math.RelativeYawIsContinuousAndViewRelative",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemScannerRelativeYawTest::RunTest(const FString& Parameters)
{
    const FVector Origin = FVector::ZeroVector;
    TestEqual(TEXT("Ahead is zero"), FItemScannerMath::CalculateRelativeYawDegrees(Origin, 0.0f, FVector(100.0f, 0.0f, 0.0f)), 0.0f);
    TestEqual(TEXT("Right is positive 90"), FItemScannerMath::CalculateRelativeYawDegrees(Origin, 0.0f, FVector(0.0f, 100.0f, 0.0f)), 90.0f);
    TestEqual(TEXT("Left is negative 90"), FItemScannerMath::CalculateRelativeYawDegrees(Origin, 0.0f, FVector(0.0f, -100.0f, 0.0f)), -90.0f);
    TestTrue(TEXT("Behind normalizes to 180"), FMath::IsNearlyEqual(FMath::Abs(FItemScannerMath::CalculateRelativeYawDegrees(Origin, 0.0f, FVector(-100.0f, 0.0f, 0.0f))), 180.0f));
    TestEqual(TEXT("View yaw is subtracted continuously"), FItemScannerMath::CalculateRelativeYawDegrees(Origin, 10.0f, FVector(100.0f, 100.0f, 0.0f)), 35.0f);
    return true;
}

#endif
