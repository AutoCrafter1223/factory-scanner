#include "ItemScannerManager.h"

#include "ItemScannerMath.h"
#include "ItemScannerModule.h"
#include "ItemScannerSettings.h"
#include "ItemScannerLocalization.h"

#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildableDroneStation.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "EngineUtils.h"
#include "FGDroneVehicle.h"
#include "FGFreightWagon.h"
#include "FGInventoryComponent.h"
#include "FGVehicle.h"
#include "GameFramework/PlayerController.h"
#include "Resources/FGItemDescriptor.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

namespace
{
#if !UE_BUILD_SHIPPING
    int32 CategoryToIndex(EItemScannerCategory Category)
    {
        return static_cast<int32>(Category);
    }
#endif

    FString CleanClassDisplayName(const UClass* Class)
    {
        // GetDisplayNameText is editor-only. Use the generated class name so
        // this fallback remains available in Steam/Epic Shipping builds.
        FString Name = Class ? Class->GetName() : FString();
        Name.RemoveFromStart(TEXT("Build_"));
        Name.RemoveFromStart(TEXT("BP_"));
        Name.RemoveFromEnd(TEXT("_C"));
        Name.RemoveFromStart(TEXT("BP "));
        Name.RemoveFromStart(TEXT("Build "));
        Name.RemoveFromEnd(TEXT(" C"));
        Name.ReplaceInline(TEXT("_"), TEXT(" "));
        return Name;
    }
}

bool UItemScannerManager::ExecuteScan(APlayerController* PlayerController, const FItemScannerScanRequest& Request, FText& OutError)
{
    CachedResults.Reset();
    OutError = FText::GetEmpty();

    if (Request.Mode == EItemScannerMode::ConnectionCheck)
    {
        return ExecuteConnectionScan(PlayerController, Request, OutError);
    }

    if (!IsValid(PlayerController) || !IsValid(PlayerController->GetPawn()))
    {
        OutError = ItemScannerLocalization::Text(TEXT("Player is not ready."),TEXT("플레이어가 준비되지 않았습니다."));
        return false;
    }

    if (!Request.TargetItem)
    {
        OutError = ItemScannerLocalization::Text(TEXT("Select a target item first."),TEXT("먼저 대상 제품을 선택하세요."));
        return false;
    }

    if (!Request.Categories.IsAnyEnabled())
    {
        OutError = ItemScannerLocalization::Text(TEXT("Enable at least one object category."),TEXT("하나 이상의 검색 분류를 켜세요."));
        return false;
    }

    const UItemScannerSettings* Settings = GetDefault<UItemScannerSettings>();
    if (!FMath::IsFinite(Request.RangeMeters) || Request.RangeMeters <= 0.0f)
    {
        OutError = ItemScannerLocalization::Text(TEXT("Enter a scan range greater than zero."),TEXT("스캔 거리는 0보다 커야 합니다."));
        return false;
    }

    const float MaxRangeMeters = FMath::Max(1.0f, Settings->MaxScanRangeMeters);
    LastRequest = Request;
    LastRequest.RangeMeters = FMath::Clamp(Request.RangeMeters, 1.0f, MaxRangeMeters);

    UWorld* World = PlayerController->GetWorld();
    if (!IsValid(World))
    {
        OutError = ItemScannerLocalization::Text(TEXT("The game world is not ready."),TEXT("게임 월드가 준비되지 않았습니다."));
        return false;
    }

    const FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    const float RangeCentimeters = LastRequest.RangeMeters * 100.0f;
#if !UE_BUILD_SHIPPING
    const double StartSeconds = FPlatformTime::Seconds();
#endif
#if !UE_BUILD_SHIPPING
    int32 CandidateCounts[4] = {0, 0, 0, 0};
#endif

    // Exactly one world actor pass is performed per explicit scan. Tracking never enters this loop.
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
        {
            continue;
        }

        const FVector ActorLocation = Actor->GetActorLocation();
        const float DistanceCentimeters = FVector::Distance(PlayerLocation, ActorLocation);
        if (DistanceCentimeters > RangeCentimeters)
        {
            continue;
        }

        EItemScannerCategory Category;
        if (!TryClassifyActor(Actor, Category) || !LastRequest.Categories.IsEnabled(Category))
        {
            continue;
        }

#if !UE_BUILD_SHIPPING
        ++CandidateCounts[CategoryToIndex(Category)];
#endif
        const int64 Amount = Category == EItemScannerCategory::Conveyor
            ? CountItemOnConveyor(Actor, LastRequest.TargetItem)
            : CountItemInActor(Actor, LastRequest.TargetItem);

        if (Amount <= 0)
        {
            continue;
        }

        FItemScannerResult& Result = CachedResults.AddDefaulted_GetRef();
        Result.TargetActor = Actor;
        Result.Category = Category;
        Result.ObjectDisplayName = ResolveActorDisplayName(Actor);
        Result.WorldPositionAtScan = ActorLocation;
        Result.TargetItem = LastRequest.TargetItem;
        Result.ItemAmount = Amount;
        Result.DistanceAtScanMeters = FItemScannerMath::CentimetersToMeters(DistanceCentimeters);
        Result.CurrentDistanceMeters = Result.DistanceAtScanMeters;
        Result.HeightDifferenceMeters = FItemScannerMath::CentimetersToMeters(ActorLocation.Z - PlayerLocation.Z);
        Result.bIsValid = true;
    }

    CachedResults.StableSort([](const FItemScannerResult& Left, const FItemScannerResult& Right)
    {
        return Left.DistanceAtScanMeters < Right.DistanceAtScanMeters;
    });

