# Item Scanner 0.1.3 crash diagnosis and fix

The September 28 crash used ItemScanner 0.1.2. The log records successful scans before an access violation at FSlateBrush::UpdateRenderingResource, followed by SImage, SBox, panels, SButton and SMenuAnchor. This is consistent with the selected combo-box entry, which remained visible after scanning.

The original generator returned a plain UHorizontalBox created under the scanner's WidgetTree. That row was not attached to the tree's RootWidget and was not referenced by a UPROPERTY. UComboBoxString::HandleGenerateWidget retains only the result of TakeWidget(). For a plain panel this does not retain the UObject hierarchy. Keeping the texture itself alive in 0.1.2 did not keep the UImage or its embedded brush alive.

Engine evidence in the installed CSS engine:

- UMG/Private/Components/ComboBoxString.cpp: HandleGenerateWidget returns Widget->TakeWidget().
- UMG/Private/Components/Widget.cpp: TakeWidget_Private wraps UUserWidget in SObjectWidget explicitly to prevent garbage collection; ordinary panels do not get that wrapper.
- UMG/Private/Slate/SObjectWidget.cpp: AddReferencedObjects retains WidgetObject.
- UMG/Public/Blueprint/WidgetTree.h: ConstructWidget creates an object, while RootWidget and NamedSlotBindings are the runtime ownership roots. Outer alone does not keep children alive.

Each generated option now uses UItemScannerItemOptionWidget, a UUserWidget with its own rooted widget tree and referenced texture. The Slate GC bridge keeps the entire row alive while displayed. Releasing the Slate row permits normal collection, without an accumulating permanent cache of generated rows.

Automated regression verification passed on September 28:

1. A legacy plain-panel entry remains in Slate, but its UImage is collected by forced GC, reproducing the premature object release without dereferencing the freed brush.
2. Twelve replacement entries survive three forced GC passes each, including their UImage and texture. Releasing each entry's Slate reference allows all three objects to be collected.
3. Tracking survives 60 seconds of simulated game time, temporary pawn loss, and target destruction. Scanner close clears results, stops the timer, and keeps results empty on the next tick.
4. Continuous view-relative yaw math passes its existing test.

Reports: release/tests-0.1.3/index.json and release/tests-0.1.3.log. Tests use NullRHI; no claim of a completed GPU-rendered playthrough in the user's save is made.

The layout is direction arrow, distance, then separately labeled height difference. Tracking has no expiry. A fresh scan replaces the snapshot; closing with F7 or the close button, or world teardown, clears it. Old TrackingDurationSeconds configuration values are no longer read.
