import os
import sys
import urllib2
import bs4
import cake
import msj
import urlparse
import mimetypes
import cPickle
import colorama
from colorama import Fore, Back, Style

excludeurls = {r"https://www.microsoft.com/misc/info/cpyright.htm",
               r"https://www.microsoft.com/msj/default.asp",
               r"https://www.microsoft.com/isapi/gomsdn.asp?TARGET=/msdnmag/subscribe.asp",
               r"https://www.microsoft.com/mind/"}

def deleteCraps(soup):
    cake.deleteScript(soup)
    cake.deleteInvisibleDiv(soup)
    cake.deleteLayer(soup)
    cake.deleteNoscript(soup)

def getLinkType(link):
    if "type" in link.attrs:
        return link["type"]
    if "rel" in link.attrs and link["rel"] == "stylesheet":
        return "text/css"
    return ""

def dumpLinkUrls(root, soup, url, depth, cacheurls, openurls):
    links = soup.find_all("link", rel=lambda r: r.lower()=="stylesheet")
    for link in links:
        href = urlparse.urljoin(url, link["href"])
        linkType = getLinkType(link)
        data = dumpPage(root, href, depth, cacheurls, openurls, linkType, None, None)
        normPath = urlparse.urlparse(href).path
        if data:
            link["href"] = os.path.splitext(normPath)[0] + data["extension"]
        else:
            if href in cacheurls:
                localPathName = cacheurls[href]
                link["href"] = os.path.splitext(normPath)[0] + os.path.splitext(localPathName)[1]

def dumpImageUrls(root, soup, url, depth, cacheurls, openurls):
    imgs = soup.find_all("img")
    for img in imgs:
        src = urlparse.urljoin(url, img["src"])
        data = dumpPage(root, src, depth, cacheurls, openurls, None, None, None)
        normPath = urlparse.urlparse(src).path
        if data:
            img["src"] = os.path.splitext(normPath)[0] + data["extension"]
        else:
            if src in cacheurls:
                localPathName = cacheurls[src]
                img["src"] = os.path.splitext(normPath)[0] + os.path.splitext(localPathName)[1]

def dumpAnchorUrls(root, soup, url, depth, cacheurls, openurls, cbSoup): 
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"].lower()
        if href.startswith("https://") or href.startswith("http://") or href.startswith("ftp://") or href.startswith("news://"):
            continue
        if href.startswith("#"):
            continue
        jOpenUrl = "javascript:openurl('"
        if href.startswith(jOpenUrl):
            href = href[len(jOpenUrl):(href.find("')", len(jOpenUrl)))]
        href = urlparse.urljoin(url, href)
        href = urlparse.urldefrag(href)[0]
        if href in excludeurls:
            continue
        data = dumpPage(root, href, depth, cacheurls, openurls, None, None, cbSoup)
        data = data or href in openurls and openurls[href]
        if data:
            normPath = urlparse.urlparse(href).path
            anchor["href"] = os.path.splitext(normPath)[0] + data["extension"]

def downloadURL(url, hinttype, hintextension):
    print "Downloading " + url + "..."
    headers = {'User-Agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko',
               'Connection': 'Keep-Alive', 'DNT':'1'}
    req = urllib2.Request(url=url, headers=headers)
    try:
        content=urllib2.urlopen(req)
    except urllib2.HTTPError as e:
        print Fore.RED+"Downloaded " + url + " failed(" +str(e.code)+", "+e.msg+")."+Style.RESET_ALL
        return None
    except urllib2.URLError as e:
        print Fore.RED+"Downloaded " + url + " failed(" +str(e)+")."+Style.RESET_ALL
        return None

    print "Downloaded " + url + "."

    if hinttype:
        maintype, subtype = hinttype.split("/")

    data = {}
    data["bytes"]       = content.read()
    data["charset"]     = content.info().getparam("charset")
    data["mimetype"]    = content.info().type or hinttype
    data["maintype"]    = content.info().maintype or (hinttype and maintype)
    data["subtype"]     = content.info().subtype or (hinttype and subtype)
    data["extension"]   = mimetypes.guess_extension(data["mimetype"]) or hintextension or ""

    normPath = urlparse.urlparse(url).path
    newUrl   = os.path.splitext(normPath)[0] + data["extension"]
    data["newurl"] = urlparse.urljoin(url, newUrl)

    return data

