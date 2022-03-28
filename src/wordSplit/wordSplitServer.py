import json
import multiprocessing
import queue
import sys
sys.path.append('..')
from utils.kafka_py import client
import wordSplit
import logging

to_index_que = queue.Queue()
to_eva_que = queue.Queue()

# 写两个 con 两个 pro 一个分词类 五个线程

def http_handler(message):
    pass


def crawl_handler(message):
    print("!! recevied !!!")
    message.value['title_list'] = wordSplit.exact_wordcut(message.value['title_list'])
    print("cur 1")
    message.value['content_list'] = wordSplit.exact_wordcut(message.value['content_list'])
    print("cut 2")
    for url in message.value['url_list']:
        print("send url to que")
        tmp= {}
        tmp['url'] = url
        to_eva_que.put(tmp, block=True) # 传入评估器队列
        print("send ok!")
    print("send all!")


class WordSplitServer(multiprocessing.Process):
    def __init__(self):
        super(WordSplitServer, self).__init__()

    def run(self) -> None:
        print("run run !!")

        logging.basicConfig(level=logging.WARNING)
        wordSplit.jieba_init()
        http_receiver = client.Consumer(topics=[client.API_Topic], groupid=client.Group_ID, handler=http_handler)
        crawl_receiver = client.Consumer(topics=[client.Pipe_Topic], groupid=client.Group_ID, handler=crawl_handler)
        to_index_producer = client.Producer(topic=client.Index_Topic, message_que=to_index_que)
        to_eva_producer = client.Producer(topic=client.URL_Topic, message_que=to_eva_que) # 最后需要把URLTopic改称eva_Topic
        # 启动
        http_receiver.start()
        crawl_receiver.start()
        to_index_producer.start()
        to_eva_producer.start()

        print("start wordsplit_server url _send !!")

        url = {}
        url['url'] = "https://blog.csdn.net/WhereIsHeroFrom/article/details/123701919"
        to_eva_que.put(url, block=True)

        # ??
        http_receiver.join()
        crawl_receiver.join()
        to_index_producer.join()
        to_eva_producer.join()

from se_crawler_dir.crawlerServer import CrawlerServer

if __name__ == '__main__':
    cs = CrawlerServer()
    ws = WordSplitServer()
    cs.start()
    ws.start()
    cs.join()
    ws.join()