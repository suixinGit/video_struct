#ifndef CARREC_H
#define CARREC_H
#include "datastruct.h"
//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv/cvaux.h>
#include <opencv/ml.h>
#include <opencv2/objdetect/objdetect.hpp>
#include<opencv2/video/background_segm.hpp>
#include "svm.h"
class CarRec
{
public:
    CarRec();

public:
    //--------------------------------全局变量--------------------------------
    cv::Size imageSize;

    cv::Rect bluewhite[7];


    void data_initial(struct CarInfo &iCarInfo);
    std::string get_plete_number(cv::Mat img_plete_rRect, int plete_color_styple, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo);//-1表示车牌获取失败
    int plete_recognise(cv::Mat img, cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo);
    void coumputeHog(const cv::Mat& src, cv::vector<float> &descriptors);
    int charRecEN(cv::Mat char_spliter, svm_model * svm, struct labelInfo &ilabelInfo);
    int charRecCH(cv::Mat char_spliter, svm_model * svm, struct labelInfo &ilabelInfo, int province);
    void delete_plete_noiseY(cv::Mat &in, cv::Mat &out);
    void delete_plete_noiseXY(cv::Mat &in, cv::Rect &out);
    int get_car_color_style(cv::Mat car);
    void sortvalue_label(int lels[], int count, int label[]);
    void swap(int *a,int *b);//用于将元素a和元素b交换
    int get_plete_color_style(cv::Mat plete);
    int car_rec(cv::Mat car,cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo);
    void CalculateIntegralMap(cv::Mat &img, cv::Mat &imgIntegral);
    //int  offX_match_best(int pletetype, cv::Mat base);
    int get_offx(int pletetype, cv::Mat imgIntegral);
    int get_offx_m1(int pletetype, cv::Mat imgIntegral);
};

#endif // CARREC_H
