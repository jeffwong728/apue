import os
import re
import sys
import shutil
import urllib
from bs4 import BeautifulSoup
import msj


fromPath = re.compile("^[a-zA-Z]+ \\d{4}$")
fromHref = re.compile("/msdnmag/issues")

invalidChars = r'\/:*?"<>|'
invalidStrs  = [r'/', r':', r'?', r'->']
replaceStrs  = [r' or ', r' -', r'', r'to']

def copyResources(thisDir, htmlDir, filePath):
    if filePath.endswith(".gif") or filePath.endswith(".css"):
        newPath = os.path.join(htmlDir, filePath.replace(os.path.join(thisDir, ""), "", 1).replace(os.sep, "_"))
        shutil.copy(filePath, newPath)

def normalizeTitle(title):
    for (invalidStr, replaceStr) in zip(invalidStrs, replaceStrs):
        title = title.replace(invalidStr, replaceStr)
    return title
    
def getYearMonth(soup):
    fromString = "" 
    subscribeTags = soup.find_all(href="/msdnmag/subscribe.htm")
    for subscribeTag in subscribeTags:
        fromTag = subscribeTag.find_previous_sibling("a", href=fromHref)
        if fromTag and fromTag.string:
            fromString = fromTag.string.strip().encode("utf8")
            break
    
    if fromString:
        month, sep, year = fromString.partition(" ")
        return (year, month)
    else:
        return ("", "")
        
def moveThisImages(srcDir, dstDir, thisDir):
    imageFiles = os.listdir(srcDir)
    imageFiles = [imageFile for imageFile in imageFiles if imageFile.endswith(".gif")]
    srcMap = {}
    for imageFile in imageFiles:
        imageFullPath = os.path.join(srcDir, imageFile)
        newImageFileName = imageFullPath.replace(os.path.join(thisDir, ""), "", 1).replace(os.sep, "_")
        srcMap[imageFile] = newImageFileName
        newImageFullPath = os.path.join(dstDir, newImageFileName)
        shutil.copyfile(imageFullPath, newImageFullPath)
    return srcMap
        
def moveHtmlResources(soup, srcDir, dstDir, thisDir): 
    srcMap = moveThisImages(srcDir, dstDir, thisDir)
    hrefTags = soup.find_all(href=True)
    for hrefTag in hrefTags:
        href = urllib.unquote(hrefTag["href"])
        if href.startswith(u"/") and not href.endswith(u".htm"):
            href = href[1:]
            srcPathName = os.path.join(thisDir, href)
            dstFileName = href.replace(u"/", "_")
            dstPathName = os.path.join(dstDir, dstFileName)
            if not os.path.exists(srcPathName): print srcDir, srcPathName
            if os.path.exists(srcPathName) and not os.path.exists(dstPathName):
                shutil.copyfile(srcPathName, dstPathName)
            hrefTag["href"] = dstFileName

    imgTags = soup.find_all("img")
    for imgTag in imgTags:
        src = urllib.unquote(imgTag["src"])
        if src.startswith(u"/"):
            src = src[1:]
            srcPathName = os.path.join(thisDir, src)
            dstFileName = src.replace(u"/", "_")
            dstPathName = os.path.join(dstDir, dstFileName)
            if not os.path.exists(srcPathName): print srcDir, srcPathName
            if os.path.exists(srcPathName) and not os.path.exists(dstPathName):
                shutil.copyfile(srcPathName, dstPathName)
            imgTag["src"] = dstFileName
            continue
        if src in srcMap:
            imgTag["src"] = srcMap[src]

def setHrWidth(soup):
    hrTags = soup.find_all("hr", width=True)
    for hrTag in hrTags:
        hrTag["width"] = "100%"

def setTableWidth(soup):
    tables = soup.find_all("table", width=True)
    for table in tables:
        table["width"] = "100%"