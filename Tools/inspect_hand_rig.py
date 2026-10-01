import unreal

paths = [
    '/Game/FactoryGame/Character/Player/Animation/FirstPerson/PortableMinerIdle_01.PortableMinerIdle_01',
]
for path in paths:
    asset = unreal.load_asset(path)
    unreal.log('SCANNER_HAND_ASSET ' + path + ' ' + str(asset))
    if not asset:
        continue
    unreal.log('SCANNER_HAND_ASSET_DIR ' + ','.join(dir(asset)))
    try:
        skeleton = asset.get_editor_property('skeleton')
        unreal.log('SCANNER_HAND_SKELETON ' + str(skeleton))
        unreal.log('SCANNER_HAND_SKELETON_DIR ' + ','.join(dir(skeleton)))
        unreal.log('SCANNER_HAND_SOCKETS ' + str(skeleton.get_editor_property('sockets')))
    except Exception as exc:
        unreal.log_error('SCANNER_HAND_ERROR ' + str(exc))

registry = unreal.AssetRegistryHelpers.get_asset_registry()
for data in registry.get_assets_by_path('/Game/FactoryGame/Character/Player/Animation/FirstPerson', recursive=True):
    name = str(data.asset_name)
    if any(term in name.lower() for term in ('idle', 'generic', 'portable', 'objectscanner', 'scanner')):
        unreal.log('SCANNER_HAND_CANDIDATE ' + str(data.package_name))
