from flask import Flask, request

webapi_app = Flask("sEngine_webapi")


@webapi_app.route('/', methods=['post', 'get'])
def webapi():
    if request.method != 'post':
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


if __name__ == '__main__':
    webapi_app.run(port=56666)
