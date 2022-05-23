import random
import sys, json, time, queue
import datetime
import requests
import scrapy
from scrapy import Request
import jieba
from jieba.analyse import textrank
from scrapy.selector import Selector
from se_crawler import items

que = queue.Queue(maxsize=100)  # csdn 上面的 url, 限制长度为100
allowed_domain = 'blog.csdn.net'  # 域名限制


# def remove_none_utf8_characters(item_list):
#     """
#     删除list中每一项的的非utf8字符
#     :param item_list:输入的列表
#     :return: item_list
#     """
#
#     for i in range(0, len(item_list)):
#         item_list[i] = item_list[i].encode('utf-8', 'ignore').decode("utf-8")
#
#     return item_list


class CsdnSpider(scrapy.Spider):
    name = "csdn"
    allowed_domains = ['blog.csdn.net']
    
    def start_requests(self):
        print("run_spider")
        url = que.get(block=True, timeout=10000)  # 设置10s的时间限制, 否则抛出异常
        yield Request(url, callback=self.parse)

    def parse(self, response, **kwargs):
        global que, allowed_domain
        print("parser")

        def extract_selector(selectors):
            contents = []
            for selector in selectors:
                content = selector.xpath("string(.)").get().strip()
                if content == "":  # 空串要被删掉
                    continue
                contents.append(content)
            return contents

        # 获取文章的主要内容选择器 article_content selector
        content_selector = response.xpath("//div[@id='content_views']")
        # 获取内容 list
        h_list = extract_selector(content_selector.xpath(".//*[starts-with(name(), 'h1')]"))  # 输出所有的 h1 标签
        # 获取 href 获取所有 blog+csdn域名开头+自动去重 的 url
        href_list = response.\
            xpath(f"//*[@href]/@href[contains(., 'article') and contains(., '{allowed_domain}')]").getall()
        # 获取 title 列表
        title = response.xpath("/html/head/title/text()").getall()
        # 获取文章的主要内容
        main_content = content_selector.xpath("./*[name()!='pre']").xpath("string(.)").getall()

        # print(response.url)

        # 数据项
        item = items.SeCrawlerItem()
        item['url'] = response.url
        item["url_list"] = href_list
        item['title_list'] = h_list + title
        item["content_list"] = main_content
        # print(item)

        item['datetime'] = str(datetime.datetime.timestamp(datetime.datetime.now()))
        yield item

        # 等待 url
        while True:
            if que.qsize() > 0:

                print("url_que.size = ", que.qsize())

                url = que.get(block=True)
                # url = "https://blog.csdn.net/WhereIsHeroFrom/article/details/123836614"
                dont_filter = True if random.randint(0, 3) > 0 else False
                yield Request(url, callback=self.parse, dont_filter=dont_filter)
            else:
                print("crawler doesn't find url from the que")
            time.sleep(3)






