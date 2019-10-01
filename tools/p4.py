import os
import sys
import marshal
import StringIO
import subprocess

os.environ["P4PORT"]    = "ssl:172.18.129.193:1666"
os.environ["P4USER"]    = "wwang"
os.environ["P4PASSWD"]  = "9ol.8ik,"
os.environ["P4CLIENT"]  = "predator_DEV_D1"

def is_login():
    p = subprocess.Popen("p4 tickets", shell=False, stdout=subprocess.PIPE)
    stdoutdata = p.communicate()[0]

    return os.environ["P4USER"] in stdoutdata

def login():
    if is_login():
        return True

    p = subprocess.Popen("p4 login", shell=False, stdin=subprocess.PIPE)
    p.stdin.write(os.environ["P4PASSWD"])
    p.stdin.write("\n")
    p.wait()

    return not bool(p.returncode)

def logout():
    return subprocess.call("p4 logout")

def client_info():
    p = subprocess.Popen("p4 -G client -o", shell=False, stdout=subprocess.PIPE)
    stdoutdata = p.communicate()[0]
    infodict = marshal.loads(stdoutdata)

    return infodict

if "__main__"==__name__:
    login()