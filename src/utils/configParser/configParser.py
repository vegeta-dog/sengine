import configparser
import os, sys


def load_config(path=os.path.join(os.path.dirname(__file__), "../../../build/config.ini")):
    """
    :param path: 配置文件的路径
    :return:
    """
    print(os.getcwd())
    parser = configparser.ConfigParser()
    parser.read(path, encoding='utf8')

    print(path)
    print(parser)

    # 只有 4 个属性, 如果需要添加, 清后续修改
    print(parser.sections())
    assert len(parser.sections()) == 5

    _conf_database = [(key, value) for key, value in parser.items("DataBase")]
    
    _conf_kafka = [(key, value) for key, value in parser.items("Kafka")]

    _conf_evaluator = [(key, value) for key, value in parser.items("Evaluator")]
    
    _conf_index_builder = [(key, value) for key, value in parser.items("indexBuilder")]

    _conf_searcher = [(key, value) for key, value in parser.items("Searcher")]

    return dict(_conf_database + _conf_kafka + _conf_evaluator + _conf_index_builder + _conf_searcher)
