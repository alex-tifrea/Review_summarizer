#! /usr/bin/python

import unicodedata
import requests
import argparse
import sys
import os
from lxml import html
from lxml import etree
import xml.etree.ElementTree as ET
import shutil
import xml.dom.minidom

# TODO: when given --get flag, get the review list in hotels_urls.out
# TODO: when given --save flag, save the reviews that are listed in
# hotels_urls.out
# TODO: when given the --roomtip flag, get the roomtip advice too

enable_roomtip = False
count_review_page = 10

# @args the div that contains a review
# extracts information about the given review
def process_review(review_info, output_file, count, city, hotel_name):
        global enable_roomtip

        # get info about the user that submitted the review
        username = review_info.xpath('//*[contains(concat(" ", \
                normalize-space(@class), " "), " username ")]')[0]. \
                xpath('span')[0].text
        userInfo = review_info.xpath('//*[@class="memberBadging"]')[0]

        # get the title of the review
        quote = review_info.xpath('//*[@class="quote"]')[0].text
        # get rid of quotes
        quote = quote[1:]
        quote = quote[:-1]

        # get the rating
        rating = review_info.xpath('//*[contains(concat(" ", \
                 normalize-space(@class), " "), " rating ")]')[0].\
                 xpath('span/img')[0].xpath('attribute::alt')[0]

        # get the text of the review
        entry = review_info.xpath('//*[@class="entry"]')[0]

        # get roomtip info if the --roomtip flag was given
        if enable_roomtip:
            roomtip = entry.xpath('div')
            if (len(roomtip) > 0):
                roomtip = etree.tostring(roomtip[0])
                roomtip = roomtip.split("<>")[0]
            else:
                roomtip = None
            print roomtip
        entry = unicode(entry.xpath('p')[0].text[1:])

        # convert entry to ASCII in case it has glyph characters
        entry = unicodedata.normalize('NFKD',entry).encode('ascii', 'ignore')

        # get the detailed rating (i.e. for location, cleanliness, service etc.)
        recommend_title = review_info.xpath('//*[@class="recommend"]')[0]. \
                          xpath('li/span')

        # if recommend_title is not empty then proceed to gettin the detailed
        # ratings
        list_ratings = []
        if recommend_title:
            recommend_title = recommend_title[0].text
            detailed_ratings = review_info.xpath('//*[@class="recommend"]')[0]. \
                               xpath('li')[0].xpath('descendant::li')
            for rate in detailed_ratings:
                value = rate.xpath('span/img')[0].xpath('attribute::alt')[0]
                key = rate.xpath('child::*')[0]
                key = etree.tostring(rate).split(">\n")
                key = key[len(key)-2][:-4]
                key = key.lower()
                key = key.replace(" ", "")
                tmp = (key, value)
                list_ratings.append(tmp)

        # generate the xml file
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

        if enable_roomtip:
            if not roomtip is None:
                roomtip_elem = ET.SubElement(entry_elem, "roomtip")
                roomtip_elem.text = roomtip

        recommend_title_elem = ET.SubElement(review, "recommend_title")
        recommend_title_elem.text = recommend_title

        # if there were any detailed ratings
        if list_ratings:
            detailed_rating_elem = ET.SubElement(review, "detailed_rating")
            for rate in list_ratings:
                detailed_rating_elem.set(rate[0], rate[1][0])

        tree = ET.ElementTree(root)
        tree.write(output_file.name)
        xmlstr = ET.tostring(root, encoding='utf8', method='xml')

        try:
            output_file.write(xmlstr)
            pretty_xml = xml.dom.minidom.parseString(xmlstr)
            output_file.write(pretty_xml.toprettyxml())
        except Exception:
            pass

# @args the url of a hotel
# extracts the reviews for the given hotel and calls process_review to create
# the coresponding xml file
def process_hotel(url, count_reviews, city, hotel_name):
    print url
    global count_review_page
    page = requests.get("http://www.tripadvisor.com"+url)
    tree = html.fromstring(page.text)

    # extract urls for the reviews in the current page
    reviews = tree.xpath('//*[@id="REVIEWS"]')[0]

    reviews = reviews.xpath('//*[contains(concat(" ", \
            normalize-space(@class), " "), " quote ")]')

    print "Pe pagina asta am",len(reviews),"review-uri"

    for myiter in reviews:
        url_rev = myiter.xpath('child::a')[0].xpath('attribute::href')[0]
        sys.stdout.write('.')
        sys.stdout.flush()
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

    # move on to the next page
    nextPage = tree.xpath('//*[contains(concat(" ", \
            normalize-space(@class), " "), " sprite-pageNext ")]')
    if (len(nextPage) == 0): # am ajuns la ultima pagina
        if count_review_page == 10:
            new_url = url.split("-Reviews")
            nextPageURL = new_url[0] + "-Reviews" + "-or10" + new_url[1][0:len(new_url[1])-1] + "#REVIEWS"
            count_review_page += 10
        else:
            new_url = url.split("-or" + str(count_review_page - 10))
            nextPageURL = new_url[0] + "-or" + str(count_review_page) + new_url[1]
            count_review_page += 10

    process_hotel(nextPageURL, count_reviews, city, hotel_name)


# @args the file that contains all the hotels that are about to be crawled
# the file name is usually hotels_urls.out
def parse_hotels(input_file):
    print input_file
    all_hotels = input_file.readlines()
    for hotel in all_hotels:
        print "Current hotel is:",hotel
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

# extracts all hotels' urls and saves them to a file (i.e. hotels_urls.out)
def get_hotel_urls(url, page_no, output_file):
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
    # URL for hotels in Bucharest
    init = "/Hotels-g294458-Bucharest-Hotels.html"

    #TODO: aceasta e linia corecta:
    #output_file = open("hotels_urls.out", 'w+')
    output_file = open("hotels_urls.out", 'r+')
    get_hotel_urls(init, 1, output_file)
    parse_hotels(output_file)
    output_file.close()
    print
