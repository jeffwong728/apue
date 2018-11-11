import cons
def IncludewxWidgets(e, l):
    e.Append(CPPDEFINES = ['__WXMSW__'])
    e.Append(CPPPATH = [r'D:\Data\wxWidgets-3.1.1\include'])
    if cons.LINK_STATIC==l:
        e.Append(LIBPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_lib'])
    else:
        e.Append(CPPDEFINES = ['WXUSINGDLL'])
        e.Append(LIBPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_dll'])

    if e['BUILD_TYPE']==cons.LINK_DEBUG:
        e.Append(CPPDEFINES = ['WXDEBUG', '__WXDEBUG__'])
        if cons.LINK_STATIC==l:
            e.Append(CPPPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_lib\mswud'])
            e.Append(LIBS = ['wxtiffd', 'wxjpegd', 'wxpngd', 'wxzlibd', 'wxregexud', 'wxexpatd'])
        else:
            e.Append(CPPPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_dll\mswud'])
        e.Append(LIBS = ['wxmsw31ud_core', 'wxbase31ud', 'wxbase31ud_net', 'wxmsw31ud_aui', 'wxmsw31ud_adv'])
    elif e['BUILD_TYPE']==cons.LINK_RELEASE:
        if cons.LINK_STATIC==l:
            e.Append(CPPPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_lib\mswu'])
            e.Append(LIBS = ['wxtiff', 'wxjpeg', 'wxpng', 'wxzlib', 'wxregexu', 'wxexpat'])
        else:
            e.Append(CPPPATH = [r'D:\Data\wxWidgets-3.1.1\lib\vc_x64_dll\mswu'])
        e.Append(LIBS = ['wxmsw31u_core', 'wxbase31u', 'wxbase31u_net', 'wxmsw31u_aui', 'wxmsw31u_adv'])