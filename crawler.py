import requests
import sys
from lxml import html

def get_review_urls(url, page_no, output_file):
    sys.stdout.write('.')
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
        get_review_urls(url, page_no+1, output_file)

if __name__ == '__main__':
    # aici ar trebui sa gasim un mod
    # generic de a obtine URL-ul pentru prima pagina
    init = "/Hotels-g294458-Bucharest-Hotels.html"

    output_file = open("hotels_urls.out", 'w')
    get_review_urls(init, 1, output_file)
    output_file.close()
    print
