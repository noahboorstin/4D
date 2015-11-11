import bpy
from array import array
from copy import deepcopy

class vec4:
    data=[]
    def __init__(self):
        self.data=[0]*4
    def add(self, vec):
        vec=vec.co
        for a in range(len(vec)):
            self.data[a]=vec[a]
        #temporary:
        self.data[3]=-1.0
    def prt(self):
        ret=""
        for a in self.data:
            ret+=str(a)+", "
        print(ret[:-len(", ")])
    def equals(self, other, tolerance):
        for a in range(3):
            if abs(self.data[a]-other.data[a])>tolerance:
                return False
        return True

def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]
def mag(a):
    return (a[0]**2+a[1]**2+a[2]**2)**.5
class triangle:
    vecs=[]
    
    def __init__(self):
        #print()
        self.vecs=[vec4() for i in range(3)]
        self.color=[0]*4
    def add(self,vec,index):
        t=vec4()
        t.add(vec)
        #self.vecs[0].prt()
        #print(".....")
        self.vecs[index]=t
    def addV(self, vec, index):
        self.vecs[index]=deepcopy(vec)
    def isPlanar(self, other, threshold):
        l1=[0]*3
        l2=[0]*3
        for a in range(3):
            l1[a]=self.vecs[0].data[a]-self.vecs[1].data[a]
            l2[a]=self.vecs[1].data[a]-self.vecs[2].data[a]
        x=cross(l1,l2)
        for a in range(3):
            l1[a]=other.vecs[0].data[a]-other.vecs[1].data[a]
            l2[a]=other.vecs[1].data[a]-other.vecs[2].data[a]
        y=cross(l1,l2)
        #print (abs(mag(cross(x,y)))/(mag(x)*mag(y)))
        return abs(mag(cross(x,y)))/(mag(x)*mag(y)) <= threshold
class line:
    a=vec4()
    b=vec4()
    tri=triangle()
    def __init__(self):
        self.a=vec4()
        self.b=vec4()
        self.tri=triangle()
    def __init__(self, v, w, t):
        self.a=v
        self.b=w
        self.tri=t
    def equals(self, other, tolerance, threshold):
        if ((self.a.equals(other.a, tolerance) & self.b.equals(other.b, tolerance)) | (self.a.equals(other.b, tolerance) & self.b.equals(other.a, tolerance))) & self.tri.isPlanar(other.tri, threshold):
            return True
        else:
            return False
    def same(self, other, tolerance):
        if ((self.a.equals(other.a, tolerance) & self.b.equals(other.b, tolerance)) | (self.a.equals(other.b, tolerance) & self.b.equals(other.a, tolerance))):
            return True
        else:
            return False
class mesh:
    triangles=[]
    color=[0]*4
    def __init__(self):
        self.triangles=[]
        self.color=[0]*4
    def toLines(self):
        ret=[]
        for a in self.triangles:
            for b in range(3):
                ret.append(line(a.vecs[b],a.vecs[(b+1)%3],a))
        return ret
        
meshes=[]

def addTri(index,vertices,msh):
    num=len(index)
    for a in range(1,num-1):
        temp=triangle()
        temp.add(vertices[index[0]],0)
        temp.add(vertices[index[a]],1)
        temp.add(vertices[index[a+1]],2)
        msh.triangles.append(temp)

for obj in bpy.context.scene.objects:
    if obj.type=="MESH":
        obj.select=True
cursor_location=bpy.context.scene.cursor_location.copy()
bpy.ops.object.mode_set(mode = 'OBJECT')
bpy.ops.object.origin_set(type='ORIGIN_CURSOR')  
bpy.context.scene.cursor_location = cursor_location
for a in bpy.data.meshes:
    m=mesh()
    for b in a.polygons:
        addTri(b.vertices,a.vertices,m)
    #do color better (correctly(?))
    color=[0]*4
    for b in range(3):
        color[b]=a.materials[0].diffuse_color[b]
    color[3]=a.materials[0].alpha
    m.color=color #[:]??
    #do animations better :///
    anim=bpy.data.objects[a.name].animation_data
    if anim!=None:
        anim=anim.action.fcurves
        t=[]
        for b in range(len(anim[0].keyframe_points)):
            u=deepcopy(m.triangles)
            for c in u:
                for d in c.vecs:
                    d.data[3]=-anim[0].keyframe_points[b].co[0]
            t.extend(deepcopy(u))
            #print(t[0].vecs[0].data[3])
        #print(m.triangles[0].vecs[0].data[3])
        lines=m.toLines()
        print(len(lines))
        b=0
        while b < len(lines):
            dup=False
            c=b+1
            while c < len(lines):
                if lines[b].equals(lines[c],0.05,0.001):
                    #print(b,c)
                    #lines[b].a.prt()
                    #lines[b].b.prt()
                    #lines[c].a.prt()
                    #lines[c].b.prt()
                    dup=True
                    del lines[c]
                else:
                    c+=1
            if dup:
                del lines[b]
            else:
                b+=1
        for c in range(len(lines)):
            d=c+1
            while d < len(lines):
                if lines[c].same(lines[d],0.05):
                    del lines[d]
                else:
                    d+=1
        print("...")
        print(len(lines))
        print(len(t))
        while len(lines) > 0:
            for c in range(len(anim[0].keyframe_points)-1):
                tri=[triangle(), triangle()]
                vec=lines[0].a
                vec.data[3]=-anim[0].keyframe_points[c].co[0]
                tri[0].addV(vec,0)
                vec.data[3]=-anim[0].keyframe_points[c+1].co[0]
                tri[0].addV(vec,1)
                tri[1].addV(vec,0)
                vec=lines[0].b
                vec.data[3]=-anim[0].keyframe_points[c+1].co[0]
                tri[1].addV(vec,1)
                vec.data[3]=-anim[0].keyframe_points[c].co[0]
                tri[0].addV(vec,2)
                tri[1].addV(vec,2)
                t.extend(deepcopy(tri))
                del lines[0]
        print(len(t))
        print("***")
        print(len(anim[0].keyframe_points))
        m.triangles=t
        print(len(m.triangles))
    meshes.append(m)
print(";;;")
f=open("/home/noah/blend/untitled.4d","wb")
w=[]
for d in meshes:
    for a in d.triangles:
        for b in a.vecs:
            #b.prt()
            for c in b.data:
                w.append(c)
            for c in d.color:
                w.append(c)
#print(w)
array("i",[int(len(w)/8)]).tofile(f)
print(len(meshes[0].triangles))
print(len(w)/8)
array("f",w).tofile(f)
array("i",[int(len(meshes))]).tofile(f)
print(int(len(meshes)))
for a in meshes:
    min=[0]*4
    max=[0]*4
    for b in a.triangles:
        for c in b.vecs:
            for d in range(len(c.data)):
                if c.data[d] < min[d]:
                    min[d]=c.data[d]
                if c.data[d] > max[d]:
                    max[d]=c.data[d]
    print(min);
    print(max);
    print();
    array("f",min).tofile(f)
    array("f",max).tofile(f)
f.close()
print(bpy.data.filepath)