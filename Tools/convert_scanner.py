"""Tessellate STEP instances, preserve CAD design, split existing screen faces.
Output coordinates are Unreal centimeters: +Y right, +Z up, front = -X.
All solids remain intact except the two planar display faces, exported separately.
"""
import json, math, os
from inspect_cad import *
from OCP.BRepMesh import BRepMesh_IncrementalMesh
from OCP.BRep import BRep_Tool
from OCP.TopLoc import TopLoc_Location
from OCP.TopAbs import TopAbs_REVERSED
import numpy as np
import hashlib

OUT = ROOT / 'model/generated'
OUT.mkdir(exist_ok=True)
os.environ['MPLCONFIGDIR'] = str(ROOT / '.tools/mpl-cache')
CENTER = np.array([-31.41618126616, 14.34216970847, 300.0])
def ue(p):
    x,y,z = (np.asarray(p)-CENTER)/10
    return [-z,x,y]

PALETTE=['#ed881c','#171d21','#080c10','#84929a','#e8e3cb','#ad3820','#081a21']
# Shared material slots: paint, structural trim, rubber, metal, marking, safety red, glass.
def face_material(part, face, f):
    b=np.asarray(bounds(f),dtype=float)
    center=(b[:3]+b[3:])/2
    x,y,z=center-CENTER
    if part=='Scanner_Body':
        # Classify by CAD space rather than STEP face order. SolidWorks changes
        # face indices whenever nearby features are edited, which previously
        # turned the orange enclosure into trim after a harmless model revision.
        dx,dy,dz=b[3:]-b[:3]
        cad_x,cad_y,cad_z=center

        # Only the diagonal hand-contact runs are rubber. The tubular elbows and
        # short attachment stubs remain painted orange.
        if (cad_x > 210.0 or cad_x < -278.0) and -100.0 <= cad_y <= 35.0:
            return 2

        # Dark front bezels/control plates. Exclude the broad enclosure face so
        # the main chassis stays painted even if its topology is regenerated.
        on_front=cad_z >= 343.5
        broad_enclosure=dx > 300.0 and dy > 300.0
        in_main_bezel=(-232.0 <= cad_x <= 13.0 and -166.0 <= cad_y <= 195.0)
        in_controls=(23.0 <= cad_x <= 168.0 and -166.0 <= cad_y <= 194.0)
        if on_front and not broad_enclosure and (in_main_bezel or in_controls):
            return 1
        return 0
    if 'Display' in part: return 6
    if part.endswith('RotaryDial'):
        return 3 if z<26 else 2
    # Each STEP rocker is one body. Its raised r10 contact portion is white
    # (safety red for the mode switch); the r20 hinge and end caps stay black.
    contact_faces={2,4,5,6,9,10,11,12,13,14,15,16,17}
    return (5 if part.endswith('ModeSwitch') else 4) if face in contact_faces else 1

def find_screen_face(shape,width,height,depth):
    matches=[]; ex=TopExp_Explorer(shape,TopAbs_FACE); face=0
    while ex.More():
        f=TopoDS.Face_s(ex.Current()); b=bounds(f)
        if abs(b[3]-b[0]-width)<.01 and abs(b[4]-b[1]-height)<.01 and abs(b[2]-depth)<.01 and abs(b[5]-depth)<.01:
            matches.append(face)
        face+=1; ex.Next()
    if len(matches)!=1: raise RuntimeError(f'Expected unique {width}x{height} screen at {depth}, got {matches}')
    return matches[0]

def tessellate(shape, pivot, part, faces_allowed=None):
    BRepMesh_IncrementalMesh(shape, 0.12, False, 0.20, True).Perform()
    vertices, triangles, material_ids = [], [], []
    ex=TopExp_Explorer(shape,TopAbs_FACE); face_id=0
    while ex.More():
        f=TopoDS.Face_s(ex.Current())
        if faces_allowed is None or faces_allowed(face_id):
            loc=TopLoc_Location(); mesh=BRep_Tool.Triangulation_s(f,loc)
            if mesh is not None:
                start=len(vertices)
                for j in range(1,mesh.NbNodes()+1):
                    p=mesh.Node(j).Transformed(loc.Transformation())
                    vertices.append((np.array(ue(p.Coord()))-pivot).tolist())
                for j in range(1,mesh.NbTriangles()+1):
                    a,b,c=mesh.Triangle(j).Get()
                    # CAD->UE mapping changes handedness, already producing UE's
                    # clockwise front-face winding. Only reverse reversed CAD faces.
                    # An additional flip incorrectly exposes the enclosure's rear.
                    if f.Orientation()==TopAbs_REVERSED: b,c=c,b
                    triangles.append([start+a-1,start+b-1,start+c-1])
                    material_ids.append(face_material(part,face_id,f))
        face_id+=1; ex.Next()
    return vertices,triangles,material_ids

