import math
import os
import struct

import unreal


OUTPUT_DIR = r"D:\codex\item scanner\model\reference"

ASSETS = {
    "PortableMiner_1P_reference": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_1PportableMiner_01",
    "PortableMiner_3P_reference": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/SK_PortableMiner_01",
    "PortableMiner_Folded_reference": "/Game/FactoryGame/Equipment/PortableMiner/Mesh/PortableMiner_Folded_static",
}


def export_to_usd(asset, filename):
    options = (
        unreal.SkeletalMeshExporterUSDOptions()
        if isinstance(asset, unreal.SkeletalMesh)
        else unreal.StaticMeshExporterUSDOptions()
    )

    # Keep the geometry in one stage so the STL conversion is deterministic.
    mesh_options = options.get_editor_property("mesh_asset_options")
    try:
        mesh_options.set_editor_property("use_payload", False)
    except Exception:
        pass
    try:
        mesh_options.set_editor_property("bake_materials", False)
    except Exception:
        pass

    task = unreal.AssetExportTask()
    task.set_editor_property("object", asset)
    task.set_editor_property("filename", filename)
    task.set_editor_property("selected", False)
    task.set_editor_property("replace_identical", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("automated", True)
    task.set_editor_property("options", options)

    if not unreal.Exporter.run_asset_export_task(task):
        raise RuntimeError("USD export failed: " + asset.get_path_name())


def write_binary_stl(path, triangles):
    header = b"Satisfactory Portable Miner reference; coordinates in millimetres"
    header = header[:80].ljust(80, b"\0")
    with open(path, "wb") as stream:
        stream.write(header)
        stream.write(struct.pack("<I", len(triangles)))
        for a, b, c in triangles:
            ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
            vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
            nx = uy * vz - uz * vy
            ny = uz * vx - ux * vz
            nz = ux * vy - uy * vx
            length = math.sqrt(nx * nx + ny * ny + nz * nz)
            if length:
                nx, ny, nz = nx / length, ny / length, nz / length
            stream.write(struct.pack("<12fH", nx, ny, nz, *a, *b, *c, 0))


def usd_to_stl(usd_path, stl_path):
    from pxr import Usd, UsdGeom

    stage = Usd.Stage.Open(usd_path)
    if stage is None:
        raise RuntimeError("Could not open exported USD: " + usd_path)

    # Convert the authored stage units into millimetres for SolidWorks.
    mm_per_stage_unit = UsdGeom.GetStageMetersPerUnit(stage) * 1000.0
    xform_cache = UsdGeom.XformCache(Usd.TimeCode.Default())
    triangles = []

    for prim in stage.Traverse():
        if not prim.IsA(UsdGeom.Mesh):
            continue
        mesh = UsdGeom.Mesh(prim)
        points = mesh.GetPointsAttr().Get(Usd.TimeCode.Default()) or []
        counts = mesh.GetFaceVertexCountsAttr().Get() or []
        indices = mesh.GetFaceVertexIndicesAttr().Get() or []
        matrix = xform_cache.GetLocalToWorldTransform(prim)

        transformed = []
        for point in points:
            value = matrix.Transform(point)
            transformed.append(
                (
                    float(value[0]) * mm_per_stage_unit,
                    float(value[1]) * mm_per_stage_unit,
                    float(value[2]) * mm_per_stage_unit,
                )
            )

        cursor = 0
        for count in counts:
            face = indices[cursor : cursor + count]
            cursor += count
            if count < 3:
                continue
            first = transformed[face[0]]
            for index in range(1, count - 1):
                triangles.append((first, transformed[face[index]], transformed[face[index + 1]]))

    if not triangles:
        raise RuntimeError("No polygon geometry found in: " + usd_path)

    write_binary_stl(stl_path, triangles)
    coords = [value for triangle in triangles for point in triangle for value in point]
    xs, ys, zs = coords[0::3], coords[1::3], coords[2::3]
    bounds = (max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs))
    return len(triangles), bounds


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    for output_name, asset_path in ASSETS.items():
        asset = unreal.load_asset(asset_path)
        if asset is None:
            raise RuntimeError("Asset not found: " + asset_path)

        usd_path = os.path.join(OUTPUT_DIR, output_name + ".usd")
        stl_path = os.path.join(OUTPUT_DIR, output_name + "_mm.stl")
        unreal.log("Exporting " + asset_path)
        export_to_usd(asset, usd_path)
        triangle_count, bounds = usd_to_stl(usd_path, stl_path)
        unreal.log(
            "STL COMPLETE: {} | triangles={} | bounds_mm={:.2f} x {:.2f} x {:.2f}".format(
                stl_path, triangle_count, bounds[0], bounds[1], bounds[2]
            )
        )


main()
