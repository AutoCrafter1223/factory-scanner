#include "ItemScannerWidget.h"

#include "ItemScannerManager.h"
#include "ItemScannerDirectionWidget.h"
#include "ItemScannerItemOptionWidget.h"
#include "ItemScannerSettings.h"
#include "ItemScannerTrackingManager.h"
#include "ItemScannerWorldSubsystem.h"
#include "ItemScannerCatalog.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "FGRecipe.h"
#include "FGRecipeManager.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "ItemAmount.h"
#include "Rendering/SlateRenderer.h"
#include "Resources/FGItemDescriptor.h"
#include "Styling/CoreStyle.h"

namespace
{
    const FLinearColor PanelColor(0.015f, 0.025f, 0.035f, 0.96f);
    const FLinearColor InnerColor(0.035f, 0.055f, 0.07f, 0.94f);
    const FLinearColor AccentColor(0.1f, 0.85f, 0.95f, 1.0f);
    const FLinearColor MutedColor(0.55f, 0.65f, 0.7f, 1.0f);

    UTextBlock* MakeText(UWidgetTree* Tree, const FText& Text, int32 Size = 15, const FLinearColor& Color = FLinearColor::White)
    {
        UTextBlock* Block = Tree->ConstructWidget<UTextBlock>();
        Block->SetText(Text);
        Block->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size));
        Block->SetColorAndOpacity(FSlateColor(Color));
        return Block;
    }

    void AddSectionLabel(UWidgetTree* Tree, UVerticalBox* Box, const FText& Label)
    {
        UTextBlock* Text = MakeText(Tree, Label, 12, AccentColor);
        UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Text);
        Slot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 3.0f));
    }

    UButton* MakeTextButton(UWidgetTree* Tree, const FText& Label, int32 FontSize = 15)
    {
        UButton* Button = Tree->ConstructWidget<UButton>();
        Button->SetBackgroundColor(AccentColor);
        UTextBlock* Text = MakeText(Tree, Label, FontSize, FLinearColor(0.0f, 0.08f, 0.1f, 1.0f));
        Button->AddChild(Text);
        return Button;
    }

}

void UItemScannerWidget::InitializeScanner(
    UItemScannerManager* InScannerManager,
    UItemScannerTrackingManager* InTrackingManager,
    UItemScannerWorldSubsystem* InHostSubsystem,
    APlayerController* InPlayerController)
{
    ScannerManager = InScannerManager;
    TrackingManager = InTrackingManager;
    HostSubsystem = InHostSubsystem;
    PlayerController = InPlayerController;
}

TSharedRef<SWidget> UItemScannerWidget::RebuildWidget()
{
    if (!IsValid(WidgetTree))
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("ItemScannerWidgetTree"));
    }
    if (!IsValid(WidgetTree->RootWidget))
    {
        BuildInterface();
    }
    return Super::RebuildWidget();
}

void UItemScannerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);

    if (IsValid(TrackingManager))
    {
        TrackingManager->OnTrackingUpdated.AddUObject(this, &UItemScannerWidget::HandleTrackingUpdated);
        TrackingManager->OnTrackingFinished.AddUObject(this, &UItemScannerWidget::HandleTrackingFinished);
    }
}

void UItemScannerWidget::PrepareForOpen()
{
    if (bItemCatalogBuilt)
    {
        return;
    }

    if (!BuildItemCatalogFromRecipes())
    {
        SetStatus(NSLOCTEXT("ItemScanner", "CatalogNotReady", "Item catalog is still loading. Reopen the scanner shortly."),
            FLinearColor(1.0f, 0.75f, 0.25f, 1.0f));
        return;
    }

    const FString Filter = IsValid(SearchBox) ? SearchBox->GetText().ToString() : FString();
    RefreshItemOptions(Filter);
    SetStatus(FText::Format(
        NSLOCTEXT("ItemScanner", "CatalogReady", "Loaded {0} items from {1} vanilla/mod recipes."),
        FText::AsNumber(AvailableItems.Num()),
        FText::AsNumber(CachedRecipeCount)), MutedColor);
}

