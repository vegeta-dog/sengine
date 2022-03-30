# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: https://docs.scrapy.org/en/latest/topics/item-pipeline.html


# useful for handling different item types with a single interface
import queue
import sys
from itemadapter import ItemAdapter
sys.path.append('../../')
from utils.kafka_py import client

print("import message _ que ")
message_que = queue.Queue()  # 爬虫与分词模块之间的消息队列


class SeCrawlerPipeline:
    # 规定了所有的item都是一个 (url, title_list, content_list, url_list, datetime)
    def open_spider(self, spider):
        print("open spider", spider.name)

    def process_item(self, item, spider):
        print("pipeline !!!")

        message = dict()    # GG
        message['url'] = item['url']
        message["url_list"] = item["url_list"]

        print(message['url_list'])

        message['title_list'] = item['title_list']
        message["content_list"] = item["content_list"]
        message['datetime'] = item['datetime']
        message_que.put(message, block=True)
        return item

    def close_spider(self, spider):
        print("end spider", spider.name)
