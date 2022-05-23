import queue
import sys
import os
import flask
import time
import redis
import kafka
import datetime
import hashlib
from flask import Flask, request, Response, abort

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(os.path.dirname(__file__), "../../"))

from utils.kafka_py import client
from utils.configParser.configParser import load_config
from utils.exception import *
import utils.logger as log

import base64

webapi_app = Flask("sEngine_webapi")


def encodeMD5(s):
    s = s.encode('utf-8')
    m = hashlib.md5()
    m.update(s)
    return m.hexdigest()


def restore_json_string(s):
    """
    还原在searcher中， 被base64编码的结果
    :param s:从redis获取到的的字符串
    :return:
    """
    return base64.b64decode(s)


@webapi_app.route('/', methods=['POST', 'GET'])
def webapi():

    try:
        if request.method != 'POST':
            return "<strong>This is sEngine webapi, please use POST method to visit!</strong>"

        # 请求中没有json数据
        if not request.json:
            return "<strong>Error: The post body doesn't has a json data in it!</strong>"

        if 'command' not in request.json:
            return "<strong>Error: No command in json.</strong>"

        # 读取要搜索的关键词
        kw = ""
        if request.json['command'] == 'search':
            kw = request.json['keyword']
        else:
            raise ParameterError("command不等于 search")

        print("kw = " + kw)
        if kw is None or kw == "":
            abort(404)

        hash_mark = encodeMD5(kw)

        res = redis_client.get("search:"+hash_mark)
        if res:
            print(res)
            res = restore_json_string(res.decode())
            print(res)
            return res

        message = dict()
        message['id'] = hash_mark
        message['raw'] = kw
        message['timestamp'] = str(datetime.datetime.timestamp(datetime.datetime.now()))

        global HTTP_request_que
        HTTP_request_que.put(message, block=True)
        print("message : ", message)

        expire_time = 30
        while expire_time:
            expire_time -= 1
            time.sleep(0.1)
            res = redis_client.get("search:"+hash_mark)
            if res:
                print(res)
                res = restore_json_string(res.decode())
                print(res)
                return res
        # 超时未返回数据，404
        abort(404)

    except ParameterError as e:
        log.log_traceback(e)
        abort(500)



if __name__ == '__main__':
    config = load_config()
    log.init_log("log_webapi.py.txt")

    HTTP_request_que = queue.Queue()  # producer to wordsplit
    redis_client = redis.StrictRedis(host=config['redis_host'], port=config['redis_port'],
                                     password=config['redis_password'])
    # 新建一个进程吃
    # proc_pool = multiprocessing.dummy.Pool(processes=128)

    API_producer = client.Producer(client.topic_api_to_wordsplit, message_que=HTTP_request_que, broker=config['kafka_brokers'])
    API_producer.start()
    # 将api producer加入线程池
    # proc_pool.apply_async(API_producer.run, args=())

    webapi_app.run(port=56666)

    API_producer.join()

