import sys
import json, time, queue
import datetime
import requests
import scrapy
from scrapy import Request
import jieba
from jieba.analyse import textrank
from scrapy.selector import Selector
from se_crawler import items

que = queue.Queue() # csdn 上面的 url
allowed_domain = 'blog.csdn.net' # 域名限制


class CsdnSpider(scrapy.Spider):
    name = "csdn"
    allowed_domains = ['blog.csdn.net']
    
    def start_requests(self):
        print("run_spider")
        url = que.get(block=True)
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
        h_list = extract_selector(content_selector.xpath(".//*[starts-with(name(), 'h')]"))  # 输出所有的 h 标签
        # 获取 href 获取所有 blog+csdn域名开头+自动去重 的 url
        href_list = response.xpath(f"//*[@href]/@href[contains(., 'article') and constains(., '{allowed_domain}')]").getall()
        # 获取 title
        title = response.xpath("/html/head/title/text()").getall()
        # 获取文章的主要内容
        # content_selector = Selector(text="<div>aaaaa<a>bbbb</a>aaaa</div>")
        main_content = content_selector.xpath("./*[name()!='pre']").xpath("string(.)").getall()

        print(response.url)

        # 数据项
        item = items.SeCrawlerItem()
        item['url'] = response.url
        item["url_list"] = href_list
        item['title_list'] = h_list + title
        item["content_list"] = main_content
        item['datetime'] = str(datetime.datetime.timestamp(datetime.datetime.now()))
        yield item

        # 等待 url
        while True:
            url = que.get(block=True)
            yield Request(url, callback=self.parse)