#if !UE_BUILD_SHIPPING
    const double ElapsedMilliseconds = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
    const FText TargetName = UFGItemDescriptor::GetItemName(LastRequest.TargetItem);
    UE_LOG(LogItemScanner, Log,
        TEXT("[ItemScanner]\nTarget = %s\nRange = %.0fm\nStorage Candidates = %d\nProduction Candidates = %d\nConveyor Candidates = %d\nLogistics Candidates = %d\nMatches = %d\nScan Time = %.2fms"),
        *TargetName.ToString(), LastRequest.RangeMeters,
        CandidateCounts[0], CandidateCounts[1], CandidateCounts[2], CandidateCounts[3],
        CachedResults.Num(), ElapsedMilliseconds);
#else
    UE_LOG(LogItemScanner, Log, TEXT("[ItemScanner] Scan complete: %d match(es)."), CachedResults.Num());
#endif

    OnScanCompleted.Broadcast(CachedResults);
    OnScanCompletedBP.Broadcast();
    return true;
}

void UItemScannerManager::ClearResults()
{
    CachedResults.Reset();
}

bool UItemScannerManager::TryClassifyActor(AActor* Actor, EItemScannerCategory& OutCategory)
{
    if (Actor->IsA<AFGBuildableConveyorBase>())
    {
        OutCategory = EItemScannerCategory::Conveyor;
        return true;
    }

    if (Actor->IsA<AFGBuildableDockingStation>() ||
        Actor->IsA<AFGBuildableTrainPlatformCargo>() ||
        Actor->IsA<AFGBuildableDroneStation>() ||
        Actor->IsA<AFGVehicle>())
    {
        OutCategory = EItemScannerCategory::Logistics;
        return true;
    }

    if (Actor->IsA<AFGBuildableStorage>())
    {
        OutCategory = EItemScannerCategory::Storage;
        return true;
    }

    if (Actor->IsA<AFGBuildableManufacturer>())
    {
        OutCategory = EItemScannerCategory::Production;
        return true;
    }

    // Compatibility fallbacks for mods using the normal FactoryGame inheritance and inventory components.
    TArray<UFGInventoryComponent*> InventoryComponents;
    Actor->GetComponents<UFGInventoryComponent>(InventoryComponents);
    if (InventoryComponents.IsEmpty())
    {
        return false;
    }

    if (Actor->IsA<AFGBuildableFactory>())
    {
        OutCategory = EItemScannerCategory::Production;
        return true;
    }

    if (Actor->IsA<AFGBuildable>())
    {
        OutCategory = EItemScannerCategory::Storage;
        return true;
    }

    return false;
}

