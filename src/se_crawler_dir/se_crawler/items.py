# Define here the models for your scraped items
#
# See documentation in:
# https://docs.scrapy.org/en/latest/topics/items.html

import scrapy


class SeCrawlerItem(scrapy.Item):
    # define the fields for your item here like:
    url = scrapy.Field()
    url_list = scrapy.Field()
    title_list = scrapy.Field()
    content_list = scrapy.Field()
    datetime = scrapy.Field()
