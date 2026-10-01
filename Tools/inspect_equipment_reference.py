import unreal
for asset in ['BoomBox/Equip_BoomBox','ObjectScanner/Equip_ObjectScanner']:
    path='/Game/FactoryGame/Equipment/'+asset
    cls=unreal.load_class(None,path+'.'+asset.split('/')[-1]+'_C')
    if not cls: continue
    cdo=unreal.get_default_object(cls)
    for key in ['mAttachSocket','mArmAnimation','m1PAnimClass','mIdlePoseAnimation','mIdlePoseAnimation3p','mSprintHeadBobCameraAnim']:
        try: unreal.log('SCANNER_REFERENCE '+asset+' '+key+' '+str(cdo.get_editor_property(key)))
        except Exception as e: unreal.log(str(e))
    for c in cdo.get_components_by_class(unreal.SceneComponent):
        unreal.log('SCANNER_COMPONENT '+c.get_name()+' '+str(c.get_relative_transform()))
