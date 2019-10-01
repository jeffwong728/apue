import os
import ndmail
import datetime
import colorama
from colorama import Fore, Back, Style

def sendReport():
    To = ["LENG.Junming <LENG.Junming@newdimchina.com>"]
    Cc = ["YANG Weiping <YANG.weiping@newdimchina.com>", "Wei Wang <WANG.Wei.1982@newdimchina.com>"]
    subj = "Weekly Report"

    mainBody = "Weekly report from wang wei"
    attachs = [r"DailyReport\Meshing_Weekly Report (2016.8.28-2016.9.18)_Sprint12_Template.xlsx"]
    return ndmail.sendPlainAttachmentMail(To, Cc, [], subj, mainBody, attachs)

if "__main__"==__name__:
    colorama.init()
    if sendReport():
        print Fore.GREEN +"Send weekly report successfully."+Style.RESET_ALL
    else:
        print Fore.RED+"Send weekly report failed."+Style.RESET_ALL

    os.system("pause")