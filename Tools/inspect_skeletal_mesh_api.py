import unreal

asset = unreal.load_asset('/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_1PportableMiner_01')
unreal.log('ASSET TYPE ' + str(type(asset)))
for name in dir(asset):
    lowered = name.lower()
    if any(token in lowered for token in ('mesh', 'lod', 'vertex', 'render', 'description', 'import')):
        unreal.log('ASSET MEMBER ' + name)

for class_name in dir(unreal):
    lowered = class_name.lower()
    if 'skeletal' in lowered and any(token in lowered for token in ('library', 'subsystem', 'editor', 'mesh')):
        unreal.log('UNREAL CLASS ' + class_name)
