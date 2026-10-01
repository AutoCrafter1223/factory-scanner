#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemScannerSettings.generated.h"

UCLASS(Config=ItemScanner, DefaultConfig, meta=(DisplayName="Item Scanner"))
class ITEMSCANNER_API UItemScannerSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category="Debug")
    bool bEnableDebugHUD = false;

    UPROPERTY(Config, EditAnywhere, Category="Connections", meta=(ClampMin="50", ClampMax="200", Units="cm"))
    float ConnectionCandidateGapCm = 100.0f;

    UPROPERTY(Config, EditAnywhere, Category="Equipment")
    FVector FirstPersonOffset = FVector(3.0f, -9.0f, 10.0f);

    UPROPERTY(Config, EditAnywhere, Category="Equipment")
    FRotator FirstPersonRotation = FRotator(-3.0f, -11.0f, -4.0f);

    /** Separate first-person scale so the readable handheld view does not enlarge the world/third-person item. */
    UPROPERTY(Config, EditAnywhere, Category="Equipment", meta=(ClampMin="0.1", ClampMax="3.0"))
    float FirstPersonScale = 1.5f;

    UPROPERTY(Config, EditAnywhere, Category="Equipment", meta=(ClampMin="0.1", ClampMax="3.0"))
    float EquipmentScale = 1.0f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Scan", meta=(ClampMin="1.0", Units="m"))
    float MaxScanRangeMeters = 10000.0f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Scan", meta=(ClampMin="1.0", Units="m"))
    float DefaultScanRangeMeters = 500.0f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Tracking", meta=(ClampMin="0.01", Units="s"))
    float TrackingUpdateIntervalSeconds = 0.1f;
};
