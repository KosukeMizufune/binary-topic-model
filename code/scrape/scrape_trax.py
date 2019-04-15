import re
import time

from bs4 import BeautifulSoup
import pandas as pd
from selenium import webdriver


def scrape_text(soup):
    """
    現在のページからレビューテキストを取得する関数
    :param soup: bs4.BeautifulSoup, html
    :return: list, レビューテキスト
    """
    target_htmls = soup.find_all("div", class_="text_content")
    text = []
    for html in target_htmls:
        tmp_text = re.sub(" Verified Review \|  ", "", html.text)
        text.append(tmp_text)
    return text


def scrape_datetime(soup):
    """
    現在のページから投稿日時を取得する関数
    :param soup: bs4.BeautifulSoup, html
    :return: list, 投稿日時
    """
    target_htmls = soup.find_all('time')
    datetime = []
    for html in target_htmls:
        datetime.append(html.text)
    return datetime


def scrape_score(soup):
    """
    現在のページからレビュー得点を取得する関数
    :param soup: bs4.BeautifulSoup, html
    :return: list, レビュー得点
    """
    target_htmls = soup.find_all('span', itemprop='ratingValue')[1:]
    sub_target_htmls = soup.find_all('div', class_='rating-10')[1:]
    score = []
    i = 0
    for sub_html in sub_target_htmls:
        if re.search(r'na', sub_html.text):
            score.append('NA')
        else:
            score.append(target_htmls[i].text)
            i += 1
    return score


def scrape_table(soup):
    """
    現在のページから各評価項目の点数、利用日時、渡航タイプ、トランジットか出発か到着か、レコメンドを取得する関数
    :param soup: bs4.BeautifulSoup, html
    :return: pd.DataFrame, 各評価項目の点数、利用日時、渡航タイプ、トランジットか出発か到着か、レコメンド
    """
    table_htmls = soup.find_all('div', class_='review-stats')
    table_dict = {'Experience At Airport': [],
                  'Date Visit': [],
                  'Type Of Traveller': [],
                  'Queuing Times': [],
                  'Terminal Cleanliness': [],
                  'Terminal Seating': [],
                  'Terminal Signs': [],
                  'Food Beverages': [],
                  'Airport Shopping': [],
                  'Wifi Connectivity': [],
                  'Airport Staff': [],
                  'Recommended': []}
    review_rate_list = ['Queuing Times',
                        'Terminal Cleanliness',
                        'Terminal Seating',
                        'Terminal Signs',
                        'Food Beverages',
                        'Airport Shopping',
                        'Wifi Connectivity',
                        'Airport Staff']
    for table_html in table_htmls:
        row_htmls = table_html.find_all('tr')
        row_name_lists = []
        for row_html in row_htmls:
            html = row_html.find_all('td')
            row_name = html[0].text
            if row_name in review_rate_list:
                review_html = html[1]
                review_rate = len(review_html.find_all('span', class_='star fill'))
                table_dict[row_name].append(review_rate)
            else:
                table_dict[row_name].append(html[1].text)
            row_name_lists.append(row_name)
        for table_key in table_dict.keys():
            if table_key not in row_name_lists:
                table_dict[table_key].append('NA')
    return pd.DataFrame(table_dict)


def get_other_page(soup):
    """
    現在のページから他のページに飛ぶURLを取得する関数
    :param soup: bs4.BeautifulSoup, html
    :return: list, URL
    """
    next_tag = soup.find_all("article", class_="comp comp_reviews-pagination querylist-pagination position-")[0]
    url_tags = next_tag.find_all("a")
    url = []
    for tag in url_tags:
        url.append('http://www.airlinequality.com' + tag['href'])
    return url


def get_html(driver, url):
    """
    htmlを取得する関数
    :param driver: selenium.webdriver.chrome.webdriver.WebDriver, ウェブドライバー
    :param url: str, html取得元のURL
    :return: bs4.BeautifulSoup, html
    """
    driver.get(url)
    content = driver.page_source
    soup = BeautifulSoup(content, 'lxml')
    return soup


def get_all_reviews(top_url):
    """
    Traxの各空港のレビュートップページからすべてのレビューを取得する関数
    :param top_url: str, Traxの各空港のレビュートップページURL
    :return: pd.DataFrame, 全データ
    """
    driver = webdriver.Chrome("/Users/kosuke/Downloads/Chromedriver")
    soup = get_html(driver, top_url)
    text = scrape_text(soup)
    datetime = scrape_datetime(soup)
    score = scrape_score(soup)
    all_reviews = scrape_table(soup)
    try:
        next_url = get_other_page(soup)[-1]
        present_url = top_url
        while next_url != present_url:
            present_url = next_url
            soup = get_html(driver, present_url)
            text.extend(scrape_text(soup))
            datetime.extend(scrape_datetime(soup))
            score.extend(scrape_score(soup))
            all_reviews = all_reviews.append(scrape_table(soup))
            next_url = get_other_page(soup)[-1]
            time.sleep(3)
    except IndexError:
        print('IndexError')
    driver.quit()
    all_reviews['text'] = text
    all_reviews['datetime'] = datetime
    all_reviews['score'] = score
    return all_reviews


