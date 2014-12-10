import requests
import sys
import os
from lxml import html
from lxml import etree
import xml.etree.ElementTree as ET
import shutil
import xml.dom.minidom

def process_review(review_info, output_file, count, city, hotel_name):
        # primeste div-ul ce contine un review si extrage informatii despre el
        username = review_info.xpath('//*[contains(concat(" ", \
                normalize-space(@class), " "), " username ")]')[0]. \
                xpath('span')[0].text
        userInfo = review_info.xpath('//*[@class="memberBadging"]')[0]

        quote = review_info.xpath('//*[@class="quote"]')[0].text
        # elimin ghilimelele
        quote = quote[1:]
        quote = quote[:-1]

        rating = review_info.xpath('//*[contains(concat(" ", \
                 normalize-space(@class), " "), " rating ")]')[0].\
                 xpath('span/img')[0].xpath('attribute::alt')[0]
        entry = review_info.xpath('//*[@class="entry"]')[0]
#         roomtip = entry.xpath('div')
#         if (len(roomtip) > 0):
#             roomtip = etree.tostring(roomtip[0])
#             roomtip = roomtip.split("<>")[0]
#         else:
#             roomtip = None
#         print roomtip
        entry = entry.xpath('p')[0].text[1:]

        recommend_title = review_info.xpath('//*[@class="recommend"]')[0]. \
                          xpath('li/span')[0].text
        detailed_ratings = review_info.xpath('//*[@class="recommend"]')[0]. \
                           xpath('li')[0].xpath('descendant::li')
        list_ratings = []
        for rate in detailed_ratings:
            value = rate.xpath('span/img')[0].xpath('attribute::alt')[0]
            key = rate.xpath('child::*')[0]
            key = etree.tostring(rate).split(">\n")
            key = key[len(key)-2][:-4]
            key = key.lower()
            key = key.replace(" ", "")
            tmp = (key, value)
            list_ratings.append(tmp)

        # generam fisierul xml
        root = ET.Element("root")
        root.set("id",str(count))

        info = ET.SubElement(root, "general_info")
        city_elem = ET.SubElement(info, "city")
        city_elem.text = city
        hotel_name_elem = ET.SubElement(info, "hotel")
        hotel_name_elem.text = hotel_name

        reviewer = ET.SubElement(root, "reviewer")
        username_elem = ET.SubElement(reviewer, "username")
        username_elem.text = username
        """ TODO
        userInfo_elem = ET.SubElement(reviewer, "user_info")
        userInfo_elem.text = userInfo
        """

        review = ET.SubElement(root, "review")
        quote_elem = ET.SubElement(review, "quote")
        quote_elem.text = quote
        rating_elem = ET.SubElement(review, "general_rating")
        rating_elem.text = rating
        entry_elem = ET.SubElement(review, "entry")
        entry_elem.text = entry
#         if not roomtip is None:
#             roomtip_elem = ET.SubElement(entry_elem, "roomtip")
#             roomtip_elem.text = roomtip
        recommend_title_elem = ET.SubElement(review, "recommend_title")
        recommend_title_elem.text = recommend_title
        detailed_rating_elem = ET.SubElement(review, "detailed_rating")
        for rate in list_ratings:
            detailed_rating_elem.set(rate[0], rate[1][0])

        tree = ET.ElementTree(root)
        tree.write(output_file.name)
        xmlstr = ET.tostring(root, encoding='utf8', method='xml')

        pretty_xml = xml.dom.minidom.parseString(xmlstr)
        output_file.write(pretty_xml.toprettyxml())


def process_hotel(url, count_reviews, city, hotel_name): # primeste url-ul unui hotel
    page = requests.get("http://www.tripadvisor.com"+url)
    tree = html.fromstring(page.text)
    # extrage url-urile review-urilor din pagina curenta
    reviews = tree.xpath('//*[@id="REVIEWS"]')[0]
    reviews = reviews.xpath('//*[@class="quote"]')

    for myiter in reviews:
        url_rev = myiter.xpath('child::a')[0].xpath('attribute::href')[0]
        page_rev = requests.get("http://www.tripadvisor.com"+url_rev)
        tree_rev = html.fromstring(page_rev.text)
        rev_info = tree_rev.xpath('//*[contains(concat(" ", \
                normalize-space(@class), " "), " review ")]')[0]
        output_file = open("review"+str(count_reviews)+".xml", 'w+')
        filename = "review"+str(count_reviews)+".xml"
#         output_file.write(etree.tostring(rev_info))
        process_review(rev_info, output_file, count_reviews, city, hotel_name)
        output_file.close()
        count_reviews = count_reviews + 1

    # trecem la pagina urmatoare
    nextPage = tree_rev.xpath('//*[contains(concat(" ", \
            normalize-space(@class), " "), " sprite-pageNext ")]')
    if (len(nextPage) == 0): # am ajuns la ultima pagina
        return

    nextPageURL = nextPage[0].xpath('attribute::href')[0]

    process_hotel(url, count_reviews, city, hotel_name)

def parse_hotels(input_file): # pentru fiecare hotel, face bucatarie
    all_hotels = input_file.readlines()
    for hotel in all_hotels:
        tokens = hotel.split('-')
        if (len(tokens) <= 1):
            continue
        city = tokens[5][:-6]
        hotel_name = tokens[4]
        dir_name = "Reviews_" + city + "_" + hotel_name
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        else:
            shutil.rmtree(dir_name)
            os.makedirs(dir_name)
        os.chdir(dir_name)
        process_hotel(hotel, 0, city, hotel_name)
        os.chdir("..")
        # TODO: remove the return
        return

def get_hotel_urls(url, page_no, output_file): # extrage toate url-urile hotelurilor
    sys.stdout.write('.\n')
    """
    sys.stdout.flush()
    output_file.write(str(page_no)+'\n')
    page = requests.get("http://www.tripadvisor.com"+url)
    tree = html.fromstring(page.text)
#     hotels = tree.xpath('/html/body/div[3]/div[6]/div/div/div[8]/div[4]/div[1]/div[4]/div[2]/div[2]/div')
    hotels = tree.xpath('//*[@id="ACCOM_OVERVIEW"]/div')
    for hot in hotels:
        url = hot.xpath('div/div[2]/div[1]/a[1]/attribute::href')
        if (len(url) == 1):
            output_file.write(url[0]+'\n')
    output_file.write('\n')

    footer = tree.xpath('//*[@id="pager_bottom"]')
    links_in_footer = footer[0].xpath('*')
    next_page = links_in_footer[len(links_in_footer) - 1]
    # daca ultimul element (care trebuie sa fie link-ul de "Next Page"
    # nu este un link, ci un span, atunci suntem la ultima pagina
    tmp = footer[0].xpath('a')
    last_link = tmp[len(tmp) - 1]
    if (next_page == last_link):
        url = next_page.xpath('attribute::href')[0]
        get_hotel_urls(url, page_no+1, output_file)
        """

if __name__ == '__main__':
    # aici ar trebui sa gasim un mod
    # generic de a obtine URL-ul pentru prima pagina
    init = "/Hotels-g294458-Bucharest-Hotels.html"

    #TODO: aceasta e linia corecta:
    #output_file = open("hotels_urls.out", 'w+')
    output_file = open("hotels_urls.out", 'r+')
    get_hotel_urls(init, 1, output_file)
    parse_hotels(output_file)
    output_file.close()
    print
