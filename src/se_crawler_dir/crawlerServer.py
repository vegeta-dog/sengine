import os
import queue
import threading
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "../se_crawler_dir"))
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))
import scrapy
import multiprocessing

from twisted.internet import reactor
from scrapy.crawler import CrawlerRunner
from scrapy.utils.project import get_project_settings
from scrapy.utils.log import configure_logging

from se_crawler.spiders import csdnSpider as csdn
from se_crawler.spiders import cnblogSpider as cnblog
from se_crawler import pipelines as pipe

from utils.kafka_py import client
from utils.configParser import configParser


server_host = ""  # kafka服务器地址

# 注册好每个爬虫的合法域名
domain_list = [
    csdn.allowed_domain,
    cnblog.allowed_domain
]

# 注册每个爬虫的url队列
que_list = [
    csdn.que,
    cnblog.que
]


def start_crawlers():
    # logger = logging.getLogger(__name__)
    # 获取原本的路径, 转换路径获取settings文件内容,然后切换回去原本的路径
    org_path = os.getcwd()
    os.chdir(os.path.join(os.path.dirname(__file__), 'se_crawler'))
    settings = get_project_settings()
    os.chdir(org_path)

    configure_logging(settings)
    runner = CrawlerRunner(settings)
    # 装载爬虫
    runner.crawl(csdn.CsdnSpider)
    # 爬虫结束后停止事件循环
    d = runner.join()
    d.addBoth(lambda _: reactor.stop())
    # 启动事件循环
    reactor.run()


# 写 一个con, 一个prod

def url_sender(message):  # simulate the evaluator send msg to crawler
    global domain_list

    for i, domain in enumerate(domain_list):
        try:
            strs = message['url'].split(":")[1][2:]
            print("strs = ", strs)
            if strs.startswith(domain):
                print("url sender: find csdn url! now send to csdn_que.")
                que_list[i].put(message['url'], block=True)
                break  # 找到一个爬虫愿意接收这个网页即可
        except TypeError as e:     # 上面的操作有可能数组越界,但是不影响
            print(e.args, "but doesn't matter.")
            break


# 用于调试的进程类 - 被抛弃
class CrawlerServer(multiprocessing.Process):
    def __init__(self):
        super(CrawlerServer, self).__init__()
        # os.chdir(os.path.join(os.path.dirname(__file__), 'se_crawler'))

    def run(self) -> None:
        pip_producer = client.Producer(topic=client.Pipe_Topic, message_que=pipe.message_que, broker=server_host)
        url_consumer = client.Consumer(topics=[client.URL_Topic], groupid=client.Group_ID_2, handler=url_sender, broker=server_host)
        url_consumer.start()
        pip_producer.start()
        # 启动爬虫
        start_crawlers()
        # 阻塞直到爬虫任务完成
        url_consumer.join()
        pip_producer.join()


if __name__ == '__main__':
    config_dic = configParser.load_config()
    server_host = config_dic['kafka_brokers']
    # 生产者 消费者

    pip_producer = client.Producer(topic=client.Pipe_Topic, message_que=pipe.message_que, broker=server_host)
    url_consumer = client.Consumer(topics=[client.URL_Topic], groupid=client.Group_ID_2, handler=url_sender, broker=server_host)

    url_consumer.start()
    pip_producer.start()
    # 启动爬虫
    start_crawlers()
    # 阻塞直到爬虫任务完成
    url_consumer.join()
    pip_producer.join()


