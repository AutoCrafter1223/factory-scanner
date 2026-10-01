#pragma once

#include "CoreMinimal.h"
#include "ItemScannerTypes.generated.h"

class UFGItemDescriptor;

UENUM(BlueprintType)
enum class EItemScannerMode : uint8 { ItemSearch, ConnectionCheck };

UENUM(BlueprintType)
enum class EItemScannerCategory : uint8
{
    Storage,
    Production,
    Conveyor,
    Logistics
};

USTRUCT(BlueprintType)
struct ITEMSCANNER_API FItemScannerCategorySelection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProduction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConveyor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLogistics = true;

    bool IsAnyEnabled() const
    {
        return bStorage || bProduction || bConveyor || bLogistics;
    }

    bool IsEnabled(EItemScannerCategory Category) const
    {
        switch (Category)
        {
        case EItemScannerCategory::Storage: return bStorage;
        case EItemScannerCategory::Production: return bProduction;
        case EItemScannerCategory::Conveyor: return bConveyor;
        case EItemScannerCategory::Logistics: return bLogistics;
        default: return false;
        }
    }
};

USTRUCT(BlueprintType)
struct ITEMSCANNER_API FItemScannerScanRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemScannerMode Mode = EItemScannerMode::ItemSearch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UFGItemDescriptor> TargetItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemScannerCategorySelection Categories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="m"))
    float RangeMeters = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="m"))
    float ConnectionGapMeters = 1.0f;
};

USTRUCT(BlueprintType)
struct ITEMSCANNER_API FItemScannerResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadOnly)
    EItemScannerCategory Category = EItemScannerCategory::Storage;

    UPROPERTY(BlueprintReadOnly)
    FText ObjectDisplayName;

    UPROPERTY(BlueprintReadOnly)
    FVector WorldPositionAtScan = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    TSubclassOf<UFGItemDescriptor> TargetItem;

    UPROPERTY(BlueprintReadOnly)
    int64 ItemAmount = 0;

    UPROPERTY(BlueprintReadOnly)
    float DistanceAtScanMeters = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CurrentDistanceMeters = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float RelativeYawDegrees = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float HeightDifferenceMeters = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid = true;

    UPROPERTY(BlueprintReadOnly)
    bool bConnectionCandidate = false;

    UPROPERTY(BlueprintReadOnly)
    bool bPipeline = false;

    UPROPERTY(BlueprintReadOnly)
    float GapCentimeters = 0.0f;

    UPROPERTY()
    TWeakObjectPtr<USceneComponent> ConnectionA;

    UPROPERTY()
    TWeakObjectPtr<USceneComponent> ConnectionB;
};
