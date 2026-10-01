import unreal


ASSET = "/Game/FactoryGame/Equipment/ObjectScanner/Equip_ObjectScanner"
asset = unreal.EditorAssetLibrary.load_asset(ASSET)
if not asset:
    raise RuntimeError("Could not load " + ASSET)

generated_class = asset.generated_class()
default_object = unreal.get_default_object(generated_class)
unreal.log("OBJECT_SCANNER_CLASS=" + generated_class.get_path_name())

properties = (
    "mEquipmentSlot",
    "mArmAnimation",
    "mAttachSocket",
    "m1PAnimClass",
    "mIdlePoseAnimation",
    "mIdlePoseAnimation3p",
    "mCrouchPoseAnimation3p",
    "mSlidePoseAnimation3p",
    "mAttachmentIdleAO",
    "mSprintHeadBobCameraAnim",
    "mEquipMontage",
)

for name in properties:
    try:
        value = default_object.get_editor_property(name)
        unreal.log("OBJECT_SCANNER_PROPERTY {}={}".format(name, value))
    except Exception as error:
        unreal.log_warning("OBJECT_SCANNER_PROPERTY_ERROR {}={}".format(name, error))

mesh = unreal.EditorAssetLibrary.load_asset(
    "/Game/FactoryGame/Equipment/ObjectScanner/Mesh/SK_ObjectScanner_01"
)
idle = unreal.EditorAssetLibrary.load_asset(
    "/Game/FactoryGame/Equipment/ObjectScanner/Animation/ObjectScannerIdle_01"
)
player_idle_1p = unreal.EditorAssetLibrary.load_asset(
    "/Game/FactoryGame/Character/Player/Animation/FirstPerson/ObjectScannerIdle_01"
)
player_idle_3p = unreal.EditorAssetLibrary.load_asset(
    "/Game/FactoryGame/Character/Player/Animation/ThirdPerson/ObjectScannerIdle_01"
)

for label, obj in (
    ("equipment_mesh", mesh),
    ("equipment_idle", idle),
    ("player_idle_1p", player_idle_1p),
    ("player_idle_3p", player_idle_3p),
):
    unreal.log("OBJECT_SCANNER_ASSET {}={}".format(label, obj.get_path_name() if obj else "NONE"))
    if obj:
        try:
            skeleton = obj.get_editor_property("skeleton")
            unreal.log("OBJECT_SCANNER_SKELETON {}={}".format(label, skeleton.get_path_name() if skeleton else "NONE"))
        except Exception as error:
            unreal.log_warning("OBJECT_SCANNER_SKELETON_ERROR {}={}".format(label, error))

for component in default_object.get_components_by_class(unreal.SceneComponent):
    try:
        mesh_asset = component.get_editor_property("skeletal_mesh_asset") if isinstance(component, unreal.SkeletalMeshComponent) else None
    except Exception:
        mesh_asset = None
    unreal.log(
        "OBJECT_SCANNER_COMPONENT name={} class={} parent={} location={} rotation={} scale={} mesh={}".format(
            component.get_name(),
            component.get_class().get_name(),
            component.get_attach_parent().get_name() if component.get_attach_parent() else "NONE",
            component.get_editor_property("relative_location"),
            component.get_editor_property("relative_rotation"),
            component.get_editor_property("relative_scale3d"),
            mesh_asset.get_path_name() if mesh_asset else "NONE",
        )
    )