void UItemScannerWidget::NativeDestruct()
{
    if (IsValid(TrackingManager))
    {
        TrackingManager->OnTrackingUpdated.RemoveAll(this);
        TrackingManager->OnTrackingFinished.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UItemScannerWidget::BuildInterface()
{
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    UBorder* MainBorder = WidgetTree->ConstructWidget<UBorder>();
    MainBorder->SetBrushColor(PanelColor);
    MainBorder->SetPadding(FMargin(18.0f));
    UCanvasPanelSlot* MainSlot = Root->AddChildToCanvas(MainBorder);
    MainSlot->SetPosition(FVector2D(40.0f, 70.0f));
    MainSlot->SetSize(FVector2D(640.0f, 800.0f));

    UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>();
    MainBorder->AddChild(MainBox);

    UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    MainBox->AddChildToVerticalBox(TitleRow);
    UTextBlock* Title = MakeText(WidgetTree, NSLOCTEXT("ItemScanner", "Title", "ITEM SCANNER"), 24, AccentColor);
    UHorizontalBoxSlot* TitleSlot = TitleRow->AddChildToHorizontalBox(Title);
    TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    TitleRow->AddChildToHorizontalBox(MakeText(WidgetTree, FText::FromString(TEXT("F7")), 12, MutedColor));
    UButton* CloseButton = MakeTextButton(WidgetTree, FText::FromString(TEXT("×")), 16);
    CloseButton->OnClicked.AddDynamic(this, &UItemScannerWidget::HandleCloseClicked);
    UHorizontalBoxSlot* CloseSlot = TitleRow->AddChildToHorizontalBox(CloseButton);
    CloseSlot->SetPadding(FMargin(12.0f, 0.0f, 0.0f, 0.0f));

    AddSectionLabel(WidgetTree, MainBox, NSLOCTEXT("ItemScanner", "TargetItem", "TARGET ITEM"));
    SearchBox = WidgetTree->ConstructWidget<UEditableTextBox>();
    SearchBox->SetHintText(NSLOCTEXT("ItemScanner", "SearchHint", "Search item name..."));
    SearchBox->OnTextChanged.AddDynamic(this, &UItemScannerWidget::HandleSearchTextChanged);
    MainBox->AddChildToVerticalBox(SearchBox)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 5.0f));

    ItemComboBox = WidgetTree->ConstructWidget<UComboBoxString>();
    ItemComboBox->OnGenerateWidgetEvent.BindDynamic(this, &UItemScannerWidget::HandleGenerateItemWidget);
    ItemComboBox->OnSelectionChanged.AddDynamic(this, &UItemScannerWidget::HandleItemSelectionChanged);
    MainBox->AddChildToVerticalBox(ItemComboBox);

    AddSectionLabel(WidgetTree, MainBox, NSLOCTEXT("ItemScanner", "Categories", "OBJECT CATEGORIES"));
    UHorizontalBox* CategoriesRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    MainBox->AddChildToVerticalBox(CategoriesRow);

    auto AddCategory = [this, CategoriesRow](const FText& Label, TObjectPtr<UCheckBox>& OutCheckBox)
    {
        UHorizontalBox* Pair = WidgetTree->ConstructWidget<UHorizontalBox>();
        OutCheckBox = WidgetTree->ConstructWidget<UCheckBox>();
        OutCheckBox->SetIsChecked(true);
        Pair->AddChildToHorizontalBox(OutCheckBox);
        UHorizontalBoxSlot* TextSlot = Pair->AddChildToHorizontalBox(MakeText(WidgetTree, Label, 13));
        TextSlot->SetPadding(FMargin(4.0f, 0.0f, 0.0f, 0.0f));
        UHorizontalBoxSlot* PairSlot = CategoriesRow->AddChildToHorizontalBox(Pair);
        PairSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        PairSlot->SetPadding(FMargin(0.0f, 2.0f));
    };

    AddCategory(NSLOCTEXT("ItemScanner", "Storage", "Storage"), StorageCheckBox);
    AddCategory(NSLOCTEXT("ItemScanner", "Production", "Production"), ProductionCheckBox);
    AddCategory(NSLOCTEXT("ItemScanner", "Conveyor", "Conveyor"), ConveyorCheckBox);
    AddCategory(NSLOCTEXT("ItemScanner", "Logistics", "Logistics"), LogisticsCheckBox);

    AddSectionLabel(WidgetTree, MainBox, NSLOCTEXT("ItemScanner", "Range", "SCAN RANGE"));
    UHorizontalBox* RangeRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    MainBox->AddChildToVerticalBox(RangeRow);
    RangeSpinBox = WidgetTree->ConstructWidget<USpinBox>();
    const UItemScannerSettings* Settings = GetDefault<UItemScannerSettings>();
    const float MaxRangeMeters = FMath::Max(1.0f, Settings->MaxScanRangeMeters);
    RangeSpinBox->SetMinValue(1.0f);
    RangeSpinBox->SetMaxValue(MaxRangeMeters);
    RangeSpinBox->SetMinSliderValue(1.0f);
    RangeSpinBox->SetMaxSliderValue(MaxRangeMeters);
    RangeSpinBox->SetDelta(10.0f);
    RangeSpinBox->SetValue(FMath::Clamp(Settings->DefaultScanRangeMeters, 1.0f, MaxRangeMeters));
    RangeSpinBox->SetMinDesiredWidth(160.0f);
    RangeRow->AddChildToHorizontalBox(RangeSpinBox);
    UHorizontalBoxSlot* MeterSlot = RangeRow->AddChildToHorizontalBox(MakeText(WidgetTree, FText::FromString(TEXT("m")), 15));
    MeterSlot->SetPadding(FMargin(6.0f, 4.0f, 0.0f, 0.0f));

    UButton* ScanButton = MakeTextButton(WidgetTree, NSLOCTEXT("ItemScanner", "Scan", "SCAN"), 18);
    ScanButton->OnClicked.AddDynamic(this, &UItemScannerWidget::HandleScanClicked);
    UVerticalBoxSlot* ScanSlot = MainBox->AddChildToVerticalBox(ScanButton);
    ScanSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 6.0f));

    StatusText = MakeText(WidgetTree, NSLOCTEXT("ItemScanner", "Ready", "Select an item, then scan."), 13, MutedColor);
    MainBox->AddChildToVerticalBox(StatusText);

    ResultsBorder = WidgetTree->ConstructWidget<UBorder>();
    ResultsBorder->SetBrushColor(InnerColor);
    ResultsBorder->SetPadding(FMargin(10.0f));
    ResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
    UVerticalBoxSlot* ResultsSlot = MainBox->AddChildToVerticalBox(ResultsBorder);
    ResultsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ResultsSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));

    UVerticalBox* ResultsContainer = WidgetTree->ConstructWidget<UVerticalBox>();
    ResultsBorder->AddChild(ResultsContainer);
    UHorizontalBox* ResultsHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
    ResultsContainer->AddChildToVerticalBox(ResultsHeader);
    SummaryText = MakeText(WidgetTree, FText::GetEmpty(), 13, AccentColor);
    UHorizontalBoxSlot* SummarySlot = ResultsHeader->AddChildToHorizontalBox(SummaryText);
    SummarySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    ResultsScrollBox = WidgetTree->ConstructWidget<UScrollBox>();
    UVerticalBoxSlot* ScrollSlot = ResultsContainer->AddChildToVerticalBox(ResultsScrollBox);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));

}

