#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "Encode.h"
#include "Util.h"
#define MYPORT  7674
#define HEADLEN  18
#define MAXINT   10
#define BUFFER_SIZE 1024
#define HEART 15 //心跳消息休眠时间间隔
#define AUTH   5 //鉴权时间间隔
#define RECON  5 //自动重连时间间隔
#define MAX_LEN 20*1024


/*
 *视频结构化线程
 */
void * structedThread(void * args)
 { 
      
    struct structedCar car;
    strcpy(car.msgId,"03");
    strcpy(car.cameraId,"124");
    strcpy(car.carColor,"09"); 
    strcpy(car.carNo,"甘555575E");
    strcpy(car.eventId,"201583144254_704daa44-5cc2-4ac8-a992-58659642a576"); 
    strcpy(car.frameId,"201887144254_97f66637-77e8-4d67-9bb2-741ced2b19be"); 
    strcpy(car.x1,"445");
    strcpy(car.y1,"112");
    strcpy(car.x2,"678"); 
    strcpy(car.y2,"554");
    strcpy(car.time,"2018-8-7 14:42:54");
    car.reliablity=0.953052; 
    char * tepBuffer=getCarEncode(car);
    int len=send(*(int *)args, tepBuffer, strlen(tepBuffer),0);
    if(len<=0)
       {
           perror("ERROR");
           printf("thread2测试时TCP连接出现异常\n");
           //return ;
           //break;
       }
};


/*
 *心跳线程
 */
void * heartThread(void * args)
{
   struct heartTheadVar *para;
   para = (struct heartTheadVar*) args;  
       
   heartMsg((*para).arg1,(*para).arg2);
  // heartMsg(para->arg1,para->arg2);
};


/*
 *获得校验码
 */
int getCheckSum(char * c)
{
  int end=strlen(c);
  if (end<=0)
     printf("校验码测试错误\n");
  int cs=0;
  for (int i=0;i<end;i++)
      cs^=*(c+i);
  printf("获得获得校验码测试%d\n",cs);
  return cs;
};

/*
 *整型转字符串
 */
char * int2String(int i)
{
  char *p=(char *)malloc(MAXINT*sizeof(char));
  sprintf(p,"%d",i);
  printf("整型转字符串测试%s\n",p);
  return p;
}

 
 /*
 *创建TCP连接
 */

int connection(int sockfd,struct sockaddr_in servaddr)
{
  int temp=connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  while (temp<0)
    {
        perror("connect");
        printf("5S后重新建立连接....\n");
        sleep(RECON); 
        temp=connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 
        continue;
    }
  return 0;
};

/*
 *心跳消息
 */
int heartMsg(int sockfd,struct sockaddr_in servaddr)
{  
  int len=0;
  char * package=getComEncode("02","140");
  while(1)
 {
   if (strlen(package)>1)
    {   
         sleep(HEART);
         sendMsg(sockfd,package,servaddr);
         printf("心跳包测试\n");
    }
 }
};


/*
 *鉴权消息(需阻塞通道)
 */
int authMsg(int sockfd,struct sockaddr_in servaddr)
{  
  
  int count=0;
  char * package=getComEncode("01","140");
  char recvbuf[BUFFER_SIZE];//接收缓冲区
  while(1)
 {
   if (strlen(package)>1)
    {   
      
      sendMsg(sockfd,package,servaddr);
      free(package);//释放动态分配内存
      recv(sockfd, recvbuf, sizeof(recvbuf),0); 
      if (strcmp(recvbuf,"$02345#")==0)
        {
            printf("鉴权成功\n");
            //heartMsg(sockfd);//发送心跳消息
            return 0;//设置摄像机权限
         }
        
     else
     {
       sleep(AUTH);
       count++;
       if(count==3)
         {
           //鉴权三次未成功则关闭TCP连接298
           return -1;
          }
     }
   
  }
 }
 //清空缓冲区
 memset(recvbuf, 0, sizeof(recvbuf));
};


