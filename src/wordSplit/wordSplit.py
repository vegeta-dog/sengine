import jieba
from jieba.analyse import textrank


def jieba_init():
    jieba.initialize()


def exact_wordcut(sentence_list):
    word_list = []
    for sentence in sentence_list:
        word_list += jieba._lcut(sentence)
    return word_list


def token_wordcut(sentence_list, topK=100):
    word_list = []
    for sentence in sentence_list:
        for word in textrank(sentence, topK=topK):
            word_list.append(word)
    return word_list


def search_wordcut(sentence_list):
    word_list = []
    for sentence in sentence_list:
        word_list += jieba._lcut_for_search(sentence)
    return word_list