bool UItemScannerWidget::BuildItemCatalogFromRecipes()
{
    UItemScannerWorldSubsystem* WorldScanner = GetWorld() ? GetWorld()->GetSubsystem<UItemScannerWorldSubsystem>() : nullptr;
    UItemScannerCatalog* Catalog = WorldScanner ? WorldScanner->GetCatalog() : nullptr;
    if (!Catalog || !Catalog->EnsureBuilt(this)) return false;
    AvailableItems = Catalog->GetItems();
    SafeItemIcons.Reset();

    // Validate and retain texture assets. The option's UUserWidget separately
    // protects the UImage and its brush while Slate is displaying the row.
    FSlateRenderer* SlateRenderer = FSlateApplication::IsInitialized()
        ? FSlateApplication::Get().GetRenderer()
        : nullptr;
    if (SlateRenderer != nullptr)
    {
        for (const TSubclassOf<UFGItemDescriptor>& ItemClass : AvailableItems)
        {
            UTexture2D* IconTexture = ItemClass ? UFGItemDescriptor::GetSmallIcon(ItemClass) : nullptr;
            if (IsValid(IconTexture) && IconTexture->GetResource() != nullptr &&
                SlateRenderer->CanRenderResource(*IconTexture))
            {
                SafeItemIcons.Add(ItemClass, IconTexture);
            }
        }
    }

    const AFGRecipeManager* RecipeManager = AFGRecipeManager::Get(this);
    CachedRecipeCount = RecipeManager ? RecipeManager->GetAllRecipes().Num() : 0;
    bItemCatalogBuilt = !AvailableItems.IsEmpty();
    return bItemCatalogBuilt;
}

