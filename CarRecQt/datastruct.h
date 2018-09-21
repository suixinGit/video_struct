#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include <opencv2/core/core.hpp>
#define PI 3.1415926
#define COLOR_CAR_STYLE_NUMBER 11 //车辆颜色,蓝色(0),黄色(1),白色(2),黑色(3),绿色(4),灰色(5),红色(6),橙色(7),青色(8),紫色(9),其他(10)
#define COLOR_CAR_BLUE			0
#define COLOR_CAR_YELLOW		1
#define COLOR_CAR_WHITE			2
#define COLOR_CAR_BLACK			3
#define COLOR_CAR_GREEN			4
#define COLOR_CAR_GRAY			5
#define COLOR_CAR_RED			6
#define COLOR_CAR_ORANGE		7
#define COLOR_CAR_CYAN			8
#define COLOR_CAR_PURPLE		9
#define COLOR_CAR_OTHERS		10



#define COLOR_PLETE_STYLE_NUMBER	5 //蓝(0)、黄(1)、白(2)、黑(3)、绿(4)、其他(5)；
#define COLOR_PLETE_BLUE_WHITE		0
#define COLOR_PLETE_YELLOW_BLACK	1
#define COLOR_PLETE_GREEN_BLACK		2
#define COLOR_PLETE_OTHER_STYLE		3
#define IF_DEBUG                    0
struct PleteInfo
{
    int colorstyle;////蓝(0)、黄(1)、白(2)、黑(3)、绿(4)、其他(5)；
    std::string pletenumber;
    cv::Rect location;
    double confidenceper;
};
struct CarInfo
{
    int colorstyle;
    cv::Rect location;
    struct PleteInfo iPleteInfo;
    cv::string imgcardetect;
};
struct labelInfo
{
    int Label;
    double confidenceper;
};

#endif // DATASTRUCT_H
