"""Convert scanner2.STEP into Object Scanner-local Unreal meshes.

The CAD was modelled over the stock Object Scanner reference.  This keeps the
same coordinate frame so the stock hand socket pose and animation can be used
without a camera-space placement approximation.
"""
import hashlib
import json
import os
from pathlib import Path

import numpy as np
from OCP.BRep import BRep_Tool
from OCP.BRepMesh import BRepMesh_IncrementalMesh
from OCP.TopAbs import TopAbs_FACE, TopAbs_REVERSED
from OCP.TopExp import TopExp_Explorer
from OCP.TopLoc import TopLoc_Location
from OCP.TopoDS import TopoDS

from inspect_cad import ROOT, bounds, read

STEP = ROOT / "model/scanner2/scanner2.STEP"
OUT = ROOT / "model/generated"
OUT.mkdir(exist_ok=True)
os.environ["MPLCONFIGDIR"] = str(ROOT / ".tools/mpl-cache")

# Alignment measured against ObjectScanner_reference_mm.stl.
# CAD X -> scanner X, CAD Z -> scanner Y, -CAD Y -> scanner Z.
CAD_X_ORIGIN = -0.297
CAD_Z_ORIGIN = 198.635
CAD_Y_ORIGIN = -39.153


def ue(point):
    x, y, z = np.asarray(point, dtype=float)
    return np.array([(x - CAD_X_ORIGIN) / 10.0,
                     -(z - CAD_Z_ORIGIN) / 10.0,
                     (-y + CAD_Y_ORIGIN) / 10.0])


PALETTE = ["#e9821c", "#171d21", "#080c10", "#84929a",
           "#e8e3cb", "#ad3820", "#07171d"]


def face_material(part, face_id, face):
    b = np.asarray(bounds(face), dtype=float)
    center = (b[:3] + b[3:]) * 0.5
    dx, dy, dz = b[3:] - b[:3]
    if part == "Scanner_MainDisplay":
        return 6
    if part == "Scanner_RotaryDial":
        # Paint recessed grooves around the entire circumference, including rear.
        from OCP.BRepAdaptor import BRepAdaptor_Surface
        from OCP.GeomAbs import GeomAbs_Cylinder
        surface = BRepAdaptor_Surface(face)
        if surface.GetType() == GeomAbs_Cylinder and abs(surface.Cylinder().Radius()-16.5)<0.01:
            return 4
        return 2
    if part == "Scanner_ModeSwitch":
        # White asymmetric inset is the direction index; dark handle avoids
        # ambiguity between its two ends. Include the inset's side walls.
        if b[2] >= 229.49 and b[5] <= 231.51 and face_id != 11:
            return 4
        return 1 if b[2] >= 221.49 and b[5] > 221.51 else 2
    if face_id in (0,1,2,3,138,139,140,141):
        return 2  # Rubber antenna sleeves.
    if face_id in (27,88,89,90,91,92,94,96,98,111):
        return 1  # Protective perimeter and raised bezel.
    if face_id in (99,101,103,105,107,109):
        return 3  # Thin exposed-metal trim, not a flat orange slab.
    if face_id in (25,51,124,125,128,132,136):
        return 2  # Rear grip and lower bumper.
    if face_id in (52,53,54,55,139,141,142,71,76,81,86):
        return 2
    if 142 <= face_id <= 168:
        return 2  # New side and bottom impact/grip details.
    if face_id >= 169:
        return 6 if face_id == 173 else 1
    if face_id in (22,24,25,26,57,58,59,60):
        return 1  # Dark side chassis, orange front cover retained.
    if center[2] >= 216.0 and center[0] < -42.0 and center[1] > -70.0:
        return 1
    if dz > 20.0 and (center[1] < -75.0 or center[1] > 85.0):
        return 2
    return 0


def find_screen_face(shape):
    matches = []
    ex = TopExp_Explorer(shape, TopAbs_FACE)
    face_id = 0
    while ex.More():
        face = TopoDS.Face_s(ex.Current())
        b = bounds(face)
        if (abs((b[3] - b[0]) - 130.0) < 0.01 and
                abs((b[4] - b[1]) - 140.0) < 0.01 and
                abs(b[2] - 214.5) < 0.01 and abs(b[5] - 214.5) < 0.01):
            matches.append(face_id)
        face_id += 1
        ex.Next()
    if len(matches) != 1:
        raise RuntimeError(f"Expected one 130x140 display face, got {matches}")
    return matches[0]


