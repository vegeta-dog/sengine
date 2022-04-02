import jieba
import regex
from jieba.analyse import textrank


def jieba_init():
    jieba.initialize()


def normalize(word):  # 规格化
    all_punc = '，。、【】“”：；（）《》‘’{}？！⑦()、%^>℃：.”“^-——=&#@￥' + string.punctuation
    return "".join([c if c not in all_punc else ' ' for c in word]).strip()


def exact_wordcut(sentence_list):
    word_list = []
    for sentence in sentence_list:
        for word in jieba.cut(sentence):
            word = normalize(word)
            if word == "":
                continue
            word_list.append(word)
    return word_list


def token_wordcut(sentence_list, topK=100):
    word_list = []
    for sentence in sentence_list:
        for word in textrank(sentence, topK=topK):
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















