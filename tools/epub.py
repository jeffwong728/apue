import os
import re
import sys
import urllib
import bs4
import json
import uuid
import collections
import ncx
import urlparse
import zipfile
import cake
import gencover
import datetime

epubRoot    = r"D:\Data\epub\wxWidgetsProgrammingGuides"
htmlSrcDir  = r"D:\Data\epub\wxWidgetsProgrammingGuidesSRC"
epubTempDir = r"D:\tools\epub"

excludeFiles = {"meta.json", "typesetter.py", "typesetter.pyc"}

sys.path.append(htmlSrcDir)
import typesetter

def normSoup(soup, rootDir, curDir, resPathTransformDic):
    typesetter.typeset(soup, rootDir, curDir, resPathTransformDic)

def makeEpubDirHierarchy(epubRoot, epubTempDir):
    metaINFDir  = os.path.join(epubRoot, "META-INF")
    oebpsDir    = os.path.join(epubRoot, "OEBPS")
    textDir     = os.path.join(oebpsDir, "Text")
    imagesDir   = os.path.join(oebpsDir, "Images")
    stylesDir   = os.path.join(oebpsDir, "Styles")

    if not os.path.exists(epubRoot):
        os.makedirs(epubRoot)

    if not os.path.exists(metaINFDir):
        os.makedirs(metaINFDir)

    if not os.path.exists(oebpsDir):
        os.makedirs(oebpsDir)

    if not os.path.exists(textDir):
        os.makedirs(textDir)

    if not os.path.exists(imagesDir):
        os.makedirs(imagesDir)

    if not os.path.exists(stylesDir):
        os.makedirs(stylesDir)

    mimeSrcPathName = os.path.join(epubTempDir, "mimetype")
    mimeDstPathName = os.path.join(epubRoot, "mimetype")
    cake.moveFile(mimeSrcPathName, mimeDstPathName)

    containerSrcPathName = os.path.join(epubTempDir, "container.xml")
    containerDstPathName = os.path.join(metaINFDir, "container.xml")
    cake.moveFile(containerSrcPathName, containerDstPathName)

    return oebpsDir, textDir, imagesDir, stylesDir

def transformFileName(fileName, prefix, prefixs):
    if prefix in prefixs:
        transPrefix = prefixs[prefix]
        fileName = (transPrefix[0]+"%06d") % transPrefix[1] + os.path.splitext(fileName)[1]
        transPrefix[1] += 1
    return fileName

def copyEpubResources(htmlSrcDir, oebpsDir):
    excludePathNames = set()
    for excludeFile in excludeFiles:
        excludePathNames.add(os.path.join(htmlSrcDir, excludeFile))

    prefixs = { "Styles" : ["style", 0], "Images" : ["image", 0] }
    resPathTransformDic = {}
    coverPath = os.path.join(htmlSrcDir, "cover.png")

    # First parse
    for root, dirs, files in os.walk(htmlSrcDir, topdown=True, followlinks=False):
        for file in files:
            fullPath = os.path.join(root, file)
            if fullPath.endswith(".html") or fullPath.endswith(".htm"):
                pass
            elif fullPath.endswith(".exe") or fullPath.endswith(".hhc") or fullPath.endswith(".hhp") or fullPath in excludePathNames:
                print "File " + fullPath + " omitted."
            else:
                prefix = cake.getFilePrefix(fullPath)
                originalFileName = cake.makeFileNameFromPath(htmlSrcDir, fullPath)
                newFileName = transformFileName(originalFileName, prefix, prefixs)
                if fullPath==coverPath:
                    dstPathName = os.path.join(oebpsDir, originalFileName)
                else:
                    dstPathName = os.path.join(oebpsDir, prefix, newFileName)
                cake.moveFile(fullPath, dstPathName)
                resPathTransformDic[originalFileName] = "../" + prefix + "/" + newFileName

    # Second parse
    for root, dirs, files in os.walk(htmlSrcDir, topdown=True, followlinks=False):
        for file in files:
            fullPath = os.path.join(root, file)
            if fullPath.endswith(".html") or fullPath.endswith(".htm"):
                prefix = cake.getFilePrefix(fullPath)
                dstPathName = os.path.join(oebpsDir, prefix, cake.makeFileNameFromPath(htmlSrcDir, fullPath))
                soup = bs4.BeautifulSoup(open(fullPath), "html5lib", from_encoding="utf-8")
                normSoup(soup, htmlSrcDir, os.path.dirname(fullPath), resPathTransformDic)
                cake.saveSoup(soup, dstPathName)

