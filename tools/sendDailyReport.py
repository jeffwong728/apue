import os
import ndmail
import datetime
import colorama
from colorama import Fore, Back, Style

def sendReport():
    with open("DailyReport\daily.txt") as fo:
        mainBody = fo.read()

    To = ["YANG Weiping <YANG.weiping@newdimchina.com>"]
    Cc = ["Wei Wang <WANG.Wei.1982@newdimchina.com>"]
    todayDate = datetime.date.today()
    subj = "Daily Report - " + todayDate.isoformat()

    return ndmail.sendPlainMail(To, Cc, [], subj, mainBody)

if "__main__"==__name__:
    colorama.init()
    if sendReport():
        print Fore.GREEN +"Send daily report successfully."+Style.RESET_ALL
    else:
        print Fore.RED+"Send daily report failed."+Style.RESET_ALL

    os.system("pause")