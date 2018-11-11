import os
import cons

def ConfigVSDebugBuild(e):
    e.Append(CCFLAGS = ['/Od', '/MDd', '/Oi'])
    e.Append(LINKFLAGS = ['/OPT:NOICF', '/OPT:NOREF', '/OPT:NOLBR', r'/DEBUG'])
    e.Append(CPPDEFINES = ['_DEBUG'])

def ConfigVSReleaseBuild(e):
    e.Append(CCFLAGS = ['/O2', '/Oi', '/GL', '/Gy', '/Gm-', '/MD'])
    e.Append(LINKFLAGS = ['/LTCG:incremental', '/OPT:REF', '/OPT:ICF'])
    
def ConfigVSReleaseWithDebugBuild(e):
    e.Append(CCFLAGS = ['/O2', '/Oi', '/GL', '/Gy', '/Gm-', '/MD'])
    e.Append(LINKFLAGS = ['/INCREMENTAL:NO', '/LTCG:incremental', '/OPT:REF', '/OPT:ICF', r'/DEBUG'])

def ConfigVSBuild(e):
    e.Append(CCFLAGS = ['/EHsc'])
    e.Append(CPPDEFINES = ['NOMINMAX', 'WIN32'])
    e.Replace(PDB = '${TARGET.filebase}.pdb')
    e.Replace(WINDOWS_EMBED_MANIFEST = 1)

    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        ConfigVSDebugBuild(e)
    else:
        ConfigVSReleaseWithDebugBuild(e)