def tessellate(shape, pivot, part, include_face=None, exclude_face=None):
    # The selector's narrow trimmed cap faces fail at coarse deflection despite
    # valid CAD topology. Fine meshing avoids silently missing cap/handle faces.
    precision = 0.01 if part == "Scanner_ModeSwitch" else 0.10
    angle = 0.10 if part == "Scanner_ModeSwitch" else 0.18
    BRepMesh_IncrementalMesh(shape, precision, False, angle, True).Perform()
    vertices, triangles, material_ids = [], [], []
    ex = TopExp_Explorer(shape, TopAbs_FACE)
    face_id = 0
    while ex.More():
        face = TopoDS.Face_s(ex.Current())
        selected = ((include_face is None or face_id == include_face) and
                    (exclude_face is None or face_id != exclude_face))
        if selected:
            loc = TopLoc_Location()
            mesh = BRep_Tool.Triangulation_s(face, loc)
            if mesh is None:
                raise RuntimeError(f"Missing triangulation: {part} face {face_id}; refusing an incomplete model")
            if mesh is not None:
                start = len(vertices)
                for node in range(1, mesh.NbNodes() + 1):
                    p = mesh.Node(node).Transformed(loc.Transformation())
                    vertices.append((ue(p.Coord()) - pivot).tolist())
                for tri in range(1, mesh.NbTriangles() + 1):
                    a, b, c = mesh.Triangle(tri).Get()
                    if face.Orientation() == TopAbs_REVERSED:
                        b, c = c, b
                    triangles.append([start + a - 1, start + b - 1, start + c - 1])
                    material_ids.append(face_material(part, face_id, face))
        face_id += 1
        ex.Next()
    return vertices, triangles, material_ids


def record(name, shape, pivot_cad, include_face=None, exclude_face=None, **extra):
    pivot = ue(pivot_cad)
    vertices, triangles, material_ids = tessellate(
        shape, pivot, name, include_face=include_face, exclude_face=exclude_face)
    return dict(name=name, pivot=pivot.tolist(), vertices=vertices,
                triangles=triangles, material_ids=material_ids, **extra)


def main():
    _, parts = read(STEP)
    if len(parts) != 3:
        raise RuntimeError(f"Expected body, rotary and selector, got {len(parts)} parts")
    body, rotary, selector = [part[2] for part in parts]
    screen_face = find_screen_face(body)
    records = [
        # The body remains in stock scanner coordinates; its component sits at zero.
        record("Scanner_Body", body, (CAD_X_ORIGIN, CAD_Y_ORIGIN, CAD_Z_ORIGIN),
               exclude_face=screen_face),
        record("Scanner_RotaryDial", rotary, (-70.297, 34.2605, 213.5)),
        record("Scanner_ModeSwitch", selector, (-70.297, 101.2605, 221.5)),
        record("Scanner_MainDisplay", body, (22.703, 3.2605, 214.5),
               include_face=screen_face, width_cm=13.0, height_cm=14.0),
    ]
    (OUT / "scanner_meshes.json").write_text(json.dumps(records), encoding="utf8")
    manifest = dict(
        source="model/scanner2/scanner2.STEP",
        step_sha256=hashlib.sha256(STEP.read_bytes()).hexdigest(),
        screen_face=screen_face,
        triangles=sum(len(r["triangles"]) for r in records),
        material_slots=["Paint", "Trim", "Rubber", "Metal", "Marking", "SafetyRed", "Glass"],
        object_scanner_alignment=dict(x_origin=CAD_X_ORIGIN, y_origin=CAD_Y_ORIGIN, z_origin=CAD_Z_ORIGIN),
    )
    (OUT / "cad_manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf8")
    for r in records:
        print(r["name"], len(r["vertices"]), "verts", len(r["triangles"]), "triangles", "pivot", r["pivot"])

    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d.art3d import Poly3DCollection
    fig = plt.figure(figsize=(10, 8), facecolor="#101922")
    ax = fig.add_subplot(projection="3d")
    polygons, colors = [], []
    for r in records:
        points = np.asarray(r["vertices"]) + np.asarray(r["pivot"])
        polygons.extend(points[np.asarray(r["triangles"])])
        colors.extend(PALETTE[m] for m in r["material_ids"])
    ax.add_collection3d(Poly3DCollection(polygons, facecolors=colors, linewidths=0, shade=True,
                                        lightsource=matplotlib.colors.LightSource(azdeg=110, altdeg=55)))
    ax.set(xlim=(-11, 11), ylim=(-4, 5), zlim=(-18, 14))
    ax.set_box_aspect((22, 9, 32))
    ax.view_init(elev=8, azim=-88)
    ax.set_axis_off(); ax.set_facecolor("#101922")
    fig.tight_layout()
    fig.savefig(OUT / "scanner_preview.png", dpi=140, facecolor=fig.get_facecolor())
    ax.set_position([0, 0, 1, 1]); fig.set_size_inches(5.12, 5.12)
    fig.savefig(OUT / "scanner_icon.png", dpi=100, transparent=True)


if __name__ == "__main__":
    main()
