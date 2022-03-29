import traceback, json
import kafka.errors
from kafka.consumer import KafkaConsumer
from kafka.producer import KafkaProducer
import threading


Pipe_Topic = "a"     # 爬虫与分词模块的通信topic
WordSplit_Topic = "b"    # 暂未使用
API_Topic = "c"     # 分词与API之间的通信topic
Evaluator_Topic = "d"    # 评估器与分词模块之间的通信topic
URL_Topic = "e"  # 评估器与爬虫之间的topic
Index_Topic = "f"    # 检索模块与分词模块的topic

Group_ID = "GPID"    # ?


class Producer(threading.Thread):
    def run(self):
        while True:
            message = self.message_que.get(block=True)
            if type(message) == type(str):
                print('producer: is not a dict')
            future = self.producer.send(self.topic, value=json.dumps(message).encode(), partition=0)
            try:
                future.get(timeout=10)
            except kafka.errors.KafkaError as e:
                print(e)
                traceback.format_exc()
        
    def __init__(self, topic, message_que, broker=''):
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
        for message in self.consumer:
            print("self id = ", self.group_id)
            if type(json.loads(message.value.decode())) is not type(dict()):
                print('consumer: not a dict')
            self.handler(json.loads(message.value.decode()))

    def __init__(self, topics, groupid, handler, auto_offset_reset='latest', broker=''):
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

