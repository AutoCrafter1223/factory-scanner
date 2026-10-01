#include "ItemScannerModConfig.h"
#include "ItemScannerLocalization.h"

#include "Configuration/ConfigManager.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Engine/GameInstance.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    const FConfigId ItemScannerConfigId{TEXT("ItemScanner"), TEXT("")};

    void ConfigureNumericProperty(UConfigPropertySection* Root, UItemScannerAngleConfigProperty* Property,
        const TCHAR* InternalName, const TCHAR* DisplayName, const TCHAR* Tooltip, float DefaultValue,
        float MinimumValue, float MaximumValue, float StepSize = 1.0f)
    {
        Property->DisplayName = FText::FromString(DisplayName);
        Property->Tooltip = FText::FromString(Tooltip);
        Property->DefaultValue = DefaultValue;
        Property->Value = DefaultValue;
        Property->MinimumValue = MinimumValue;
        Property->MaximumValue = MaximumValue;
        Property->StepSize = StepSize;
        Property->bRequiresWorldReload = false;
        Root->SectionProperties.Add(InternalName, Property);
    }

    const UConfigPropertyFloat* FindFloat(const UObject* WorldContext, const TCHAR* Name)
    {
        const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
        UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
        UConfigManager* Manager = GameInstance ? GameInstance->GetSubsystem<UConfigManager>() : nullptr;
        UConfigPropertySection* Root = Manager ? Manager->GetConfigurationRootSection(ItemScannerConfigId) : nullptr;
        if (!Root) return nullptr;
        const TObjectPtr<UConfigProperty>* Found = Root->SectionProperties.Find(Name);
        return Found ? Cast<UConfigPropertyFloat>(*Found) : nullptr;
    }
}

UUserWidget* UItemScannerAngleConfigProperty::CreateEditorWidget_Implementation(UUserWidget* ParentWidget) const
{
    if (!ParentWidget) return nullptr;
    UItemScannerAngleConfigEditorWidget* Widget = CreateWidget<UItemScannerAngleConfigEditorWidget>(
        ParentWidget->GetOwningPlayer(), UItemScannerAngleConfigEditorWidget::StaticClass());
    if (!Widget) Widget = NewObject<UItemScannerAngleConfigEditorWidget>(ParentWidget);
    Widget->InitializeProperty(const_cast<UItemScannerAngleConfigProperty*>(this));
    return Widget;
}

UUserWidget* UItemScannerAngleConfigSection::CreateEditorWidget_Implementation(UUserWidget* ParentWidget) const
{
    if (!ParentWidget) return nullptr;
    UItemScannerAngleConfigSectionWidget* Widget = CreateWidget<UItemScannerAngleConfigSectionWidget>(
        ParentWidget->GetOwningPlayer(), UItemScannerAngleConfigSectionWidget::StaticClass());
    if (!Widget) Widget = NewObject<UItemScannerAngleConfigSectionWidget>(ParentWidget);
    Widget->InitializeSection(const_cast<UItemScannerAngleConfigSection*>(this));
    return Widget;
}

void UItemScannerAngleConfigSectionWidget::InitializeSection(UItemScannerAngleConfigSection* InSection)
{
    Section = InSection;
}

