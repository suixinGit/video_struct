#include "carrec.h"
#define NUM_CLASSES_CH 31
#define NUM_CLASSES_EN 34
CarRec::CarRec()
{
    imageSize = cv::Size(16, 32);
    for(int i=0; i<7; i++)
    {
        bluewhite[i].width = 19;
        bluewhite[i].height = 32;
        bluewhite[i].y = 0;
    }
    bluewhite[0].x = 0;
    bluewhite[1].x = 19;
    bluewhite[2].x = 47;
    bluewhite[3].x = 67;
    bluewhite[4].x = 87;
    bluewhite[5].x = 108;
    bluewhite[6].x = 128;
}
//get max value of 1-D array
double GetMaxD(double *ptr, int len)
{
    if(len < 1)
        return -1;
    double max = *ptr;
    for(int i = 0; i < len; i++)
    {
        if(*(ptr+i)>max)
            max = *(ptr+i);
    }
    return max;
}
//get max value and index
int GetMaxID(double *ptr, int len, struct labelInfo &ilabelInfo)
{
    if(len < 1)
        return -1;
    double max = *ptr;
    int maxindex = 0;
    for(int i = 0; i < len; i++)
    {
        if(*(ptr+i)>max)
        {
            max = *(ptr+i);
            maxindex = i;
        }
    }
    ilabelInfo.Label = maxindex;
    ilabelInfo.confidenceper = max;
    return 0;
}

/************************************************
* gamma correction
* template<...> _Tpsaturate_cast(_Tp2v): Template function for
* accurate conversion from one primitive type to another
*/
void GammaCorrection(cv::Mat& src, cv::Mat& dst, float fGamma)
{
    // build look up table
    unsigned char lut[256];
    for( int i = 0; i < 256; i++ ) {
        lut[i] = cv::saturate_cast<uchar>(pow((float)(i/255.0), fGamma) * 255.0f);
    }

    dst = src.clone();
    const int channels = dst.channels();
    switch(channels)
    {
    case 1: {
        cv::MatIterator_<uchar> it, end;
        for( it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++ )
            *it = lut[(*it)];
        break;
    }
    case 3: {
        cv::MatIterator_<cv::Vec3b> it, end;
        for( it = dst.begin<cv::Vec3b>(), end = dst.end<cv::Vec3b>(); it != end; it++ ) {
            (*it)[0] = lut[((*it)[0])];
            (*it)[1] = lut[((*it)[1])];
            (*it)[2] = lut[((*it)[2])];
        }
        break;
    }
    }
}

void CarRec::data_initial(struct CarInfo &iCarInfo)
{
    iCarInfo.colorstyle = -1;
    iCarInfo.location = cv::Rect(0,0,0,0);
    iCarInfo.iPleteInfo.colorstyle = -1;
    iCarInfo.iPleteInfo.location = cv::Rect(0,0,0,0);
    iCarInfo.iPleteInfo.pletenumber = "NONE";
    iCarInfo.iPleteInfo.confidenceper = 0;
    iCarInfo.imgcardetect;
}

int CarRec::car_rec(cv::Mat car,cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo)
{
    //---------------------------------车辆颜色------------------------------------------------------------
    iCarInfo.colorstyle = get_car_color_style(car);
    //---------------------------------车牌定位------------------------------------------------------------
    if (-1 == plete_recognise(car, element_close, element_open, svm_ch, svm_en, iCarInfo))
    {
        //printf("车牌定位失败！\n");
        return -1;
    }
    return 0;
}


//--------------------------------函数定义--------------------------------

void CarRec::coumputeHog(const cv::Mat& src, cv::vector<float> &descriptors)
{
    cv::vector<float> descriptors1(1000);
    cv::HOGDescriptor myHog0 = cv::HOGDescriptor(imageSize, cv::Size(16, 16), cvSize(8, 8), cvSize(8, 8), 9);
    cv::HOGDescriptor myHog1 = cv::HOGDescriptor(imageSize, cv::Size(8, 8), cvSize(4, 4), cvSize(4, 4), 9);
    myHog0.compute(src.clone(),descriptors,cv::Size(1,1),cv::Size(0,0));
    myHog1.compute(src.clone(),descriptors1,cv::Size(1,1),cv::Size(0,0));
    descriptors.insert(descriptors.end(),descriptors1.begin(),descriptors1.end());
}

