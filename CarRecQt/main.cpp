//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv/cvaux.h>
#include <opencv/ml.h>
#include <opencv2/objdetect/objdetect.hpp>
#include<opencv2/video/background_segm.hpp>

//C
#include <stdio.h>
#include <stdlib.h>
//C++
#include <string>
#include<iostream>
#include <fstream>
#include<sstream>
//#include <SDKDDKVer.h>


#include "vector"
#include "time.h"
#include <math.h>
#include "datastruct.h"
#include "carrec.h"

//trans
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Encode.h"
#include "Util.h"
#define MYPORT  7672


//lib depend
#include <uuid/uuid.h>
//#define IF_DEBUG
#include "svm.h"

#define SCALE4CARDETECT  5

int car_detect_recognise(cv::Mat frame, int width_mini, int eight_mini, cv::BackgroundSubtractorMOG &bgSubtractor, cv::Mat &mask,cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo);
int send_message(cv::Mat frame, struct CarInfo &iCarInfo, int sockfd, struct sockaddr_in servaddr);
const char *gettime_guid();
int generate_bluewhite_plete(cv::Mat &intemplete);


int main()
{
    //---------------------------------------------trans---------------------------------------------
    int sockfd = socket(AF_INET,SOCK_STREAM, 0);//套接字定义
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.20.86");

    int  chanelActiva=-1;//通道活性，0:已激活(连接+鉴权）;-1:未激活
    chanelActiva=startTCP(sockfd,servaddr);//启动TCP及鉴权机制

    pthread_t t0,t1;
    struct heartTheadVar args;
    args.arg1=sockfd;
    args.arg2=servaddr;
    pthread_create(&t0, NULL, heartThread,&args);//创建心跳线程
    //pthread_join(t1, NULL);
    //pthread_join(t0,NULL);

    //---------------------------------------------recognise---------------------------------------------
	struct CarInfo iCarInfo;
	cv::Mat element_close;
	cv::Mat element_open;
    element_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 5));
	element_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	
    //CvSVM mySVM_EN;
    //CvSVM mySVM_CH;
    //mySVM_EN.load("/home/wang/CarRec/mysvm_EN.xml");
    //mySVM_CH.load("/home/wang/CarRec/mysvm_CH.xml");
    svm_model * svm_en = svm_load_model("/home/wang/CarRec/libsvm_src_feature_model_en.model");
    svm_model * svm_ch = svm_load_model("/home/wang/CarRec/libsvm_src_feature_model_ch.model");
    cv::Mat tem;
    generate_bluewhite_plete(tem);

    //---------------------------------------------车辆检测---------------------------------------------strat

    ////////////////////////////////////////////////////////////////////////////////////////
    cv::VideoCapture cap;
    cap.open("/home/wang/CarRec/00390.mp4"); //打开视频,等价于   VideoCapture cap("E://2.avi");
    if(!cap.isOpened())
        return 0;

    int width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));  //帧宽度
    int height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT)); //帧高度
    int width_mini = int(width/SCALE4CARDETECT);  //帧宽度
    int height_mini = int(height/SCALE4CARDETECT); //帧高度
    cv::BackgroundSubtractorMOG bgSubtractor(100, 10, 0.5, false);
    cv::Mat mask; //前景
    while(cap.isOpened())
    {

        cv::Mat frame;    //定义一个Mat变量，用于存储每一帧的图像
        cap>>frame;//等价于cap.read(frame);

        if(frame.empty())
        {
            continue;
        }

        if(0 != car_detect_recognise(frame, width_mini, height_mini, bgSubtractor, mask, element_close, element_open, svm_ch , svm_en, iCarInfo))
        {
            continue;
        }
        if("NONE" == iCarInfo.iPleteInfo.pletenumber)
        {
            continue;
        }
        //----------------information display----------------strat
        if(0 != send_message(frame,iCarInfo, sockfd, servaddr))
        {
            continue;
        }
        //----------------information display----------------end
    }

