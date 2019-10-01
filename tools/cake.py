import os
import re
import sys
import bs4
import shutil

winPath = re.compile("[A-Z]:[/\\\\](?:[^\\\\/:*?\"<>|]+[/\\\\])*[^\\\\/:*?\"<>|]*")

def isValidPath(path):
    mo = winPath.match(path)
    if mo and mo.end()-mo.start()==len(path):
        return True
    else:
        return False

def getCurDirPath():
    dirPath = os.path.dirname(sys.argv[0])
    if not dirPath:
        dirPath = os.getcwd()
    return dirPath

def isSingleCellTable(table):
    trs = table.find_all("tr")
    tds = table.find_all("td")

    if trs and tds:
        trs = [tr for tr in trs if table==tr.find_parent("table")]
        tds = [td for td in tds if table==td.find_parent("table")]
        if (1==len(trs)) and (1==len(tds)):
            return True
    
    return False

def isSingleColumnTable(table):
    trs = table.find_all("tr")
    if trs:
        trs = [tr for tr in trs if table==tr.find_parent("table")]
    else:
        return False

    for tr in trs:
        tds = tr.find_all("td")
        if tds:
            tds = [td for td in tds if table==td.find_parent("table")]
            if not 1==len(tds):
                return False
        else:
            return False

    return True

def deleteScript(soup):
    scripts = soup.find_all("script")
    for script in scripts:
        script.decompose()

def deleteForm(soup):
    forms = soup.find_all("form")
    for form in forms:
        form.decompose()

def deleteObject(soup):
    objs = soup.find_all("object")
    for obj in objs:
        obj.decompose()

def deleteLayer(soup):
    layers = soup.find_all("layer")
    for layer in layers:
        if u"visibility" in layer.attrs and layer["visibility"]=="hide":
            layer.decompose()

def deleteNoscript(soup):
    noscript = soup.find("noscript")
    if noscript:
        noscript.decompose()

def deleteInvisibleDiv(soup):
    divs = soup.find_all("div", style="display:none")
    for div in divs:
        div.decompose()

def deleteTrashLink(soup):
    links = soup.find_all("link", rel=lambda rel: rel and rel.lower()!="stylesheet")
    for link in links:
        link.decompose()

def deleteComplexCSSLink(soup):
    links = soup.find_all("link", rel="stylesheet")
    for link in links:
        link.decompose()

def linkKindleStylesheet(soup):
    link = soup.new_tag("link", href="/kindle.css", rel="stylesheet", type="text/css")
    soup.head.append(link)

def unwrapSingleCellTable(soup):
    while True:
        tables = soup.find_all("table")
        for table in tables:
            if isSingleCellTable(table):
                if table.td:
                    if table.td.has_attr("bgcolor") or table.td.has_attr("class"):
                        #table.td["class"] = "clsCell"
                        if table.td.has_attr("bgcolor"):
                            bgcolor = table.td["bgcolor"]
                            style = "background:"+bgcolor+";"
                            table.td["style"] = style if not table.td.has_attr("style") else style + table.td["style"]
                            del table.td["bgcolor"]
                        table.td.name = "div"
                    else:
                        table.td.unwrap()
                if table.tr:
                    table.tr.unwrap()
                if table.tbody:
                    table.tbody.unwrap()
                if table.has_attr("class"):
                    table.name = "div"
                    for attr in list(table.attrs):
                        if not attr=="class":
                            del table[attr]
                else:
                    table.unwrap()
                break
        else:
            break

def unwrapSingleColumnTable(soup):
    while True:
        tables = soup.find_all("table")
        for table in tables:
            if isSingleColumnTable(table):
                tds = table.find_all("td")
                if tds:
                    tds = [td for td in tds if table==td.find_parent("table")]
                    for td in tds:
                        if td.has_attr("bgcolor"):
                            bgcolor = td["bgcolor"]
                            style = "background:"+bgcolor+";"
                            td["style"] = style if not td.has_attr("style") else style + td["style"]
                            del td["bgcolor"]
                        td.name = "div"

                trs = table.find_all("tr")
                if trs:
                    trs = [tr for tr in trs if table==tr.find_parent("table")]
                    for tr in trs:
                        tr.unwrap()

                if table.tbody:
                    table.tbody.unwrap()
                if table.has_attr("class"):
                    table.name = "div"
                    for attr in list(table.attrs):
                        if not attr=="class":
                            del table[attr]
                else:
                    table.unwrap()
                break
        else:
            break

def deleteXMLNameSpace(soup):
    tags = soup.find_all(xmlns=True)
    for tag in tags:
        del tag['xmlns']

def makeFileNameFromPath(startPath, fullPath):
    relPath = os.path.relpath(fullPath, startPath)
    return relPath.replace(os.sep, '_')

def moveFile(srcPathName, dstPathName):
    if os.path.exists(srcPathName) and not os.path.exists(dstPathName):
        shutil.copy(srcPathName, dstPathName)

