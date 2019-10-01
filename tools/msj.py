import os
import sys
import urllib
from bs4 import BeautifulSoup

def isMSDNTable(table):
    trs = table.find_all("tr")
    if trs:
        trs = [tr for tr in trs if table==tr.find_parent("table")]
        if len(trs)==2:
            contentTr = trs[0]
            footerTr  = trs[1]

            contentTds = contentTr.find_all("td")
            footerTds  = footerTr.find_all("td")

            if contentTds and footerTds: 
                contentTds = [contentTd for contentTd in contentTds if contentTr==contentTd.find_parent("tr")]
                footerTds  = [footerTd for footerTd in footerTds if footerTr==footerTd.find_parent("tr")]

                if len(contentTds)==1 and len(footerTds)==2:
                    return True
    return False

def unwrapMSDNTable(soup):
    tables = soup.find_all("table")
    for table in tables:
        if isMSDNTable(table):
            trs = table.find_all("tr")
            tds = trs[0].find_all("td")
            trMainContent = trs[0] if len(tds)==1 else trs[1]
            trSignature   = trs[0] if len(tds)==2 else trs[1]
            
            tdMainContent = trMainContent.find("td")
            tdMainContent.name = "div"
            tdMainContent["class"] = "mainContent"   
            trMainContent.unwrap()
            
            tds = trSignature.find_all("td")
            for td in tds:
                td.unwrap()

            trSignature.name = "div"
            trSignature["class"] = "signature"

            if table.tbody:
                table.tbody.unwrap()

            table.unwrap()
            break

def wrapFooter(soup):
    signature = soup.find(class_='signature')
    if signature:
        nextTag = signature.find_next_sibling('i')
        if nextTag:
            signature.append(nextTag.extract())

def centerTitle(soup):
    msjheader = soup.find(src="msjheader02.gif")
    if msjheader:
        msjheader.parent['class'] = 'middle'

        nextTag = msjheader.parent.find_next_sibling('p')
        if nextTag:
            nextTag['class'] = 'middle'

        prevTag = msjheader.parent.find_previous_sibling('font')
        if prevTag:
            prevTag.wrap(soup.new_tag("div"))['class'] = 'middle'

def deleteFooter(soup):
    footer = soup.find(id="msviFooter")
    if footer:
        footer.decompose()

def deleteHeader(soup):
    header = soup.find(id="msviMasthead")
    if header:
        header.decompose()

def deleteSideTd(soup):
    em=soup.find(id="mnpMenuTop")
    if em and em.parent:
        em.parent.decompose()

def deleteEyeBrow(soup):
    eyebrow = soup.find("table", class_="downleveleyebrow")
    if eyebrow:
        eyebrow.decompose()

def changeCenterToDiv(soup):
    centers = soup.find_all("center")
    for center in centers:
        center.name = "div"

def deleteTrashAttr(soup):
    for attr in soup.body.attrs.keys():
        del soup.body[attr]

    hrs = soup.find_all("hr")
    for hr in hrs:
        if "width" in hr.attrs:
            del hr["width"]

    crs = soup.find_all(color=True)
    for cr in crs:
        if "color" in cr.attrs:
            del cr["color"]

def insertLineBreak(soup):
    imgs = soup.find_all("img", src="/msj/images/dingbats/indent.gif")
    for img in imgs:
        img.decompose()

    if imgs:
        brs = soup.find_all("br")
        for br in brs:
            el = br.next_element
            if el and el.name!="br":
                br.insert_before(soup.new_tag("br"))

def normalizeHref(soup):
    tags = soup.find_all(href=True)
    for tag in tags:
        href = urllib.unquote(tag["href"])
        if u"javascript:OpenUrl(" in href:
            tag["href"] = href[(href.find(u"'")+1):href.rfind("'")].replace(u"/", u"-")
            continue
        if u"https://www.microsoft.com/msj/archive" in href:
            tag["href"] = href[(href.rfind(u"/")+1):]
            continue
        if not href.startswith(u"http://") and not href.startswith(u"http://"):
            tag["href"] = href[(href.rfind(u"/")+1):]
            continue            

def normalizeSrc(soup):
    tags = soup.find_all(src=True)
    for tag in tags:
        src = urllib.unquote(tag["src"])
        if not src.startswith(u"http://") and not src.startswith(u"http://"):
            tag["src"] = src[(src.rfind(u"/")+1):]
            continue

def unwrapBlockQuote(soup):
    tables = soup.find_all("table")
    for table in tables:
        tags = table.find_all("blockquote", class_="dtBlock")
        for tag in tags:
            tag.unwrap()

def disableTrashLink(soup):
    anchors = soup.find_all("a", href=True)
    for anchor in anchors:
        href = anchor["href"].lower()
        if href.startswith('/') and not href.startswith('/msj'):
            anchor.name = "span"
            del anchor["href"]