void UItemScannerManager::CollectInventories(AActor* Actor, TSet<UFGInventoryComponent*>& OutInventories)
{
    TArray<UFGInventoryComponent*> Components;
    Actor->GetComponents<UFGInventoryComponent>(Components);
    for (UFGInventoryComponent* Inventory : Components)
    {
        if (IsValid(Inventory))
        {
            OutInventories.Add(Inventory);
        }
    }

    auto AddInventory = [&OutInventories](UFGInventoryComponent* Inventory)
    {
        if (IsValid(Inventory))
        {
            OutInventories.Add(Inventory);
        }
    };

    if (AFGBuildableStorage* Storage = Cast<AFGBuildableStorage>(Actor))
    {
        AddInventory(Storage->GetStorageInventory());
    }
    if (AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Actor))
    {
        AddInventory(Manufacturer->GetInputInventory());
        AddInventory(Manufacturer->GetOutputInventory());
    }
    if (AFGBuildableDockingStation* Station = Cast<AFGBuildableDockingStation>(Actor))
    {
        AddInventory(Station->GetInventory());
        AddInventory(Station->GetFuelInventory());
    }
    if (AFGBuildableTrainPlatformCargo* CargoPlatform = Cast<AFGBuildableTrainPlatformCargo>(Actor))
    {
        AddInventory(CargoPlatform->GetInventory());
    }
    if (AFGBuildableDroneStation* DroneStation = Cast<AFGBuildableDroneStation>(Actor))
    {
        AddInventory(DroneStation->GetInputInventory());
        AddInventory(DroneStation->GetOutputInventory());
        AddInventory(DroneStation->GetFuelInventory());
    }
    if (AFGWheeledVehicle* WheeledVehicle = Cast<AFGWheeledVehicle>(Actor))
    {
        AddInventory(WheeledVehicle->GetStorageInventory());
        AddInventory(WheeledVehicle->GetFuelInventory());
    }
    if (AFGDroneVehicle* Drone = Cast<AFGDroneVehicle>(Actor))
    {
        AddInventory(Drone->GetStorageInventory());
    }
    if (AFGFreightWagon* FreightWagon = Cast<AFGFreightWagon>(Actor))
    {
        AddInventory(FreightWagon->GetFreightInventory());
    }
}

int64 UItemScannerManager::CountItemInActor(AActor* Actor, TSubclassOf<UFGItemDescriptor> TargetItem)
{
    TSet<UFGInventoryComponent*> UniqueInventories;
    CollectInventories(Actor, UniqueInventories);

    int64 Total = 0;
    for (UFGInventoryComponent* Inventory : UniqueInventories)
    {
        Total += Inventory->GetNumItems(TargetItem);
    }
    return Total;
}

int64 UItemScannerManager::CountItemOnConveyor(AActor* Actor, TSubclassOf<UFGItemDescriptor> TargetItem)
{
    AFGBuildableConveyorBase* Conveyor = Cast<AFGBuildableConveyorBase>(Actor);
    if (!IsValid(Conveyor))
    {
        return 0;
    }

    TArray<FConveyorBeltItem*> Items;
    Conveyor->GetConveyorBeltItems(Items);

    int64 Total = 0;
    for (const FConveyorBeltItem* Item : Items)
    {
        if (Item != nullptr && Item->Item.IsValid() && Item->Item.GetItemClass() == TargetItem)
        {
            ++Total;
        }
    }
    return Total;
}

FText UItemScannerManager::ResolveActorDisplayName(AActor* Actor)
{
    if (const AFGBuildable* Buildable = Cast<AFGBuildable>(Actor))
    {
        const TSubclassOf<UFGItemDescriptor> Descriptor = Buildable->GetBuiltWithDescriptor();
        if (Descriptor)
        {
            const FText ItemName = UFGItemDescriptor::GetItemName(Descriptor);
            if (!ItemName.IsEmpty())
            {
                return ItemName;
            }
        }
    }

    if (const AFGVehicle* Vehicle = Cast<AFGVehicle>(Actor))
    {
        const TSubclassOf<UFGItemDescriptor> Descriptor = Vehicle->GetBuiltWithDescriptor();
        if (Descriptor)
        {
            const FText ItemName = UFGItemDescriptor::GetItemName(Descriptor);
            if (!ItemName.IsEmpty())
            {
                return ItemName;
            }
        }
    }

    return FText::FromString(CleanClassDisplayName(Actor->GetClass()));
}
