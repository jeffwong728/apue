import os
import sys
import bs4
import cake

fullPath = r"G:\eh\How a C++ compiler implements exception handling - CodeProject.htm"
trashClasses = {"site-top-menu", "site-header", "sub-headerbar",
                "article-wing-left", "article-wing-right", "voting-bar",
                "theme1-background", "extended tiny-text", "share-list", "pic-wrapper"}

def deleteFrills(soup):
    frills = soup.find_all(class_ = lambda cls : cls in trashClasses)
    for frill in frills:
        frill.decompose()

    div = soup.find(class_="container-breadcrumb")
    if div:
        div = div.find_parent("div", class_="clearfix")
        if div:
            div.decompose()

    msg = soup.find(id="_MessageBoardctl00_MessageBoard")
    if msg:
        msg.decompose()

    div = soup.find("div", class_="article")
    next = div.next_sibling
    while next:
        div = next
        next = next.next_sibling
        if not isinstance(div, bs4.NavigableString):
            div.decompose()

    tag = soup.find(class_="container-article-parts")
    next = tag.next_sibling
    while next:
        tag = next
        next = next.next_sibling
        if not isinstance(tag, bs4.NavigableString):
            tag.decompose()

if "__main__"==__name__:
    soup = bs4.BeautifulSoup(open(fullPath, "rb"), "html5lib", from_encoding="utf-8-sig")
    cake.deleteScript(soup)
    cake.deleteObject(soup)
    cake.deleteNoscript(soup)
    cake.deleteTrashLink(soup)
    cake.deleteAllMeta(soup)
    cake.addEncodingMeta(soup)
    deleteFrills(soup)
    cake.saveSoup(soup, fullPath)