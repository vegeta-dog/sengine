import redis
import kafka
import datetime
import hashlib
from flask import Flask, request

from utils.kafka_py import client

webapi_app = Flask("sEngine_webapi")
redis_host = "localhost"
redis_port = 6379


def encodeMD5(s):
    s = s.encode('utf-8')
    m = hashlib.md5()
    m.update(s)
    return m.hexdigest()


@webapi_app.route('/', methods=['POST', 'GET'])
def webapi():
    if request.method != 'POST':
        return "<strong>This is sEngine webapi, please use POST method to visit!</strong>"

    # 请求中没有json数据
    if not request.json:
        return "<strong>Error: The post body doesn't has a json data in it!</strong>"

    if 'command' not in request.json:
        return "<strong>Error: No command in json.</strong>"

    # 读取要搜索的关键词
    if request.json['command'] == 'search':
        kw = request.json['keyword']
        # todo:接下来需要调用检索器

    value = 从前端获取的字符串

    hash_mark = encodeMD5(value)

    if redis_client.get(hash_mark) != None:
        return flask

    message = dict()
    message['id'] = hash_mark
    message['raw'] = value
    message['timestamp'] = str(datetime.datetime.timestamp(datetime.datetime.now()))

    global HTTP_request_que
    HTTP_request_que.put(message, block=True)

    while True:
        time.sleep(0.1)
        if redis_client.get(hash_mark) != None:
            return


if __name__ == '__main__':
    webapi_app.run(port=56666)
    HTTP_request_que = queue.Queue()
    redis_client = redis.StrictRedis(host=redis_host, port=redis_port)
    API_producer = client.Producer(client.API_Topic, message_que=HTTP_request_que)
