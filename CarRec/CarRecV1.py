# encoding=utf-8  #or gbk 这样才能使用中文
__author__ = 'Zhangjie'

import cv2
import numpy as np 
import datetime 
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
import json
import csv
import random
#python生成GUID
import uuid

def Baidu_Acess():
    #百度AI个人API
    APP_ID = '11092056'
    API_KEY = 'C78v7yl1stPdGLz2gLPqZPmm'
    SECRET_KEY = '1EsdqbQi6geDdffF5uZ5KtNznNwVnw0L'

    client = AipImageClassify(APP_ID, API_KEY, SECRET_KEY)
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
    return  access_token

    """ 读取图片 """
def get_file_content(filePath):
    # 二进制方式打开图文件
    with open(filePath, 'rb') as fp:
        return fp.read()

def Cardetection(filePath):
    # 读取图片
    image = get_file_content(filePath)
    # 参数image：图像base64编码
    img = base64.b64encode(image)

    """ 车牌识别 """
    """ 车牌识别 """
    #车牌识别api
    platerecognise_url_api = 'https://aip.baidubce.com/rest/2.0/ocr/v1/license_plate'
    platerecognise_url = platerecognise_url_api+'?access_token=' + access_token
    platerecognise_params = {"image": img}
    #params = urllib.parse.urlencode(params).encode(encoding='utf-8')
    platerecognise_params = urllib.parse.urlencode(platerecognise_params)
    platerecognise_params = platerecognise_params.encode(encoding='UTF8')

    platerecognise_request = urllib.request.Request(platerecognise_url, platerecognise_params)
    platerecognise_request.add_header('Content-Type', 'application/x-www-form-urlencoded')
    platerecognise_response = urllib.request.urlopen(platerecognise_request)
    platerecognise_content = platerecognise_response.read()
    platerecognise_content1 = platerecognise_content.decode('utf-8')
    platerecognise_json = json.loads(platerecognise_content1)


    #print("plate_color:")
    #print(plate_color)
    #print("plate_number:")
    #print(plate_number)
    #print("plate_location:")
    #print(plate_location)

    """ 调用车辆识别 """
    cardrecognise_url_api = "https://aip.baidubce.com/rest/2.0/image-classify/v1/car"
    cardrecognise_url = cardrecognise_url_api + "?access_token=" + access_token
    cardrecognise_params = {"image":img,"top_num":5}
    cardrecognise_params = urllib.parse.urlencode(cardrecognise_params)
    cardrecognise_params = cardrecognise_params.encode(encoding='UTF8')

    cardrecognise_request = urllib.request.Request(url=cardrecognise_url, data=cardrecognise_params)
    cardrecognise_request.add_header('Content-Type', 'application/x-www-form-urlencoded')
    cardrecognise_response = urllib.request.urlopen(cardrecognise_request)
    cardrecognise_content = cardrecognise_response.read()
    cardrecognise_content1 = cardrecognise_content.decode('utf-8')
    cardrecognise_json = json.loads(cardrecognise_content1)


    if(len(platerecognise_json)==2 and (cardrecognise_json['location_result']["width"] > 0)):
        plate_color = platerecognise_json['words_result']['color']
        plate_number = platerecognise_json['words_result']['number']
        plate_location = platerecognise_json['words_result']['vertexes_location']       

        car_location = cardrecognise_json['location_result']
        #print(car_location)
        car_attribute = cardrecognise_json['result']
        car_color = cardrecognise_json['color_result']
        #print(car_color)

        rec_result = {'plate_color':plate_color,'plate_number':plate_number,'plate_location':plate_location,'car_attribute':car_attribute,'car_color':car_color,'car_location':car_location}        
    else:
        rec_result = False
    return rec_result

def write2CSV(data,filename):
    with open(filename, 'w+',encoding='utf-8-sig') as dstfile:   #写入方式选择wb，否则有空行
        writer = csv.writer(dstfile)
        writer.writerows(data)  # 批量写入
def getzhixindu():
    a=0
    while(a<0.9):
        a= random.random()
    return a
def getcode_zhen_num():
    code_zhen_num = datetime.datetime.now().strftime("%Y%m%d%H%M") + '_' + str(uuid.uuid1())
    return code_zhen_num

