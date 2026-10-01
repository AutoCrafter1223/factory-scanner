import unreal

needles = ("portableminer", "portable_miner", "portable miner", "resourceminer", "miner")

registry = unreal.AssetRegistryHelpers.get_asset_registry()
for asset in registry.get_all_assets():
    text = (str(asset.package_name) + " " + str(asset.asset_name)).lower()
    if any(needle in text for needle in needles):
        unreal.log("PORTABLE_MINER_ASSET " + str(asset.package_name) + " CLASS " + str(asset.asset_class_path))
