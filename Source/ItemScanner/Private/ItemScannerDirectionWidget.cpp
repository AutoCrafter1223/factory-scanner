#include "ItemScannerDirectionWidget.h"

#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"

namespace
{
    class SItemScannerDirection final : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SItemScannerDirection) {}
        SLATE_END_ARGS()

        void Construct(const FArguments& InArgs) {}

        virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
        {
            return FVector2D(32.0f, 32.0f);
        }

        virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
            const FSlateRect& CullingRect, FSlateWindowElementList& Elements,
            int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const override
        {
            const FVector2f Size(Geometry.GetLocalSize());
            const FVector2f Scale = Size / 32.0f;
            const TArray<FVector2f> Shaft = {
                FVector2f(16.0f, 27.0f) * Scale, FVector2f(16.0f, 5.0f) * Scale };
            const TArray<FVector2f> Head = {
                FVector2f(8.0f, 15.0f) * Scale, FVector2f(16.0f, 5.0f) * Scale,
                FVector2f(24.0f, 15.0f) * Scale };
            const FLinearColor Tint = Style.GetColorAndOpacityTint();
            const FPaintGeometry PaintGeometry = Geometry.ToPaintGeometry();
            const float ThicknessScale = FMath::Min(Scale.X, Scale.Y);
            // All points are copied into draw elements. No texture, font, or
            // UObject-owned brush is referenced during rendering.
            for (int32 Pass = 0; Pass < 2; ++Pass)
            {
                const FLinearColor Color = (Pass == 0
                    ? FLinearColor(0.005f, 0.02f, 0.03f, 0.95f)
                    : FLinearColor(0.1f, 0.85f, 0.95f, 1.0f)) * Tint;
                const float Thickness = (Pass == 0 ? 6.0f : 3.5f) * ThicknessScale;
                FSlateDrawElement::MakeLines(Elements, Layer + Pass, PaintGeometry,
                    Shaft, ESlateDrawEffect::None, Color, true, Thickness);
                FSlateDrawElement::MakeLines(Elements, Layer + Pass, PaintGeometry,
                    Head, ESlateDrawEffect::None, Color, true, Thickness);
            }
            return Layer + 1;
        }
    };
}

TSharedRef<SWidget> UItemScannerDirectionWidget::RebuildWidget()
{
    return SNew(SItemScannerDirection);
}
