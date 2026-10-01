#include "ItemScannerModule.h"

DEFINE_LOG_CATEGORY(LogItemScanner);

void FItemScannerModule::StartupModule()
{
    UE_LOG(LogItemScanner, Log, TEXT("[ItemScanner] Runtime module loaded."));
}

void FItemScannerModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FItemScannerModule, ItemScanner)