//    cv::Mat frame = cv::imread("/home/wang/build-CarRec-Desktop_Qt_5_7_1_GCC_64bit-Debug/i20180822162630_75c61b30-4def-4cd1-bc73-1d9038c4a2e4.jpg");
//    CarRec iCarRec;
//    iCarRec.data_initial(iCarInfo);
//    if (-1 == iCarRec.car_rec(frame, element_close, element_open, svm_ch , svm_en, iCarInfo))
//    {
//        //printf("rec failure");
//        return -1;
//    }
    ////////////////////////////////////////////////////////////////////////////////////////
    svm_free_model_content(svm_ch);
    svm_free_model_content(svm_en);
    //-----------cap.release();
    //---------------------------------------------车辆检测---------------------------------------------end
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief car_detect_recognise for car recognise, include car color, plete number, plete color
/// \param frame input scene
/// \param width_mini for detect use gauss bg fast
/// \param height_mini for detect use gauss bg fast
/// \param bgSubtractor
/// \param mask
/// \param element_close
/// \param element_open
/// \param svm_ch for chinese model
/// \param svm_en for english model
/// \param iCarInfo instore information
/// \return 0 sucess -1 failure
int car_detect_recognise(cv::Mat frame, int width_mini, int height_mini, cv::BackgroundSubtractorMOG &bgSubtractor, cv::Mat &mask,cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo)
{
    CarRec iCarRec;
    iCarRec.data_initial(iCarInfo);
    cv::Mat Imggray; // background
    cv::Mat Imgmini; // background
    cv::Mat srcImage;//连通分量
    cv::Mat ROI;
    //Ptr<BackgroundSubtractorMOG2> pMOG2 = createBackgroundSubtractorMOG2(200, 36.0, false); // MOG2 Background subtractor
    //cv::Mat result;    //结果
    //cv::imwrite("frame.jpg", frame);
    //用混合高斯模型训练背景图像
    //! the full constructor that takes the length of the history, the number of gaussian mixtures, the background ratio parameter and the noise strength
    // CV_WRAP BackgroundSubtractorMOG(int history, int nmixtures, double backgroundRatio, double noiseSigma=0);
    //history为使用历史帧的数目
    //nmixtures为混合高斯数量
    //backgroundRatio为背景比例
    //noiseSigma为噪声权重
    //cv::BackgroundSubtractorMOG bgSubtractor(20, 10, 0.5, false);
    //外层vector的size代表了图像中轮廓的个数，里面vector的 size代表了轮廓上点的个数
    cv::vector<cv::Mat> contours(10000);
    cv::vector<cv::Vec4i> hierarchy(10000);

    resize(frame, Imgmini, cv::Size(width_mini, height_mini), cv::INTER_LINEAR);
    cvtColor(Imgmini, Imggray,CV_RGB2GRAY);
    //Imgmini.copyTo(result);
    //frame.copyTo(result);
    bgSubtractor(Imggray, mask, 0.01);
    //cv::imwrite("Imggray.jpg", Imggray);
    //imshow("原视频", frame);  //显示当前帧
    //imshow("混合高斯建模", mask);
    //对前景先进行中值滤波，再进行形态学膨胀操作，以去除伪目标和连接断开的小目标
    medianBlur(mask, mask, 3);
    //测试：先开运算再闭运算
    morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7)));
    //medianBlur(mask, mask, 3);
    //morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
    //cv::imwrite("mask.jpg", mask);
    //imshow("混合高斯建模", mask);
    mask.copyTo(srcImage);

    //各联通分量的轮廓

    //只获取最外轮廓，获取每个轮廓的每个像素，并相邻两个像素位置差不超过1
    findContours(srcImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    //测试轮廓获取
    //cv::imwrite("轮廓获取.jpg", srcImage);
    if (contours.size() < 1)
        return -1;
    //外接矩阵
    cv::Rect rct;

    int mini_length = int(width_mini*height_mini/16);
    int max_length = int(width_mini*height_mini);
    for (int i = 0; i < (int)contours.size(); i++)
    {
        //int sasdasd = (int)contours.size();
        //int sasdasd1 = contourArea(contours[i]);
        //当第i个连通分量的外接矩阵面积小于最大面积的1/8，则认为是伪目标
        if (cv::contourArea(contours[i]) < mini_length || cv::contourArea(contours[i]) > max_length )
            continue;
        //包含轮廓的最小矩阵

        rct = cv::boundingRect(contours[i]);
        int min_width = 200/SCALE4CARDETECT;
        int min_height = 200/SCALE4CARDETECT;
        if((rct.x + rct.width) > (width_mini - 10) || (rct.y + rct.height) > (height_mini - 10) || rct.width < min_width || rct.height < min_height)
            continue;
        iCarInfo.location.x = int(SCALE4CARDETECT*rct.x);
        iCarInfo.location.y = int(SCALE4CARDETECT*rct.y);
        iCarInfo.location.width = int(SCALE4CARDETECT*rct.width);
        iCarInfo.location.height = int(SCALE4CARDETECT*rct.height);
        ///--------------------------------she ding xu ni jian ce qu yu----------------------------///
        int vr_x = frame.cols/4;
        int vr_y = frame.rows/8;
        int vr_width = int(frame.cols*0.5);
        int vr_height = int(frame.rows*0.75);
        float zhongxin_x = iCarInfo.location.x + iCarInfo.location.width/2;
        float zhongxin_y = iCarInfo.location.y + iCarInfo.location.height/2;
        if(zhongxin_x < vr_x|| zhongxin_x > (vr_x + vr_width)|| zhongxin_y < vr_y|| zhongxin_y > (vr_y + vr_height))
        {
           return -1;
        }

        //rectangle(result, rct, cv::Scalar(0, 255, 0), 2);
        ROI = frame(cv::Rect(iCarInfo.location.x, iCarInfo.location.y,  iCarInfo.location.width, iCarInfo.location.height));

        if (-1 == iCarRec.car_rec(ROI, element_close, element_open, svm_ch , svm_en, iCarInfo))
        {
            //printf("rec failure");
            return -1;
        }
    }
    //imshow("mask", mask);
    //imshow("Imgmini", Imgmini);
    //cv::waitKey(1);
    return 0;
    //imshow("video", frame);
}
int send_message(cv::Mat frame, struct CarInfo &iCarInfo, int sockfd, struct sockaddr_in servaddr)
{
    char str_time[100];
    char str_time_uuid[100];
    time_t now ;
    struct tm *tm_now ;
    time(&now) ;
    tm_now = localtime(&now) ;
    sprintf(str_time,"%d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    sprintf(str_time_uuid,"%d%02d%02d%02d%02d%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    uuid_t uuid;
    char str_uuid[36];
    uuid_generate(uuid);
    uuid_unparse(uuid, str_uuid);
    //iCarRec.random_uuid(guid);
    std::string  str0 = str_time_uuid;
    std::string  str1 = str_uuid;
    std::string  string_time_guid= str0 + "_" + str1;

    std::string  P_string_time_guid= string_time_guid + ".jpg";
    cv::imwrite(P_string_time_guid,frame);
    iCarInfo.imgcardetect = P_string_time_guid;

    //cv::Mat imgroi = frame(cv::Rect(iCarInfo.location.x, iCarInfo.location.y,  iCarInfo.location.width, iCarInfo.location.height));
    //std::string  iP_string_time_guid= "i" + string_time_guid + ".jpg";
    //cv::imwrite(iP_string_time_guid,imgroi);

    //char *url = (char *)iCarInfo.imgcardetect;
    char *url=NULL;
    url = (char *)iCarInfo.imgcardetect.c_str();
    //strcpy(url,iCarInfo.imgcardetect.c_str());
    pictureMsg(url,sockfd,servaddr);
    char str[10];
    struct structedCar car;
    strcpy(car.msgId,"03");
    sprintf(str, "%02d", iCarInfo.colorstyle);
    strcpy(car.carColor,str);
    strcpy(car.carNo,iCarInfo.iPleteInfo.pletenumber.c_str());
    strcpy(car.eventId,string_time_guid.c_str());

    strcpy(car.frameId,string_time_guid.c_str());
    sprintf(str, "%d", iCarInfo.location.x);
    strcpy(car.x1,str);
    sprintf(str, "%d", iCarInfo.location.y);
    strcpy(car.y1,str);
    sprintf(str, "%d", (iCarInfo.location.width+iCarInfo.location.x));
    strcpy(car.x2,str);
    sprintf(str, "%d", (iCarInfo.location.height+iCarInfo.location.y));
    strcpy(car.y2,str);
    strcpy(car.time,str_time);
    car.reliablity=float(iCarInfo.iPleteInfo.confidenceper);
    char * tepBuffer=getCarEncode(car);
    int len=send(sockfd, tepBuffer, strlen(tepBuffer),0);
    if(len<=0)
       {
           perror("ERROR");
           printf("thread2测试时TCP连接出现异常\n");
           return -1;
           //break;
       }
    return 0;
}
const char *gettime_guid()
{

}
int generate_bluewhite_plete(cv::Mat &intemplete)
{
    cv::Mat templete = cv::Mat::zeros(cv::Size(405,90), CV_8UC1);
    for(int i=0; i < templete.cols; i++)
        for(int j=0; j < templete.rows; j++)
        {
            if(i<=45||(i>=57&&i<=102)||(i>=136&&i<=181)||(i>=193&&i<=238)||(i>=250&&i<=295)||(i>=307&&i<=352)||(i>=364&&i<=409))
            {
                templete.at<uchar>(j,i) = 255;
            }
        }
    cv::imwrite("templete.jpg", templete);
    cv::resize(templete,intemplete,cv::Size(int(405*32/90.0),32));
}
