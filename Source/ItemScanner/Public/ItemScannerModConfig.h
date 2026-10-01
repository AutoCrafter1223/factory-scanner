#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Configuration/ModConfiguration.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Module/GameInstanceModule.h"
#include "ItemScannerModConfig.generated.h"

UCLASS(EditInlineNew)
class ITEMSCANNER_API UItemScannerAngleConfigProperty final : public UConfigPropertyFloat
{
    GENERATED_BODY()

public:
    virtual UUserWidget* CreateEditorWidget_Implementation(UUserWidget* ParentWidget) const override;

    UPROPERTY() float MinimumValue = -45.0f;
    UPROPERTY() float MaximumValue = 45.0f;
    UPROPERTY() float StepSize = 1.0f;
};

UCLASS()
class ITEMSCANNER_API UItemScannerAngleConfigEditorWidget final : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeProperty(UItemScannerAngleConfigProperty* InProperty);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UItemScannerAngleConfigProperty> Property = nullptr;
};

UCLASS(EditInlineNew)
class ITEMSCANNER_API UItemScannerAngleConfigSection final : public UConfigPropertySection
{
    GENERATED_BODY()

public:
    UItemScannerAngleConfigSection();
    virtual UUserWidget* CreateEditorWidget_Implementation(UUserWidget* ParentWidget) const override;
};

UCLASS()
class ITEMSCANNER_API UItemScannerAngleConfigSectionWidget final : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeSection(UItemScannerAngleConfigSection* InSection);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UItemScannerAngleConfigSection> Section = nullptr;
};

/** Settings exposed through SML's in-game Mods configuration screen. */
UCLASS()
class ITEMSCANNER_API UItemScannerModConfiguration final : public UModConfiguration
{
    GENERATED_BODY()

public:
    UItemScannerModConfiguration();
};

/** Registers the Item Scanner configuration with SML. */
UCLASS()
class ITEMSCANNER_API UItemScannerGameInstanceModule final : public UGameInstanceModule
{
    GENERATED_BODY()

public:
    UItemScannerGameInstanceModule();
};

namespace ItemScannerModConfig
{
    /** Returns the live first-person rotation selected in the Mods menu. */
    ITEMSCANNER_API FRotator GetFirstPersonRotation(const UObject* WorldContext, const FRotator& Fallback);
    ITEMSCANNER_API FVector GetFirstPersonOffset(const UObject* WorldContext, const FVector& Fallback);
    ITEMSCANNER_API float GetFirstPersonScale(const UObject* WorldContext, float Fallback);
}
