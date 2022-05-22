import json
import os
import multiprocessing
import queue
import sys
import logging
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(os.path.dirname(__file__), "../../"))

import wordSplit
from utils.kafka_py import client
from se_crawler_dir.crawlerServer import CrawlerServer
from utils.configParser import configParser

to_index_que = queue.Queue()
to_eva_que = queue.Queue()

server_host = ""  # kafka服务器地址


# 写两个 con 两个 pro 一个分词类 五个线程

def http_handler(message):
    print(message)
    message['content'] = [message['raw']]
    # message['content'] = wordSplit.exact_wordcut(message['raw'])
    # message['content'] += wordSplit.search_wordcut(message['raw'])
    # message['content'].append(message['raw'])
    print("content = ", message['content'])
    to_index_que.put(message, block=True)


def crawl_handler(message):
    message['title'] = wordSplit.exact_wordcut(message['title_list'])
    message['content'] = wordSplit.exact_wordcut(message['content_list'])
    to_eva_que.put(message, block=True)


class WordSplitServer(multiprocessing.Process):
    def __init__(self):
        super(WordSplitServer, self).__init__()

    def run(self) -> None:

        logging.basicConfig(level=logging.WARNING)
        wordSplit.jieba_init()
        http_receiver = client.Consumer(topics=[client.topic_api_to_wordsplit], groupid=client.Group_ID, handler=http_handler, broker=server_host)
        crawl_receiver = client.Consumer(topics=[client.Pipe_Topic], groupid=client.Group_ID_3, handler=crawl_handler, broker=server_host)
        to_index_producer = client.Producer(topic=client.topic_to_searcher, message_que=to_index_que, broker=server_host)
        to_eva_producer = client.Producer(topic=client.Evaluator_Topic, message_que=to_eva_que, broker=server_host)  # 最后需要把URLTopic改称eva_Topic
        # 启动
        http_receiver.start()
        crawl_receiver.start()
        to_index_producer.start()
        to_eva_producer.start()

        print("start wordsplit_server url _send !!")

        msg = {
            'url': "https://blog.csdn.net/WhereIsHeroFrom/article/details/123701919",
            'title': ["title", 'title test'],
            'content': ["ttestt"],
            'url_list': ["https://blog.csdn.net/WhereIsHeroFrom/article/details/123701919"],
            'timestamp': 0
        }
        # msg = {'url': "https://blog.csdn.net/WhereIsHeroFrom/article/details/123701919"}
        to_eva_que.put(msg, block=True)

        # ??
        http_receiver.join()
        crawl_receiver.join()
        to_index_producer.join()
        to_eva_producer.join()


if __name__ == '__main__':
    
    dic = configParser.load_config()
    server_host = dic['kafka_brokers']
    print(dic)

    # cs = CrawlerServer()
    ws = WordSplitServer()
    # cs.start()
    ws.start()
    # cs.join()
    ws.join()
