import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))
import trace_pfw_tree

continuousMeshName = u'Mesh 1'

def getContinuousMeshByName(connMeshName):
    spIEOMProject = _SP(SCA.getService("SCA.EOM.EOMCore", "SCA.EOM.IEOMProject"))
    spEOMModel = spIEOMProject.getCurrentModel()

    connMeshes = []
    objList = [spEOMModel.getCoreObject()]
    while objList:
        obj = objList.pop()
        if not obj:
            continue
        r, typeName = obj.getTypeName()
        r, objName = obj.getName()
        if u"Simulation.FEM.ContinuousMesh" == typeName and objName == connMeshName:
            r, connMesh  = obj.getInterface("SCA.PersistenceFW.CAEData.IContinuousMesh")
            connMeshes.append(connMesh)

        r, children = obj.getChildren()
        objList.extend(children)

    return connMeshes

def traceContinuousMesh(connMeshName):
    connMeshes = getContinuousMeshByName(connMeshName)
    level = [0]
    pattern = [0]
    print connMeshName
    if connMeshes:
        for connMesh in connMeshes:
            trace_pfw_tree.tracePFWSubtree(connMesh, level, pattern, True, True)
    else:
        print u'\u2514\u2500\u2500 Not Found'.encode('utf-8')

if "__main__"==__name__:
    traceContinuousMesh(continuousMeshName)