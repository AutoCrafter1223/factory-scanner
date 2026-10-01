import unreal
base='/ItemScanner/Equipment/'
edit=unreal.MaterialEditingLibrary
for name in ['M_IndustrialPaint','M_IndustrialTrim','M_IndustrialRubber','M_IndustrialMetal','M_IndustrialMarking']:
    m=unreal.load_asset(base+name)
    node=edit.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.log('SCANNER_COLOR '+name+' '+str(node)+' '+str(node.get_editor_property('constant') if node else None))
mesh=unreal.load_asset(base+'Scanner_Body')
unreal.log('SCANNER_NANITE '+str(mesh.get_editor_property('nanite_settings')))
slots=unreal.GeometryScript_AssetUtils.get_section_material_list_from_static_mesh(mesh,unreal.GeometryScriptMeshReadLOD())
unreal.log('SCANNER_SLOT_LIST '+str(slots))
unreal.log('SCANNER_SECTIONS '+str(mesh.get_num_sections(0)))
for section in range(mesh.get_num_sections(0)):
    material_index=slots[1][section]
    vertices,triangles,*_=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh,0,section)
    unreal.log(f'SCANNER_SECTION {section} material={material_index} triangles={len(triangles)//3} first={vertices[:3]}')
