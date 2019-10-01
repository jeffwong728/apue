import os
import sys
import bs4

sys.path.append(r"D:\tools")
import cake
import msj
import dumppage

dstDir = r"G:\msj"
trashLinks = ['/misc/info/cpyright.htm', '/isapi/gomsdn.asp?TARGET=/msdnmag/subscribe.asp',
                '/msj/default.asp', 'isapi/gomsdn.asp?TARGET=/', '/msj/', '/business/products/webplatform/']
trashSrcs = ["/msj/images/dingbats/indent.gif"]

tocUrl = r"https://www.microsoft.com/msj/backissues98.aspx"

def fixExtension(soup):
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"]
        if href.endswith(".aspx"):
            anchor["href"] = href.replace(".aspx", ".html")

def deleteCraps(soup):
    cake.deleteScript(soup)
    msj.deleteFooter(soup)
    msj.deleteHeader(soup)
    msj.deleteSideTd(soup)
    msj.deleteEyeBrow(soup)
    msj.disableTrashLink(soup)
    cake.deleteInvisibleDiv(soup)
    cake.deleteLayer(soup)
    cake.deleteNoscript(soup)
    cake.deleteAllMeta(soup)
    cake.addEncodingMeta(soup)
    cake.unwrapSingleCellTable(soup)
    cake.unwrapSingleColumnTable(soup)
    cake.disableTrashLink(soup, trashLinks)
    cake.deleteTrashImages(soup, trashSrcs)
    
if "__main__"==__name__:
    dumppage.downloadWebPage(dstDir or cake.getCurDirPath(), tocUrl, 2, deleteCraps)