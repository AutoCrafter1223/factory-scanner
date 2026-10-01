#include "ItemScannerItemOptionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"

void UItemScannerItemOptionWidget::SetItem(const FText& InName, UTexture2D* InIcon)
{
    ItemName = InName;
    ItemIcon = InIcon;
}

TSharedRef<SWidget> UItemScannerItemOptionWidget::RebuildWidget()
{
    if (!IsValid(WidgetTree))
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("ItemOptionTree"));
    }
    if (!IsValid(WidgetTree->RootWidget))
    {
        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
        WidgetTree->RootWidget = Row;
        USizeBox* IconSize = WidgetTree->ConstructWidget<USizeBox>();
        IconSize->SetWidthOverride(30.0f);
        IconSize->SetHeightOverride(30.0f);
        if (IsValid(ItemIcon))
        {
            UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ItemIcon"));
            Icon->SetBrushFromTexture(ItemIcon, false);
            IconSize->AddChild(Icon);
        }
        else
        {
            UTextBlock* Fallback = WidgetTree->ConstructWidget<UTextBlock>();
            Fallback->SetText(FText::FromString(TEXT("◆")));
            Fallback->SetJustification(ETextJustify::Center);
            IconSize->AddChild(Fallback);
        }
        Row->AddChildToHorizontalBox(IconSize);
        UTextBlock* Name = WidgetTree->ConstructWidget<UTextBlock>();
        Name->SetText(ItemName);
        Name->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 14));
        Row->AddChildToHorizontalBox(Name)->SetPadding(FMargin(8.0f, 4.0f, 4.0f, 0.0f));
    }
    return Super::RebuildWidget();
}
