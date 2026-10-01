"""Run with UnrealEditor-Cmd -run=pythonscript. Reproducible mesh/material assets."""
import unreal, json
from pathlib import Path
ROOT=Path(r'D:/codex/item scanner')
BASE='/ItemScanner/Equipment'
tools=unreal.AssetToolsHelpers.get_asset_tools()
lib=unreal.EditorAssetLibrary
lib.make_directory(BASE)
unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([BASE],True)

def material(name,color,metal=0.0,rough=0.55,force_rebuild=False):
    path=BASE+'/'+name
    existing=unreal.load_asset(path) if (Path(r'D:/SatisfactoryModding/StarterProject-CL502094/Mods/ItemScanner/Content/Equipment')/(name+'.uasset')).exists() else None
    if existing and not force_rebuild: return existing
    if existing:
        lib.delete_asset(path)
    m=tools.create_asset(name,BASE,unreal.Material,unreal.MaterialFactoryNew())
    edit=unreal.MaterialEditingLibrary
    c=edit.create_material_expression(m,unreal.MaterialExpressionConstant3Vector)
    c.set_editor_property('constant',unreal.LinearColor(*color,1))
    edit.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    for value,prop in [(metal,unreal.MaterialProperty.MP_METALLIC),(rough,unreal.MaterialProperty.MP_ROUGHNESS)]:
        e=edit.create_material_expression(m,unreal.MaterialExpressionConstant)
        e.set_editor_property('r',value); edit.connect_material_property(e,'',prop)
    edit.recompile_material(m); lib.save_loaded_asset(m)
    return m

body=material('M_IndustrialPaint',(0.70,0.255,0.032),0.12,0.43,True)
trim=material('M_IndustrialTrim',(0.028,0.038,0.052),0.45,0.46,True)
marking=material('M_IndustrialMarking',(0.88,0.90,0.92),0.08,0.32,True)
mode=material('M_IndustrialSafetyRed',(0.40,0.026,0.009),0.1,0.4)
dial=material('M_IndustrialMetal',(0.33,0.38,0.41),0.85,0.29,True)
glass=material('M_ScannerGlass',(0.003,0.015,0.022),0.1,0.22)
rubber=material('M_IndustrialRubber',(0.005,0.007,0.008),0.0,0.88)
materials=[body,trim,rubber,dial,marking,mode,glass]
if not lib.does_asset_exist(BASE+'/M_StatusLED'):
    led=tools.create_asset('M_StatusLED',BASE,unreal.Material,unreal.MaterialFactoryNew())
    led.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    tint=unreal.MaterialEditingLibrary.create_material_expression(led,unreal.MaterialExpressionVectorParameter)
    tint.set_editor_property('parameter_name','LEDColor')
    tint.set_editor_property('default_value',unreal.LinearColor(0.025,0.8,0.12,1))
    unreal.MaterialEditingLibrary.connect_material_property(tint,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(led)
    lib.save_loaded_asset(led)
screen_path=BASE+'/M_ScannerScreen'
if not lib.does_asset_exist(screen_path):
    screen=tools.create_asset('M_ScannerScreen',BASE,unreal.Material,unreal.MaterialFactoryNew())
    screen.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    tex=unreal.MaterialEditingLibrary.create_material_expression(screen,unreal.MaterialExpressionTextureSampleParameter2D)
    tex.set_editor_property('parameter_name','SlateUI')
    tex.set_editor_property('texture',unreal.load_asset('/Engine/EngineResources/WhiteSquareTexture'))
    unreal.MaterialEditingLibrary.connect_material_property(tex,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(screen)
    lib.save_loaded_asset(screen)
records=json.loads((ROOT/'model/generated/scanner_meshes.json').read_text())
for r in records:
    name=r['name']; mesh=lib.load_asset(BASE+'/'+name) if lib.does_asset_exist(BASE+'/'+name) else lib.duplicate_asset('/Engine/BasicShapes/Cube',BASE+'/'+name)
    dm=unreal.DynamicMesh()
    buf=unreal.GeometryScriptSimpleMeshBuffers()
    buf.vertices=[unreal.Vector(*v) for v in r['vertices']]
    buf.triangles=[unreal.IntVector(*t) for t in r['triangles']]
    # Planar UVs are sufficient for solid materials; screen UI uses its own surface RT.
    buf.uv0=[unreal.Vector2D(v[1]/30+0.5,0.5-v[2]/20) for v in r['vertices']]
    dm.append_buffers_to_mesh(buf)
    for triangle,material_id in enumerate(r['material_ids']):
        _,valid=dm.set_triangle_material_id(triangle,material_id,True)
        if not valid: raise RuntimeError(f'Invalid triangle {triangle} in {name}')
    dm.recompute_normals(unreal.GeometryScriptCalculateNormalsOptions())
    opt=unreal.GeometryScriptCopyMeshToAssetOptions()
    opt.replace_materials=True; opt.new_materials=materials
    opt.new_material_slot_names=[unreal.Name(n) for n in ['Paint','Trim','Rubber','Metal','Marking','SafetyRed','Glass']]
    opt.enable_recompute_normals=False; opt.enable_recompute_tangents=True
    _,outcome=unreal.GeometryScript_AssetUtils.copy_mesh_to_static_mesh(dm,mesh,opt,unreal.GeometryScriptMeshWriteLOD())
    if outcome!=unreal.GeometryScriptOutcomePins.SUCCESS: raise RuntimeError(str(outcome)+' '+name)
    lib.save_loaded_asset(mesh)
    unreal.log('SCANNER_ASSET '+name+' '+str(mesh.get_bounding_box())+' triangles='+str(mesh.get_num_triangles(0)))
lib.save_directory(BASE,only_if_is_dirty=True,recursive=True)
task=unreal.AssetImportTask()
task.filename=str(ROOT/'model/generated/scanner_icon.png'); task.destination_path=BASE
task.destination_name='T_ScannerIcon'; task.automated=True; task.replace_existing=True; task.save=True
tools.import_asset_tasks([task])
icon=unreal.load_asset(BASE+'/T_ScannerIcon')
if not icon: raise RuntimeError('Icon import failed')
icon.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
lib.save_loaded_asset(icon)
unreal.log('SCANNER_ASSETS_SUCCESS')
