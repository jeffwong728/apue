import os
import cons

env = Environment(tools=['default', AddLibs])

Export('env')
env['VS_VARIANT'] = 'Debug|x64'
env['VARIANT_SUFFIX'] = 'debug'
if 'debug' in COMMAND_LINE_TARGETS:
    env['BUILD_TYPE'] = cons.LINK_DEBUG
elif 'release' in COMMAND_LINE_TARGETS:
    env['BUILD_TYPE'] = cons.LINK_RELEASE
    env['VARIANT_SUFFIX'] = 'release'
    env['VS_VARIANT'] = 'Release|x64'
else:
    env['BUILD_TYPE'] = cons.LINK_DEBUG
    
env.ConfigVSBuild()

targets = []
pyext = SConscript(r'pyext\SConscript', variant_dir=os.path.join(r'#build\pyext', env['VARIANT_SUFFIX']), duplicate=0)
spam = SConscript(r'spam\SConscript', duplicate=0)
test = SConscript(r'test\SConscript', variant_dir=os.path.join(r'#build\test', env['VARIANT_SUFFIX']), duplicate=0)

targets.append(pyext)
targets.append(spam)
targets.append(test)
targets = env.Flatten(targets)

projs = [s for s in targets if str(s).endswith('.vcxproj') ]
progs = [s for s in targets if not str(s).endswith('.vcxproj') ]
#sln = env.MSVSSolution(target = 'apue' + env['MSVSSOLUTIONSUFFIX'],
#                       projects = projs,
#                       variant = env['VS_VARIANT'])
#targets.append(sln)

env.Alias('sln', projs)
env.Alias('debug', progs)
env.Alias('release', progs)
Default(spam)