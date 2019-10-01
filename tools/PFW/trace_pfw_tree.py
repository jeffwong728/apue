import os
import re
import codecs
import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))
gscMain = _SP(SCA.getService("msc.apex.gsc.GSCMain", "msc.apex.gsc.SCAIGSCServer"))
r, gscSession = gscMain.GetActiveSession()

meshPartsDetail = dict()
rootDir = os.path.join(os.environ["HOMEDRIVE"], os.environ["HOMEPATH"], "Desktop")
geomNamePat = re.compile(u'GEOM.[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}&([0-9]+)')

def dumpMeshPartDetail(meshPart, details):
    r, guid = meshPart.getGUID()
    print '{0:016X}'.format(guid.part1 ^ guid.part2)

def tracePFWTree(fs):
    level = [0]
    pattern = [0]

    spIEOMProject = _SP(SCA.getService("SCA.EOM.EOMCore", "SCA.EOM.IEOMProject"))
    spEOMModel = spIEOMProject.getCurrentModel()
    tracePFWSubtree(fs, spEOMModel.getCoreObject(), level, pattern, True, False, False)

def isFEMesh(obj):
    r, typeName = obj.getTypeName()
    return u"Simulation.FEM.FEMesh" == typeName

def isContinuousMesh(obj):
    r, typeName = obj.getTypeName()
    return u"Simulation.FEM.ContinuousMesh" == typeName

def getMeshBindingGeometryVTKId(mesh):
    r, geom = mesh.getGeomRef()
    if geom:
        r, geomRef = geom.getGeomRef()
        return geomRef.transientId
    else:
        return None

def getEntityString(obj):
    r, typeName = obj.getTypeName()
    if u"Simulation.FEM.FEMesh" == typeName:
        return getFEMeshString(obj)
    elif u"Simulation.FEM.ContinuousMesh" == typeName:
        return getContinuousMeshString(obj)
    elif u"Simulation.MeshBody" == typeName:
        return getMeshBodyString(obj)
    elif u"Simulation.MeshPart" == typeName:
        return getMeshPartString(obj)
    else:
        return getObjString(obj)

def getMeshPartString(obj):
    r, name = obj.getName()
    r, typeName = obj.getTypeName()
    r, parent = obj.getParent()
    parentTrack = "F"
    if parent:
        parentTrack = "T"
    r, id           = obj.getId()
    r, guid         = obj.getGUID()
    r, meshPart     = obj.getInterface("SCA.PersistenceFW.CAEData.IMeshPart")
    r, elemIds      = meshPart.getElementIds(False)
    r, nodeIds      = meshPart.getNodeIds(False)
    r, numElems     = meshPart.getNumElements()
    r, numNodes     = meshPart.getNumNodes()
    r, maxElemId    = meshPart.getMaxElementId()
    r, maxNodeId    = meshPart.getMaxNodeId()
    r, maxElemIdx   = meshPart.getMaxElementIndex()
    r, maxNodeIdx   = meshPart.getMaxNodeIndex()

    outStr = r'<span class="MeshPart">'
    outStr += name + "{"
    outStr += typeName
    outStr += ", Id=" + str(id)
    outStr += ", P=" + parentTrack
    outStr += ", N=" + str(len(nodeIds)) + "(" + str(numNodes) + ")"
    outStr += ", E=" + str(len(elemIds)) + "(" + str(numElems) + ")"
    outStr += ", ME=" + str(maxElemId)
    outStr += ", MN=" + str(maxNodeId)
    outStr += ", XE=" + str(maxElemIdx)
    outStr += ", XN=" + str(maxNodeIdx)
    outStr += ", GUID={{{0:#x}, {1:#x}}}".format(guid.part1, guid.part2)
    outStr += "}</span>"

    return outStr

