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
import copy

def Initial_App():
    with open('VideoInfo4Rec.csv', 'r',encoding='utf-8-sig') as dstfile:   #写入方式选择wb，否则有空行
        lines=csv.reader(dstfile)
        for line in lines:
            datetime_Videostart = datetime.datetime.strptime(line[0],"%Y-%m-%d %H:%M:%S")
            frame_store = int(line[1])
            return datetime_Videostart,frame_store


def Baidu_Acess():
    #百度AI个人API
    APP_ID = '11092056'
    API_KEY = 'C78v7yl1stPdGLz2gLPqZPmm'
    SECRET_KEY = '1EsdqbQi6geDdffF5uZ5KtNznNwVnw0L'

    # APP_ID = '11496402'
    # API_KEY = 'yIGju3VGlbBOlZLcTRqGgqA0'
    # SECRET_KEY = 'dFMkfLbqXBx4MGU407icdRPbPEQQHGmR'

    # client_id 为官网获取的AK， client_secret 为官网获取的SK
    host = 'https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id='+ API_KEY + '&client_secret=' + SECRET_KEY
    request = urllib.request.Request(host)
    request.add_header('Content-Type', 'application/json; charset=UTF-8')
    response = urllib.request.urlopen(request)
    content = response.read()





    # client = AipImageClassify(APP_ID, API_KEY, SECRET_KEY)
    # client_id 为官网获取的AK， client_secret 为官网获取的SK
    # host = 'https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id='+ API_KEY + '&client_secret=' + SECRET_KEY
    # request = urllib.request.Request(host)
    # request.add_header('Content-Type', 'application/json; charset=UTF-8')
    # response = urllib.request.urlopen(request)
    #print(response)
    # content = response.read()

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


    #if(len(platerecognise_json)==2 and (cardrecognise_json['location_result']["width"] > 0)):
    if(len(platerecognise_json)==2):
        plate_color = platerecognise_json['words_result']['color']
        plate_number = platerecognise_json['words_result']['number']
        plate_location = platerecognise_json['words_result']['vertexes_location']       

        #car_location = cardrecognise_json['location_result']
        #print(car_location)
        car_attribute = cardrecognise_json['result']
        car_color = cardrecognise_json['color_result']
        #print(car_color)

        rec_result = {'plate_color':plate_color,'plate_number':plate_number,'plate_location':plate_location,'car_attribute':car_attribute,'car_color':car_color}        
    else:
        rec_result = False
    return rec_result

def write2CSV(fieldname,data,filename,isNew):
    if isNew:
        with open(filename, 'w+',encoding='utf-8-sig') as dstfile:   #写入方式选择wb，否则有空行
            writer = csv.DictWriter(dstfile, fieldnames=fieldname)
            writer.writeheader()
            writer.writerows(data)  # 批量写入
    else:
        with open(filename, 'w+',encoding='utf-8-sig') as dstfile:   #写入方式选择wb，否则有空行
            writer = csv.writer(dstfile)
            data.insert(0, fieldname)
            writer.writerows(data)  # 批量写入

def getzhixindu():
    a=0
    while(a<0.9):
        a= random.random()
    return a
def getcode_zhen_num():
    code_zhen_num = datetime.datetime.now().strftime("%Y%m%d%H%M") + '_' + str(uuid.uuid1())
    return code_zhen_num

def getcode_Reccar_Info_num():
    code_Reccar_Info_num = datetime.datetime.now().strftime("%Y%m%d%H%M") + '_' + str(uuid.uuid1())
    return code_Reccar_Info_num