void UItemScannerWidget::RefreshItemOptions(const FString& Filter)
{
    if (!IsValid(ItemComboBox))
    {
        return;
    }

    const FString NormalizedFilter = Filter.TrimStartAndEnd().ToLower();
    const FString PreviousSelection = SelectedItemClass ? SelectedItemClass->GetPathName() : FString();
    ItemComboBox->ClearOptions();

    for (const TSubclassOf<UFGItemDescriptor>& ItemClass : AvailableItems)
    {
        if (!ItemClass)
        {
            continue;
        }
        const FString DisplayName = UFGItemDescriptor::GetItemName(ItemClass).ToString();
        if (NormalizedFilter.IsEmpty() || DisplayName.ToLower().Contains(NormalizedFilter))
        {
            ItemComboBox->AddOption(ItemClass->GetPathName());
        }
    }

    if (!PreviousSelection.IsEmpty() && ItemComboBox->FindOptionIndex(PreviousSelection) != INDEX_NONE)
    {
        ItemComboBox->SetSelectedOption(PreviousSelection);
    }
    else if (!Filter.Len() && ItemComboBox->GetOptionCount() > 0)
    {
        ItemComboBox->SetSelectedIndex(0);
    }
}

void UItemScannerWidget::BuildResultRows()
{
    ResultsScrollBox->ClearChildren();
    ResultRows.Reset();

    if (!IsValid(ScannerManager))
    {
        return;
    }

    const TArray<FItemScannerResult>& Results = ScannerManager->GetCachedResults();
    if (Results.IsEmpty())
    {
        const int32 Range = FMath::RoundToInt(ScannerManager->GetLastRequest().RangeMeters);
        UTextBlock* EmptyText = MakeText(
            WidgetTree,
            FText::Format(NSLOCTEXT("ItemScanner", "NoMatches", "No matching items found within {0}m."), FText::AsNumber(Range)),
            15,
            FLinearColor(1.0f, 0.75f, 0.25f, 1.0f));
        ResultsScrollBox->AddChild(EmptyText);
        return;
    }

    for (int32 Index = 0; Index < Results.Num(); ++Index)
    {
        const FItemScannerResult& Result = Results[Index];
        UBorder* RowBorder = WidgetTree->ConstructWidget<UBorder>();
        RowBorder->SetBrushColor(Index % 2 == 0 ? FLinearColor(0.06f, 0.09f, 0.11f, 0.9f) : FLinearColor(0.04f, 0.07f, 0.09f, 0.9f));
        RowBorder->SetPadding(FMargin(8.0f, 6.0f));
        ResultsScrollBox->AddChild(RowBorder);

        UVerticalBox* RowBox = WidgetTree->ConstructWidget<UVerticalBox>();
        RowBorder->AddChild(RowBox);

        FResultRowWidgets& Row = ResultRows.AddDefaulted_GetRef();
        UHorizontalBox* NameRow = WidgetTree->ConstructWidget<UHorizontalBox>();
        RowBox->AddChildToVerticalBox(NameRow);
        const FString Prefix = FString::Printf(TEXT("%d  "), Index + 1);
        NameRow->AddChildToHorizontalBox(MakeText(WidgetTree, FText::FromString(Prefix), 15, AccentColor));
        Row.NameText = MakeText(WidgetTree, Result.ObjectDisplayName, 15);
        NameRow->AddChildToHorizontalBox(Row.NameText.Get());

        UHorizontalBox* TrackingRow = WidgetTree->ConstructWidget<UHorizontalBox>();
        UVerticalBoxSlot* TrackingSlot = RowBox->AddChildToVerticalBox(TrackingRow);
        TrackingSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));

        USizeBox* ArrowSize = WidgetTree->ConstructWidget<USizeBox>();
        ArrowSize->SetWidthOverride(32.0f);
        ArrowSize->SetHeightOverride(32.0f);
        Row.DirectionArrow = WidgetTree->ConstructWidget<UItemScannerDirectionWidget>();
        Row.DirectionArrow->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        ArrowSize->AddChild(Row.DirectionArrow.Get());
        TrackingRow->AddChildToHorizontalBox(ArrowSize);

        Row.DistanceText = MakeText(WidgetTree, FText::GetEmpty(), 14);
        UHorizontalBoxSlot* DistanceSlot = TrackingRow->AddChildToHorizontalBox(Row.DistanceText.Get());
        DistanceSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
        DistanceSlot->SetVerticalAlignment(VAlign_Center);
        DistanceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

        Row.HeightText = MakeText(WidgetTree, FText::GetEmpty(), 14);
        UHorizontalBoxSlot* HeightSlot = TrackingRow->AddChildToHorizontalBox(Row.HeightText.Get());
        HeightSlot->SetPadding(FMargin(14.0f, 0.0f, 0.0f, 0.0f));
        HeightSlot->SetVerticalAlignment(VAlign_Center);
        HeightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

        Row.AmountText = MakeText(WidgetTree, FText::GetEmpty(), 13, MutedColor);
        UVerticalBoxSlot* AmountSlot = RowBox->AddChildToVerticalBox(Row.AmountText.Get());
        AmountSlot->SetPadding(FMargin(28.0f, 2.0f, 0.0f, 0.0f));
    }

    RefreshResultValues();
}