def getMeshBodyString(obj):
    r, name = obj.getName()
    r, id = obj.getId()
    r, typeName = obj.getTypeName()
    r, parent = obj.getParent()
    parentTrack = "F"
    if parent:
        parentTrack = "T"
    r, guid         = obj.getGUID()
    r, meshBody     = obj.getInterface("SCA.PersistenceFW.CAEData.IMeshBody")
    r, elemOrder    = meshBody.getElementOrder()
    r, elemIds      = meshBody.getElementIds(False)
    r, nodeIds      = meshBody.getNodeIds(False)
    r, numElems     = meshBody.getNumElements()
    r, numNodes     = meshBody.getNumNodes()
    r, maxElemId    = meshBody.getMaxElementId()
    r, maxNodeId    = meshBody.getMaxNodeId()
    r, geomBody     = meshBody.getGeomBody()
    vtkId = None
    if geomBody:
        r, geomRef = geomBody.getGeomRef()
        vtkId      = geomRef.transientId

    outStr = r'<span class="MeshBody">'
    outStr += name + "{"
    outStr += typeName
    outStr += ", Id=" + str(id)
    outStr += ", VTKId=NA" if vtkId is None else ", VTKId=" + str(vtkId)
    outStr += ", P=" + parentTrack
    outStr += ", O=" + str(elemOrder)
    outStr += ", N=" + str(len(nodeIds)) + "(" + str(numNodes) + ")"
    outStr += ", E=" + str(len(elemIds)) + "(" + str(numElems) + ")"
    outStr += ", ME=" + str(maxElemId)
    outStr += ", MN=" + str(maxNodeId)
    outStr += ", GUID={{{0:#x}, {1:#x}}}".format(guid.part1, guid.part2)
    outStr += "}</span>"

    return outStr

def getContinuousMeshString(obj):
    r, name = obj.getName()
    r, id = obj.getId()
    r, typeName = obj.getTypeName()
    r, parent = obj.getParent()
    parentTrack = "F"
    if parent:
        parentTrack = "T"
    r, guid      = obj.getGUID()
    r, connMesh  = obj.getInterface("SCA.PersistenceFW.CAEData.IContinuousMesh")
    r, dim       = connMesh.getDimension()
    r, elemOrder = connMesh.getElementOrder()
    r, elemIds   = connMesh.getElementIds(False)
    r, nodeIds   = connMesh.getNodeIds(False)
    r, feMeshes  = connMesh.getFEMeshes()
    r, maxElemId = connMesh.getMaxElementId()
    r, numElems  = connMesh.getNumElements()
    vtkId        = getMeshBindingGeometryVTKId(connMesh)

    outStr = r'<span class="ContinuousMesh">'
    outStr += name + "{"
    outStr += typeName
    outStr += ", Id=" + str(id)
    outStr += ", VTKId=NA" if vtkId is None else ", VTKId=" + str(vtkId)
    outStr += ", P=" + parentTrack
    outStr += ", D=" + str(dim)
    outStr += ", O=" + str(elemOrder)
    outStr += ", F=" + str(len(feMeshes))
    outStr += ", N=" + str(len(nodeIds))
    outStr += ", E=" + str(len(elemIds)) + "(" + str(numElems) + ")"
    outStr += ", ME=" + str(maxElemId)
    outStr += ", GUID={{{0:#x}, {1:#x}}}".format(guid.part1, guid.part2)
    outStr += "}</span>"

    return outStr
    
def dumpFEMeshNodes(feMesh):
    r, guid    = feMesh.getGUID()
    r, nodeIds = feMesh.getNodeIds(False)
    nodeIds.sort()
    with open(os.path.join(rootDir, "{0:#x}_{1:#x}.txt".format(guid.part1, guid.part2)), 'w') as f:
        for nodeId in nodeIds:
            f.write(str(nodeId)+'\n')

def getFEMeshString(obj):
    r, name = obj.getName()
    r, id = obj.getId()
    r, typeName = obj.getTypeName()
    r, parent = obj.getParent()
    parentTrack = "F"
    if parent:
        parentTrack = "T"
    r, guid      = obj.getGUID()
    r, feMesh    = obj.getInterface("SCA.PersistenceFW.CAEData.IFEMesh")
    r, dim       = feMesh.getDimension()
    r, elemOrder = feMesh.getElementOrder()
    r, elemIds   = feMesh.getElementIds(False)
    r, nodeIds   = feMesh.getNodeIds(False)
    r, subMeshes = feMesh.getSubMeshes(True)
    r, maxElemId = feMesh.getMaxElementId()
    r, numElems  = feMesh.getNumElements()
    vtkId        = getMeshBindingGeometryVTKId(feMesh)
    #dumpFEMeshNodes(feMesh)
    outStr = r'<span class="FEMesh">' if elemIds else (r'<span class="SubMesh">' if nodeIds else r'<span class="EmptyMesh">')
    outStr += name + "{"
    outStr += typeName
    outStr += ", Id=" + str(id)
    outStr += ", VTKId=None" if vtkId is None else ", VTKId=" + str(vtkId)
    outStr += ", P=" + parentTrack
    outStr += ", D=" + str(dim)
    outStr += ", O=" + str(elemOrder)
    outStr += ", S=" + str(len(subMeshes))
    outStr += ", N=" + str(len(nodeIds))
    outStr += ", E=" + str(len(elemIds)) + "(" + str(numElems) + ")"
    outStr += ", ME=" + str(maxElemId)
    outStr += ", GUID={{{0:#x}, {1:#x}}}".format(guid.part1, guid.part2)
    outStr += "}</span>"

    return outStr