if __name__ == '__main__':
    #获取百度接口权限
    access_token = Baidu_Acess()
    #读入视频文件
    camera = cv2.VideoCapture('./3.avi')
    cv2.namedWindow("detection",cv2.WINDOW_NORMAL);
    cv2.resizeWindow("detection", 640, 480);
    #获取视频属性信息
    fps=camera.get(cv2.CAP_PROP_FPS)
    size=(int(camera.get(cv2.CAP_PROP_FRAME_WIDTH)),int(camera.get(cv2.CAP_PROP_FRAME_HEIGHT)))
    #车辆检测
    bs = cv2.createBackgroundSubtractorKNN(detectShadows = True)# 背景分割器，该函数计算了前景掩码 
    es = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3)) 
    Gaussian_kernel_size = (5, 5)
    Gaussian_sigma = 1.5
    
    #视频帧存储表
    data_table_videostore = []
    #识别车辆信息表
    data_table_carrec = []


    fortmat_zhen = 'JPG'
    CodeCameraNum = [13,54,58,100,122,102]

    if camera.isOpened(): #判断是否正常打开 
        ret, frame0 = camera.read() 
    else: 
        ret = False

with open('InfoDetect.txt', 'w') as file_detect: 
    while ret: #循环读取视频帧 
        ret, frame0 = camera.read()
        #灰度化处理
        #framegrey=cv2.cvtColor(frame0,cv2.COLOR_BGR2GRAY)
        #gray_lap = cv2.Laplacian(framegrey,cv2.CV_16S,ksize = 3)
        #dst = cv2.convertScaleAbs(gray_lap)

        #if (qingxidu>80): #每隔timeF帧进行存储操作
            #print(frame0.shape[0])   #0,h;1,w;2,c
        frame = cv2.GaussianBlur(frame0, Gaussian_kernel_size, Gaussian_sigma);
        fgmask = bs.apply(frame)  
        fg2 = fgmask.copy()  
        # 二值化阈值处理，前景掩码含有前景的白色值以及阴影的灰色值，在阈值化图像中，将非纯白色（244~255）的所有像素都设为0，而不是255
        th = cv2.threshold(fg2,200,255,cv2.THRESH_BINARY)[1]  
        # 形态学膨胀
        dilated = cv2.dilate(th,es,iterations = 2)  
        # 该函数计算一幅图像中目标的轮廓
        image, contours, hier = cv2.findContours(dilated,cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)  

        for c in contours:  
            if cv2.contourArea(c) > 10000:  
                (t_x,t_y,t_w,t_h) = cv2.boundingRect(c)

                if(200<t_w<600 and 200<t_h<600 and (3*frame0.shape[0]/4)<(t_y+t_h)):
                    now_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
                    #print(now_time)
                    #print('target location is',(t_x,t_y,t_w,t_h))
                    roi_img = frame0[t_y:(t_y+t_h), t_x:(t_x+t_w)]
                    filename_roi = now_time + '.jpg'
                    cv2.imwrite(filename_roi,roi_img)
                    roi_img = cv2.imread(filename_roi)
                    cv2.imshow("roi_img",roi_img)
                    framegrey=cv2.cvtColor(roi_img,cv2.COLOR_BGR2GRAY)
                    qingxidu = cv2.Laplacian(framegrey,cv2.CV_64F).var()
                    cv2.rectangle(frame0,(t_x,t_y),(t_x+t_w,t_y+t_h),(0,255,0),2) 
                    contralIn = input('请输入d检测或者a放弃：')
                    if(qingxidu>100 and contralIn=='d'):
                        detect_result = Cardetection(filename_roi)
                        if detect_result is not False:
                            #print(type(detect_result))
                            #print(detect_result)
                            file_detect.write('时间：'+now_time + '\n')
                            file_detect.write('车牌号：'+ detect_result['plate_number']+'\n')
                            file_detect.write('车牌颜色：'+ detect_result['plate_color']+'\n')
                            file_detect.write('地点：三环路蓝天立交段由东向西'+ '\n')
                            file_detect.write('车辆位置：top_x=%d, top_y=%d, width=%d, height=%d \n'%(t_x,t_y,t_w,t_h))
                            file_detect.write('车辆颜色：'+ detect_result['car_color']+ '\n')
                            file_detect.write('车辆型号：'+ detect_result['car_attribute'][0]['name']+ '\n')
                            file_detect.write('车辆款式：'+ detect_result['car_attribute'][0]['year']+ '\n')
                            file_detect.write('车辆图片路径：./' + now_time + '.jpg'+ '\n')
                            file_detect.write('车辆视频路径：./3.mp4'+ '\n')
                            print('检测成功！')
                    else:
                        pass
    


            #cv2.imshow("mog",fgmask)  
            #cv2.imshow("thresh",th)  
        cv2.imshow("detection",frame0)

        if cv2.waitKey(20) & 0xff == 27:  
            break       
camera.release()
cv2.destroyAllWindows()
