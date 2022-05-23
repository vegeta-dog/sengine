import jieba
import regex as re
import string
from jieba.analyse import textrank


def jieba_init():
    jieba.load_userdict("sEngine_dict.txt")
    jieba.initialize()


def ignore_not_utf8_and_emo(word):
    """
    通过utf-8重新编码去掉字符串里面的表情以及不可读字符
    :param word:
    :return:
    """
    word = word.encode("utf-8", "ignore").decode("utf-8", "ignore")
    try:
        co = re.compile(u'['u'\U0001F300-\U0001F64F' u'\U0001F680-\U0001F6FF'u'\u2600-\u2B55]+')
    except re.error:
        co = re.compile(u'('u'\ud83c[\udf00-\udfff]|'u'\ud83d[\udc00-\ude4f\ude80-\udeff]|'u'[\u2600-\u2B55])+')
    return co.sub(" ", word)


def normalize(word):  # 规格化
    """
    规格化: 去掉字符串里面的中文标点符号+空格+表情符号
    :param word:
    :return: string
    """
    word = ignore_not_utf8_and_emo(word)
    all_punc = '，。、【】“”：；（）《》‘’{}？！⑦()、%^>℃：.”“^-——=&#@￥〓' + string.punctuation + string.whitespace
    return "".join([c if c not in all_punc else ' ' for c in word]).strip()


def exact_wordcut(sentence_list):
    word_list = []
    for sentence in sentence_list:
        for word in jieba.cut(sentence, HMM=True):
            word = normalize(word)
            if word == "":
                continue
            word_list.append(word)
    return word_list


def token_wordcut(sentence_list, topK=5000):
    word_list = []
    for sentence in sentence_list:
        for word in textrank(sentence, topK=topK, HMM=True):
            word = normalize(word)
            if word == "":
                continue
            word_list.append(word)
    return word_list


def search_wordcut(sentence_list):
    word_list = []
    for sentence in sentence_list:
        for word in jieba.cut_for_search(sentence):
            word = normalize(word)
            if word == "":
                continue
            word_list.append(word)
    return word_list


# print(normalize("你好不好哈🌲🌲喵喵🐶🐶西哦小鱼玉✖"))