def saveSoup(soup, pathName):
    with open(os.path.join(pathName), 'wb') as ofstream:
        ofstream.write(soup.prettify().encode("utf-8"))

def makeFileNameFromUrl(url, rootDir, curDir):
    if url.startswith('/'):
        url = url[1:]
        absPath = os.path.join(rootDir, url)
    else:
        absPath = os.path.join(curDir, url)

    return makeFileNameFromPath(rootDir, absPath)

def normLinkPath(soup, rootDir, curDir, resPathTransformDic):
    links = soup.find_all("link", href=True)
    for link in links:
        href = link["href"]
        if href.startswith("https://") or href.startswith("http://"):
            continue
        originalFileName = makeFileNameFromUrl(href, rootDir, curDir)
        if originalFileName in resPathTransformDic:
            link["href"] = resPathTransformDic[originalFileName]
        else:
            link["href"] = originalFileName

def normAnchorPath(soup, rootDir, curDir):
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"]
        if href.startswith("https://") or href.startswith("http://"):
            continue
        if href.startswith("#"):
            continue
        jOpenUrl = "javascript:OpenUrl('"
        if href.startswith(jOpenUrl):
            href = href[len(jOpenUrl):(href.find("')", len(jOpenUrl)))]
        anchor["href"] = makeFileNameFromUrl(href, rootDir, curDir)

def normImgPath(soup, rootDir, curDir, resPathTransformDic):
    imgs = soup.find_all("img", src=True)
    for img in imgs:
        src = img["src"]
        if src.startswith("https://") or src.startswith("http://"):
            continue
        originalFileName = makeFileNameFromUrl(src, rootDir, curDir)
        if originalFileName in resPathTransformDic:
            img["src"] = resPathTransformDic[originalFileName]
        else:
            img["src"] = originalFileName

def normTagPath(soup, rootDir, curDir, resPathTransformDic):
    normLinkPath(soup, rootDir, curDir, resPathTransformDic)
    normAnchorPath(soup, rootDir, curDir)
    normImgPath(soup, rootDir, curDir, resPathTransformDic)

def processPreLine(line):
    words = line.split(" ")
    longestWord = max(words, key=lambda w: len(w))
    if len(longestWord) > 50:
        idx = words.index(longestWord)
        mid = len(longestWord)/2
        wbrWord = longestWord[0:mid]+r"<wbr/>"+longestWord[mid:]
        words[idx] = wbrWord
        return " ".join(words)
    else:
        return line

def splitPreLine(soup):
    pres = soup.find_all("pre")
    for pre in pres:
        if pre.string:
            lines = pre.string.splitlines()
            lines = [processPreLine(line) for line in lines]
            preString = "\n".join(lines) + "\n"
            preStrings = preString.split(r"<wbr/>")
            pre.clear()
            pre.append(preStrings[0])
            for str in preStrings[1:]:
                pre.append(soup.new_tag("wbr"))
                pre.append(str)

def getPageUrlsFromToc(soup):
    nextElement = soup.body.next_element
    urls = []
    while nextElement:
        if nextElement.name=="a" and "href" in nextElement.attrs:
            urls.append(nextElement["href"])
        nextElement = nextElement.next_element
    return urls

def getFilePrefix(fullPath):
    ext = os.path.splitext(fullPath)[1].lower()
    fpDict = {".htm" : "Text",
              ".html" : "Text",
              ".css" : "Styles",
              ".jpe": "Images",
              ".jpg": "Images",
              ".jpeg": "Images",
              ".gif": "Images",
              ".png": "Images",
              ".giff": "Images"}
    if ext in fpDict:
        return fpDict[ext]
    else:
        return ""

def deleteAllMeta(soup):
    metas = soup.find_all("meta")
    for meta in metas:
        meta.decompose()

def addEncodingMeta(soup):
    head = soup.head
    encodingMeta = soup.new_tag("meta")
    encodingMeta["http-equiv"] = "content-type"
    encodingMeta["content"] = "text/html; charset=UTF-8"
    head.insert(0, encodingMeta)

def disableExternalHyperlink(soup):
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"]
        if href.startswith("https://") or href.startswith("http://") or href.endswith(".exe"):
            anchor.name = "span"
            del anchor["href"]

def disableTrashLink(soup, hrefs):
    hrefs = { h.lower() for h in hrefs }
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"].lower()
        if href in hrefs:
            anchor.name = "span"
            del anchor["href"]

def deleteTrashImages(soup, srcs):
    srcs = { s.lower() for s in srcs }
    imgs = soup.find_all("img", src=True)
    for img in imgs:
        src = img["src"].lower()
        if src in srcs:
            img.decompose()

def regularizeTable(soup):
    tables = soup.find_all("table")
    for table in tables:
        for attr in table.attrs.keys():
            del table[attr]