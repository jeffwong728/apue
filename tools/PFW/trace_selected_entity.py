import os
import codecs
import StringIO
import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

SCA.loadTypes("msc", 1)
import msc
import trace_pfw_tree

class SelectionManagerDumper:
    kNodes              = 0
    kElements           = 1
    kFEMeshs            = 2
    kContinuousMeshes   = 3
    kBody               = 4
    kRegion             = 5
    kFace               = 6
    kEdge               = 7
    kSubEdge            = 8
    kVertex             = 9
    kPart               = 10
    kAssembly           = 11
    kEntityGuard        = 12
    rootDir = os.path.join(os.environ["HOMEDRIVE"], os.environ["HOMEPATH"], "Desktop")
    elemTypeNames       = [u'FEM.Simulation.1DElement', u'FEM.Simulation.2DElement', u'FEM.Simulation.3DElement']
    conMeshTypeNames    = [u'FEM.Simulation.0DContinuousMesh', u'FEM.Simulation.1DContinuousMesh', u'FEM.Simulation.2DContinuousMesh', u'FEM.Simulation.3DContinuousMesh']
    meshTypeNames       = [u'FEM.Simulation.0DMesh', u'FEM.Simulation.1DMesh', u'FEM.Simulation.2DMesh', u'FEM.Simulation.3DMesh']
    edgeTypeNames       = [u'EDGEGENERAL', u'EDGE', u'EDGESHARE', u'EDGELAMINAR', u'EDGENORMAL', u'EDGEWIREFRAME', u'EDGEUNKNOW']
    subEdgeTypeNames    = [u'SUBEDGEWIREFRAME', u'SUBEDGELAMINAR', u'SUBEDGENORMAL', u'SUBEDGEGENERAL', u'SUBEDGESHARED']
    faceTypeNames       = [u'FACE', u'FACESHARED']
    bodyTypeNames       = [u'BODY', u'ACORNBODY', u'SHEETBODY', u'WIREBODY', u'SOLIDBODY', u'GENERALBODY', u'FACETSHEET', u'FACETSOLID', u'FACETWIRE', u'FACETGENERAL']
    partTypeNames       = [u'Simulation.Part', u'Simulation.PartInstance']
    assemblyTypeNames   = [u'Simulation.Assembly', u'Simulation.AssemblyInstance']

    def __init__(self):
        self.selMgr                 = _SP(SCA.getService('msc.apex.appfw.SelectionManager', 'msc.apex.appfw.ISelectionManager'))
        self.rootMgr                = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
        self.typeRegistry           = _SP(SCA.getService('SCA.Services.TypeRegistry', 'SCA.Services.TypeRegistry.ITypeRegistry'))
        self.nodeTypeId             = self.typeRegistry.registerTypeName(u'Simulation.FEM.Node')
        self.regionTypeId           = self.typeRegistry.registerTypeName(u'REGION')
        self.vertexTypeId           = self.typeRegistry.registerTypeName(u'VERTEX')
        self.elementTypeIds         = { self.typeRegistry.registerTypeName(typeName) for typeName in self.elemTypeNames }
        self.continuousMeshTypeIds  = { self.typeRegistry.registerTypeName(typeName) for typeName in self.conMeshTypeNames }
        self.meshTypeIds            = { self.typeRegistry.registerTypeName(typeName) for typeName in self.meshTypeNames }
        self.edgeTypeIds            = { self.typeRegistry.registerTypeName(typeName) for typeName in self.edgeTypeNames }
        self.subEdgeTypeIds         = { self.typeRegistry.registerTypeName(typeName) for typeName in self.subEdgeTypeNames }
        self.faceTypeIds            = { self.typeRegistry.registerTypeName(typeName) for typeName in self.faceTypeNames }
        self.bodyTypeIds            = { self.typeRegistry.registerTypeName(typeName) for typeName in self.bodyTypeNames }
        self.partTypeIds            = { self.typeRegistry.registerTypeName(typeName) for typeName in self.partTypeNames }
        self.assemblyTypeIds        = { self.typeRegistry.registerTypeName(typeName) for typeName in self.assemblyTypeNames }
        self.allEntities            = [[] for i in range(0, self.kEntityGuard)]
        self.elementShapes   = { SCA.PersistenceFW.CAEData.BAR : 'BAR',
                                 SCA.PersistenceFW.CAEData.TRIA : 'TRIA',
                                 SCA.PersistenceFW.CAEData.QUAD : 'QUAD',
                                 SCA.PersistenceFW.CAEData.TETRA : 'TETRA',
                                 SCA.PersistenceFW.CAEData.PENTA : 'PENTA',
                                 SCA.PersistenceFW.CAEData.HEXA : 'HEXA',
                                 SCA.PersistenceFW.CAEData.MASS : 'MASS'}
        geomCommonService   = _SP(SCA.getService("msc.apex.plugin.geometry.GeometryCommonService", "msc.apex.plugin.geometry.SCAIGeomCommonService"))
        r, self.geomSession = geomCommonService.getActiveSession()

    def dumpHTMLReport(self):
        with codecs.open(os.path.join(self.rootDir, 'selected_entities.html'), 'w+', encoding='utf-8') as fs:
            fs.write('<!DOCTYPE html>\n')
            fs.write('<html>\n')
            fs.write(' <head>\n')
            fs.write(' <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">\n')
            fs.write('  <title>\n')
            fs.write('   Selected Entities Details\n')
            fs.write('  </title>\n')
            fs.write('  <style type="text/css">\n')
            fs.write('    * { box-sizing: border-box; }\n')
            fs.write('    .FEMesh { color:Blue; }\n')
            fs.write('    .SubMesh { color:Purple; }\n')
            fs.write('    .EmptyMesh { color:Red; }\n')
            fs.write('    .ContinuousMesh { color:Fuchsia; }\n')
            fs.write('    .MeshBody { color:Green; }\n')
            fs.write('    .MeshPart { color:Maroon; }\n')
            fs.write('    .column { float: left; padding: 5px; }\n')
            fs.write('    .left { width: 40%; }\n')
            fs.write('    .right { width: 60%; }\n')
            fs.write('    .row:after { content: ""; display: table; clear: both; }\n')
            fs.write('    @media screen and (max-width: 1600px) { .column { width: 100%; } }\n')
            fs.write('    table, th, td { border: 1px solid black; border-collapse: collapse; }\n')
            fs.write('    th, td { padding: 0px 5px; }\n')
            fs.write('    thead { background-color: #7695FE; }\n')
            fs.write('    table>caption { font-weight: bold; font-size: 24px; line-height: 2em; }\n')
            fs.write('  </style>\n')
            fs.write(' </head>\n')
            fs.write(' <body>\n')
            self.dumpSelectedEntities(fs)
            fs.write(' </body>\n')
            fs.write('</html>\n')
        os.startfile(os.path.join(self.rootDir, 'selected_entities.html'))

    def dumpSelectedEntities(self, fs):
        selList = self.selMgr.selectionList()
        it      = selList.iterator()
        n, ent  = it.nextItem()

        fs.write(' <div class="row">\n')
        fs.write('  <div id="SelectedEntityReferencesList" class="column left">\n   <table>\n')
        fs.write('    <caption>Selected Entity References List</caption>\n')
        fs.write('    <thead><tr><th>ParentRefName</th><th>ParentRefTypeName</th><th>TypeName</th><th>EntityType</th><th>PrivateId</th><th>SubId</th></tr></thead>\n')
        fs.write('    <tbody>\n')

        while n:
            name         = u""
            typeName     = u""
            parentRefObj = self.getParentRefObject(ent)
            if parentRefObj:
                r, typeName = parentRefObj.getTypeName()
                r, name     = parentRefObj.getName()
            entityTypeName  =  self.typeRegistry.getTypeName(ent.entityType)
            self.sortOutEntities(ent, parentRefObj)
            fs.write('     <tr><td>{0}</td><td>{1}</td><td>{2}</td><td>{3:d}</td><td>{4:d}</td><td>{5:d}</td></tr>\n'.format(name, typeName, entityTypeName, ent.entityType, ent.privateId, ent.subId))
            n, ent = it.nextItem()
        fs.write('    </tbody>\n   </table>\n  </div>\n')

        fs.write('  <div class="column right">\n')
        self.dumpNodes(fs)
        self.dumpElements(fs)
        self.dumpContinuousMeshes(fs)
        self.dumpFEMeshes(fs)
        self.dumpBodies(fs)
        self.dumpGeometries(fs, self.kRegion,  'RegionList',  'Region')
        self.dumpGeometries(fs, self.kFace,    'FaceList',    'Face')
        self.dumpGeometries(fs, self.kEdge,    'EdgeList',    'Edge')
        self.dumpGeometries(fs, self.kSubEdge, 'SubEdgeList', 'SubEdge')
        self.dumpGeometries(fs, self.kVertex,  'VertexList',  'Vertex')
        fs.write('  </div>\n')
        fs.write(' </div>\n')

    def sortOutEntities(self, ent, parentRefObj):
        if ent.entityType == self.nodeTypeId:
            self.allEntities[self.kNodes].append((parentRefObj, ent))
        elif ent.entityType == self.vertexTypeId:
            self.allEntities[self.kVertex].append((parentRefObj, ent))
        elif ent.entityType == self.regionTypeId:
            self.allEntities[self.kRegion].append((parentRefObj, ent))
        elif ent.entityType in self.elementTypeIds:
            self.allEntities[self.kElements].append((parentRefObj, ent))
        elif ent.entityType in self.continuousMeshTypeIds:
            self.allEntities[self.kContinuousMeshes].append((parentRefObj, ent))
        elif ent.entityType in self.meshTypeIds:
            self.allEntities[self.kFEMeshs].append((parentRefObj, ent))
        elif ent.entityType in self.bodyTypeIds:
            self.allEntities[self.kBody].append((parentRefObj, ent))
        elif ent.entityType in self.faceTypeIds:
            self.allEntities[self.kFace].append((parentRefObj, ent))
        elif ent.entityType in self.edgeTypeIds:
            self.allEntities[self.kEdge].append((parentRefObj, ent))
        elif ent.entityType in self.subEdgeTypeIds:
            self.allEntities[self.kSubEdge].append((parentRefObj, ent))
        elif ent.entityType in self.partTypeIds:
            self.allEntities[self.kPart].append((parentRefObj, ent))
        elif ent.entityType in self.assemblyTypeIds:
            self.allEntities[self.kAssembly].append((parentRefObj, ent))
        else:
            pass

    def getGeometry(self, ent):
        r, spGeom     = self.geomSession.getIGeometryByVtkId(ent.privateId)
        r, spIPO      = spGeom.getParent()
        r, spGeomBody = spIPO.getInterface("SCA.PersistenceFW.CAEData.IGeomBody")
        return spGeom, spGeomBody

    def dumpNodes(self, fs):
        if self.allEntities[self.kNodes]:
            fs.write('  <div id="NodesList">\n   <table>\n')
            fs.write('    <caption>Selected Nodes List</caption>\n')
            fs.write('    <thead><tr><th>ID</th><th>Index</th><th>X</th><th>Y</th><th>Z</th><th>Coordinates</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[self.kNodes]:
                meshPart  = self.getMeshPartByPartInstance(parentRefObj)
                nodeIndex = ent.privateId
                r, nodeId = meshPart.getNodeIdFromIndex(nodeIndex)
                r, nodeStruct = meshPart.getNodeStruct(nodeIndex, True)
                fs.write('     <tr><td>{0:d}</td><td>{1:d}</td><td>{2:.9f}</td><td>{3:.9f}</td><td>{4:.9f}</td><td>({2:.9f}, {3:.9f}, {4:.9f})</td></tr>\n'.format(nodeId, nodeIndex, nodeStruct.location.x, nodeStruct.location.y, nodeStruct.location.z))
            fs.write('    </tbody>\n   </table>\n  </div>\n')

    def dumpElements(self, fs):
        if self.allEntities[self.kElements]:
            fs.write('  <div id="ElementsList">\n   <table>\n')
            fs.write('    <caption>Selected Elements List</caption>\n')
            fs.write('    <thead><tr><th>ID</th><th>Index</th><th>IDConnectivity</th><th>IndexConnectivity</th><th>MeshId</th><th>NumNodes</th><th>Shape</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[self.kElements]:
                meshPart  = self.getMeshPartByPartInstance(parentRefObj)
                elementIndex = ent.privateId
                r, elementId = meshPart.getElementIdFromIndex(elementIndex)
                elemData = SCA.PersistenceFW.CAEData.ElementData()
                elemData.options = SCA.PersistenceFW.CAEData.INCLUDE_NODE_IDS | SCA.PersistenceFW.CAEData.IDS_AS_INDEXES
                elemData.elements = []
                elemData.nodes = []
                elemData.nodeIds = []
                r, elemData = meshPart.getElementStruct(elementIndex, elemData, True)
                elementStruct = elemData.elements[0]
                nodeIndexs = elemData.nodeIds
                r, nodeIds = meshPart.getNodeIdsFromIndexes(nodeIndexs)
                fs.write('     <tr><td>{0:d}</td><td>{1:d}</td><td>{2}</td><td>{3}</td><td>{4:d}</td><td>{5:d}</td><td>{6}</td></tr>\n'.format(elementId, elementIndex, nodeIds, nodeIndexs, elementStruct.mesh_id, elementStruct.num_nodes, self.elementShapes[elementStruct.shape]))
            fs.write('    </tbody>\n   </table>\n  </div>\n')

    def dumpContinuousMeshes(self, fs):
        if self.allEntities[self.kContinuousMeshes]:
            fs.write('  <div id="ContinuousMeshesList">\n   <table>\n')
            fs.write('    <caption>Selected Continuous Meshes List</caption>\n')
            fs.write('    <thead><tr><th>Name</th><th>ID</th><th>Hierarchy</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[self.kContinuousMeshes]:
                meshPart    = self.getMeshPartByPartInstance(parentRefObj)
                meshId      = ent.privateId
                r, mesh     = meshPart.getMeshById(meshId)
                r, connMesh = mesh.getInterface("SCA.PersistenceFW.CAEData.IContinuousMesh")
                r, name     = connMesh.getName()
                meshEntity  = self.rootMgr.getByReference(ent)
                fullName    = meshEntity.getFullName()
                level = [0]
                pattern = [0L]
                ss = StringIO.StringIO()
                trace_pfw_tree.tracePFWSubtree(ss, connMesh, level, pattern, True, True, False)
                fs.write(u'     <tr><td>{0}</td><td>{1:d}</td><td><pre>{2}\n{3}</pre></td></tr>\n'.format(name, meshId, fullName, ss.getvalue()))
            fs.write('    </tbody>\n   </table>\n  </div>\n')

    def dumpFEMeshes(self, fs):
        if self.allEntities[self.kFEMeshs]:
            fs.write('  <div id="FEMeshesList">\n   <table>\n')
            fs.write('    <caption>Selected FEMeshes List</caption>\n')
            fs.write('    <thead><tr><th>Name</th><th>ID</th><th>Hierarchy</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[self.kFEMeshs]:
                meshPart    = self.getMeshPartByPartInstance(parentRefObj)
                meshId      = ent.privateId
                r, mesh     = meshPart.getMeshById(meshId)
                r, feMesh   = mesh.getInterface("SCA.PersistenceFW.CAEData.IFEMesh")
                r, name     = feMesh.getName()
                meshEntity  = self.rootMgr.getByReference(ent)
                fullName    = meshEntity.getFullName()
                level = [0]
                pattern = [0L]
                ss = StringIO.StringIO()
                trace_pfw_tree.tracePFWSubtree(ss, feMesh, level, pattern, True, True, False)
                fs.write(u'     <tr><td>{0}</td><td>{1:d}</td><td><pre>{2}\n{3}</pre></td></tr>\n'.format(name, meshId, fullName, ss.getvalue()))
            fs.write('    </tbody>   </table>\n  </div>\n')

    def dumpBodies(self, fs):
        if self.allEntities[self.kBody]:
            fs.write('  <div id="BodyList">\n   <table>\n')
            fs.write('    <caption>Selected Bodies List</caption>\n')
            fs.write('    <thead><tr><th>Name</th><th>VTK ID</th><th>Hierarchy</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[self.kBody]:
                spGeom, spGeomBody = self.getGeometry(ent)
                r, name     = spGeomBody.getName()
                bodyEntity  = self.rootMgr.getByReference(ent)
                fullName    = bodyEntity.getFullName()
                level = [0]
                pattern = [0L]
                ss = StringIO.StringIO()
                trace_pfw_tree.tracePFWSubtree(ss, spGeomBody, level, pattern, True, False, 1)
                ss.write(name + u' Associated Meshes\n')
                level = [0]
                pattern = [0L]
                trace_pfw_tree.tracePFWSubtree(ss, spGeom, level, pattern, True, False, 2)
                ss.write(name + u' Ref Meshes\n')
                level = [0]
                pattern = [0L]
                trace_pfw_tree.tracePFWSubtree(ss, spGeom, level, pattern, True, False, 3)
                fs.write(u'     <tr><td>{0}</td><td>{1:d}</td><td><pre>{2}\n{3}</pre></td></tr>\n'.format(name, ent.privateId, fullName, ss.getvalue()))
            fs.write('    </tbody>   </table>\n  </div>\n')

    def dumpGeometries(self, fs, geomType, id, caption):
        if self.allEntities[geomType]:
            fs.write('  <div id="{0}">\n   <table>\n'.format(id))
            fs.write('    <caption>Selected {0} List</caption>\n'.format(caption))
            fs.write('    <thead><tr><th>VTK ID</th><th>Hierarchy</th></tr></thead>\n')
            fs.write('    <tbody>\n')
            for parentRefObj, ent in self.allEntities[geomType]:
                spGeom, spGeomBody = self.getGeometry(ent)
                r, name     = spGeom.getName()
                level = [0]
                pattern = [0L]
                ss = StringIO.StringIO()
                ss.write(u'Associated Meshes\n')
                trace_pfw_tree.tracePFWSubtree(ss, spGeom, level, pattern, True, False, 2)
                level = [0]
                pattern = [0L]
                ss.write(u'Ref Meshes\n')
                trace_pfw_tree.tracePFWSubtree(ss, spGeom, level, pattern, True, False, 3)
                fs.write(u'     <tr><td>{0:d}</td><td><pre>{1}</pre></td></tr>\n'.format(ent.privateId, ss.getvalue()))
            fs.write('    </tbody>   </table>\n  </div>\n')

    @staticmethod
    def getMeshPartByPartInstance(partObj):
        r, partInstance = partObj.getInterface("SCA.PersistenceFW.CAEData.IInstance")
        r, masterPart   = partInstance.getMaster()
        r, part         = masterPart.getInterface("SCA.PersistenceFW.CAEData.IPart")
        r, meshParts    = part.getMeshParts()

        return meshParts[0]

    @staticmethod
    def getParentRefObject(entRef):
        if entRef.parentRef:
            r, parentRefObj = entRef.parentRef.getInterface("SCA.PersistenceFW.CAEData.IPersistentObject")
            return parentRefObj
        else:
            return None

if "__main__"==__name__:
    selMgrDumper = SelectionManagerDumper()
    selMgrDumper.dumpHTMLReport()