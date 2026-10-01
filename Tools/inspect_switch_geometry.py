"""Report cylindrical axes and face bounds for the scanner switch parts."""
import sys
from inspect_cad import read, bounds
from OCP.BRepAdaptor import BRepAdaptor_Surface
from OCP.GeomAbs import GeomAbs_Cylinder
from OCP.TopAbs import TopAbs_FACE
from OCP.TopExp import TopExp_Explorer
from OCP.TopoDS import TopoDS


def main():
    _, parts = read(sys.argv[1] if len(sys.argv) > 1 else None)
    for index, name, shape in parts[1:]:
        print(f"\n{name} {bounds(shape)}")
        explorer = TopExp_Explorer(shape, TopAbs_FACE)
        face_index = 0
        while explorer.More():
            face = TopoDS.Face_s(explorer.Current())
            surface = BRepAdaptor_Surface(face)
            print(f"  face {face_index:3d} type={surface.GetType()} bounds={bounds(face)}")
            if surface.GetType() == GeomAbs_Cylinder:
                cylinder = surface.Cylinder()
                axis = cylinder.Axis()
                location = axis.Location()
                direction = axis.Direction()
                print(
                    f"             cylinder r={cylinder.Radius():.4f} "
                    f"origin=({location.X():.4f},{location.Y():.4f},{location.Z():.4f}) "
                    f"axis=({direction.X():.4f},{direction.Y():.4f},{direction.Z():.4f}) "
                    f"bounds={bounds(face)}"
                )
            face_index += 1
            explorer.Next()


if __name__ == "__main__":
    main()
