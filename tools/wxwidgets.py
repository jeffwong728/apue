import os
import sys
import urllib2
import bs4
import msj
import cake
import urlparse
import mimetypes
import cPickle
import colorama
import dumppage
import time
from colorama import Fore, Back, Style

def deleteScript(soup):
    scripts = soup.find_all("script")
    for script in scripts:
        script.decompose()
        
def deleteHeader(soup):
    header = soup.find("div", id="top")
    if header:
        header.decompose()
        
def deleteFooter(soup):
    footer = soup.find("address", class_="footer")
    if footer:
        footer.decompose()
        
def deleteToc(soup):
    div = soup.find("div", class_="toc")
    if div:
        div.decompose()

def deleteCraps(soup):
    deleteHeader(soup)
    deleteFooter(soup)
    deleteToc(soup)
    deleteScript(soup)

if "__main__"==__name__:
    colorama.init()
    tocPathName = r"D:\Data\wxWidgets Programming Guides.htm"
    root = r"D:\wxWidgets Programming Guides"

    tocSoup = bs4.BeautifulSoup(open(tocPathName), "html5lib", from_encoding="utf-8")
    urls = cake.getPageUrlsFromToc(tocSoup)

    cacheListFilePathName = os.path.join(root, r"cacheurls.txt")

    if not os.path.exists(root):
        os.makedirs(root)

    if os.path.exists(cacheListFilePathName):
        with open(cacheListFilePathName, 'rb') as pklfile:
            cacheurls = cPickle.load(pklfile)
    else:
        cacheurls = {}

    openurls = {}
    depth = {"depth":0, "maxdepth": 0}

    for url in urls:
        dumppage.dumpPage(root, url, depth, cacheurls, openurls, None, None, deleteCraps)
        time.sleep(1)

    with open(cacheListFilePathName, 'wb') as pklfile:
        cPickle.dump(cacheurls, pklfile, 0)