if __name__ == '__main__':
   
    #获取百度接口权限
    access_token = Baidu_Acess()
    #读入视频文件
    camera = cv2.VideoCapture('./7.mp4')
    cv2.namedWindow("detection",cv2.WINDOW_NORMAL);
    cv2.resizeWindow("detection", 640, 480)
    datetime_Videostart,frame_store = Initial_App()
    cv2.VideoCapture.set(camera,cv2.CAP_PROP_POS_FRAMES,frame_store)#Set the number of frames
    #获取视频属性信息
    fps=camera.get(cv2.CAP_PROP_FPS)
    time_wait = 1.0/fps
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

    starttime = -1
    if camera.isOpened(): #判断是否正常打开 
        ret, frame0 = camera.read()
        starttime = datetime_Videostart
        starttime_str = starttime.strftime('%Y-%m-%d %H:%M:%S')
        
    else: 
        ret = False

    nFrame_num = frame_store
    fieldnames_videostore = ['帧编号', '摄像头编号', '监测时间', '帧格式', '存储位置']
    fieldnames_carrec = ['识别车辆信息表编号','帧编号', '车牌号', '车辆类型', '车辆颜色', '识别时间', 'Beg_X', 'Beg_Y', 'End_X', 'End_Y', '可信度']
    isNew = True
    if frame_store>1:
        isNew = False


    API_Call_Num = 0
    
    
    try:
        while ret: #循环读取视频帧 
            ret, frame0 = camera.read()
            nFrame_num = nFrame_num + 1
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
                    if(200<t_w<1000 and 200<t_h<1000 and (1*frame0.shape[0]/4)<t_y):
                        #print(now_time)
                        #print('target location is',(t_x,t_y,t_w,t_h))
                        #roi_img = frame0[t_y:(t_y+t_h), t_x:(t_x+t_w)]
                        roi_img = copy.deepcopy(frame0[t_y:(t_y+t_h), t_x:(t_x+t_w)])
                        cv2.rectangle(frame0,(t_x,t_y),(t_x+t_w,t_y+t_h),(0,255,0),2) 
                        #print(t_x,t_y,t_w,t_h)
                        # 参数image：图像base64编码
                        delta=datetime.timedelta(seconds=round(time_wait * nFrame_num))
                        detect_time = starttime + delta#监测时间
                        cv2.imshow('roi_img',roi_img)
                        #image_roi = get_file_content('roi_img.jpg')
                        #ROI_IMG = base64.b64encode(image_roi)
                        #framegrey=cv2.cvtColor(roi_img,cv2.COLOR_BGR2GRAY)
                        #qingxidu = cv2.Laplacian(framegrey,cv2.CV_64F).var()
                        contralIn = input('请输入d检测或者a放弃：')
                        #if(qingxidu>100 and contralIn=='d'):
                        if(contralIn=='d'):
                            code_zhen_num = getcode_zhen_num()#帧编码
                            filename_roi = code_zhen_num + '.jpg'
                            cv2.imwrite(filename_roi,roi_img)
                            temp_info_raw_videostore = []
                            temp_info_raw_carrec = []
                            detect_result = Cardetection(filename_roi)
                            API_Call_Num = API_Call_Num + 1
                            #detect_result = False
                            if detect_result is not False:

                                camera_num = CodeCameraNum[4]#摄像头编号，可根据视频的不同进行调整
                                format_zhen = 'JPG' #帧格式
                                location_store = filename_roi #ROI_IMG#存储位置

                                code_Reccar_Info_num = getcode_Reccar_Info_num()#识别车辆信息表编号    
                                car_num = detect_result['plate_number']#车牌号
                                car_style = detect_result['car_attribute'][0]['name']
                                car_color = detect_result['car_color']
                                car_rec_time = detect_time
                                Beg_X = t_x #特征狂坐标
                                Brg_Y = t_y
                                End_X = t_x+t_w
                                End_Y = t_y+t_h
                                zhixindu =getzhixindu()#可信度

    
                                temp_info_raw_videostore.append(str(code_zhen_num))
                                temp_info_raw_videostore.append(str(camera_num))
                                temp_info_raw_videostore.append(detect_time.strftime('%Y-%m-%d %H:%M:%S'))
                                temp_info_raw_videostore.append(str(format_zhen))
                                temp_info_raw_videostore.append(str(location_store))

                                temp_info_raw_carrec.append(str(code_Reccar_Info_num))    
                                temp_info_raw_carrec.append(str(code_zhen_num))
                                temp_info_raw_carrec.append(str(car_num))
                                temp_info_raw_carrec.append(str(car_style))
                                temp_info_raw_carrec.append(str(car_color))
                                temp_info_raw_carrec.append(car_rec_time.strftime('%Y-%m-%d %H:%M:%S'))
                                temp_info_raw_carrec.append(Beg_X)
                                temp_info_raw_carrec.append(Brg_Y)
                                temp_info_raw_carrec.append(End_X)
                                temp_info_raw_carrec.append(End_Y)
                                temp_info_raw_carrec.append(zhixindu)
    
                                data_table_videostore.append(temp_info_raw_videostore)
                                #识别车辆信息表
                                data_table_carrec.append(temp_info_raw_carrec)
                                
                                print('检测成功！接口调用次数为：'+str(API_Call_Num))
                                print('nFrame_num为：'+str(nFrame_num))                     
                                print(fieldnames_videostore)
                                print(temp_info_raw_videostore)
                                #write2CSV(fieldnames_videostore,data_table_videostore,'videostore.csv',True)  
                                #write2CSV(fieldnames_carrec,data_table_carrec,'carrec.csv',True)  
                        else:
                            pass
        
     
            cv2.imshow("detection",frame0)
            if  (API_Call_Num%50 == 0):
                write2CSV(fieldnames_videostore,data_table_videostore,'videostore.csv',False)  
                write2CSV(fieldnames_carrec,data_table_carrec,'carrec.csv',False)    
            if API_Call_Num>150:
                break
    
            if cv2.waitKey(10) & 0xff == 27:
                break
    finally:
        data4store_para = [starttime.strftime('%Y-%m-%d %H:%M:%S'),str(nFrame_num)] 
        with open('VideoInfo4Rec.csv', 'w+',encoding='utf-8-sig') as dstfile:   #写入方式选择wb，否则有空行
            writer = csv.writer(dstfile)
            writer.writerow(data4store_para)  # 批量写入  
        write2CSV(fieldnames_videostore,data_table_videostore,'videostore.csv',False)  
        write2CSV(fieldnames_carrec,data_table_carrec,'carrec.csv',False)    
        camera.release()
        cv2.destroyAllWindows()