def makeOPF(oebpsDir, epubTempDir, meta):
    opfTempPathName = os.path.join(epubTempDir, "content.opf")
    tocPathName = os.path.join(oebpsDir, "Text", "toc.html")

    resources = []
    for root, dirs, files in os.walk(oebpsDir, topdown=True, followlinks=False):
        for file in files:
            if not file.endswith(".ncx") and not file.endswith(".opf"):
                fullPath = os.path.join(root, file)
                resources.append(os.path.relpath(fullPath, oebpsDir).replace('\\', '/'))

    extensionTypeDict = {".html":"application/xhtml+xml",
                         ".htm":"application/xhtml+xml",
                         ".png":"image/png",
                         ".jpg":"image/jpeg",
                         ".jpe":"image/jpeg",
                         ".gif":"image/gif",
                         ".css":"text/css"}

    resInfoDict = collections.OrderedDict()
    for (id, href) in zip(range(len(resources)), resources):
        extension = os.path.splitext(href)[1].lower()
        if extension:
            mediaType = extensionTypeDict[extension]
            resInfoDict[href] = {"id" : "id%06d" % id, "media-type" : mediaType}
    resInfoDict["cover.png"]["id"] = "id_cover"

    tocSoup = bs4.BeautifulSoup(open(tocPathName), "html5lib", from_encoding="utf-8")
    opfSoup = bs4.BeautifulSoup(open(opfTempPathName), "xml", from_encoding="utf-8")

    manifest = opfSoup.manifest
    manifest.clear()

    ncx = opfSoup.new_tag("item")
    ncx["id"] = "id_toc"
    ncx["href"] = "toc.ncx"
    ncx["media-type"] = "application/x-dtbncx+xml"
    manifest.append(ncx)

    for resInfo in resInfoDict:
        newItem = opfSoup.new_tag("item")
        newItem["id"] = resInfoDict[resInfo]["id"]
        newItem["href"] = resInfo
        newItem["media-type"] = resInfoDict[resInfo]["media-type"]
        manifest.append(newItem)

    spine = opfSoup.spine
    spine.clear()

    tocItemref = opfSoup.new_tag("itemref")
    tocItemref["idref"] = resInfoDict["Text/toc.html"]["id"]
    tocItemref["linear"] = "yes"
    spine.append(tocItemref)

    anchors = tocSoup.find_all("a", href=True)
    hrefs = ["Text/"+urlparse.urldefrag(a["href"])[0] for a in anchors]
    hrefs = [href for href in hrefs if href in resInfoDict]
    hrefSet = set()
    idrefs = []
    for href in hrefs:
        if href not in hrefSet:
            hrefSet.add(href)
            idrefs.append(resInfoDict[href]["id"])

    for idref in idrefs:
        newItemref = opfSoup.new_tag("itemref")
        newItemref["idref"] = idref
        spine.append(newItemref)

    guide = opfSoup.guide
    guide.clear()

    newRef = opfSoup.new_tag("reference")
    newRef["type"] = "toc"
    newRef["title"] = "Table of Contents"
    newRef["href"] = "Text/toc.html"
    guide.append(newRef)

    newRef = opfSoup.new_tag("reference")
    newRef["type"] = "text"
    newRef["title"] = "Welcome"
    newRef["href"] = "Text/toc.html"
    guide.append(newRef)

    incorporateOPFMeta(opfSoup, meta)

    opfPathName = os.path.join(oebpsDir, "content.opf")
    cake.saveSoup(opfSoup, opfPathName)

    return tocSoup

def fixMeta(meta):
    if not meta["isbn"]:
        meta["isbn"] = "urn:uuid:" + str(uuid.uuid1())
    if not meta["date"]:
        meta["date"] = datetime.date.today().isoformat() 

def incorporateOPFMeta(opfSoup, meta):
    metaTagNames = [("title", "title"), ("identifier", "isbn"),
               ("creator", "creator"), ("publisher", "publisher"),
               ("subject", "subject"), ("date", "date"),
               ("description", "description")]
    for metaTagName in metaTagNames:
        metaTag = opfSoup.find(metaTagName[0])
        metaTag.string = meta[metaTagName[1]]
        #metaTag.name = "dc:"+metaTag.name

def incorporateNCXMeta(opfSoup, meta):
    metaTagNames = [("docTitle", "title"), ("docAuthor", "creator")]
    for metaTagName in metaTagNames:
        metaTag = opfSoup.find(metaTagName[0]).find("text")
        metaTag.string = meta[metaTagName[1]]

def makeNCX(tocSoup, oebpsDir, epubTempDir, meta):
    ncxTempPathName = os.path.join(epubTempDir, "toc.ncx")
    ncxSoup = ncx.makeNCXSoup(tocSoup, ncxTempPathName)

    incorporateNCXMeta(ncxSoup, meta)

    ncxPathName = os.path.join(oebpsDir, "toc.ncx")
    cake.saveSoup(ncxSoup, ncxPathName)

def bundleEpub(epubRoot):
    mimePathName = "mimetype"
    parentPath = os.path.join(epubRoot, ".."+os.sep)
    parentPath = os.path.normpath(parentPath)
    epubPathName = os.path.join(parentPath, os.path.basename(epubRoot)+".epub")

    os.chdir(epubRoot)
    with zipfile.ZipFile(epubPathName, 'w') as epub:
        epub.write("mimetype", compress_type=zipfile.ZIP_STORED)
        print "Added " + mimePathName
        for root, dirs, files in os.walk(epubRoot, topdown=True, followlinks=False):
            for file in files:
                fullPathName = os.path.relpath(os.path.join(root, file), epubRoot)
                if not fullPathName==mimePathName:
                    epub.write(fullPathName, compress_type=zipfile.ZIP_DEFLATED)
                    print "Added " + fullPathName

def genBookCover(epubTempDir, htmlSrcDir, title):
    tempCover = os.path.join(epubTempDir, "cover.gif")
    gencover.genCover(tempCover, title, (150, 150), 200, htmlSrcDir)

def loadMetaData(htmlSrcDir):
    metaPath = os.path.join(htmlSrcDir, "meta.json")
    with open(metaPath, "r") as mf:
        meta = json.load(mf, encoding="utf-8")
    return meta


if "__main__"==__name__:
    # <reference type="toc" title="Table of Contents" href="Text/bk01-toc.html"/>
    # <reference href="cover.html" type="cover" title="Cover"/>
    # <reference href="index.html" type="text"/>

    meta = loadMetaData(htmlSrcDir)
    fixMeta(meta)
    genBookCover(epubTempDir, htmlSrcDir, meta["title"])
    oebpsDir, textDir, imagesDir, stylesDir = makeEpubDirHierarchy(epubRoot, epubTempDir)
    copyEpubResources(htmlSrcDir, oebpsDir)
    tocSoup = makeOPF(oebpsDir, epubTempDir, meta)
    makeNCX(tocSoup, oebpsDir, epubTempDir, meta)
    bundleEpub(epubRoot)