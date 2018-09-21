#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include "cJSON.h"
#include "cJSON.c"
#include <unistd.h>
#include <fcntl.h>  
#include "Encode.h"
#define HEADLEN  18
#define BUFFER_SIZE 1024
#define HEADSIGN  "$"
#define TAILSIGN  "#"
#define DEMIMETER ","
 
/*
 *消息头定义
 */
struct packHead
{ 
  char packId[2] ;//消息id:识别消息类型
  char caremaId[8] ;//摄像头id
  char packLength[4];//消息长度
  //int  bodyPro; //消息体属性
  //int  encrtpy;//数据加密方式
  //int  hasSubPack;//是否分包
  //int  reversed;//保留位
  //int  flowId;//消息流水号
  //int  packFild;//消息包封装项
  //int  subPackSum;//分包总数
  //int  subpackSeq;//分包序列号
}; 


/*
 *获得消息头(@@@@@@@@@@@@@@@@@@@@@@@packbody需与人脸识别进行对接）
 */
struct packHead myPackHead;
/*char *  getpackHead(char *packY4kZLxZFknvREId0,char* caremaId0,int  length)
{ 
  strcpy(myPackHead.packId,packId0);
  strcpy(myPackHead.caremaId,caremaId0);
  strcpy(myPackHead.packLength,int2String(length));
  int sendLength0=sizeof(myPackHead);
  char *buffer0=(char*)malloc(sendLength0);
  memcpy(buffer0,&myPackHead,sendLength0);
  printf("获得消息头测试%s/n",buffer0);
  return buffer0;
};*/

/*
 *获得通用消息(消息头+消息体）
 */
char * getPackagedata(char * c,char * c1)
{
  int tempLength=HEADLEN+strlen(c1);
  char *temp=(char*)malloc(tempLength);
  strcpy(temp,c);
  strcat(temp,c1);
  printf("获得通用消息测试%s/n",temp);
  return temp;
};
 

/*
 *通用消息的封装(定义结构体通过memcopy实现）
 */
/*struct packagedata myPackData;
char * getPackage(char * msgData,int  checkSum0)
{
  strcpy(myPackData.headSign,HEADSIGN);
  strcpy(myPackData.tailSign,TAILSIGN);
  strcpy(myPackData.pack_data,msgData);
  int sendLength=sizeof(myPackData);
  strcpy(myPackData.checkSum,int2String(checkSum0));
  
  
  char *buffer=(char*)malloc(sendLength);
  memcpy(buffer,&myPackData,sendLength);
  printf("获得通用消息的封装%s/n",buffer);
  return buffer;
};*/

/*
 *通用消息的封装(心跳消息、鉴权消息)
 */
char * getComEncode(char * msgId,char* cameraId)
{
  
  int  tempLength=strlen(msgId)+strlen(cameraId)+2;
  char *temp=(char*)malloc(tempLength);

  strcpy(temp,HEADSIGN);
  strcat(temp,msgId);
  strcat(temp,DEMIMETER);
  strcat(temp,cameraId);
  strcat(temp,TAILSIGN);
 
  printf("获得通用消息的封装%s/n",temp);
  
  return temp;

};

/*
 *结构化车辆消息的封装
 */

char * getCarEncode(struct structedCar car)
{
   //创建一个空的文档（对象）
   cJSON *json = cJSON_CreateObject();
      
  //向文档中添加键值对
   cJSON_AddItemToObject(json,"msgId",cJSON_CreateString(car.msgId));
   cJSON_AddItemToObject(json,"cameraId",cJSON_CreateString(car.cameraId));
   cJSON_AddItemToObject(json,"carNo",cJSON_CreateString(car.carNo));
   cJSON_AddItemToObject(json,"carType",cJSON_CreateString(car.carType));
   cJSON_AddItemToObject(json,"carColor",cJSON_CreateString(car.carColor));
   cJSON_AddItemToObject(json,"eventId",cJSON_CreateString(car.eventId));
   cJSON_AddItemToObject(json,"frameId",cJSON_CreateString(car.frameId));
   cJSON_AddItemToObject(json,"x1",cJSON_CreateString(car.x1));
   cJSON_AddItemToObject(json,"y1",cJSON_CreateString(car.y1));
   cJSON_AddItemToObject(json,"x2",cJSON_CreateString(car.x2));
   cJSON_AddItemToObject(json,"y2",cJSON_CreateString(car.y2));
   cJSON_AddNumberToObject(json,"reliablity",car.reliablity);
   cJSON_AddItemToObject(json,"time",cJSON_CreateString(car.time));
   
   //将json结构格式化到缓冲区
   char *buf = cJSON_Print(json);
  
   //释放json结构所占用的内存
   cJSON_Delete(json);
  
   return buf;

};

 
/*
 *图片base64编码
 */
char * base64_encode(char * bindata,char * base64,int binlength)

{
    const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j;
    
    unsigned char current;
    for ( i = 0, j = 0 ; i < binlength ; i += 3 )

    {

        current = (bindata[i] >> 2) ;
        current &= ( char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( ( char)(bindata[i] << 4 ) ) & ( ( char)0x30 ) ;
         
        if ( i + 1 >= binlength )
        {

            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';        
            break;
        }
        
        current |= ( ( char)(bindata[i+1] >> 4) ) & ( ( char) 0x0F );
        base64[j++] = base64char[(int)current];
        current = ( ( char)(bindata[i+1] << 2) ) & ( ( char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
           
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        
        current |= ( ( char)(bindata[i+2] >> 6) ) & ( ( char) 0x03 );
        base64[j++] = base64char[(int)current];
        current = ( ( char)bindata[i+2] ) & ( ( char)0x3F ) ;
        base64[j++] = base64char[(int)current];
       
    }
    
 
    base64[j] = '\0';
    
    return base64;

};



/*
 *图片消息的封装
 */

char * getPicEncode(int sumPack,int subPack,char * subData,char * url)
{
   //创建一个空的文档（对象）
   cJSON *json = cJSON_CreateObject();
      
  //向文档中添加键值对
   cJSON_AddItemToObject(json,"msgId",cJSON_CreateString("07"));
   cJSON_AddNumberToObject(json,"sumPack",sumPack); 
   cJSON_AddNumberToObject(json,"subPack",subPack);   
   cJSON_AddItemToObject(json,"subData",cJSON_CreateString(subData));
   cJSON_AddItemToObject(json,"picUrl",cJSON_CreateString(url));
   //将json结构格式化到缓冲区
   char *buf = cJSON_Print(json);
  
   //释放json结构所占用的内存
   cJSON_Delete(json);
   return buf;

};

/*
 *获得消息体属性
 */
int getBodyPro(int  packLength,int  encrtpy,int  hasSubPack,int  reversed)
{
  if(packLength>=BUFFER_SIZE)
     printf("消息长度超过发送缓冲区限制");
  int ret=(packLength&0x3FF)|((encrtpy<<10)&0x1C00)|((hasSubPack<<13)
      &0x2000)|((reversed<<14)&0xC000);
  printf("获得消息体属性测试%d\n",ret&0xffff);
  return ret;  

};