def main():
    doc,parts=read(); records=[]
    if len(parts)!=7: raise RuntimeError('Unexpected assembly; inspect before conversion')
    screen_faces=[find_screen_face(parts[0][2],240,360,344),find_screen_face(parts[0][2],136,145,344)]
    names=['Scanner_Body','Scanner_Switch_Storage','Scanner_Switch_Production',
           'Scanner_Switch_Conveyor','Scanner_Switch_Logistics','Scanner_ModeSwitch','Scanner_RotaryDial']
    pivots=[CENTER.tolist()]+[[142.58381873384,y,331] for y in [180.34216970847,154.34216970847,128.34216970847,102.34216970847]]+[
        [51.58381873384,-120.65783029153,331],[122.08381873384,-120.65783029153,347]]
    for (i,name,shape),outname,p in zip(parts,names,pivots):
        pivot=ue(p)
        v,t,m=tessellate(shape,pivot,outname,(lambda f:f not in screen_faces) if i==1 else None)
        records.append(dict(name=outname,pivot=pivot,vertices=v,triangles=t,material_ids=m))
    for name,face,p,w,h in [('Scanner_MainDisplay',screen_faces[0],[-107.41618126616,14.34216970847,344],24,36),
                          ('Scanner_ProductDisplay',screen_faces[1],[94.58381873384,6.84216970847,344],13.6,14.5)]:
        pivot=ue(p); v,t,m=tessellate(parts[0][2],pivot,name,lambda f:f==face)
        records.append(dict(name=name,pivot=pivot,vertices=v,triangles=t,material_ids=m,width_cm=w,height_cm=h))
    (OUT/'scanner_meshes.json').write_text(json.dumps(records),encoding='utf8')
    (OUT/'cad_manifest.json').write_text(json.dumps(dict(step_sha256=hashlib.sha256((ROOT/'model/scanner.STEP').read_bytes()).hexdigest(),screen_faces=screen_faces,triangles=sum(len(r['triangles']) for r in records),material_slots=['Paint','Trim','Rubber','Metal','Marking','SafetyRed','Glass']),indent=2),encoding='utf8')
    for r in records:
        print(r['name'],len(r['vertices']),'verts',len(r['triangles']),'triangles','pivot',r['pivot'])
    # CAD-derived visual QA; no redesign or image generation.
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d.art3d import Poly3DCollection
    fig=plt.figure(figsize=(14,9),facecolor='#101922'); ax=fig.add_subplot(projection='3d')
    all_polys=[]; all_colors=[]
    for r in records:
        pts=np.array(r['vertices'])+r['pivot']; xyz=pts[:,[1,0,2]]
        all_polys.extend(xyz[np.array(r['triangles'])]); all_colors.extend([PALETTE[m] for m in r['material_ids']])
    coll=Poly3DCollection(all_polys,facecolors=all_colors,linewidths=0,shade=True,
                          lightsource=matplotlib.colors.LightSource(azdeg=110,altdeg=55))
    ax.add_collection3d(coll)
    ax.set(xlim=(-40,40),ylim=(-8,3),zlim=(-22,22)); ax.set_box_aspect((80,11,44))
    ax.view_init(elev=14,azim=-80); ax.set_axis_off(); ax.set_facecolor('#101922')
    fig.tight_layout(); fig.savefig(OUT/'scanner_preview.png',dpi=140,facecolor=fig.get_facecolor())
    ax.set_position([0,0,1,1]); fig.set_size_inches(5.12,5.12)
    fig.savefig(OUT/'scanner_icon.png',dpi=100,transparent=True)

if __name__=='__main__': main()
