import SCA
from MSC.Scripting.Commands import (_PIEx, _SCAEx, _SP, _EntityList)
from MSC.Scripting.Aliases  import *

ROOTMGR = _SP(SCA.getService('msc.apex.appfw.EntityFW', 'msc.apex.appfw.IRootManager'))
SCA.loadTypes("msc", 1)
import msc
UNITMGR = _SP(SCA.getService('msc.apex.appfw.Units', 'msc.apex.appfw.IUnits'))

def traceEntityTree():
    level = [0]
    pattern = [0]

    allEnts = ROOTMGR.getTopEntities().getEntities()
    isLastItems = [False for i in range(0, len(allEnts))]
    if isLastItems:
        isLastItems[-1] = True

    for (ent, isLastItem) in zip(allEnts, isLastItems):
        traceEntitySubtree(ent, level, pattern, isLastItem)

def traceEntitySubtree(ent, level, pattern, isLastItem):
    for n in range(0, level[0]):
        if pattern[0] & (1 << n):
            print u'\u2502   '.encode('utf-8'),
        else:
            print "    ",

    if isLastItem:
        print u'\u2514\u2500\u2500 '.encode('utf-8'), ent.getName(), "(", ent.getTypeName(), ")"
    else:
        print u'\u251C\u2500\u2500 '.encode('utf-8'), ent.getName(), "(", ent.getTypeName(), ")"

    allEnts = ent.getChildren().getEntities()
    isLastItems = [False for i in range(0, len(allEnts))]
    if isLastItems:
        isLastItems[-1] = True

    if allEnts:
        if isLastItem:
            newPattern = [pattern[0]]
        else:
            newPattern = [pattern[0] | (1<<level[0])]
        for (ent, isLastItem) in zip(allEnts, isLastItems):
            traceEntitySubtree(ent, [level[0]+1], newPattern, isLastItem)

traceEntityTree()