def openLocalHTMLFile(fullPathName):
    mimeType = mimetypes.guess_type(fullPathName)[0]
    if mimeType:
        maintype, subtype = mimeType.split("/")
    data = {}

    data["bytes"]       = ""
    data["charset"]     = "utf-8"
    data["mimetype"]    = mimeType
    data["maintype"]    = mimeType and maintype
    data["subtype"]     = mimeType and subtype
    data["extension"]   = os.path.splitext(fullPathName)[1]

    if data["mimetype"] == "text/html":
        with open(fullPathName, "rb") as ifstream:
            data["bytes"] = ifstream.read()

    return data

def dumpPage(root, url, depth, cacheurls, openurls, hinttype, hintextension, cbSoup):
    if depth["depth"] > depth["maxdepth"]:
        return None

    url             = urlparse.urldefrag(url)[0]
    urlPath         = urlparse.urlparse(url).path[1:]
    urlExtension    = os.path.splitext(urlPath)[1]
    urlMimeType     = mimetypes.guess_type(url)[0]

    if url not in cacheurls or not os.path.exists(cacheurls[url]) and url not in openurls:
        if url in cacheurls:
            url = cacheurls[cacheurls[url]]
        data = downloadURL(url, urlMimeType or hinttype, urlExtension or hintextension)
        if data is None:
            return None

        openurls[url]            = data
        openurls[data["newurl"]] = data
        dstPathName = os.path.join(root, urlPath)
        dstPathName = os.path.splitext(dstPathName)[0]+data["extension"]
    else:
        if url in cacheurls and cacheurls[url] not in openurls and url not in openurls:
            data = openLocalHTMLFile(cacheurls[url])
            dstPathName = cacheurls[url]
            if not data["bytes"]:
                return None
            openurls[cacheurls[url]] = data
        else:
            return None

    if data["mimetype"] == 'text/html':
        encoding = data["charset"]
        soup = bs4.BeautifulSoup(data["bytes"], "html5lib", from_encoding=encoding)
        cbSoup(soup)
        dumpLinkUrls(root, soup, url, depth.copy(), cacheurls, openurls)
        dumpImageUrls(root, soup, url, depth.copy(), cacheurls, openurls)

        depth = depth.copy()
        depth["depth"] += 1
        dumpAnchorUrls(root, soup, url, depth, cacheurls, openurls, cbSoup)

        data["bytes"] = soup.prettify().encode("utf-8")

    dstDir = os.path.dirname(dstPathName)
    if not os.path.exists(dstDir):
        os.makedirs(dstDir)

    if data["maintype"] == "text":
        mode = "w"
    else:
        mode = "wb"

    with open(dstPathName, mode) as ofstream:
        ofstream.write(data["bytes"])

    if "newurl" in data:
        cacheurls[data["newurl"]] = dstPathName
        cacheurls[dstPathName] = url
    cacheurls[url]    = dstPathName
    return data

def downloadWebPage(dstDir, url, depth, cbSoup):
    colorama.init()
    root = dstDir
    cacheListFilePathName = os.path.join(root, r"cacheurls.txt")

    if not os.path.exists(root):
        os.makedirs(root)

    if os.path.exists(cacheListFilePathName):
        with open(cacheListFilePathName, 'rb') as pklfile:
            cacheurls = cPickle.load(pklfile)
    else:
        cacheurls = {}

    traceDepth = {"depth":0, "maxdepth": depth}
    openurls = {}

    try:
        dumpPage(root, url, traceDepth, cacheurls, openurls, None, None, cbSoup)
    finally:
        with open(cacheListFilePathName, 'wb') as pklfile:
            cPickle.dump(cacheurls, pklfile, 0)

if "__main__"==__name__:
    url  = r"https://www.microsoft.com/msj/1297/complus2/complus2.aspx"
    dstDir = r"G:\msj"
    downloadWebPage(dstDir, url, 1, deleteCraps)