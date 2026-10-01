import unreal
for cls in ['GeometryScript_MeshBasicEditFunctions','GeometryScript_AssetUtils','GeometryScript_MeshNormalsFunctions',
            'GeometryScriptSimpleMeshBuffers','DynamicMesh','StaticMesh','GeometryScriptCopyMeshToAssetOptions']:
    obj=getattr(unreal,cls,None)
    unreal.log('SCANNER_PROBE '+cls+': '+str(dir(obj)))
for cls,method in [('DynamicMesh','append_buffers_to_mesh'),('DynamicMesh','recompute_normals'),
                   ('GeometryScript_AssetUtils','copy_mesh_to_static_mesh')]:
    unreal.log('SCANNER_DOC '+str(getattr(getattr(unreal,cls),method).__doc__))
