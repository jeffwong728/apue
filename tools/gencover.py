import os
import sys
import PIL.Image
import PIL.ImageFont
import PIL.ImageDraw

def genCover(tempCover, title, pos, maxWidth, dstDir):
    cover = PIL.Image.open(tempCover)
    fnt = PIL.ImageFont.truetype('timesbd.ttf', 40, index=0)
    dc = PIL.ImageDraw.Draw(cover)

    lns = splitLines(title, fnt, maxWidth)

    dc.multiline_text(pos, "\n".join(lns), font=fnt, fill=0)
    cover.save(os.path.join(dstDir, "cover.png"), "GIF")

def splitLines(strText, fnt, maxWidth):
    words = strText.split(" ") 
    start = 0
    lns = []
    while start<len(words):
        end = start+1
        ln = " ".join(words[start:end])
        w, h = fnt.getsize(ln)
        while w<maxWidth and end<len(words):
            end += 1
            ln = " ".join(words[start:end])
            w, h = fnt.getsize(ln)
        lns.append(ln)
        start = end
    return lns

if "__main__"==__name__:
    tempCover = r"D:\tools\epub\cover.gif"
    dstDir = r"F:\epub\TBBSRC"
    title = "CSS3 The Missing Manual"
    genCover(tempCover, title, (150, 150), 200, dstDir)