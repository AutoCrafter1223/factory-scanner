"""Read-only CAD inspection. Original SolidWorks/STEP files are never modified."""
import sys, json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / '.tools/cad'))
from OCP.STEPCAFControl import STEPCAFControl_Reader
from OCP.XCAFDoc import XCAFDoc_DocumentTool
from OCP.TDocStd import TDocStd_Document
from OCP.TCollection import TCollection_ExtendedString
from OCP.TDF import TDF_LabelSequence, TDF_Label
from OCP.TDataStd import TDataStd_Name
from OCP.Bnd import Bnd_Box
from OCP.BRepBndLib import BRepBndLib
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_FACE
from OCP.TopoDS import TopoDS
from OCP.BRepAdaptor import BRepAdaptor_Surface
from OCP.GProp import GProp_GProps
from OCP.BRepGProp import BRepGProp

def bounds(shape):
    b = Bnd_Box(); BRepBndLib.Add_s(shape, b)
    return [round(x, 4) for x in b.Get()]

def read(path=None):
    path = Path(path) if path else ROOT / 'model/scanner.STEP'
    reader = STEPCAFControl_Reader()
    reader.ReadFile(str(path))
    doc = TDocStd_Document(TCollection_ExtendedString('scanner'))
    reader.Transfer(doc)
    st = XCAFDoc_DocumentTool.ShapeTool_s(doc.Main())
    roots = TDF_LabelSequence(); st.GetFreeShapes(roots)
    components = TDF_LabelSequence(); st.GetComponents_s(roots.Value(1), components)
    result = []
    for i in range(1, components.Length()+1):
        label = components.Value(i)
        ref = TDF_Label(); st.GetReferredShape_s(label, ref)
        name = TDataStd_Name(); ref.FindAttribute(TDataStd_Name.GetID_s(), name)
        shape = st.GetShape_s(label)
        result.append((i, name.Get().ToExtString(), shape))
    return doc, result

if __name__ == '__main__':
    doc, parts = read(sys.argv[1] if len(sys.argv) > 1 else None)
    for i, name, shape in parts:
        print(i, name, bounds(shape))
        if i != 1: continue
        faces = TopExp_Explorer(shape, TopAbs_FACE); n = 0
        while faces.More():
            f = TopoDS.Face_s(faces.Current()); surf = BRepAdaptor_Surface(f)
            g = GProp_GProps(); BRepGProp.SurfaceProperties_s(f, g)
            if g.Mass() > 100:
                print(' face', n, str(surf.GetType()), round(g.Mass(),2), bounds(f))
            n += 1; faces.Next()
