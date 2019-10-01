import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))
import trace_pfw_tree
import trace_continuous_mesh

continuousMeshName = u'Mesh 1'
elemId = 98

def getFEMeshByContinuousMeshNameAndElementId(connMeshName, eId):
    connMeshes = trace_continuous_mesh.getContinuousMeshByName(connMeshName)
    outFEMeshes = []
    if connMeshes:
        for connMesh in connMeshes:
            r, feMeshes  = connMesh.getFEMeshes()
            for feMesh in feMeshes:
                r, elemIds   = feMesh.getElementIds(False)
                if eId in elemIds:
                    outFEMeshes.append(feMesh)
    return outFEMeshes

def traceFEMesh(connMeshName, eId):
    feMeshes = getFEMeshByContinuousMeshNameAndElementId(connMeshName, eId)
    level = [0]
    pattern = [0]
    print connMeshName + " + element " + str(eId)
    if feMeshes:
        for feMesh in feMeshes:
            trace_pfw_tree.tracePFWSubtree(feMesh, level, pattern, True, True)
    else:
        print u'\u2514\u2500\u2500 Not Found'.encode('utf-8')

if "__main__"==__name__:
    traceFEMesh(continuousMeshName, elemId)