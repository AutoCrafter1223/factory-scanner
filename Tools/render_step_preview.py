"""Render read-only STEP assembly previews for visual inspection."""
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / ".tools/cad"))
os.environ["MPLCONFIGDIR"] = str(ROOT / ".tools/mpl-cache")

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from OCP.BRep import BRep_Tool
from OCP.BRepMesh import BRepMesh_IncrementalMesh
from OCP.TopAbs import TopAbs_FACE, TopAbs_REVERSED
from OCP.TopExp import TopExp_Explorer
from OCP.TopLoc import TopLoc_Location
from OCP.TopoDS import TopoDS

from inspect_cad import read


def triangles(shape):
    BRepMesh_IncrementalMesh(shape, 0.15, False, 0.25, True).Perform()
    result = []
    explorer = TopExp_Explorer(shape, TopAbs_FACE)
    while explorer.More():
        face = TopoDS.Face_s(explorer.Current())
        location = TopLoc_Location()
        mesh = BRep_Tool.Triangulation_s(face, location)
        if mesh:
            points = []
            for index in range(1, mesh.NbNodes() + 1):
                point = mesh.Node(index).Transformed(location.Transformation())
                points.append(point.Coord())
            for index in range(1, mesh.NbTriangles() + 1):
                a, b, c = mesh.Triangle(index).Get()
                if face.Orientation() == TopAbs_REVERSED:
                    b, c = c, b
                result.append([points[a - 1], points[b - 1], points[c - 1]])
        explorer.Next()
    return result


def main():
    source = Path(sys.argv[1])
    target = Path(sys.argv[2])
    _, parts = read(source)
    palettes = ["#d98222", "#22282d", "#d8d7ce", "#8b969e"]
    geometry = [(triangles(shape), palettes[(index - 1) % len(palettes)]) for index, _, shape in parts]

    figure = plt.figure(figsize=(12, 6), facecolor="#121a21")
    for plot_index, (elevation, azimuth, title) in enumerate(((90, -90, "FRONT"), (27, -62, "PERSPECTIVE")), 1):
        axis = figure.add_subplot(1, 2, plot_index, projection="3d")
        all_points = []
        for faces, color in geometry:
            axis.add_collection3d(Poly3DCollection(faces, facecolors=color, edgecolors="#12171a", linewidths=0.08, shade=True))
            all_points.extend(np.asarray(faces).reshape(-1, 3))
        points = np.asarray(all_points)
        low, high = points.min(axis=0), points.max(axis=0)
        center = (low + high) / 2
        radius = max(high - low) / 2 * 1.08
        axis.set_xlim(center[0] - radius, center[0] + radius)
        axis.set_ylim(center[1] - radius, center[1] + radius)
        axis.set_zlim(center[2] - radius, center[2] + radius)
        axis.set_box_aspect((1, 1, 1))
        axis.view_init(elev=elevation, azim=azimuth)
        axis.set_axis_off()
        axis.set_facecolor("#121a21")
        axis.set_title(title, color="#dce7eb", fontsize=11)
    figure.tight_layout()
    target.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(target, dpi=170, facecolor=figure.get_facecolor())
    print(target)


if __name__ == "__main__":
    main()
