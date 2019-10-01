import os
import sys
from bs4 import BeautifulSoup
from Tkinter import *
from tkMessageBox import showinfo

PREDATOR_SRC_DIR = r"D:\apex_DEV_D1_PROJ_H"

def extract_all_BBTs(bbtLogPath, efBBTPath):
    soup = BeautifulSoup(open(bbtLogPath), 'html5lib')

    trTags = soup.tbody.find_all("tr")
    firstLine = True

    with open(efBBTPath, "w") as w:
        for trTag in trTags[1:]:
            statusTag = trTag.contents[3]
            if not firstLine:
                w.write("\n")
            w.write(unicode(trTag.contents[1].string))     
            firstLine = False

def extract_all_RTests(rtestLogPath, efRTestPath):
    soup = BeautifulSoup(open(rtestLogPath), 'html5lib')
    trTags = soup.tbody.find_all("tr")

    rowSpan = 0
    firstLine = True

    with open(efRTestPath, "w") as w:
        for trTag in trTags[2:]:
            firstTd = trTag.contents[1]
            if not rowSpan:
                rowSpan = int(firstTd["rowspan"])
                masterTdTag = firstTd
                statusTag = trTag.contents[5]
            else:
                statusTag = trTag.contents[3]

            if not firstLine:
                w.write("\n")
            w.write(unicode(masterTdTag.string))     
            firstLine = False

            rowSpan -= 1

def extractCases(entries, chks):
    efBBTPath       = os.path.join(PREDATOR_SRC_DIR, r"qa\ef_BBT.txt")
    efHeadlessPath  = os.path.join(PREDATOR_SRC_DIR, r"qa\ef_Headless.txt")
    efUIPath        = os.path.join(PREDATOR_SRC_DIR, r"qa\ef_UI.txt")

    bbtLogPath      = entries['BBT Log Path:'].get()
    uiLogPath       = entries['UI Log Path:'].get()
    headlessLogPath = entries['Headless Log Path:'].get()

    bbtLog      = chks['BBT Log Path:'].get()
    uiLog       = chks['UI Log Path:'].get()
    headlessLog = chks['Headless Log Path:'].get()

    if os.path.exists(bbtLogPath) and bbtLog:
        extract_all_BBTs(bbtLogPath, efBBTPath)
        os.startfile(efBBTPath)

    if os.path.exists(uiLogPath) and uiLog:
        extract_all_RTests(uiLogPath, efUIPath)
        os.startfile(efUIPath)

    if os.path.exists(headlessLogPath) and headlessLog:
        extract_all_RTests(headlessLogPath, efHeadlessPath)
        os.startfile(efHeadlessPath)

if "__main__" == __name__:
    logPathes = ('BBT Log Path:', 'UI Log Path:', 'Headless Log Path:')

    window = Tk()
    window.title("Extract NG Cases")
    window.config(width=300, height=200)
    window.minsize(width=300, height=150)

    top = Menu(window)
    window.config(menu=top)

    file = Menu(top, tearoff=False)
    file.add_command(label='Quit', command=window.quit, underline=0)
    top.add_cascade(label='File', menu=file, underline=0)
    
    form = Frame(window)
    form.pack(expand=YES, fill=BOTH, padx=10, pady=10)
    entries = {}
    chks = {}

    for (ix, label) in enumerate(logPathes):
        lab = Label(form, text=label)
        ent = Entry(form, width=80)
        var = IntVar()
        chk = Checkbutton(form, variable=var)
        lab.grid(row=ix, column=0, sticky=W)
        ent.grid(row=ix, column=1, sticky=EW)
        chk.grid(row=ix, column=2, sticky=E)
        entries[label] = ent
        chks[label] = var

    form.columnconfigure(1, weight=1)

    fetchBtn = Button(window, text="Fetch", command=lambda : extractCases(entries, chks))
    fetchBtn.pack(side=LEFT, padx=10, pady=10)

    quitBtn = Button(window, text="Quit", command=window.quit)
    quitBtn.pack(side=RIGHT, padx=10, pady=10)

    window.mainloop()