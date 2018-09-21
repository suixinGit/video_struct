# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include<stdlib.h>
# include<string.h>
//# include "cJSON.h"
//# include "cJSON.c"
# include "Encode.h" 
  int main(void)
  {
      pic_encode();
     /*//创建一个空的文档（对象）
      cJSON *json = cJSON_CreateObject();
      
     //向文档中增加一个键值对{"name":"王大锤"}
     char * temp="王大锤qwer";
     cJSON_AddItemToObject(json,"name",cJSON_CreateString(temp));
     //向文档中添加一个键值对
     //cJSON_AddItemToObject(json,"age",cJSON_CreateNumber(29));
     cJSON_AddNumberToObject(json,"age",29);
     
     cJSON *array = NULL;
     cJSON_AddItemToObject(json,"love",array=cJSON_CreateArray());
     cJSON_AddItemToArray(array,cJSON_CreateString("LOL"));
     cJSON_AddItemToArray(array,cJSON_CreateString("NBA"));
     cJSON_AddItemToArray(array,cJSON_CreateString("Go shopping"));
     
     cJSON_AddNumberToObject(json,"score",59);
     cJSON_AddStringToObject(json,"address","beijing");
     
     //将json结构格式化到缓冲区
     char *buf = cJSON_Print(json);
     printf("@@@@@@@@@@%s/n",buf);
     //打开文件写入json内容
    FILE *fp = fopen("create.json","w");
     fwrite(buf,strlen(buf),1,fp);
     free(buf);
     fclose(fp);
     //释放json结构所占用的内存
     cJSON_Delete(json);

    return 0;*/
 }
