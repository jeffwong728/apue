import os
import sys
import shutil

CodeDirPath = r"E:\predator_DEV_D1"
ModeDirVarName = "WORKSPACE_OUTPUT_ROOT"
ScriptToolDirPath = r"D:\tools"

class DirError(IOError):
    def __init__(self, dirPath):
        IOError.__init__(self)
        self.filename = dirPath

def getCurDirPath():
    dirPath = os.path.dirname(sys.argv[0])
    if not dirPath:
        dirPath = os.getcwd()
    return dirPath
    
def copyBBTFile():
    rootPath = r"\\10.122.65.165\Devl\builds\predator\BBT\DEV_D1"
    curDirPath = getCurDirPath()
    modCL = os.path.basename(curDirPath)[-6:]
    BBTFileName = "BBT-DEV_D1-"+modCL+".7z"
    BBTFilePath = os.path.join(rootPath, modCL, BBTFileName)
    
    if not os.path.exists(BBTFilePath):
        print "Can't locate BBT 7z file: " + BBTFilePath
        return
    
    dstPath = os.path.join(CodeDirPath, r"qa\tests\gui\BBT", BBTFileName)
    if not os.path.exists(dstPath):
        print "Copy BBT 7z file: " + BBTFilePath + " to " + dstPath + "..."
        shutil.copy(BBTFilePath, dstPath)
    else:
        print dstPath + " already exists!!!"
        
    path7Z = r"C:\Program Files\7-Zip;"
    if path7Z not in os.environ["PATH"]:
        os.environ["PATH"] = path7Z + os.environ["PATH"]
    
    print "Start decompress " + dstPath + "..."
    os.system("7z.exe x " + dstPath + " -y")
    
def modifyLicenseSetting(runBatFileName):
    curDirPath      = getCurDirPath()
    runBatFilePath  = os.path.join(curDirPath, runBatFileName)
    print "Modify license setting in " + runBatFileName
    backFilePath    = runBatFilePath + ".back"
    
    try:
        print "Backup file " + runBatFileName + " to " + runBatFileName + ".back" + " before modify."
        shutil.copyfile(runBatFilePath, backFilePath)
        with open(backFilePath) as f, open(runBatFilePath, 'w') as w:
            modified = 0
            for ln in f:
                if modified<2 and 'set "MSC_LICENSE_FILE=' in ln:
                    if modified<1:
                        modLn ='    set "MSC_LICENSE_FILE=%MSC_LICENSE_FILE%;27500@10.122.65.100"' + "\n"
                    else:
                        modLn ='    set "MSC_LICENSE_FILE=27500@10.122.65.100"' + "\n"
                    
                    print "<-"+ln,
                    print "->"+modLn,
                    w.write(modLn)
                    modified += 1
                else:
                    w.write(ln)
    except Exception:
        shutil.copyfile(backFilePath, runBatFilePath)
        raise
    finally:
        if os.path.exists(backFilePath):
            print "Delete backup file " + backFilePath
            os.remove(backFilePath)
    
def modifyModPath():
    curDirPath = getCurDirPath()
    if not os.path.exists(curDirPath):
        raise IOError(curDirPath)
        
    implFilePath = os.path.join(CodeDirPath, "tools\\sandbox\\implementation\\sandboxInit_Impl_local.bat")
    backFilePath = implFilePath + ".back"
    
    try:
        shutil.copyfile(implFilePath, backFilePath)
        with open(backFilePath) as f, open(implFilePath, 'w') as w:
            modified = False
            for ln in f:
                if not modified and ModeDirVarName in ln:
                    pos = ln.find(ModeDirVarName)
                    modLn = ln[:pos+len(ModeDirVarName)+1] + curDirPath + "\n"
                    print "<-"+ln,
                    print "->"+modLn,
                    w.write(modLn)
                    modified = True
                else:
                    w.write(ln)
    except Exception:
        shutil.copyfile(backFilePath, implFilePath)
        raise
    finally:
        if os.path.exists(backFilePath):
            os.remove(backFilePath)
            
def modifyTestBat():
    testBatPath = os.path.join(CodeDirPath, r"qa\runWangWeiTest.bat")
	
def newRunMSCApexQABat():
	with open('runMSC_Apex_qa.bat', 'w') as w:
		w.write('call runMSC_Apex.bat -qa')
        
def copyFEMScript():
    femScriptSrcPath = os.path.join(ScriptToolDirPath, "copyCleanFEM.py")
    if os.path.exists(femScriptSrcPath):
        femScriptDstPath = os.path.join(getCurDirPath(), r"Plugins\copyCleanFEM.py")
        shutil.copyfile(femScriptSrcPath, femScriptDstPath)

if "__main__"==__name__:
    modifyModPath()
    modifyLicenseSetting("runModTest.bat")
    modifyLicenseSetting("runMSC_Apex.bat")
    copyBBTFile()
    newRunMSCApexQABat()
    copyFEMScript()
    os.system("pause")