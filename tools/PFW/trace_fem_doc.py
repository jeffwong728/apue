import os
import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))

def dumpFEMPFWObjs(label1, label2, geomIds, pfwObjs):
    outStr = label1
    outStr += ": (" + str(len(geomIds)) + ")\n"
    
    for (geomId, pfwObj) in zip(geomIds, pfwObjs):
        outStr += label2 + "[" + str(geomId) + " --> "
        if pfwObj:
            feMesh = None
            meshBody = None
            try:
                r, feMesh   = pfwObj.getInterface("SCA.PersistenceFW.CAEData.IFEMesh")
            except SCA.SCASystemException:
                pass

            try:
                r, meshBody = pfwObj.getInterface("SCA.PersistenceFW.CAEData.IMeshBody")
            except SCA.SCASystemException:
                pass

            if feMesh:
                r, name    = feMesh.getName()
                r, typeName = feMesh.getTypeName()
                r, id      = feMesh.getId()
                outStr += "(" + name + "=="
                outStr += typeName + ", Id=" + str(id) + ")"

            if meshBody:
                r, name    = meshBody.getName()
                r, typeName = meshBody.getTypeName()
                r, id      = meshBody.getId()
                outStr += "(" + name + "=="
                outStr += typeName + ", Id=" + str(id) + ")"
        else:
            outStr += "NULLSP"

        outStr += "]\n"

    return outStr

def dumpFEMObjIdToGeomIdMap(label1, label2, femObjIdToGeomIds):
    outStr = label1 + ": (" + str(len(femObjIdToGeomIds)/2) + ")\n"
    for ii in range(0, len(femObjIdToGeomIds)/2):
        outStr += label2 + "[" + str(femObjIdToGeomIds[2*ii]) + " --> " + str(femObjIdToGeomIds[2*ii+1]) + "]\n"

    return outStr

def dumpFEMPFWObjsEx(label1, label2, geomIds, pfwObjs):
    outStr = label1 + ": (" + str(len(geomIds)) + ")\n"
    for (geomId, pfwObj) in zip(geomIds, pfwObjs):
        if pfwObj:
            mesh = None
            try:
                r, mesh = pfwObj.getInterface("SCA.PersistenceFW.CAEData.IMesh")
            except SCA.SCASystemException:
                pass

            if mesh:
                r, name    = mesh.getName()
                r, typeName = mesh.getTypeName()
                r, id      = mesh.getId()
                outStr += label2 + "----[" + str(geomId) + " --> "
                outStr += "(" + name + "=="
                outStr += typeName + ", Id=" + str(id) + ")]"
            else:
                outStr += label2 + "----[" + str(geomId) + " --> NULLSP]"

        elif geomId:
            outStr += label2 + "GeomBody ID = " + str(geomId)
        else:
            outStr += label2 + "----[" + str(geomId) + " --> NULLSP]"
        outStr += "\n"

    return outStr

def dumpFEMDoc():
    femTest = _SP(SCA.getService("msc.apex.plugin.fem.FEMPluginTest", "msc.apex.plugin.fem.SCAIFEMTest"))
    idSeqSeq, ifSeqSeq = femTest.getFEMDocData()
    outStr = "+FEMDoc\n"

    outStr += dumpFEMPFWObjs(" |---GeomId to MeshBody", " |-------", idSeqSeq[0], ifSeqSeq[0])
    outStr += dumpFEMObjIdToGeomIdMap(" |---MeshBodyId to GeomId", " |-------", idSeqSeq[1])

    outStr += dumpFEMPFWObjs(" |---GeomId to FEMesh", " |-------", idSeqSeq[2], ifSeqSeq[2])
    outStr += dumpFEMObjIdToGeomIdMap(" |---FEMeshId to GeomId", " |-------", idSeqSeq[3])

    outStr += dumpFEMPFWObjs(" |---GeomId to SubMesh", " |-------", idSeqSeq[4], ifSeqSeq[4])
    outStr += dumpFEMObjIdToGeomIdMap(" |---SubMeshId to GeomId", " |-------", idSeqSeq[5])
    
    outStr += dumpFEMPFWObjsEx(" |---GeomId to ConntinuousMeshes", " |-------", idSeqSeq[6], ifSeqSeq[6])
    outStr += dumpFEMObjIdToGeomIdMap(" |---ConntinuousMeshId to GeomId", " |-------", idSeqSeq[7])

    outStr += dumpFEMPFWObjsEx(" |---GeomId to 1D ExtractMeshes", " |-------", idSeqSeq[8], ifSeqSeq[8])
    outStr += dumpFEMPFWObjsEx(" |---GeomId to 2D ExtractMeshes", " |-------", idSeqSeq[9], ifSeqSeq[9])

    print outStr

dumpFEMDoc()