int CarRec::plete_recognise(cv::Mat img, cv::Mat element_close, cv::Mat element_open, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo)
{
    cv::Mat img_Gaussian;
    cv::Mat img_Gray;
    cv::Mat img_Edgedetect;
    cv::Mat img_Threshold;
    cv::Mat img_Morphology;
    cv::Mat img_plete_rRect;
    cv::Mat rotmat;
    cv::Mat deal_img;
    cv::Mat Hist;

    //GaussianBlur(img, img_Gaussian, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);//高斯滤波
    cvtColor(img, img_Gray, CV_RGB2GRAY);
    //imshow("img_Gray", img_Gray);
    //cv::waitKey();
    //zeng qiang
    equalizeHist(img_Gray,Hist);
    //cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
    //cv::filter2D(img_Gray, Hist, CV_8UC3, kernel);
    //imshow("Hist", Hist);
    //cv::waitKey();

    medianBlur(Hist, img_Gaussian, 3);
    //imshow("img_Gaussian", img_Gaussian);
    //cv::waitKey();
    Sobel(img_Gaussian, img_Gaussian, CV_16S, 1, 0, 3, 1, 1, cv::BORDER_DEFAULT);
    convertScaleAbs(img_Gaussian, img_Edgedetect);
    //imshow("img_Edgedetect", img_Edgedetect);
    //cv::waitKey();
    threshold(img_Edgedetect, img_Threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
    //imshow("img_Threshold", img_Threshold);
    //cv::waitKey();
    morphologyEx(img_Threshold, img_Morphology, cv::MORPH_CLOSE, element_close);
    //imshow("img_Morphology", img_Morphology);
    //cv::waitKey();
    morphologyEx(img_Morphology, img_Morphology, cv::MORPH_OPEN, element_open);
    //imshow("img_Morphology", img_Morphology);
    //cv::waitKey();
    //cv::imwrite("img_Morphology.jpg",img_Morphology);


    //外层vector的size代表了图像中轮廓的个数，里面vector的 size代表了轮廓上点的个数
    cv::vector<cv::Mat> contours(10000);
    cv::vector<cv::Vec4i> hierarchy(10000);
    //各联通分量的轮廓
    //只获取最外轮廓，获取每个轮廓的每个像素，并相邻两个像素位置差不超过1
    findContours(img_Morphology, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    if (contours.size() < 1)
        return -1;

    //外接矩阵
    cv::Rect rct;
    int hastarget = -1;

    for (int i_plete_num = 0; i_plete_num < (int)contours.size(); i_plete_num++)
    {
        //当第i个连通分量的外接矩阵面积小于最大面积的1/6，则认为是伪目标
        //int aera = contours[i].rows * contours[i].cols;
        int area = int(contourArea(contours[i_plete_num]));
        if (area < 1000 || area >  1000000)
        {
            continue;
            hastarget = -1;
        }

        cv::RotatedRect rect=minAreaRect(contours[i_plete_num]);
        float width2height = rect.size.width/rect.size.height;
        if(rect.size.width >1000 || rect.size.height >1000 || (0.4< width2height && width2height<2.5))
            continue;
        float angle = 0.0;
        cv::Size si = rect.size;
        if (rect.size.width <= rect.size.height)
        {
            angle = rect.angle + 90;
            int tm = si.width;
            si.width = si.height;
            si.height = tm;
        }
        else
        {
            angle = rect.angle;
        }
        si.width = int(si.width*cos(angle*PI/180));
        if (si.width < 30 || si.height < 30)//车牌至少在30像素以上
        {
            return -1;
        }
        rotmat = getRotationMatrix2D(rect.center, angle, 1);
        warpAffine(img, deal_img, rotmat, img.size(), cv::INTER_LINEAR);
        //waitKey();
        imwrite("deal_img.jpg",deal_img);

        getRectSubPix(deal_img, si, rect.center, img_plete_rRect);
        imwrite("img_plete_rRect.jpg",img_plete_rRect);
        iCarInfo.iPleteInfo.location.width = si.width;
        iCarInfo.iPleteInfo.location.height = si.height;
        iCarInfo.iPleteInfo.location.x = int(rect.center.x-si.width/2);
        iCarInfo.iPleteInfo.location.y = int(rect.center.y-si.height/2);
        //---------------------------------车牌颜色------------------------------------------------------------
        int pletestyle = get_plete_color_style(img_plete_rRect);
        iCarInfo.iPleteInfo.colorstyle = pletestyle;
        //---------------------------------车牌识别------------------------------------------------------------
        std::string plete_number = get_plete_number(img_plete_rRect, iCarInfo.iPleteInfo.colorstyle, svm_ch, svm_en, iCarInfo);
        if ("-1" == plete_number)
        {
            //printf("车牌识别失败！\n");
            continue;
        }
        else
        {
            //printf("%s",plete_number.c_str());
            //std::cout<<plete_number<<std::endl;
            iCarInfo.iPleteInfo.pletenumber = plete_number;
            hastarget = 0;
            break;
        }
        //imwrite("识别的车牌.jpg", img_plete_rRect);
        //plete = img_plete_rRect.clone();
    }
    return hastarget;
}


std::string CarRec::get_plete_number(cv::Mat img_plete_rRect, int plete_color_styple, svm_model * svm_ch , svm_model * svm_en, struct CarInfo &iCarInfo)
{
    cv::Mat pletehsv;
    cv::Mat pletehsv_threshold;
    cv::Mat img_plete_color;
    cv::Mat img_plete_gray;
    cv::Mat img_plete_median;
    cv::Mat img_plete_threshold;
    cv::Mat img_plete_open;
    cv::Mat img_deleteYnoise;
    cvtColor(img_plete_rRect,pletehsv,CV_BGR2HSV);
    //imshow("img_plete_rRect",img_plete_rRect);
    //cv::waitKey();

    switch (plete_color_styple)
    {
    case COLOR_PLETE_BLUE_WHITE:
        cv::inRange(pletehsv, cv::Scalar(90, 10, 40), cv::Scalar(130, 255, 255), pletehsv_threshold); //Threshold the image
        break;
    case COLOR_PLETE_YELLOW_BLACK:
        cv::inRange(pletehsv, cv::Scalar(10, 43, 46), cv::Scalar(34, 255, 255), pletehsv_threshold); //Threshold the image
        break;
    case COLOR_PLETE_GREEN_BLACK:
        cv::inRange(pletehsv, cv::Scalar(40, 43, 46), cv::Scalar(80, 255, 255), pletehsv_threshold); //Threshold the image
        break;
    case COLOR_PLETE_OTHER_STYLE:
        return "-1";
        break;
    }
    cv::imwrite("pletehsv_threshold.jpg",pletehsv_threshold);
    cv::Rect char_rect;
    delete_plete_noiseXY(pletehsv_threshold, char_rect);
    if (char_rect.width<20 || char_rect.height<20 || (char_rect.height/char_rect.width<2.5&&char_rect.height/char_rect.width>0.4))
    {
        return "-1";
    }
    //cv::Rect plete_rect;
    //plete_rect = cv::boundingRect(contours);
    img_plete_color = img_plete_rRect(char_rect);

    cvtColor(img_plete_color, img_plete_gray, CV_RGB2GRAY);
    //imshow("img_plete_gray", img_plete_gray);
    //cv::waitKey();
    //cv::Mat pgamma;
    //GammaCorrection(img_plete_gray, pgamma, 0.5);
    //imshow("pgamma", pgamma);
    //cv::waitKey();
    if(COLOR_PLETE_YELLOW_BLACK == plete_color_styple || COLOR_PLETE_GREEN_BLACK == plete_color_styple)
    {
        img_plete_gray = ~ img_plete_gray;
    }

    cv::Mat Hist;
    //equalizeHist(img_plete_gray, Hist);
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << -1, -1, -1, -1, 9, -1, -1, -1, -1);
    cv::filter2D(img_plete_gray, Hist, CV_8UC1, kernel);
    if(Hist.cols < 1 || Hist.rows < 1)
    {
        return "-1";
    }
    //imshow("Hist", Hist);
    //cv::waitKey();
    cv::medianBlur(Hist,img_plete_median,3);
    threshold(img_plete_median, img_plete_threshold, 0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
    cv::imwrite("img_plete_threshold.jpg",img_plete_threshold);
    //cv::Scalar tempVal = cv::mean(Hist);
    //float matMean = tempVal.val[0];

    //    cv::Mat median_p1;
    //    medianBlur( Hist, median_p1, 3);
    //    imshow("median_p1", median_p1);
    //    cv::waitKey();
    //    //medianBlur( img_plete_gray, img_plete_median, 3);
    //    //equalizeHist(img_plete_gray, img_plete_hist);
    //    threshold(median_p1, img_plete_threshold, 0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
    //    imshow("img_plete_threshold", img_plete_threshold);
    //    cv::waitKey();
    //    //imwrite("img_plete_threshold.jpg",img_plete_threshold);
    //    medianBlur( img_plete_threshold, img_plete_median, 3);
    //    imwrite("img_plete_median.jpg",img_plete_median);

    cv::Mat ML_rows(img_plete_threshold.rows, 1, CV_32F, cv::Scalar(0));
    cv::Mat ML_cols(1, Hist.cols, CV_32F, cv::Scalar(0));
    //矩阵被简化后的维数索引.0意味着矩阵被处理成一行,1意味着矩阵被处理成为一列,-1时维数将根据输出向量的大小自动选择.
    cv::reduce(img_plete_threshold, ML_rows, 1, CV_REDUCE_SUM, CV_32F);
    cv::reduce(Hist, ML_cols, 0, CV_REDUCE_AVG, CV_32F);
//    for (int i = 0; i < ML_rows.rows; i++)
//    {
//        int t = ML_rows.at<float>(i,0);
//    }
//    for (int i = 0; i < ML_cols.cols; i++)
//    {
//        int t = ML_cols.at<float>(0,i);
//    }
    int pointX_array_start[1][10] = {0};
    int pointX_array_end[1][10] = {0};
    int pointY_array_start[1][2] = {0};
    int pointY_array_end[1][2] = {Hist.rows-1, Hist.rows-1};
    int card_width = Hist.cols;
    int card_width_min = Hist.cols/30;
    int card_height_min = Hist.rows/15;
    //ML_rows = ML_rows - matMean;
    //y方向确定范围
    for (int i = ML_rows.rows/2; i > 0; i--)
    {
        if(ML_rows.at<float>(i,0)<255*20)
        {
            pointY_array_start[0][0] = i;
            break;
        }

    }
    for (int i = ML_rows.rows/2; i < ML_rows.rows; i++)
    {
        if(ML_rows.at<float>(i,0)<255*20)
        {
            pointY_array_end[0][0] = i;
            break;
        }

    }
    cv::Mat img_background = Hist(cv::Rect(0,pointY_array_start[0][0], Hist.cols, pointY_array_end[0][0]-pointY_array_start[0][0]));
    //imshow("img_background", img_background);
    //cv::waitKey();
    if(img_background.cols < 1 || img_background.rows < 1)
    {
        return "-1";
    }
    cv::resize(img_background,img_background,cv::Size(int(32.0*img_background.cols/img_background.rows), 32));
    cv::Mat img_base;
    img_background.copyTo(img_base);
    if(img_base.cols < 120)
    {
        return "-1";
    }
    cv::medianBlur(img_background,img_background,3);
    cv::imwrite("img_base.jpg", img_base);
    //cv::waitKey();
    threshold(img_background, img_background, 0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
    //imshow("img_background", img_background);
    //cv::waitKey();
    cv::Mat imgIntegral(1, img_background.cols, CV_32S, cv::Scalar(0));
    //矩阵被简化后的维数索引.0意味着矩阵被处理成一行,1意味着矩阵被处理成为一列,-1时维数将根据输出向量的大小自动选择.
    cv::imwrite("img_background.jpg", img_background);
    cv::reduce(img_background, imgIntegral, 0, CV_REDUCE_SUM, CV_32S);
    int offx = get_offx_m1(plete_color_styple, imgIntegral);



    //CalculateIntegralMap(img_background, imgIntegral);
    //int offx = offX_match_best(plete_color_styple, imgIntegral);

    /*
    //long card_max = ML_cols.at<long>(0,0);
    //long card_min = ML_cols.at<long>(0,0);
    long card_pre = ML_cols.at<float>(0,0);
    long card_now = ML_cols.at<float>(0,0);
    int isincrese_pre = 0;//0,平；1升；-1降
    int isincrese_now = 0;
    int point_y_start_num = 0;
    int point_y_end_num = 0;
    bool istracing = false;
    int char_start = 0;
    int char_lenth_start = 0;
    int char_lenth_end = 0;
    for (int i = 0; i < img_plete_median.cols; i++)
    {
        //printf("%d,",ML_cols.at<long>(0,i));
        card_now = ML_cols.at<long>(0,i);
        if ((card_now > long(card_pre*3)) && card_now>card_height_min*255)
        {
            isincrese_now = 1;
            if (0 == char_start)
            {
                char_start = i;
            }
        }
        else if (card_now < long(card_pre*0.3))
        {
            isincrese_now = -1;
        }
        else
        {
            isincrese_now = 0;
        }
        //printf("--%d,%d--\n",i,card_now);
        if (1 == isincrese_now && isincrese_pre < 1  && point_y_start_num < 10 && false == istracing)
        {
            //isincrese_now = false;
            if (i>1)
            {
                pointX_array_start[0][point_y_start_num] = i-2;
            }
            else if (i>0)
            {
                pointX_array_start[0][point_y_start_num] = i-1;
            }
            else
            {
                pointX_array_start[0][point_y_start_num] = i;
            }
            char_lenth_start = pointX_array_start[0][point_y_start_num];
            //printf("start:%d,%d\n",point_y_start_num,pointX_array_start[0][point_y_start_num]);
            point_y_start_num = point_y_start_num + 1;
            istracing = true;
        }
        int temp = int(card_width/8-char_start);
        if (i<temp)
        {
            isincrese_now = 0;
        }
        if (true == istracing && -1 == isincrese_now && point_y_end_num < 10)
        {
            if (i<(img_plete_median.cols-2))
            {
                pointX_array_end[0][point_y_end_num] = i+2;
            }
            else if (i<(img_plete_median.cols-1))
            {
                pointX_array_end[0][point_y_end_num] = i+1;
            }
            else
            {
                pointX_array_end[0][point_y_end_num] = i;
            }
            //printf("end:%d,%d\n",point_y_end_num,pointX_array_end[0][point_y_end_num]);
            char_lenth_end = pointX_array_end[0][point_y_end_num];
            if ((char_lenth_end-char_lenth_start)<card_width_min)
            {
                point_y_start_num = point_y_start_num - 1;
                point_y_end_num = point_y_end_num-1;
            }
            point_y_end_num = point_y_end_num + 1;
            istracing = false;
        }
        card_pre = card_now;
        isincrese_pre = isincrese_now;
    }
    if (true == istracing)
    {
        pointX_array_end[0][point_y_end_num] = img_plete_median.cols-1;
        char_lenth_end = pointX_array_end[0][point_y_end_num];
        if ((char_lenth_end-char_lenth_start)<card_width_min)
        {
            point_y_start_num = point_y_start_num - 1;
            point_y_end_num = point_y_end_num-1;
        }
        point_y_end_num = point_y_end_num + 1;
        istracing =false;
    }
    int char_num = point_y_end_num;
    if (char_num<6)
    {
        return "-1";
    }
    else if(char_num>8)
    {
        char_num = 8;
    }*/

    //获取类别和数字、字母、中文的映射
    FILE *fp = NULL;
    //判断按读方式打开一个名叫test的文件是否失败
    if((fp=fopen("/home/wang/CarRec/labelMap.txt","r")) == NULL)//打开操作不成功
    {
        printf("The file can not be opened.\n");
        exit(1);//结束程序的执行
    }
    //int ch=fgetc(fp); //从fp所指文件的当前指针位置读取一个字符
    char line[10];
    const char *delims={",\n"};
    char *p;
    std::string labelmap[65];
    while(!feof(fp)) //判断刚读取的字符是否是文件结束符
    {
        fgets(line,10,fp);
        int num = atoi(strtok(line,delims));
        //p=strtok(line,delims);
        p = strtok(NULL,delims);
        if(0x0 != p)
        {
            labelmap[num] = p;
        }
        //p=strtok(NULL,delims);
    } //完成将fp所指文件的内容输出到屏幕上显示
    fclose(fp); //关闭fp所指文件
    //imshow("img_base", img_base);
    //cv::waitKey();
    //std::string plete_number;
    std::string plete_number_s = "";
    //const char *plete_number;
    int label_index = 0;
    struct labelInfo ilabelInfo;
    int province = 21;
    switch (plete_color_styple)
    {
    case COLOR_PLETE_BLUE_WHITE:
        for(int char_id = 0; char_id < 7; char_id++)
        {
            cv::Mat char_spliter;
            if((bluewhite[char_id].x+offx) > (img_base.cols-bluewhite[char_id].width))
            {
                char_spliter = img_base(cv::Rect(img_base.cols-bluewhite[char_id].width-1,bluewhite[char_id].y, bluewhite[char_id].width, bluewhite[char_id].height));
            }
            else if((bluewhite[char_id].x+offx) < 0)
            {
                char_spliter = img_base(cv::Rect(0,bluewhite[char_id].y, bluewhite[char_id].width, bluewhite[char_id].height));
            }
            else
            {
                char_spliter = img_base(cv::Rect(bluewhite[char_id].x+offx,bluewhite[char_id].y, bluewhite[char_id].width, bluewhite[char_id].height));

            }
            char str[10];
            sprintf(str, "%d", char_id);
            //itoa(i, str, 10);
            std::string strfile = str;
            strfile  = strfile + ".jpg";
            imwrite(strfile,char_spliter);
            if (0 == char_id)
            {
                charRecCH(char_spliter, svm_ch, ilabelInfo, province);
                iCarInfo.iPleteInfo.confidenceper = ilabelInfo.confidenceper;
                if(iCarInfo.iPleteInfo.confidenceper > 1.0)
                {
                    iCarInfo.iPleteInfo.confidenceper = 0.999888666;
                }
                ilabelInfo.Label = ilabelInfo.Label + 34;
            }
            else
            {
                charRecEN(char_spliter, svm_en, ilabelInfo);
            }
            plete_number_s = plete_number_s + labelmap[ilabelInfo.Label];
        }
        break;
    case COLOR_PLETE_YELLOW_BLACK:
        return "-1";
        break;
    case COLOR_PLETE_GREEN_BLACK:
        return "-1";
        break;
    case COLOR_PLETE_OTHER_STYLE:
        return "-1";
        break;
    }
    /*for (int i = 0; i < char_num; i++)
    {
        int width = pointX_array_end[0][i] - pointX_array_start[0][i];
        int height = pointY_array_end[0][0] - pointY_array_start[0][0];
        char str[10];
        int length_char;
        if (width<height)
        {
            length_char = height;
        }
        else
        {
            length_char = width;
        }
        cv::Mat char_spliter(length_char, length_char, CV_8UC1, cv::Scalar::all(0));
        for (int ichar_r = 0; ichar_r < height; ichar_r++)
            for (int ichar_c = 0; ichar_c < width; ichar_c++)
            {
                int x_off= int(std::abs(length_char-width)/2.0); //水平平移量
                int y_off= int(std::abs(length_char-height)/2.0); //竖直平移量
                char_spliter.at<uchar>(y_off + ichar_r, x_off + ichar_c) = img_plete_threshold.at<uchar>(pointY_array_start[0][0] + ichar_r, pointX_array_start[0][i] + ichar_c);
            }
        //imwrite("char_spliter.jpg",char_spliter);

        sprintf(str, "%d", i);
        //itoa(i, str, 10);
        std::string strfile = str;
        strfile  = strfile + ".jpg";
        imwrite(strfile,char_spliter);
        if (0 == i)
        {
            charRecCH(char_spliter, svm_ch, ilabelInfo, province);
            iCarInfo.iPleteInfo.confidenceper = ilabelInfo.confidenceper;
            if(iCarInfo.iPleteInfo.confidenceper > 1.0)
            {
                iCarInfo.iPleteInfo.confidenceper = 0.999888666;
            }
            ilabelInfo.Label = ilabelInfo.Label + 34;
        }
        else
        {
            charRecEN(char_spliter, svm_en, ilabelInfo);
        }
        plete_number_s = plete_number_s + labelmap[ilabelInfo.Label];
        //plete_number = plete_number_s.c_str();
    }
    //plete_number =  plete_number_s.c_str();*/

    return plete_number_s;
}
void CarRec::delete_plete_noiseY(cv::Mat &in, cv::Mat &out)
{
    int YUstart = 0;
    int YDstart = 0;
    cv::Mat ML_rows(in.rows, 1, CV_32S, cv::Scalar(0));
    reduce(in, ML_rows, 1, CV_REDUCE_SUM, CV_32S);//行向量,对各列求和
    int thread4keep = 255*in.cols*2/3;
    for (int irows = 0; irows < in.rows; irows++)
    {
        if (ML_rows.at<int>(irows,0)>thread4keep)
        {
            YUstart = irows;
            break;
        }
    }
    for (int irows = in.rows-1; irows >0; irows--)
    {
        if (ML_rows.at<int>(irows,0)>thread4keep)
        {
            YDstart = irows;
            break;
        }
    }
    out = in(cv::Rect(0,YUstart,in.cols,YDstart-YUstart));

}
void CarRec::delete_plete_noiseXY(cv::Mat &in, cv::Rect &out)
{
    int XLstart = 0;
    int XRstart = 0;
    int YUstart = 0;
    int YDstart = 0;
    cv::Mat ML_rows(in.rows, 1, CV_32S, cv::Scalar(0));
    cv::Mat ML_cols(1, in.cols, CV_32S, cv::Scalar(0));
    reduce(in, ML_rows, 1, CV_REDUCE_SUM, CV_32S);//行向量,对各列求和
    reduce(in, ML_cols, 0, CV_REDUCE_SUM, CV_32S);//行向量,对各列求和
    int thread4keep = 255;
    for (int irows = 0; irows < in.rows; irows++)
    {
        if (ML_rows.at<int>(irows,0)>thread4keep)
        {
            YUstart = irows;
            break;
        }
    }
    for (int irows = in.rows-1; irows >0; irows--)
    {
        if (ML_rows.at<int>(irows,0)>thread4keep)
        {
            YDstart = irows;
            break;
        }
    }
    for (int icols = 0; icols < in.cols; icols++)
    {
        if (ML_cols.at<int>(0,icols)>thread4keep)
        {
            XLstart = icols;
            break;
        }
    }
    for (int icols = in.cols-1; icols >0; icols--)
    {
        if (ML_cols.at<int>(0,icols)>thread4keep)
        {
            XRstart = icols;
            break;
        }
    }
    out = cv::Rect(XLstart,YUstart,XRstart-XLstart,YDstart-YUstart);
}

int CarRec::get_car_color_style(cv::Mat car)
{
    int colornum_car[COLOR_CAR_STYLE_NUMBER] = {0};
    int colornum_car_label[COLOR_CAR_STYLE_NUMBER];
    for (int i=0;i<COLOR_CAR_STYLE_NUMBER;i++)
    {
        colornum_car_label[i]=i;
    }
    ////////////blue////////////
    int blueh_min = 100;
    int blueh_max = 124;
    int blues_min = 43;
    int blues_max = 255;
    int bluev_min = 46;
    int bluev_max = 255;
    ////////////yellow////////////
    int yellowh_min = 26;
    int yellowh_max = 34;
    int yellows_min = 43;
    int yellows_max = 255;
    int yellowv_min = 46;
    int yellowv_max = 255;
    ////////////green////////////
    int greenh_min = 35;
    int greenh_max = 77;
    int greens_min = 43;
    int greens_max = 255;
    int greenv_min = 46;
    int greenv_max = 255;
    ////////////white////////////
    int whiteh_min = 0;
    int whiteh_max = 180;
    int whites_min = 0;
    int whites_max = 30;
    int whitev_min = 221;
    int whitev_max = 255;
    ////////////black////////////
    int blackh_min = 0;
    int blackh_max = 180;
    int blacks_min = 0;
    int blacks_max = 255;
    int blackv_min = 0;
    int blackv_max = 46;
    ////////////gray////////////
    int grayh_min = 0;
    int grayh_max = 180;
    int grays_min = 0;
    int grays_max = 43;
    int grayv_min = 46;
    int grayv_max = 220;
    ////////////red0////////////
    int red0h_min = 0;
    int red0h_max = 10;
    int red0s_min = 43;
    int red0s_max = 255;
    int red0v_min = 46;
    int red0v_max = 255;
    ////////////red1////////////
    int red1h_min = 156;
    int red1h_max = 180;
    //int red1s_min = 43;
    //int red1s_max = 255;
    //int red1v_min = 46;
    //int red1v_max = 255;
    ////////////orange////////////
    int orangeh_min = 11;
    int orangeh_max = 25;
    int oranges_min = 43;
    int oranges_max = 255;
    int orangev_min = 46;
    int orangev_max = 255;
    ////////////cyan////////////
    int cyanh_min = 78;
    int cyanh_max = 99;
    int cyans_min = 43;
    int cyans_max = 255;
    int cyanv_min = 46;
    int cyanv_max = 255;
    ////////////purple////////////
    int purpleh_min = 125;
    int purpleh_max = 155;
    int purples_min = 43;
    int purples_max = 255;
    int purplev_min = 46;
    int purplev_max = 255;

    cv::Mat mat_hsv;
    cv::cvtColor(car,mat_hsv,CV_BGR2HSV);
    cv::vector<cv::Mat> channels(mat_hsv.rows*mat_hsv.cols);
    cv::split(mat_hsv, channels);
    cv::Mat imgH =  channels.at(0);
    cv::Mat imgS = channels.at(1);
    cv::Mat imgV = channels.at(2);

    int nRows = imgH.rows;
    int nCols = imgH.cols * imgH.channels();

    if (imgH.isContinuous())
    {
        nCols *= nRows;
        nRows = 1; // 如果连续，外层循环执行只需一次，以此提高执行效率
    }

    for (int i = 0; i < nRows; ++i)
    {
        uchar* datah = mat_hsv.ptr<uchar>(i);
        uchar* datas = mat_hsv.ptr<uchar>(i);
        uchar* datav = mat_hsv.ptr<uchar>(i);
        for (int j = 0; j < nCols; ++j)
        {
            //////////////////h通道/-/////////////////////
            uchar hvalue=datah[j];  //
            //////////////////s通道/-/////////////////////
            uchar svalue=datas[j];  //
            //////////////////v通道/-/////////////////////
            uchar vvalue=datav[j];  //
            // printf("x=%d,y=%d,HSV: H=%d,S=%d,V=%d\n",i,j,hvalue,svalue,vvalue);
            if ((hvalue>blueh_min&&hvalue<blueh_max)&&(svalue>blues_min&&svalue<blues_max)&&(vvalue>bluev_min&&vvalue<bluev_max))//hsv图像在蓝色区域里
            {
                //蓝色+1
                colornum_car[0]++;
            }
            else if((hvalue>yellowh_min&&hvalue<yellowh_max)&&(svalue>yellows_min&&svalue<yellows_max)&&(vvalue>yellowv_min&&vvalue<yellowv_max))//hsv在黄色区域里
            {
                //黄色+1
                colornum_car[1]++;
            }
            else if ((hvalue>whiteh_min&&hvalue<whiteh_max)&&(svalue>whites_min&&svalue<whites_max)&&(vvalue>whitev_min&&vvalue<whitev_max))//hsv在白色区域里
            {
                //白色+1
                colornum_car[2]++;
            }
            else if((hvalue>blackh_min&&hvalue<blackh_max)&&(svalue>blacks_min&&svalue<blacks_max)&&(vvalue>blackv_min&&vvalue<blackv_max))//hsv在黑色区域里
            {
                colornum_car[3]++;
                //黑色+1
            }
            else if((hvalue>greenh_min&&hvalue<greenh_max)&&(svalue>greens_min&&svalue<greens_max)&&(vvalue>greenv_min&&vvalue<greenv_max))//hsv在绿色区域里
            {
                colornum_car[4]++;
                //绿色+1
            }
            else if((hvalue>grayh_min&&hvalue<grayh_max)&&(svalue>grays_min&&svalue<grays_max)&&(vvalue>grayv_min&&vvalue<grayv_max))//hsv在绿色区域里
            {
                colornum_car[5]++;
                //灰色+1
            }
            else if(((hvalue>red0h_min&&hvalue<red0h_max)|| (hvalue>red1h_min&&hvalue<red1h_max))&&(svalue>red0s_min&&svalue<red0s_max)&&(vvalue>red0v_min&&vvalue<red0v_max))//hsv在绿色区域里
            {
                colornum_car[6]++;
                //红色+1
            }
            else if((hvalue>orangeh_min&&hvalue<orangeh_max)&&(svalue>oranges_min&&svalue<oranges_max)&&(vvalue>orangev_min&&vvalue<orangev_max))//hsv在绿色区域里
            {
                colornum_car[7]++;
                //橙色+1
            }
            else if((hvalue>cyanh_min&&hvalue<cyanh_max)&&(svalue>cyans_min&&svalue<cyans_max)&&(vvalue>cyanv_min&&vvalue<cyanv_max))//hsv在绿色区域里
            {
                colornum_car[8]++;
                //青色+1
            }
            else if((hvalue>purpleh_min&&hvalue<purpleh_max)&&(svalue>purples_min&&svalue<purples_max)&&(vvalue>purplev_min&&vvalue<purplev_max))//hsv在绿色区域里
            {
                colornum_car[9]++;
                //紫色+1
            }
        }
    }
    sortvalue_label(colornum_car, COLOR_CAR_STYLE_NUMBER, colornum_car_label);
    return colornum_car_label[0];
}

//int province start from 0
//ilabelInfo.label start from 0
int CarRec::charRecCH(cv::Mat char_spliter, svm_model * svm, struct labelInfo &ilabelInfo, int province)
{
    resize(char_spliter, char_spliter, imageSize);
    cv::vector<float> imageDescriptor(1000);
    coumputeHog(char_spliter, imageDescriptor);
    cv::Mat testDescriptor = cv::Mat::zeros(1, imageDescriptor.size(), CV_32FC1);
    for (size_t i = 0; i < imageDescriptor.size(); i++)
    {
        testDescriptor.at<float>(0, i) = imageDescriptor[i];
    }
    svm_node * inputVector = new svm_node [imageDescriptor.size()+1];
    int n = 0;
    for(cv::vector<float>::iterator iter=imageDescriptor.begin();iter!=imageDescriptor.end();iter++)
    {
        inputVector[n].index = n;
        inputVector[n].value = *iter;
        n++;
    }
    inputVector[n].index = -1;
    double prob_estimates[NUM_CLASSES_CH] = {0};
    double resultLabel = svm_predict_probability(svm,inputVector,prob_estimates);//chinese charrec
    if(-1 != province)
    {
        //such as chuan ,the province is 5
        prob_estimates[province] = prob_estimates[province]*5;//byes
        GetMaxID(prob_estimates, NUM_CLASSES_CH, ilabelInfo);

    }
    else
    {
        //float label = mySVM.predict(testDescriptor);
        ilabelInfo.Label = int(resultLabel-34);
        ilabelInfo.confidenceper = GetMaxD(prob_estimates, NUM_CLASSES_CH);
    }
    delete [] inputVector;
    return 0;
    //int label_index = int(label);
}

int CarRec::charRecEN(cv::Mat char_spliter, svm_model * svm, struct labelInfo &ilabelInfo)
{
    resize(char_spliter, char_spliter, imageSize);
    cv::vector<float> imageDescriptor(1000);
    coumputeHog(char_spliter, imageDescriptor);
    cv::Mat testDescriptor = cv::Mat::zeros(1, imageDescriptor.size(), CV_32FC1);
    for (size_t i = 0; i < imageDescriptor.size(); i++)
    {
        testDescriptor.at<float>(0, i) = imageDescriptor[i];
    }
    svm_node * inputVector = new svm_node [imageDescriptor.size()+1];
    int n = 0;
    for(cv::vector<float>::iterator iter=imageDescriptor.begin();iter!=imageDescriptor.end();iter++)
    {
        inputVector[n].index = n;
        inputVector[n].value = *iter;
        n++;
    }
    inputVector[n].index = -1;
    double prob_estimates[NUM_CLASSES_EN] = {0};
    double resultLabel = svm_predict_probability(svm,inputVector,prob_estimates);//English charrec
    //float label = mySVM.predict(testDescriptor);
    //imshow("char_spliter",char_spliter);
    ilabelInfo.confidenceper = GetMaxD(prob_estimates, NUM_CLASSES_EN);
    int label_index = int(resultLabel);
    ilabelInfo.Label = label_index;
    delete [] inputVector;
    return 0;
    //int label_index = int(label);
}
/**
 * @brief swap 用于将元素a和元素b交换
 * @param a 要交换的数字a
 * @param b 要交换的数字b
 */
void CarRec::swap(int *a,int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief rest 用于对数组进行排序，从小到大排列
 * @param lels  要被排序的数组
 * @param count 被排序的数组元素的个数
 */
void CarRec::sortvalue_label(int lels[], int count, int label[])
{
    /** 暂时使用冒泡排序 **/
    /** 临时变量i,j **/
    int i,j;
    for(i = 0;i < count-1;i++){
        for(j = i+1; j < count;j++){
            if(lels[i] < lels[j])
            {
                swap(&lels[i],&lels[j]);
                swap(&label[i],&label[j]);
            }
        }
    }
}

int CarRec::get_plete_color_style(cv::Mat plete)
{
    if (plete.cols < 1 || plete.rows < 1)
    {
        return -1;
    }
    int colornum_plete[COLOR_PLETE_STYLE_NUMBER] = {0};
    int colornum_plete_label[COLOR_PLETE_STYLE_NUMBER];
    for (int i=0;i<COLOR_PLETE_STYLE_NUMBER;i++)
    {
        colornum_plete_label[i]=i;
    }
    ////////////blue////////////
    int bh_min = 90;
    int bh_max = 130;
    int bs_min = 43;
    int bs_max = 255;
    int bv_min = 46;
    int bv_max = 255;
    ////////////yellow////////////
    int yh_min = 10;
    int yh_max = 35;
    int ys_min = 90;
    int ys_max = 255;
    int yv_min = 90;
    int yv_max = 255;
    ////////////green////////////
    int gh_min = 40;
    int gh_max = 80;
    int gs_min = 43;
    int gs_max = 255;
    int gv_min = 70;
    int gv_max = 255;
    ////////////white////////////
    int wh_min = 0;
    int wh_max = 180;
    int ws_min = 0;
    int ws_max = 80;
    int wv_min = 120;
    int wv_max = 255;
    ////////////black////////////
    int bkh_min = 0;
    int bkh_max = 180;
    int bks_min = 0;
    int bks_max = 255;
    int bkv_min = 0;
    int bkv_max = 60;

    cv::Mat mat_hsv;
    cv::cvtColor(plete,mat_hsv,CV_BGR2HSV);
    cv::vector<cv::Mat> channels(mat_hsv.rows*mat_hsv.cols);
    cv::split(mat_hsv, channels);
    cv::Mat imgH =  channels.at(0);
    cv::Mat imgS = channels.at(1);
    cv::Mat imgV = channels.at(2);

    int nRows = imgH.rows;
    int nCols = imgH.cols * imgH.channels();

    if (imgH.isContinuous())
    {
        nCols *= nRows;
        nRows = 1; // 如果连续，外层循环执行只需一次，以此提高执行效率
    }

    for (int i = 0; i < nRows; ++i)
    {
        uchar* datah = mat_hsv.ptr<uchar>(i);
        uchar* datas = mat_hsv.ptr<uchar>(i);
        uchar* datav = mat_hsv.ptr<uchar>(i);
        for (int j = 0; j < nCols; ++j)
        {
            //////////////////h通道/-/////////////////////
            uchar hvalue=datah[j];  //
            //////////////////s通道/-/////////////////////
            uchar svalue=datas[j];  //
            //////////////////v通道/-/////////////////////
            uchar vvalue=datav[j];  //
            // printf("x=%d,y=%d,HSV: H=%d,S=%d,V=%d\n",i,j,hvalue,svalue,vvalue);
            if ((hvalue>bh_min&&hvalue<bh_max)&&(svalue>bs_min&&svalue<bs_max)&&(vvalue>bv_min&&vvalue<bv_max))//hsv图像在蓝色区域里
            {
                //蓝色+1
                colornum_plete[0]++;
            }
            else if((hvalue>yh_min&&hvalue<yh_max)&&(svalue>ys_min&&svalue<ys_max)&&(vvalue>yv_min&&vvalue<yv_max))//hsv在黄色区域里
            {
                //黄色+1
                colornum_plete[1]++;
            }
            else if ((hvalue>wh_min&&hvalue<wh_max)&&(svalue>ws_min&&svalue<ws_max)&&(vvalue>wv_min&&vvalue<wv_max))//hsv在白色区域里
            {
                //白色+1
                colornum_plete[2]++;
            }
            else if((hvalue>bkh_min&&hvalue<bkh_max)&&(svalue>bks_min&&svalue<bks_max)&&(vvalue>bkv_min&&vvalue<bkv_max))//hsv在黑色区域里
            {
                colornum_plete[3]++;
                //黑色+1
            }
            else if((hvalue>gh_min&&hvalue<gh_max)&&(svalue>gs_min&&svalue<gs_max)&&(vvalue>gv_min&&vvalue<gv_max))//hsv在绿色区域里
            {
                colornum_plete[4]++;
                //绿色+1
            }
            //else
            //{
            //其他颜色
            //colornum_plete[5]++;
            //}
        }
    }
    sortvalue_label(colornum_plete, COLOR_PLETE_STYLE_NUMBER, colornum_plete_label);
    if (0==colornum_plete_label[0] || 0==colornum_plete_label[1])
    {
        return COLOR_PLETE_BLUE_WHITE;
    }
    else if (1==colornum_plete_label[0] || 1==colornum_plete_label[1])
    {
        return COLOR_PLETE_YELLOW_BLACK;
    }
    else if (4==colornum_plete_label[0] || 4==colornum_plete_label[1])
    {
        return COLOR_PLETE_GREEN_BLACK;
    }
    else
    {
        return COLOR_PLETE_OTHER_STYLE;
    }
}
///
/// \brief CalculateIntegralMap
/// \param img 1-D
/// \param imgIntegral 1-D
///
void CarRec::CalculateIntegralMap(cv::Mat &img, cv::Mat &imgIntegral)
{
    imgIntegral = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_32SC1);
    int rowNumber = img.rows;
    int colNumber = img.cols * img.channels();
    for (int i = 0; i < rowNumber;i++)
    {
        uchar* data = img.ptr<uchar>(i);
        uchar* dataIntegral = imgIntegral.ptr<uchar>(i);
        if(i>0)
        {
            uchar* dataIntegralpre = imgIntegral.ptr<uchar>(i-1);
            for (int j = 0; j < colNumber; j++)
            {
                if(0 == j)
                {
                    dataIntegral[j] = dataIntegralpre[j] + data[j];
                }
                else
                {
                    dataIntegral[j] = dataIntegral[j-1] + data[j] + dataIntegralpre[j] - dataIntegralpre[j-1];
                }
            }
        }
        else
        {
            for (int j = 0; j < colNumber; j++)
            {
                if(0 == j)
                {
                    dataIntegral[j] = data[j];
                }
                else
                {
                    dataIntegral[j] = dataIntegral[j-1] + data[j];
                }
            }

        }
    }
}
///////////
/// \brief CarRec::offX_match_best
/// \param tem
/// \param base
/// \return
//int CarRec::offX_match_best(int pletetype, cv::Mat base)
//{
//    int base_rows = base.rows;
//    int base_cols = base.cols;
//    int offX=0;
//    switch (pletetype) {
//    case COLOR_PLETE_BLUE_WHITE:
//        if(base_cols<147)
//            return offX;
//        else
//        {
//            long max_sum = 0;
//            for(int index_max=1; index_max< (base_cols-145); index_max++)
//            {
//                long sum = 0;
//                for(int i=0; i<7; i++)
//                {
//                    sum = sum + base.at<int>(bluewhite[i].height-1,bluewhite[i].x+bluewhite[i].width-1)-base.at<int>(bluewhite[i].height-1,bluewhite[i].x);
//                }
//                if(sum>max_sum)
//                {
//                    max_sum = sum;
//                    offX = index_max;
//                }
//            }
//            return offX;
//        }
//        break;
//    default:
//        return -1;
//        break;
//    }
//}

int CarRec::get_offx(int pletetype, cv::Mat imgIntegral)
{
    int rowNumber = imgIntegral.rows;
    int colNumber = imgIntegral.cols * imgIntegral.channels();
    int offX=0;
    switch (pletetype)
    {
    case COLOR_PLETE_BLUE_WHITE:
        if(imgIntegral.cols<147)
            return offX;
        else
        {
            long max_sum = 0;
            for(int index_max=1; index_max< (imgIntegral.cols-145); index_max++)
            {
                long sum = 0;
                for (int i = 0; i < rowNumber;i++)
                {
                    int* data = imgIntegral.ptr<int>(i);
                    for (int j = 0; j < colNumber; j++)
                    {
                        if(((0+index_max)<=j&&j<=(46+index_max))||(j>=(57+index_max)&&j<=(103+index_max))\
                                ||(j>=(136+index_max)&&j<=(182+index_max))||(j>=(193+index_max)&&j<=(239+index_max))\
                                ||(j>=(250+index_max)&&j<=(296+index_max))||(j>=(307+index_max)&&j<=(353+index_max))\
                                ||(j>=(364+index_max)&&j<=(410+index_max)))
                        {
                            sum = sum + data[j];
                        }
                    }
                }
                if(sum>max_sum)
                {
                    max_sum = sum;
                    offX = index_max;
                }
            }
            return offX;
        }
        break;
    default:
        return -1;
        break;
    }
}
///
/// \brief CarRec::get_offx_m1  back model sum mini
/// \param pletetype
/// \param imgIntegral
/// \return
///
int CarRec::get_offx_m1(int pletetype, cv::Mat imgIntegral)
{
    int rowNumber = imgIntegral.rows;
    int colNumber = imgIntegral.cols * imgIntegral.channels();
    int offX=0;
    int offX0=0;
    switch (pletetype)
    {
    case COLOR_PLETE_BLUE_WHITE:
        if(imgIntegral.cols<147)
            return offX;
        else
        {
            long min_sum = 255*imgIntegral.cols*imgIntegral.cols/5;
            for(int index_max=-5; index_max< (imgIntegral.cols-145); index_max++)
            {
                long sum = 0;
                for (int i = 0; i < rowNumber;i++)
                {
                    int* data = imgIntegral.ptr<int>(i);
                    for (int j = 0; j < colNumber; j++)
                    {
                        if(((17+index_max)<j&&j<=(19+index_max))||(j>(37+index_max)&&j<=(47+index_max))\
                                ||(j>(65+index_max)&&j<=(67+index_max))||(j>(85+index_max)&&j<=(87+index_max))\
                                ||(j>(105+index_max)&&j<=(107+index_max))||(j>(125+index_max)&&j<=(127+index_max)))
                        {
                            sum = sum + data[j];
                        }
                    }
                }
                if(sum<min_sum)
                {
                    min_sum = sum;
                    offX0 = index_max;
                }
            }
            if(offX0 < 0)
                offX0 = 0;
            for(int index_j=-5; index_j< 5; index_j++)
            {
                int index_temp = offX0 + index_j;
                int sum = 0;
                int min_sum = 255*16*8;
                int* data = imgIntegral.ptr<int>(0);
                for (int j = 0; j < colNumber; j++)
                {
                    if(((37+index_temp)<j&&j<=(40+index_temp))||(j>(45+index_temp)&&j<=(48+index_temp)))
                    {
                        sum = sum + data[j];
                    }
                }

                if(sum<min_sum)
                {
                    min_sum = sum;
                    offX = index_temp;
                }
            }
            return offX;
        }
        break;
    default:
        return -1;
        break;
    }
}

