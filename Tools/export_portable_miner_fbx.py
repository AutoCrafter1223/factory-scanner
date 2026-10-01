import os

import unreal


OUTPUT_DIR = r"D:\codex\item scanner\model\reference"
ASSETS = {
    "PortableMiner_1P_reference.fbx": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_1PportableMiner_01",
    "PortableMiner_3P_reference.fbx": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_PortableMiner_01",
    "PortableMiner_Folded_reference.fbx": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/PortableMiner_Folded_static",
}


try:
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
        options = unreal.FbxExportOption()
        options.set_editor_property("level_of_detail", False)
        options.set_editor_property("collision", False)
        options.set_editor_property("export_morph_targets", False)
        options.set_editor_property("export_preview_mesh", False)
        task.set_editor_property("options", options)

        if isinstance(asset, unreal.SkeletalMesh):
            task.set_editor_property("exporter", unreal.SkeletalMeshExporterFBX())
        else:
            task.set_editor_property("exporter", unreal.StaticMeshExporterFBX())

        if not unreal.Exporter.run_asset_export_task(task):
            raise RuntimeError("FBX export failed: " + asset_path)
        unreal.log("FBX COMPLETE: " + os.path.join(OUTPUT_DIR, filename))
finally:
    if not unreal.SystemLibrary.is_running_commandlet():
        unreal.SystemLibrary.quit_editor()
