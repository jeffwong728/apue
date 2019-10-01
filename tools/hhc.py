import bs4

soup = bs4.BeautifulSoup(open(r"D:\Data\epub\TCPSRC\0201633469.hhc"), "html5lib")

objs = soup.find_all("object")

for obj in objs:
    nameTag = obj.find("param", {"name":"Name"})
    localTag = obj.find("param", {"name":"Local"})
    if localTag:
        aTag = soup.new_tag("a", href=localTag["value"])
        aTag.string = nameTag["value"]
        obj.replace_with(aTag)
    elif nameTag:
        spanTag = soup.new_tag("span")
        spanTag.string = nameTag["value"]
        obj.replace_with(spanTag)

tags = []
nextElement = soup.body.next_element
while nextElement:
    if not isinstance(nextElement, bs4.NavigableString):
        if nextElement.name=="a" and "href" in nextElement.attrs:
            for tag in tags:
                tag.name = "a"
                tag["href"] = nextElement["href"]
            tags = []
        elif nextElement.string:
            tags.append(nextElement)
    nextElement = nextElement.next_element


with open(r"D:\Data\epub\TCPSRC\toc.html", "wb") as ofstream:
    ofstream.write(soup.prettify().encode("utf-8"))