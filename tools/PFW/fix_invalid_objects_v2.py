# coding: utf-8

import apex
import SCA

apex.setScriptUnitSystem(unitSystemName = r'''mm-kg-s-N''')
model_1 = apex.currentModel()

#spCore = SCA.getService('SCA.EOM.EOMCore', 'SCA.EOM.IEOMUndoRedo')

print (" ")
#spCore.notifyCommandBegin(True)
res = apex.mesh.getDuplicatedSubMeshes()
#spCore.notifyCommandEnd()

print (len(res.fatalSubMeshes)," fatal meshes are detected and deleted")
for id in res.fatalSubMeshes:
    print ("fatal submesh deleted automatically : ", id)
print (" ")

print (len(res.subMeshes)," bad meshes are detected and suggested to be deleted")
for submesh in res.subMeshes:
    print ("bad submesh should be deleted : ", submesh.id)
print (" ")

if len(res.subMeshes) > 0 :
    apex.deleteEntities(res.subMeshes)
print (len(res.subMeshes)," bad meshes are deleted")
print (" ")

print (len(res.associatedGeometries)," edges may need to be remeshed to correct the model")
for geom in res.associatedGeometries:
    print ("geometry potentially needs remesh : ", geom.id)