def getObjString(obj):
    r, name = obj.getName()
    r, typeName = obj.getTypeName()
    r, id = obj.getId()
    r, parent = obj.getParent()
    parentTrack = "F"
    if parent:
        parentTrack = "T"
    m = geomNamePat.match(name)
    if m:
        vtkId = int(m.group(1))
        r, label = gscSession.GetLabelingByVTKID(vtkId)
        return name + "{" + typeName + ", Id=" + str(id) + ", VTKId=" + str(vtkId) + ", L='" + label + "', P=" + parentTrack + "}"
    else:
        return name + "{" + typeName + ", Id=" + str(id) +", P=" + parentTrack + "}"

def tracePFWSubtree(fs, obj, level, pattern, isLastItem, expandMesh, expandGeom):
    if not obj:
        return

    for n in range(0, level[0]):
        if pattern[0] & (1 << n):
            fs.write(u'\u2502   ')
        else:
            fs.write(u"    ")

    if isLastItem:
        fs.write(u'\u2514\u2500\u2500 '+unicode(getEntityString(obj))+u"\n")
    else:
        fs.write(u'\u251C\u2500\u2500 '+unicode(getEntityString(obj))+u"\n")
    
    children = []
    if expandMesh and isFEMesh(obj):
        r, feMesh = obj.getInterface("SCA.PersistenceFW.CAEData.IFEMesh")
        r, children = feMesh.getSubMeshes(True)
    elif expandMesh and isContinuousMesh(obj):
        r, connMesh  = obj.getInterface("SCA.PersistenceFW.CAEData.IContinuousMesh")
        r, children  = connMesh.getFEMeshes()
    elif 1 == expandGeom:
        r, children  = obj.getMeshBodies()
    elif 2 == expandGeom:
        r, children  = obj.getAssociatedMeshes()
    elif 3 == expandGeom:
        r, children  = obj.getRefMeshes()
    else:
        r, children = obj.getChildren()

    isLastItems = [False for i in range(0, len(children))]
    if isLastItems:
        isLastItems[-1] = True

    if children:
        if isLastItem:
            newPattern = [pattern[0]]
        else:
            newPattern = [pattern[0] | (1<<level[0])]
        for (obj, isLastItem) in zip(children, isLastItems):
            tracePFWSubtree(fs, obj, [level[0]+1], newPattern, isLastItem, expandMesh, False)

if "__main__"==__name__:
    with codecs.open(os.path.join(rootDir, 'pfw_tree.html'), 'w+', encoding='utf-8') as fs:
        fs.write('<!DOCTYPE html>\n')
        fs.write('<html>\n')
        fs.write(' <head>\n')
        fs.write('  <title>\n')
        fs.write('   PFW Object Tree\n')
        fs.write('  </title>\n')
        fs.write('  <style type="text/css">\n')
        fs.write('    .FEMesh { color:Blue; }\n')
        fs.write('    .SubMesh { color:Purple; }\n')
        fs.write('    .EmptyMesh { color:Red; }\n')
        fs.write('    .ContinuousMesh { color:Fuchsia; }\n')
        fs.write('    .MeshBody { color:Green; }\n')
        fs.write('    .MeshPart { color:Maroon; }\n')
        fs.write('  </style>\n')
        fs.write(' </head>\n')
        fs.write(' <body>\n')
        fs.write('<pre style="font-weight:bold">\n')
        fs.write('<b>PFW Object Tree</b>\n')
        tracePFWTree(fs)
        fs.write('</pre>\n')
        fs.write(' </body>\n')
        fs.write('</html>\n')
    os.startfile(os.path.join(rootDir, 'pfw_tree.html'))