void UItemScannerWidget::RefreshResultValues()
{
    if (!IsValid(ScannerManager))
    {
        return;
    }

    const TArray<FItemScannerResult>& Results = ScannerManager->GetCachedResults();
    const int32 Count = FMath::Min(Results.Num(), ResultRows.Num());
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const FItemScannerResult& Result = Results[Index];
        FResultRowWidgets& Row = ResultRows[Index];

        if (!Row.NameText.IsValid() || !Row.DistanceText.IsValid() || !Row.HeightText.IsValid() ||
            !Row.AmountText.IsValid() || !Row.DirectionArrow.IsValid())
        {
            continue;
        }

        if (!Result.bIsValid)
        {
            Row.NameText->SetText(FText::Format(NSLOCTEXT("ItemScanner", "Removed", "{0} (removed)"), Result.ObjectDisplayName));
            Row.DistanceText->SetText(NSLOCTEXT("ItemScanner", "Invalid", "Invalid"));
            Row.HeightText->SetText(FText::GetEmpty());
            Row.DirectionArrow->SetVisibility(ESlateVisibility::Hidden);
            continue;
        }

        Row.NameText->SetText(Result.ObjectDisplayName);
        Row.DistanceText->SetText(FText::Format(
            NSLOCTEXT("ItemScanner", "Meters", "{0} m"),
            FText::AsNumber(FMath::RoundToInt(Result.CurrentDistanceMeters))));
        Row.DirectionArrow->SetVisibility(ESlateVisibility::Visible);
        Row.DirectionArrow->SetRenderTransformAngle(Result.RelativeYawDegrees);

        const int32 Height = FMath::RoundToInt(Result.HeightDifferenceMeters);
        if (Height > 0)
        {
            Row.HeightText->SetText(FText::FromString(FString::Printf(TEXT("Height: +%dm"), Height)));
        }
        else if (Height < 0)
        {
            Row.HeightText->SetText(FText::FromString(FString::Printf(TEXT("Height: %dm"), Height)));
        }
        else
        {
            Row.HeightText->SetText(FText::FromString(TEXT("Height: 0m")));
        }

        const FText ItemName = UFGItemDescriptor::GetItemName(Result.TargetItem);
        Row.AmountText->SetText(FText::Format(
            NSLOCTEXT("ItemScanner", "Amount", "{0} ×{1}"),
            ItemName,
            FText::AsNumber(Result.ItemAmount)));
    }

}

void UItemScannerWidget::SetStatus(const FText& Message, const FLinearColor& Color)
{
    if (IsValid(StatusText))
    {
        StatusText->SetText(Message);
        StatusText->SetColorAndOpacity(FSlateColor(Color));
    }
}

UTexture2D* UItemScannerWidget::GetCachedSafeItemIcon(TSubclassOf<UFGItemDescriptor> ItemClass) const
{
    if (const TObjectPtr<UTexture2D>* Found = SafeItemIcons.Find(ItemClass))
    {
        return IsValid(Found->Get()) ? Found->Get() : nullptr;
    }
    return nullptr;
}