TSharedRef<SWidget> UItemScannerAngleConfigSectionWidget::RebuildWidget()
{
    TSharedRef<SVerticalBox> Layout = SNew(SVerticalBox);
    const TCHAR* OrderedNames[] = {
        TEXT("ObjectScannerPitch"), TEXT("ObjectScannerYaw"), TEXT("ObjectScannerRoll"),
        TEXT("ObjectScannerX"), TEXT("ObjectScannerY"), TEXT("ObjectScannerZ"), TEXT("ObjectScannerScale")};
    for (const TCHAR* Name : OrderedNames)
    {
        UItemScannerAngleConfigProperty* Property = Section
            ? Cast<UItemScannerAngleConfigProperty>(Section->SectionProperties.FindRef(Name).Get())
            : nullptr;
        if (!Property) continue;
        const TWeakObjectPtr<UItemScannerAngleConfigProperty> WeakProperty(Property);
        const TWeakObjectPtr<UItemScannerAngleConfigSection> WeakSection(Section);
        Layout->AddSlot().AutoHeight().Padding(0.0f, 3.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(12.0f, 8.0f))
            .BorderBackgroundColor(FLinearColor(0.04f, 0.06f, 0.06f, 0.78f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(Property->DisplayName)
                    .ToolTipText(Property->Tooltip)
                    .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14))
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SSpinBox<float>)
                    .MinValue(Property->MinimumValue)
                    .MaxValue(Property->MaximumValue)
                    .MinSliderValue(Property->MinimumValue)
                    .MaxSliderValue(Property->MaximumValue)
                    .Delta(Property->StepSize)
                    .MinDesiredWidth(180.0f)
                    .Value_Lambda([WeakProperty]() { return WeakProperty.IsValid() ? WeakProperty->Value : 0.0f; })
                    .OnValueChanged_Lambda([WeakProperty,WeakSection](float NewValue)
                    {
                        if (WeakProperty.IsValid() && !FMath::IsNearlyEqual(WeakProperty->Value, NewValue))
                        {
                            WeakProperty->Value = NewValue;
                            WeakProperty->MarkDirty();
                            if (WeakSection.IsValid()) WeakSection->MarkDirty();
                        }
                    })
                    .OnValueCommitted_Lambda([WeakProperty,WeakSection](float NewValue,ETextCommit::Type)
                    {
                        if (!WeakProperty.IsValid() || !WeakSection.IsValid()) return;
                        WeakProperty->Value=NewValue;
                        WeakSection->MarkDirty();
                        // Persist when the edit finishes, even while the game is paused.
                        if (auto* Manager=WeakSection->GetTypedOuter<UConfigManager>()) Manager->FlushPendingSaves();
                    })
                ]
            ]
        ];
    }
    return Layout;
}

void UItemScannerAngleConfigEditorWidget::InitializeProperty(UItemScannerAngleConfigProperty* InProperty)
{
    Property = InProperty;
}

TSharedRef<SWidget> UItemScannerAngleConfigEditorWidget::RebuildWidget()
{
    return SNew(SBorder)
        .Padding(FMargin(8.0f, 5.0f))
        .BorderBackgroundColor(FLinearColor(0.04f, 0.06f, 0.06f, 0.65f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                    .Text(Property ? Property->DisplayName : FText::GetEmpty())
                    .ToolTipText(Property ? Property->Tooltip : FText::GetEmpty())
                    .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12))
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SSpinBox<float>)
                    .MinValue(Property ? Property->MinimumValue : -45.0f)
                    .MaxValue(Property ? Property->MaximumValue : 45.0f)
                    .MinSliderValue(Property ? Property->MinimumValue : -45.0f)
                    .MaxSliderValue(Property ? Property->MaximumValue : 45.0f)
                    .Delta(Property ? Property->StepSize : 1.0f)
                    .MinDesiredWidth(150.0f)
                    .Value_Lambda([this]() { return Property ? Property->Value : 0.0f; })
                    .OnValueChanged_Lambda([this](float NewValue)
                    {
                        if (Property && !FMath::IsNearlyEqual(Property->Value, NewValue))
                        {
                            Property->Value = NewValue;
                            Property->MarkDirty();
                        }
                    })
            ]
        ];
}

UItemScannerModConfiguration::UItemScannerModConfiguration()
{
    ConfigId = ItemScannerConfigId;
    DisplayName = ItemScannerLocalization::Text(TEXT("Equipment Transform"),TEXT("장비 위치 조정"));
    Description = ItemScannerLocalization::Text(
        TEXT("Adjust the first-person scanner transform. Changes apply immediately."),
        TEXT("1인칭 스캐너의 위치와 회전을 조정합니다. 변경 사항은 즉시 적용됩니다."));

    RootSection = CreateDefaultSubobject<UItemScannerAngleConfigSection>(TEXT("RootSection"));
    RootSection->DisplayName = DisplayName;
}

