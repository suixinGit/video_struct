# encoding=utf-8  #or gbk 这样才能使用中文
__author__ = 'Zhangjie'

import cv2
import numpy as np
import os
#车辆识别依赖模块
from aip import AipImageClassify
#车牌识别依赖模块
import sys
import ssl
import urllib.request
import json
import string
import base64
#文件选择对话框对话框依赖模块
import time

#百度AI个人API
APP_ID = '11092056'
API_KEY = 'C78v7yl1stPdGLz2gLPqZPmm'
SECRET_KEY = '1EsdqbQi6geDdffF5uZ5KtNznNwVnw0L'
client = AipImageClassify(APP_ID, API_KEY, SECRET_KEY)

#车牌识别api
cardrecognise_url_api = 'https://aip.baidubce.com/rest/2.0/ocr/v1/license_plate'


# client_id 为官网获取的AK， client_secret 为官网获取的SK
host = 'https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id='+ API_KEY + '&client_secret=' + SECRET_KEY
request = urllib.request.Request(host)
request.add_header('Content-Type', 'application/json; charset=UTF-8')
response = urllib.request.urlopen(request)
#print(response)
content = response.read()
if (content):
    #print(content)
    data = eval(content)
    access_token = data["access_token"]

# 图像路径
filepath = 'D:/'
filename = 'timg2.jpg'
filepath_in = filepath + filename
#print(filepath_in)

""" 读取图片 """
def get_file_content(filePath):
    # 二进制方式打开图文件
    with open(filePath, 'rb') as fp:
        return fp.read()

image = get_file_content(filepath_in)

""" 车牌识别 """
start = time.time()
cardrecognise_url = cardrecognise_url_api+'?access_token=' + access_token
# 二进制方式打开图文件

# 参数image：图像base64编码
img = base64.b64encode(image)
params = {"image": img}
#params = urllib.parse.urlencode(params).encode(encoding='utf-8')
params = urllib.parse.urlencode(params)
params = params.encode(encoding='UTF8')
request = urllib.request.Request(cardrecognise_url, params)
request.add_header('Content-Type', 'application/x-www-form-urlencoded')
response = urllib.request.urlopen(request)
content = response.read()
content = content.decode('utf-8')
if (content):
    #print(content)
    #print(type(content))
    pass
end = time.time()
print('card recognise time cost is %f'%(end-start))

""" 调用车辆识别 """
start = time.time()
client.carDetect(image);

""" 如果有可选参数 """
options = {}
options["top_num"] = 5

""" 带参数调用车辆识别 """
result = client.carDetect(image, options)
print(result)
car_location = result['location_result']
#print(car_location)
car_attribute = result['result']
car_color = result['color_result']
#print(car_color)
end = time.time()
print('car recognise time cost is %f'%(end-start))

#img = cv2.imread(filepath_in, cv2.IMREAD_COLOR)   # 读取彩色图像
#img_g = cv2.imread(filepath_in, 0)  # 读取绘图图像


#cv2.imshow('test',img)
#cv2.waitKey(0)
#cv2.destroyAllWindows()