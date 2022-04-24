import traceback, json
import kafka.errors
from kafka.consumer import KafkaConsumer
from kafka.producer import KafkaProducer
import threading

server_host = ""

Pipe_Topic = "Crawler2WordSplit"     # 爬虫与分词模块的通信topic
WordSplit_Topic = "WordSplit2Evaluator"    # WordSplit to Evaluator
API_Topic = "API2WordSplit"     # 分词与API之间的通信topic
Evaluator_Topic = "Crawler2Evaluator"    # 评估器与分词模块之间的通信topic
URL_Topic = "Evaluator2Crawler"  # 评估器与爬虫之间的topic
Index_Topic = "WordSplit2Searcher"    # 检索模块与分词模块的topic

Group_ID = "GPID"    # 不同消费者设置了不同的分组名,不懂为什么topic不同还需要分组也不同
Group_ID_2 = "GPID2"
Group_ID_3 = "GPID3"


class Producer(threading.Thread):
    def run(self):
        while True:
            message = self.message_que.get(block=True)
            if type(message) == type(str):
                print('producer: is not a dict')
            print("producer's topic :", self.topic, " | send message:", message)
            future = self.producer.send(self.topic, value=json.dumps(message).encode(), partition=0)
            try:
                future.get(timeout=10)
            except kafka.errors.KafkaError as e:
                print(e)
                traceback.format_exc()
        
    def __init__(self, topic, message_que, broker=server_host):
        super(Producer, self).__init__()
        self.producer = KafkaProducer(
            bootstrap_servers=[broker],
            # key_serializer=lambda k: json.dumps(k).encode(),
            # value_serializer=lambda v: json.dumps(v).encode()
        )
        self.topic = topic
        self.message_que = message_que
        
        
class Consumer(threading.Thread):
    def run(self):
        print("consumer run begin")
        for message in self.consumer:
            if type(json.loads(message.value.decode())) is not type(dict()):
                print('consumer: not a dict')
            print("consumer's topic", self.group_id, "| message:", json.loads(message.value.decode()))
            self.handler(json.loads(message.value.decode()))
        print("cinsumer run over")

    def __init__(self, topics, groupid, handler, auto_offset_reset='latest', broker=server_host):
        super(Consumer, self).__init__()
        self.consumer = KafkaConsumer(
            bootstrap_servers=broker,
            group_id=groupid,
            auto_offset_reset=auto_offset_reset,
            # value_deserializer=lambda m: json.loads(m.decode('utf8')),

        )
        self.group_id = topics[0]
        self.consumer.subscribe(topics)
        self.handler = handler