UItemScannerAngleConfigSection::UItemScannerAngleConfigSection()
{
    // SML instances the section, not the configuration object. Its numeric
    // children must therefore belong to the section to be cloned and saved.
    auto* RootSection=this;
    UItemScannerAngleConfigProperty* Pitch = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerPitch"));
    UItemScannerAngleConfigProperty* Yaw = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerYaw"));
    UItemScannerAngleConfigProperty* Roll = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerRoll"));
    UItemScannerAngleConfigProperty* X = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerX"));
    UItemScannerAngleConfigProperty* Y = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerY"));
    UItemScannerAngleConfigProperty* Z = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerZ"));
    UItemScannerAngleConfigProperty* Scale = CreateDefaultSubobject<UItemScannerAngleConfigProperty>(TEXT("ObjectScannerScale"));
    ConfigureNumericProperty(RootSection, Pitch, TEXT("ObjectScannerPitch"), TEXT("Pitch"), TEXT("Pitch"), -3.0f, -180.0f, 180.0f);
    ConfigureNumericProperty(RootSection, Yaw, TEXT("ObjectScannerYaw"), TEXT("Yaw"), TEXT("Yaw"), -11.0f, -180.0f, 180.0f);
    ConfigureNumericProperty(RootSection, Roll, TEXT("ObjectScannerRoll"), TEXT("Roll"), TEXT("Roll"), -4.0f, -180.0f, 180.0f);
    ConfigureNumericProperty(RootSection, X, TEXT("ObjectScannerX"), TEXT("X"), TEXT("X"), 3.0f, -100.0f, 100.0f);
    ConfigureNumericProperty(RootSection, Y, TEXT("ObjectScannerY"), TEXT("Y"), TEXT("Y"), -9.0f, -100.0f, 100.0f);
    ConfigureNumericProperty(RootSection, Z, TEXT("ObjectScannerZ"), TEXT("Z"), TEXT("Z"), 10.0f, -100.0f, 100.0f);
    ConfigureNumericProperty(RootSection, Scale, TEXT("ObjectScannerScale"), TEXT("Scale"), TEXT("Scale"), 1.5f, 0.5f, 3.0f, 0.05f);
}

UItemScannerGameInstanceModule::UItemScannerGameInstanceModule()
{
    bRootModule = true;
    ModConfigurations.Add(UItemScannerModConfiguration::StaticClass());
}

FRotator ItemScannerModConfig::GetFirstPersonRotation(const UObject* WorldContext, const FRotator& Fallback)
{
    const UConfigPropertyFloat* Pitch = FindFloat(WorldContext, TEXT("ObjectScannerPitch"));
    const UConfigPropertyFloat* Yaw = FindFloat(WorldContext, TEXT("ObjectScannerYaw"));
    const UConfigPropertyFloat* Roll = FindFloat(WorldContext, TEXT("ObjectScannerRoll"));
    return FRotator(
        Pitch ? FMath::Clamp(Pitch->Value, -180.0f, 180.0f) : Fallback.Pitch,
        Yaw ? FMath::Clamp(Yaw->Value, -180.0f, 180.0f) : Fallback.Yaw,
        Roll ? FMath::Clamp(Roll->Value, -180.0f, 180.0f) : Fallback.Roll);
}

FVector ItemScannerModConfig::GetFirstPersonOffset(const UObject* WorldContext, const FVector& Fallback)
{
    const UConfigPropertyFloat* X = FindFloat(WorldContext, TEXT("ObjectScannerX"));
    const UConfigPropertyFloat* Y = FindFloat(WorldContext, TEXT("ObjectScannerY"));
    const UConfigPropertyFloat* Z = FindFloat(WorldContext, TEXT("ObjectScannerZ"));
    return FVector(
        X ? FMath::Clamp(X->Value, -100.0f, 100.0f) : Fallback.X,
        Y ? FMath::Clamp(Y->Value, -100.0f, 100.0f) : Fallback.Y,
        Z ? FMath::Clamp(Z->Value, -100.0f, 100.0f) : Fallback.Z);
}

float ItemScannerModConfig::GetFirstPersonScale(const UObject* WorldContext, float Fallback)
{
    const UConfigPropertyFloat* Scale = FindFloat(WorldContext, TEXT("ObjectScannerScale"));
    return Scale ? FMath::Clamp(Scale->Value, 0.5f, 3.0f) : Fallback;
}
