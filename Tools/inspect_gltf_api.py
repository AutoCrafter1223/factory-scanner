import unreal

for class_name in dir(unreal):
    if 'gltf' in class_name.lower():
        unreal.log('GLTF CLASS ' + class_name)