def main():
    """
    https://en.wikipedia.org/wiki/List_of_busiest_airports_by_passenger_traffic#2016_statistics
    より2016年のランキング上位を抽出
    Notice: Chengdu, Kunming, Shenzhen, shanghai hongqqiao空港はレビュー数が少ないので除外. Sydneyは地域対象外
    """
    output = pd.read_csv("airport_review_complete.csv")
    airports = output['airport']
    urls = {
        'Atlanta': 'http://www.airlinequality.com/airport-reviews/atlanta-hartsfield-airport/',
        'Beijing Capital': 'http://www.airlinequality.com/airport-reviews/beijing-capital-airport/',
        'Dubai': 'http://www.airlinequality.com/airport-reviews/dubai-airport/',
        'Los Angeles': 'http://www.airlinequality.com/airport-reviews/los-angeles-lax-airport/',
        'Tokyo Handeda': 'http://www.airlinequality.com/airport-reviews/tokyo-haneda-airport/',
        'Chicago Ohare': 'http://www.airlinequality.com/airport-reviews/chicago-ohare-airport/',
        'London Heathrow': 'http://www.airlinequality.com/airport-reviews/london-heathrow-airport/',
        'hong Kong': 'http://www.airlinequality.com/airport-reviews/hong-kong-airport/',
        'Shanghai Pudong': 'http://www.airlinequality.com/airport-reviews/shanghai-pudong-airport/',
        'Paris CDG': 'http://www.airlinequality.com/airport-reviews/paris-cdg-airport/',
        'Dallas Fort-Worth': 'http://www.airlinequality.com/airport-reviews/dallas-fort-worth-airport/',
        'Amsterdam': 'http://www.airlinequality.com/airport-reviews/amsterdam-schiphol-airport/',
        'Frankfurt Main': 'http://www.airlinequality.com/airport-reviews/frankfurt-main-airport/',
        'Istanbul Ataturk': 'http://www.airlinequality.com/airport-reviews/istanbul-ataturk-airport/',
        'Guangzhou': 'http://www.airlinequality.com/airport-reviews/guangzhou-airport/',
        'New York': 'http://www.airlinequality.com/airport-reviews/new-york-jfk-airport/',
        'Singapore Changi': 'http://www.airlinequality.com/airport-reviews/singapore-changi-airport/',
        'Denver': 'http://www.airlinequality.com/airport-reviews/denver-airport/',
        'Incheon': 'http://www.airlinequality.com/airport-reviews/incheon-airport/',
        'Delhi': 'http://www.airlinequality.com/airport-reviews/delhi-airport/',
        'Bangkok Survarnabhumi': 'http://www.airlinequality.com/airport-reviews/bangkok-suvarnabhumi-airport/',
        'Jakarta': 'http://www.airlinequality.com/airport-reviews/jakarta-airport/',
        'San Francisco': 'http://www.airlinequality.com/airport-reviews/san-francisco-airport/',
        'Kuala Lumpur': 'http://www.airlinequality.com/airport-reviews/klia-kuala-lumpur-airport/',
        'Madrid Barajas': 'http://www.airlinequality.com/airport-reviews/madrid-barajas-airport/',
        'Las Vegas': 'http://www.airlinequality.com/airport-reviews/las-vegas-airport/',
        'Seattle': 'http://www.airlinequality.com/airport-reviews/seattle-airport/',
        'Mumbai': 'http://www.airlinequality.com/airport-reviews/mumbai-airport/',
        'Miami': 'http://www.airlinequality.com/airport-reviews/miami-airport/',
        'Charlotte': 'http://www.airlinequality.com/airport-reviews/charlotte-airport/',
        'Toronto': 'http://www.airlinequality.com/airport-reviews/toronto-pearson-airport/',
        'Barcelona': 'http://www.airlinequality.com/airport-reviews/barcelona-airport/',
        'Phoenix': 'http://www.airlinequality.com/airport-reviews/phoenix-airport/',
        'London Gatwick': 'http://www.airlinequality.com/airport-reviews/london-gatwick-airport/',
        'Taipei': 'http://www.airlinequality.com/airport-reviews/taipei-taoyuan-airport/',
        'Munich': 'http://www.airlinequality.com/airport-reviews/munich-airport/',
        'Sydney': 'http://www.airlinequality.com/airport-reviews/sydney-airport/',
        'Orlando': 'http://www.airlinequality.com/airport-reviews/orlando-airport/',
        'Rome Fiumicino': 'http://www.airlinequality.com/airport-reviews/rome-fiumicino-airport/',
        'Houston IAH': 'http://www.airlinequality.com/airport-reviews/houston-george-bush-intercontinental-airport/',
        'Mexico City': 'http://www.airlinequality.com/airport-reviews/mexico-city-airport/',
        'Newark': 'http://www.airlinequality.com/airport-reviews/newark-airport/',
        'Manila': 'http://www.airlinequality.com/airport-reviews/manila-ninoy-aquino-airport/',
        'Tokyo Narita': 'http://www.airlinequality.com/airport-reviews/tokyo-narita-airport/',
        'Minneapolis': 'http://www.airlinequality.com/airport-reviews/minneapolis-st-paul-airport/',
        'Hamad Doha': 'http://www.airlinequality.com/airport-reviews/hamad-doha-airport/'
    }
    for airport, url in urls.items():
        if airport in list(airports):
            continue
        else:
            print(airport)
            data = get_all_reviews(url)
            data['airport'] = airport
            output = output.append(data)
    output.to_csv("airport_review_complete.csv", index=False)