/*
 *图片消息
 */
void * pictureMsg(char * url,int sockfd,struct sockaddr_in servaddr)
{
    //打开图片
    FILE *fp = NULL;
    unsigned int size;         //图片字节数
    char *buffer;
    char *buffer1;
    size_t result;
    char * ret1; 
    unsigned int length;
    
    fp = fopen(url,"rb");

    if (NULL == fp)

    {
        printf("open_error");
	exit(1);
    }

    //获取图片大小
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    //分配内存存储整个图片
    buffer = (char *)malloc((size/4+1)*4);
    if (NULL == buffer)
    {
	 printf("memory_error");
	 exit(2);

    }

     //将图片拷贝到buffer
     result = fread(buffer, 1, size, fp);
     if (result != size)  
    {  
        printf("reading_error");  
        exit (3);  
    }  
    fclose(fp);
 
     //base64编码
     buffer1 = (char *)malloc((size/4+1)*8);
     if (NULL == buffer1)
    {
	 printf("memory_error");
	 exit(2);

    }
    ret1 = base64_encode(buffer, buffer1, size); 
    
    char * package=getPicEncode(0,0,ret1,url);
    
    int temp= send(sockfd,package,strlen(package),0);
        
    if( temp<=0 )

    {
      perror("write");
    }
    free(buffer1);
    free(buffer);

    return ret1;

}; 

/*
 *图片消息(分包发送）
 */
/*void * pictureMsg(int sockfd,struct sockaddr_in servaddr,char* time)
{
  int     rn;
  char    *buf;
  char    *buf1;
  FILE    *fq;
  int     len,sumPack=0,subPack=1,opt = 1; 
  int     temv=20*1024;
  unsigned int size;         //图片字节数

  fq = fopen("timg.jpg","rb");
  if (NULL == fq)
    {
        perror("open_error");
	exit(1);
    }
  fseek(fq, 0L, SEEK_END);
  size = ftell(fq);
  fseek(fq, 0L, SEEK_SET);
  sumPack=(size-1)/temv+1;//分包总数
  buf = (char *)malloc((temv/4+1)*4);
  if (NULL == buf)
      {
         perror("memory_error");
	 exit(2);

      }
   buf1 = (char *)malloc((temv/4+1)*8);
   if (NULL == buf1)
      {
	  perror("memory_error");
	  exit(2);

      }
  while( !feof(fq) )
     {
       fseek(fq,0,SEEK_CUR);
       //将图片拷贝到buffer
       len = fread(buf,1,temv,fq);
       if (len<= 0)  
         {  
             perror("reading_error");  
             exit (3);  
            }  
       
        //base64编码
        base64_encode(buf,buf1,temv);
       
        char * package=getPicEncode(sumPack,subPack,buf1,time);
        if (strlen(package)>1)
           {        
             send(sockfd,package,strlen(package),0); 
           }
        
       subPack++;
      }
  free(buf1);
  free(buf);  
  fclose(fq);

}; */


/*
 *启动TCP连接
 */
int startTCP(int sockfd,struct sockaddr_in servaddr)
{ 
   int conTem=-1;
   int authTem=-1;

   while (conTem==-1)
    { 
      
      conTem=connection(sockfd,servaddr);

   }
};


/*
 *发送函数
 */
void sendMsg(int sockfd,char * str,struct sockaddr_in servaddr)
{   
    int len=0;
    len=send(sockfd, str, strlen(str),0); 
    if(len<=0)
    {
       perror("ERROR in sendMsg");
       printf("信息发送过程中TCP连接出现异常\n");
       shutdown(sockfd,2);//关闭连接
       close(sockfd);//关闭套接字 
       int temp=socket(AF_INET,SOCK_STREAM, 0);//创建新的套接字
       startTCP(temp,servaddr);//发送过程中TCP断开启动自动重连机制
        //break;
     }
};
