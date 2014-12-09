import requests
import sys
import os
from lxml import html
from lxml import etree

def process_review(url, output_file): # primeste url-ul unui review si extrage
                                      # informatii despre el
    print "ceva"

def process_hotel(url, count_reviews): # primeste url-ul unui hotel
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
        output_file = open("review"+str(count_reviews)+".out", 'w+')
        output_file.write(etree.tostring(rev_info))
        count_reviews = count_reviews + 1

    # trecem la pagina urmatoare
    nextPage = tree_rev.xpath('//*[contains(concat(" ", \
            normalize-space(@class), " "), " sprite-pageNext ")]')
    if (len(nextPage) == 0): # am ajuns la ultima pagina
        return

    nextPageURL = nextPage[0].xpath('attribute::href')[0]

    process_hotel(url, count_reviews)

def parse_hotels(input_file): # pentru fiecare hotel, face bucatarie
    all_hotels = input_file.readlines()
    for hotel in all_hotels:
        tokens = hotel.split('-')
        if (len(tokens) <= 1):
            continue
        print tokens[4] + " " + tokens[5][:-6]
        dir_name = "Reviews_" + tokens[5][:-6] + "_" + tokens[4]
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
        os.chdir(dir_name)
        process_hotel(hotel, 0)
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
