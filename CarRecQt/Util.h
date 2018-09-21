#define Util.h

#ifdef __cplusplus 
extern "C" {
#endif

//视频结构化线程
void *structedThread(void * args);
//心跳线程
void * heartThread(void * args);
//启动函数
int startTCP(int sockfd,struct sockaddr_in servaddr);
//图片消息
void * pictureMsg(char *url,int sockfd,struct sockaddr_in servaddr);


#ifdef __cplusplus 
}
#endif
//心跳线程参数
struct heartTheadVar
{ 
  int arg1;
  struct sockaddr_in arg2;
};
/**********/
int    getCheckSum(char * c);

char * int2String(int i); 
//连接请求
int connection(int sockfd,struct sockaddr_in servaddr);
//鉴权消息
int authMsg(int sockfd,struct sockaddr_in servaddr);
//发送消息
void sendMsg(int sockfd,char * str,struct sockaddr_in servaddr);
//心跳消息
int heartMsg(int sockfd,struct sockaddr_in servaddr);





