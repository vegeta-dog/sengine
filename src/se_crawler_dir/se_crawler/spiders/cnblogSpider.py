import sys
import json, time, queue
import requests
import scrapy
from scrapy import Request
import jieba
from jieba.analyse import textrank
from scrapy.selector import Selector
from se_crawler import items

que = queue.Queue()
allowed_domain = ""