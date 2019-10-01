import os
import re
import sys
import shutil
import urllib
import bs4

def insertNavPoint(p, a, ncxSoup, playOrder):
    if isinstance(a, bs4.NavigableString):
        return

    if a.name == "a" and "href" in a.attrs:
        navPoint = ncxSoup.new_tag("navPoint")
        playOrder[0] += 1
        navPoint["playOrder"] = str(playOrder[0])
        navPoint["id"]        = "id%06d" % playOrder[0]
        navLabel = ncxSoup.new_tag("navLabel")
        content  = ncxSoup.new_tag("content")
        text     = ncxSoup.new_tag("text")
        text.string = a.string
        content["src"] = "Text/"+a["href"]

        navLabel.append(text)
        navPoint.append(navLabel)
        navPoint.append(content)
        p.append(navPoint)
        return navPoint
    else:
        for child in a.children:
            if isinstance(child, bs4.NavigableString):
                continue
            if child.name == "a" and "href" in child.attrs:
                p = insertNavPoint(p, child, ncxSoup, playOrder)
            else:
                insertNavPoint(p, child, ncxSoup, playOrder)

def makeNCXSoup(tocSoup, ncxTempPath):
    ncxSoup = bs4.BeautifulSoup(open(ncxTempPath), "xml", from_encoding="utf-8")
    ncxSoup.navMap.clear(True)
    playOrder = [0]

    p = ncxSoup.navMap
    for child in tocSoup.body.children:
        if isinstance(child, bs4.NavigableString):
            continue
        if child.name == "a" and "href" in child.attrs:
            p = insertNavPoint(ncxSoup.navMap, child, ncxSoup, playOrder)
        else:
            insertNavPoint(p, child, ncxSoup, playOrder)
    return ncxSoup

if "__main__"==__name__:
    srcHtml = r"F:\InsideTheCPPObjectModel\0201834545.html"
    dstPathName = os.path.join(os.path.dirname(srcHtml), "toc.ncx")
    ncxTempPath = r"F:\epub\toc.ncx"

    tocSoup = bs4.BeautifulSoup(open(srcHtml), "html5lib")
    ncxSoup = makeNCXSoup(tocSoup, ncxTempPath)

    with open(dstPathName, 'wb') as ofstream:
        ofstream.write(ncxSoup.prettify().encode("utf-8"))