import os

import unreal


OUTPUT_DIR = r"D:\codex\item scanner\model\reference"
ASSETS = {
    "PortableMiner_1P_reference.glb": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_1PportableMiner_01",
    "PortableMiner_3P_reference.glb": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_PortableMiner_01",
    "PortableMiner_Folded_reference.glb": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/PortableMiner_Folded_static",
}


os.makedirs(OUTPUT_DIR, exist_ok=True)
for filename, asset_path in ASSETS.items():
    asset = unreal.load_asset(asset_path)
    if asset is None:
        raise RuntimeError("Asset not found: " + asset_path)

    task = unreal.AssetExportTask()
    task.set_editor_property("object", asset)
    task.set_editor_property("filename", os.path.join(OUTPUT_DIR, filename))
    task.set_editor_property("selected", False)
    task.set_editor_property("replace_identical", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("automated", True)
    task.set_editor_property("write_empty_files", False)
    task.set_editor_property("options", unreal.GLTFExportOptions())
    task.set_editor_property(
        "exporter",
        unreal.GLTFSkeletalMeshExporter()
        if isinstance(asset, unreal.SkeletalMesh)
        else unreal.GLTFStaticMeshExporter(),
    )

    if not unreal.Exporter.run_asset_export_task(task):
        raise RuntimeError("GLB export failed: " + asset_path)
    unreal.log("GLB COMPLETE: " + os.path.join(OUTPUT_DIR, filename))
