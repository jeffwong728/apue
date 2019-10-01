import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))
import trace_pfw_tree
import trace_continuous_mesh

VTKId = 458

def getGeometry(vtkId):
    geomCommonService = _SP(SCA.getService("msc.apex.plugin.geometry.GeometryCommonService", "msc.apex.plugin.geometry.SCAIGeomCommonService"))
    r, geomSession = geomCommonService.getActiveSession()
    r, spIGeometry = geomSession.getIGeometryByVtkId(vtkId)
    if spIGeometry:
        return None, spIGeometry
    else:
        eomBody = geomSession.getEOMBodyByVtkId(vtkId)
        if eomBody:
            spGeomBody = eomBody.getCoreObject()
            r, spGeomBody = spGeomBody.getInterface("SCA.PersistenceFW.CAEData.IGeomBody")
            return spGeomBody, None
        else:
            return None, None

def traceGeomBody(spGeomBody):
    r, meshBodies = spGeomBody.getMeshBodies()
    print "traceGeomBody ", str(len(meshBodies))

def traceIGeometry(spIGeometry):
    r, refMeshes = spIGeometry.getRefMeshes()
    r, assoMeshes = spIGeometry.getAssociatedMeshes()
    level = [1]
    pattern = [1]
    print "RefMeshes"
    for feMesh in refMeshes:
            trace_pfw_tree.tracePFWSubtree(feMesh, level, pattern, True, False)
    print "AssociatedMeshes"
    for feMesh in assoMeshes:
            trace_pfw_tree.tracePFWSubtree(feMesh, level, pattern, True, False)

def traceGeometry(vtkId):
    spGeomBody, spIGeometry = getGeometry(vtkId)
    if spGeomBody:
        traceGeomBody(spGeomBody)
    elif spIGeometry:
        traceIGeometry(spIGeometry)
    else:
        print u'\u2514\u2500\u2500 Not Found Geometry '.encode('utf-8'), str(vtkId)

if "__main__"==__name__:
    traceGeometry(VTKId)