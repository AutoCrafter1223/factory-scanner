#!/usr/bin/env python3
"""Convert an ActorX PSK mesh to a binary STL in millimetres."""

from __future__ import annotations

import math
import struct
import sys
from pathlib import Path


def read_chunks(path: Path) -> dict[str, tuple[int, int, bytes]]:
    chunks: dict[str, tuple[int, int, bytes]] = {}
    with path.open("rb") as stream:
        while header := stream.read(32):
            if len(header) != 32:
                raise ValueError("Truncated PSK chunk header")
            chunk_id = header[:20].rstrip(b"\0").decode("ascii", "replace")
            _, data_size, data_count = struct.unpack("<iii", header[20:])
            payload = stream.read(data_size * data_count)
            if len(payload) != data_size * data_count:
                raise ValueError(f"Truncated PSK chunk: {chunk_id}")
            if data_size and data_count:
                chunks[chunk_id] = (data_size, data_count, payload)
    return chunks


def convert(source: Path, target: Path) -> None:
    chunks = read_chunks(source)
    point_size, point_count, point_data = chunks["PNTS0000"]
    wedge_name = "VTXW3200" if "VTXW3200" in chunks else "VTXW0000"
    face_name = "FACE3200" if "FACE3200" in chunks else "FACE0000"
    wedge_size, wedge_count, wedge_data = chunks[wedge_name]
    face_size, face_count, face_data = chunks[face_name]

    if point_size < 12 or wedge_size < 4 or face_size < 12:
        raise ValueError("Unsupported PSK record layout")

    # Unreal units are centimetres. SolidWorks STL import is most predictable in mm.
    points = [
        tuple(value * 10.0 for value in struct.unpack_from("<3f", point_data, i * point_size))
        for i in range(point_count)
    ]
    point_index_format = "<I" if wedge_name == "VTXW3200" else "<H"
    wedges = [
        struct.unpack_from(point_index_format, wedge_data, i * wedge_size)[0]
        for i in range(wedge_count)
    ]
    face_index_format = "<3I" if face_name == "FACE3200" else "<3H"

    triangles: list[tuple[tuple[float, float, float], ...]] = []
    for i in range(face_count):
        wedge_indices = struct.unpack_from(face_index_format, face_data, i * face_size)
        triangle = tuple(points[wedges[index]] for index in wedge_indices)
        ax, ay, az = triangle[0]
        bx, by, bz = triangle[1]
        cx, cy, cz = triangle[2]
        ux, uy, uz = bx - ax, by - ay, bz - az
        vx, vy, vz = cx - ax, cy - ay, cz - az
        nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
        length = math.sqrt(nx * nx + ny * ny + nz * nz)
        if length <= 1e-12:
            continue
        normal = (nx / length, ny / length, nz / length)
        triangles.append((normal, *triangle))

    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open("wb") as stream:
        description = f"Satisfactory {source.stem} - millimetres".encode("ascii")[:80]
        stream.write(description.ljust(80, b"\0"))
        stream.write(struct.pack("<I", len(triangles)))
        for normal, a, b, c in triangles:
            stream.write(struct.pack("<12fH", *normal, *a, *b, *c, 0))

    axes = list(zip(*points))
    dimensions = tuple(max(axis) - min(axis) for axis in axes)
    print(
        f"{target.name}: {len(triangles)} triangles, "
        f"{dimensions[0]:.1f} x {dimensions[1]:.1f} x {dimensions[2]:.1f} mm"
    )


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit("usage: psk_to_stl.py SOURCE.psk TARGET.stl")
    convert(Path(sys.argv[1]), Path(sys.argv[2]))


if __name__ == "__main__":
    main()
