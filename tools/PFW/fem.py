import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1) 
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))
UNITMGR.setCurrentUnitSystem(r'''mm-kg-s-N''')

def getConnMesh(meshName):
    meshEnt          = ROOTMGR.getByFullName(meshName)
    ret, ifMeshGroup = meshEnt.getInterface("msc.apex.plugin.fem.IPFWSupport")
    ifPFWObj         = ifMeshGroup.getPFWIPO()
    ret, ifConnMesh  = ifPFWObj.getInterface("SCA.PersistenceFW.CAEData.IContinuousMesh")
    return ifConnMesh

def getMeshPart(connMesh):
    ret, meshBody = connMesh.getParent()
    ret, meshPart = meshBody.getParent()
    ret, meshPart  = meshPart.getInterface("SCA.PersistenceFW.CAEData.IMeshPart")
    return meshPart

def getElement(meshPart, elemid):
    ret, spElement = meshPart.getElementById(elemid, False)
    return spElement

def getMeshByElement(spElement):
    ret, spMesh = spElement.getMesh()
    return spMesh

def getMeshId(spMesh):
    ret, meshId = spMesh.getId()
    return meshId

def getFEMService():
    return _SP(SCA.getService('msc.apex.plugin.fem.FEMService', 'msc.apex.plugin.fem.SCAIFEMService'))