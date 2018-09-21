#define Encode.h
#include <netinet/in.h>


/*
 *摄像头结构化车辆消息定义（一个汉字三个字节）
 */
struct structedCar
{ 
  char msgId[3];
  char cameraId[10];
  char carNo[10];
  char carType[20];
  char carColor[4];
  char eventId[60];
  char frameId[60];
  char x1[10];
  char y1[10];
  char x2[10];
  char y2[10];
  float reliablity;
  char time[30];
};



/*
 *摄像头结构化人消息定义
 */

struct structedper
{



};
/*
 *服务器通用消息应答
 */
struct comResp
{
  int  msgId;
  char cameraId;
  int  result;
};

/**********/
int    getBodyPro(int  packLength,int  encrtpy,int  hasSubPack,int  reversed);
/**********/
char * getpackHead(char packId0[],char caremaId0[],int  length);
/**********/
char * getPackage(char * msgData,int  checkSum0); 
/**********/
char * getPackage0(char * msgData,int  checkSum0); 
/**********/
int    getCheckSum(char * c);
/**********/
//char * int2String(int i);
/**********/
char * getComEncode(char * msgId,char *  cameraId);
/**********/
#ifdef __cplusplus
extern "C"
{
#endif
char * getCarEncode(struct structedCar car);
#ifdef __cplusplus
}
#endif
/**********/
char * base64_encode(  char * bindata, char * base64, int binlength );

char * getPicEncode(int sumPack,int subPack,char * subData,char * url);


