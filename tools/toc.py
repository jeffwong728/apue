import os
import sys
import bs4
import cake
import dumppage

#div class="optimization-notice"
#div class="book-navigation" id="book-navigation-506039"

def appendExtension(soup):
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"]
        if not href.endswith(".html") and not href.endswith(".htm"):
            anchor["href"] = href+".html"

def deleteLongClass(soup):
    ancs = soup.find_all(class_=True)
    for anc in ancs:
        del anc["class"]
    pres = soup.find_all(prefix=True)
    for pre in pres:
        del pre["prefix"]


def deleteAllLinks(soup):
    lnks = soup.find_all("link")
    for lnk in lnks:
        lnk.decompose()

def deleteCraps(soup):
    bbn = soup.find("div", id="block-book-navigation")
    bbn = bbn.extract()

    btag = soup.body
    btag.clear()
    btag.append(bbn)

    cake.deleteScript(soup)
    cake.deleteAllMeta(soup)
    cake.deleteNoscript(soup)
    cake.deleteObject(soup)
    deleteLongClass(soup)
    deleteAllLinks(soup)

if "__main__"==__name__:
    tocUrl = r"https://software.intel.com/en-us/tbb-documentation"
    dumppage.downloadWebPage(r"D:\TBB", tocUrl, 0, deleteCraps)