TSubclassOf<UFGItemDescriptor> UItemScannerWidget::FindItemByPath(const FString& ClassPath) const
{
    for (const TSubclassOf<UFGItemDescriptor>& ItemClass : AvailableItems)
    {
        if (ItemClass && ItemClass->GetPathName() == ClassPath)
        {
            return ItemClass;
        }
    }
    return nullptr;
}

void UItemScannerWidget::HandleScanClicked()
{
    if (!IsValid(ScannerManager) || !IsValid(TrackingManager) || !PlayerController.IsValid())
    {
        SetStatus(NSLOCTEXT("ItemScanner", "Unavailable", "Scanner is not ready."), FLinearColor::Red);
        return;
    }

    FItemScannerScanRequest Request;
    Request.TargetItem = SelectedItemClass;
    Request.RangeMeters = RangeSpinBox->GetValue();
    Request.Categories.bStorage = StorageCheckBox->IsChecked();
    Request.Categories.bProduction = ProductionCheckBox->IsChecked();
    Request.Categories.bConveyor = ConveyorCheckBox->IsChecked();
    Request.Categories.bLogistics = LogisticsCheckBox->IsChecked();

    TrackingManager->StopTracking(true);
    FText Error;
    if (!ScannerManager->ExecuteScan(PlayerController.Get(), Request, Error))
    {
        ResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
        SetStatus(Error, FLinearColor(1.0f, 0.25f, 0.2f, 1.0f));
        return;
    }

    const FText TargetName = UFGItemDescriptor::GetItemName(SelectedItemClass);
    SummaryText->SetText(FText::Format(
        NSLOCTEXT("ItemScanner", "Summary", "TARGET: {0}   RANGE: {1}m"),
        TargetName,
        FText::AsNumber(FMath::RoundToInt(ScannerManager->GetLastRequest().RangeMeters))));
    ResultsBorder->SetVisibility(ESlateVisibility::Visible);
    BuildResultRows();
    TrackingManager->StartTracking(PlayerController.Get());

    const int32 MatchCount = ScannerManager->GetCachedResults().Num();
    SetStatus(FText::Format(NSLOCTEXT("ItemScanner", "Found", "Scan complete: {0} match(es)."), FText::AsNumber(MatchCount)), AccentColor);
}

void UItemScannerWidget::HandleCloseClicked()
{
    if (UItemScannerWorldSubsystem* Host = HostSubsystem.Get())
    {
        Host->SetScannerVisible(false);
    }
}

void UItemScannerWidget::HandleSearchTextChanged(const FText& Text)
{
    RefreshItemOptions(Text.ToString());
}

void UItemScannerWidget::HandleItemSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    const TSubclassOf<UFGItemDescriptor> Found = FindItemByPath(SelectedItem);
    if (Found)
    {
        SelectedItemClass = Found;
    }
}

UWidget* UItemScannerWidget::HandleGenerateItemWidget(FString Item)
{
    const TSubclassOf<UFGItemDescriptor> ItemClass = FindItemByPath(Item);
    // A plain panel returned here is only held by Slate and can be GC'd even
    // while its SImage still uses &UImage::Brush. Use the UUserWidget GC bridge.
    UItemScannerItemOptionWidget* Row = CreateWidget<UItemScannerItemOptionWidget>(this);
    if (IsValid(Row))
    {
        Row->SetItem(ItemClass ? UFGItemDescriptor::GetItemName(ItemClass) : FText::FromString(Item),
            GetCachedSafeItemIcon(ItemClass));
    }
    return Row;
}

void UItemScannerWidget::HandleTrackingUpdated()
{
    RefreshResultValues();
}

void UItemScannerWidget::HandleTrackingFinished()
{
    ClearScan();
}

void UItemScannerWidget::ClearScan()
{
    if (IsValid(TrackingManager))
    {
        TrackingManager->StopTracking(true);
    }
    ResultRows.Reset();
    if (IsValid(ResultsScrollBox))
    {
        ResultsScrollBox->ClearChildren();
    }
    if (IsValid(ResultsBorder))
    {
        ResultsBorder->SetVisibility(ESlateVisibility::Collapsed);
    }
    SetStatus(NSLOCTEXT("ItemScanner", "Ready", "Select an item, then scan."), MutedColor);
}
