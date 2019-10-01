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
        
def deleteFeedback(soup):
    div = soup.find(id="block-idz-submit-feedback-qualtrics-doc-feedback")
    if div:
        div.decompose()
        
def deleteHeader(soup):
    header = soup.find("header", class_="l-main-menu")
    if header:
        header.decompose()
        
def deleteFooter(soup):
    footer = soup.find("footer", class_="l-footer-first")
    if footer:
        footer.decompose()
        
    footer = soup.find("footer", class_="l-footer-second")
    if footer:
        footer.decompose()
        
def deleteBook(soup):
    div = soup.find("div", typeof="Book", vocab="http://schema.org/")
    if div:
        div.decompose()
        
def deleteSurvey(soup):
    div = soup.find(id="survey_container")
    if div:
        div.decompose()
        
def deleteSearchBook(soup):
    div = soup.find(id="block-idz-search-books-idz-search-book-block")
    if div:
        div.decompose()

def deleteSkipLink(soup):
    div = soup.find(id="skip-link")
    if div:
        div.decompose()
        
def deleteEmptyDiv(soup):
    div = soup.find("div", string="")
    while div:
        div.decompose()
        div = soup.find("div", string="")
        
def deleteToc(soup):
    div = soup.find("div", class_="book_sidebar_first")
    if div:
        div.decompose()

def deleteCraps(soup):
    deleteHeader(soup)
    deleteFooter(soup)
    deleteBook(soup)
    deleteFeedback(soup)
    deleteSurvey(soup)
    deleteSearchBook(soup)
    deleteSkipLink(soup)
    deleteToc(soup)
    deleteScript(soup)
    deleteInvisibleDiv(soup)
    deleteLayer(soup)
    deleteNoscript(soup)

def noDeleteCraps(soup):
    pass

if "__main__"==__name__:
    colorama.init()
    tocPathName = r"D:\Data\SWIG Users Manual.htm"
    root = r"D:\SWIGUsersManual"

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
        dumppage.dumpPage(root, url, depth, cacheurls, openurls, None, None, noDeleteCraps)
        time.sleep(1)

    with open(cacheListFilePathName, 'wb') as pklfile:
        cPickle.dump(cacheurls, pklfile, 0)